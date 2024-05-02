#ifndef PLUTOBOOK_DOCUMENT_H
#define PLUTOBOOK_DOCUMENT_H

#include "cssstylesheet.h"
#include "globalstring.h"
#include "heapstring.h"
#include "url.h"

#include <forward_list>

namespace plutobook {

class CSSProperty;
class Counters;
class ContainerNode;
class Box;

class Node : public HeapMember {
public:
    Node(Document* document);

    virtual ~Node();
    virtual bool isTextNode() const { return false; }
    virtual bool isContainerNode() const { return false; }
    virtual bool isElementNode() const { return false; }
    virtual bool isDocumentNode() const { return false; }
    virtual bool isHTMLElement() const { return false; }
    virtual bool isSVGElement() const { return false; }
    virtual bool isHTMLDocument() const { return false; }
    virtual bool isSVGDocument() const { return false; }
    virtual bool isXMLDocument() const { return false; }

    bool isRootNode() const;
    bool isSVGRootNode() const;
    bool isOfType(const GlobalString& namespaceURI, const GlobalString& tagName) const;

    const GlobalString& namespaceURI() const;
    const GlobalString& tagName() const;

    void reparent(ContainerNode* newParent);
    void remove();

    Document* document() const { return m_document; }
    ContainerNode* parentNode() const { return m_parentNode; }
    Node* nextSibling() const { return m_nextSibling; }
    Node* previousSibling() const { return m_previousSibling; }

    void setParentNode(ContainerNode* parentNode) { m_parentNode = parentNode; }
    void setNextSibling(Node* nextSibling) { m_nextSibling = nextSibling; }
    void setPreviousSibling(Node* previousSibling) { m_previousSibling = previousSibling; }

    Node* firstChild() const;
    Node* lastChild() const;

    void setBox(Box* box) { m_box = box; }
    Box* box() const { return m_box; }
    BoxStyle* style() const;
    Heap* heap() const;

    virtual Node* cloneNode(bool deep) = 0;
    virtual Box* createBox(const RefPtr<BoxStyle>& style) = 0;
    virtual void buildBox(Counters& counters, Box* parent) = 0;
    virtual void serialize(std::ostream& o) const = 0;
    virtual void finishParsingDocument() {}

private:
    Document* m_document;
    ContainerNode* m_parentNode{nullptr};
    Node* m_nextSibling{nullptr};
    Node* m_previousSibling{nullptr};
    Box* m_box{nullptr};
};

class TextNode final : public Node {
public:
    TextNode(Document* document, const HeapString& data);

    bool isTextNode() const final { return true; }

    const HeapString& data() const { return m_data; }
    void setData(const HeapString& data) { m_data = data; }
    void appendData(const std::string_view& data);

    bool isHidden(const Box* parent) const;

    Node* cloneNode(bool deep) final;
    Box* createBox(const RefPtr<BoxStyle>& style) final;
    void buildBox(Counters& counters, Box* parent) final;
    void serialize(std::ostream& o) const final;

private:
    HeapString m_data;
};

template<>
struct is_a<TextNode> {
    static bool check(const Node& value) { return value.isTextNode(); }
};

class ContainerNode : public Node {
public:
    ContainerNode(Document* document);
    ~ContainerNode() override;

    bool isContainerNode() const final { return true; }

    Node* firstChild() const { return m_firstChild; }
    Node* lastChild() const { return m_lastChild; }

    void setFirstChild(Node* child) { m_firstChild = child; }
    void setLastChild(Node* child) { m_lastChild = child; }

    void appendChild(Node* newChild);
    void insertChild(Node* newChild, Node* nextChild);
    void removeChild(Node* child);

    void reparentChildren(ContainerNode* newParent);
    void cloneChildren(ContainerNode* newParent);
    std::string textFromChildren() const;

    void buildBox(Counters& counters, Box* parent) override;
    void serialize(std::ostream& o) const override;
    void finishParsingDocument() override;

private:
    Node* m_firstChild{nullptr};
    Node* m_lastChild{nullptr};
};

template<>
struct is_a<ContainerNode> {
    static bool check(const Node& value) { return value.isContainerNode(); }
};

inline Node* Node::firstChild() const
{
    if(auto container = to<ContainerNode>(this))
        return container->firstChild();
    return nullptr;
}

inline Node* Node::lastChild() const
{
    if(auto container = to<ContainerNode>(this))
        return container->lastChild();
    return nullptr;
}

class Attribute {
public:
    Attribute() = default;
    Attribute(const GlobalString& name, const HeapString& value)
        : m_name(name), m_value(value)
    {}

    const GlobalString& name() const { return m_name; }
    void setName(const GlobalString& name) { m_name = name; }

    const HeapString& value() const { return m_value; }
    void setValue(const HeapString& value) { m_value = value; }

    bool empty() const { return m_value.empty(); }

private:
    GlobalString m_name;
    HeapString m_value;
};

inline bool operator==(const Attribute& a, const Attribute& b) { return a.name() == b.name() && a.value() == b.value(); }
inline bool operator!=(const Attribute& a, const Attribute& b) { return a.name() != b.name() || a.value() != b.value(); }

using AttributeList = std::pmr::forward_list<Attribute>;
using ClassNameList = std::pmr::forward_list<HeapString>;
using CSSPropertyList = std::pmr::vector<CSSProperty>;

class Element : public ContainerNode {
public:
    Element(Document* document, const GlobalString& namespaceURI, const GlobalString& tagName);

    bool isElementNode() const final { return true; }
    bool isOfType(const GlobalString& namespaceURI, const GlobalString& tagName) const { return m_namespaceURI == namespaceURI && m_tagName == tagName; }

    const GlobalString& namespaceURI() const { return m_namespaceURI; }
    const GlobalString& tagName() const { return m_tagName; }
    const AttributeList& attributes() const { return m_attributes; }

    const HeapString& lang() const;
    const HeapString& id() const { return m_id; }
    const ClassNameList& classNames() const { return m_classNames; }

    const Attribute* findAttribute(const GlobalString& name) const;
    bool hasAttribute(const GlobalString& name) const;
    const HeapString& getAttribute(const GlobalString& name) const;
    void setAttributes(const AttributeList& attributes);
    void setAttribute(const Attribute& attribute);
    void setAttribute(const GlobalString& name, const HeapString& value);
    void removeAttribute(const GlobalString& name);
    virtual void parseAttribute(const GlobalString& name, const HeapString& value);
    virtual void collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const {}

    CSSPropertyList inlineStyle();
    CSSPropertyList presentationAttributeStyle();

    Element* parentElement() const;
    Element* previousElement() const;
    Element* nextElement() const;

    Node* cloneNode(bool deep) override;
    Box* createBox(const RefPtr<BoxStyle>& style) override;
    void buildBox(Counters& counters, Box* parent) override;
    void serialize(std::ostream& o) const override;

private:
    GlobalString m_namespaceURI;
    GlobalString m_tagName;
    HeapString m_id;
    ClassNameList m_classNames;
    AttributeList m_attributes;
};

template<>
struct is_a<Element> {
    static bool check(const Node& value) { return value.isElementNode(); }
};

inline bool Node::isOfType(const GlobalString& namespaceURI, const GlobalString& tagName) const
{
    if(auto element = to<Element>(this))
        return element->isOfType(namespaceURI, tagName);
    return false;
}

inline const GlobalString& Node::namespaceURI() const
{
    if(auto element = to<Element>(this))
        return element->namespaceURI();
    return emptyGlo;
}

inline const GlobalString& Node::tagName() const
{
    if(auto element = to<Element>(this))
        return element->tagName();
    return emptyGlo;
}

class Resource;
class TextResource;
class ImageResource;
class FontResource;
class Font;

struct FontDescription;
struct FontDataDescription;

using DocumentElementMap = std::pmr::multimap<HeapString, Element*, std::less<>>;
using DocumentResourceMap = std::pmr::map<Url, RefPtr<Resource>>;
using DocumentFontMap = std::pmr::map<FontDescription, RefPtr<Font>>;

class BoxView;
class GraphicsContext;
class Rect;

class Book;
class PageSize;
class PageMargins;
class PageBox;

using PageBoxList = std::pmr::vector<std::unique_ptr<PageBox>>;

class Document : public ContainerNode {
public:
    Document(Book* book, Heap* heap, Url url);
    ~Document() override;

    bool isDocumentNode() const final { return true; }
    bool isSVGImageDocument() const { return !m_book && isSVGDocument(); }

    Book* book() const { return m_book; }
    Heap* heap() const { return m_heap; }

    const Url& baseUrl() const { return m_baseUrl; }
    void setBaseUrl(Url baseUrl) { m_baseUrl = std::move(baseUrl); }
    Url completeUrl(const std::string_view& value) const { return m_baseUrl.complete(value); }

    BoxView* box() const;
    float width() const;
    float height() const;

    float viewportWidth() const;
    float viewportHeight() const;

    TextNode* createTextNode(const std::string_view& value);
    Element* createElement(const GlobalString& namespaceURI, const GlobalString& tagName);
    Element* rootElement() const { return m_rootElement; }
    Element* bodyElement() const;
    BoxStyle* backgroundStyle() const;
    BoxStyle* rootStyle() const;

    Element* getElementById(const std::string_view& id) const;
    void addElementById(const HeapString& id, Element* element);
    void removeElementById(const HeapString& id, Element* element);

    void addAuthorJavaScript(const std::string_view& content);
    void addUserJavaScript(const std::string_view& content);

    void addAuthorStyleSheet(const std::string_view& content, Url baseUrl);
    void addUserStyleSheet(const std::string_view& content);

    bool supportsMediaType(CSSMediaType mediaType) const;
    bool supportsMediaQueries(const CSSMediaList& queries) const;
    bool supportsMedia(const std::string_view& type, const std::string_view& media) const;

    RefPtr<BoxStyle> styleForElement(Element* element, const BoxStyle& parentStyle) const;
    RefPtr<BoxStyle> pseudoStyleForElement(Element* element, PseudoType pseudoType, const BoxStyle& parentStyle) const;

    RefPtr<BoxStyle> styleForPage(const GlobalString& pageName, size_t pageIndex, PseudoType pseudoType) const;
    RefPtr<BoxStyle> styleForPageMargin(const GlobalString& pageName, size_t pageIndex, PageMarginType marginType, const BoxStyle& pageStyle) const;

    std::string getCounterText(int value, const GlobalString& listType);
    std::string getMarkerText(int value, const GlobalString& listType);
    RefPtr<FontData> getFontData(const GlobalString& family, const FontDataDescription& description);
    RefPtr<Font> createFont(const FontDescription& description);

    RefPtr<TextResource> fetchTextResource(const Url& url);
    RefPtr<ImageResource> fetchImageResource(const Url& url);
    RefPtr<FontResource> fetchFontResource(const Url& url);

    virtual bool load(const std::string_view& content) = 0;

    Node* cloneNode(bool deep) override;
    Box* createBox(const RefPtr<BoxStyle>& style) override;

    void finishParsingDocument() override;
    void buildBox(Counters& counters, Box* parent) override;
    void build();
    void layout();

    void render(GraphicsContext& context, const Rect& rect);

    PageBoxList& pages() { return m_pages; }
    const PageBoxList& pages() const { return m_pages; }

    void renderPage(GraphicsContext& context, uint32_t pageIndex);
    PageSize pageSizeAt(uint32_t pageIndex) const;
    uint32_t pageCount() const;

    void paginate();

private:
    template<typename ResourceType>
    RefPtr<ResourceType> fetchResource(const Url& url);
    Element* m_rootElement{nullptr};
    Book* m_book;
    Heap* m_heap;
    Url m_baseUrl;
    PageBoxList m_pages;
    DocumentElementMap m_idCache;
    DocumentResourceMap m_resourceCache;
    DocumentFontMap m_fontCache;
    CSSStyleSheet m_styleSheet;
};

inline bool Node::isRootNode() const
{
    return this == m_document->rootElement();
}

inline Heap* Node::heap() const
{
    return m_document->heap();
}

template<>
struct is_a<Document> {
    static bool check(const Node& value) { return value.isDocumentNode(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_DOCUMENT_H
