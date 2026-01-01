/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_DOCUMENT_H
#define PLUTOBOOK_DOCUMENT_H

#include "cssstylesheet.h"
#include "fragmentbuilder.h"
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

    bool inHTMLDocument() const;
    bool inSVGDocument() const;
    bool inXMLDocument() const;

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

    Box* nextSiblingBox() const;
    Box* previousSiblingBox() const;

    Node* firstChild() const;
    Node* lastChild() const;

    void setBox(Box* box) { m_box = box; }
    Box* box() const { return m_box; }
    BoxStyle* style() const;
    Heap* heap() const;

    virtual Node* cloneNode(bool deep) = 0;
    virtual Box* createBox(const RefPtr<BoxStyle>& style) = 0;
    virtual void buildBox(Counters& counters, Box* parent) = 0;
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

    void buildChildrenBox(Counters& counters, Box* parent);
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

    GlobalString foldCase(const GlobalString& name) const;
    GlobalString foldTagNameCase() const { return foldCase(m_tagName); }

    const GlobalString& namespaceURI() const { return m_namespaceURI; }
    const GlobalString& tagName() const { return m_tagName; }
    const AttributeList& attributes() const { return m_attributes; }

    const HeapString& lang() const;
    const HeapString& id() const { return m_id; }
    const ClassNameList& classNames() const { return m_classNames; }

    const Attribute* findAttribute(const GlobalString& name) const;
    const Attribute* findAttributePossiblyIgnoringCase(const GlobalString& name) const;
    bool hasAttribute(const GlobalString& name) const;
    const HeapString& getAttribute(const GlobalString& name) const;
    Url getUrlAttribute(const GlobalString& name) const;

    void setAttributes(const AttributeList& attributes);
    void setAttribute(const Attribute& attribute);
    void setAttribute(const GlobalString& name, const HeapString& value);
    void removeAttribute(const GlobalString& name);

    virtual void parseAttribute(const GlobalString& name, const HeapString& value);
    virtual void collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const {}
    virtual void collectAdditionalAttributeStyle(std::string& output) const {}

    CSSPropertyList inlineStyle();
    CSSPropertyList presentationAttributeStyle();

    Element* parentElement() const;
    Element* firstChildElement() const;
    Element* lastChildElement() const;
    Element* previousSiblingElement() const;
    Element* nextSiblingElement() const;

    void setIsCaseSensitive(bool value) { m_isCaseSensitive = value; }
    bool isCaseSensitive() const { return m_isCaseSensitive; }

    void setIsLinkDestination(bool value) { m_isLinkDestination = value; }
    bool isLinkDestination() const { return m_isLinkDestination; }

    void setIsLinkSource(bool value) { m_isLinkSource = value; }
    bool isLinkSource() const { return m_isLinkSource; }

    Node* cloneNode(bool deep) override;
    Box* createBox(const RefPtr<BoxStyle>& style) override;
    void buildBox(Counters& counters, Box* parent) override;
    void finishParsingDocument() override;

private:
    GlobalString m_namespaceURI;
    GlobalString m_tagName;
    HeapString m_id;
    ClassNameList m_classNames;
    AttributeList m_attributes;

    bool m_isCaseSensitive{false};
    bool m_isLinkDestination{false};
    bool m_isLinkSource{false};
};

template<>
struct is_a<Element> {
    static bool check(const Node& value) { return value.isElementNode(); }
};

inline GlobalString Element::foldCase(const GlobalString& name) const
{
    return m_isCaseSensitive ? name : name.foldCase();
}

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


class CSSMediaQuery;
class CSSMediaFeature;

using CSSMediaQueryList = std::pmr::forward_list<CSSMediaQuery>;
using CSSMediaFeatureList = std::pmr::forward_list<CSSMediaFeature>;

class Resource;
class ResourceData;
class TextResource;
class ImageResource;
class FontResource;
class ResourceFetcher;
class Font;

struct FontDescription;
struct FontDataDescription;

using CounterMap = std::map<GlobalString, std::vector<int>>;

using DocumentElementMap = std::pmr::multimap<HeapString, Element*, std::less<>>;
using DocumentResourceMap = std::pmr::map<Url, RefPtr<Resource>>;
using DocumentFontMap = std::pmr::map<FontDescription, RefPtr<Font>>;
using DocumentCounterMap = std::pmr::map<HeapString, CounterMap, std::less<>>;
using DocumentRunningStyleMap = std::pmr::map<GlobalString, RefPtr<BoxStyle>>;

class BoxView;
class GraphicsContext;
class Size;
class Rect;

class Book;
class PageSize;
class PageMargins;
class PageBox;

using PageBoxList = std::pmr::vector<std::unique_ptr<PageBox>>;

class Document : public ContainerNode, public FragmentBuilder {
public:
    Document(Book* book, Heap* heap, ResourceFetcher* fetcher, Url baseUrl);
    ~Document() override;

    bool isDocumentNode() const final { return true; }
    bool isSVGImageDocument() const { return !m_book && isSVGDocument(); }

    Book* book() const { return m_book; }
    Heap* heap() const { return m_heap; }
    ResourceFetcher* customResourceFetcher() const { return m_customResourceFetcher; }

    const Url& baseUrl() const { return m_baseUrl; }
    void setBaseUrl(Url baseUrl) { m_baseUrl = std::move(baseUrl); }
    Url completeUrl(const std::string_view& value) const { return m_baseUrl.complete(value); }

    BoxView* box() const;
    float width() const;
    float height() const;

    float viewportWidth() const;
    float viewportHeight() const;

    float containerWidth() const { return m_containerWidth; }
    float containerHeight() const { return m_containerHeight; }

    bool setContainerSize(float containerWidth, float containerHeight);

    TextNode* createTextNode(const std::string_view& value);
    Element* createElement(const GlobalString& namespaceURI, const GlobalString& tagName);

    Element* rootElement() const { return m_rootElement; }
    Element* bodyElement() const;

    BoxStyle* rootStyle() const;
    BoxStyle* bodyStyle() const;

    Element* getElementById(const std::string_view& id) const;
    void addElementById(const HeapString& id, Element* element);
    void removeElementById(const HeapString& id, Element* element);

    void addRunningStyle(const GlobalString& name, RefPtr<BoxStyle> style);
    RefPtr<BoxStyle> getRunningStyle(const GlobalString& name) const;

    void addTargetCounters(const HeapString& id, const CounterMap& counters);

    HeapString getTargetCounterText(const HeapString& fragment, const GlobalString& name, const GlobalString& listStyle, const HeapString& separator);
    HeapString getCountersText(const CounterMap& counters, const GlobalString& name, const GlobalString& listStyle, const HeapString& separator);

    void runJavaScript(const std::string_view& script);

    void addAuthorStyleSheet(const std::string_view& content, Url baseUrl);
    void addUserStyleSheet(const std::string_view& content);

    bool supportsMediaFeature(const CSSMediaFeature& feature) const;
    bool supportsMediaFeatures(const CSSMediaFeatureList& features) const;
    bool supportsMediaQuery(const CSSMediaQuery& query) const;
    bool supportsMediaQueries(const CSSMediaQueryList& queries) const;
    bool supportsMedia(const std::string_view& type, const std::string_view& media) const;

    RefPtr<BoxStyle> styleForElement(Element* element, const BoxStyle* parentStyle) const;
    RefPtr<BoxStyle> pseudoStyleForElement(Element* element, PseudoType pseudoType, const BoxStyle* parentStyle) const;

    RefPtr<BoxStyle> styleForPage(const GlobalString& pageName, uint32_t pageIndex, PseudoType pseudoType) const;
    RefPtr<BoxStyle> styleForPageMargin(const GlobalString& pageName, uint32_t pageIndex, PageMarginType marginType, const BoxStyle* pageStyle) const;

    std::string getCounterText(int value, const GlobalString& listType);
    std::string getMarkerText(int value, const GlobalString& listType);

    RefPtr<FontData> getFontData(const GlobalString& family, const FontDataDescription& description);
    RefPtr<Font> createFont(const FontDescription& description);

    RefPtr<TextResource> fetchTextResource(const Url& url);
    RefPtr<ImageResource> fetchImageResource(const Url& url);
    RefPtr<FontResource> fetchFontResource(const Url& url);

    virtual bool parse(const std::string_view& content) = 0;

    Node* cloneNode(bool deep) override;
    Box* createBox(const RefPtr<BoxStyle>& style) override;
    void buildBox(Counters& counters, Box* parent) override;
    void finishParsingDocument() override;

    void serialize(std::ostream& o) const;

    void build();
    void layout();
    void paginate();

    void render(GraphicsContext& context, const Rect& rect);

    PageBoxList& pages() { return m_pages; }
    const PageBoxList& pages() const { return m_pages; }

    void renderPage(GraphicsContext& context, uint32_t pageIndex);
    PageSize pageSizeAt(uint32_t pageIndex) const;
    uint32_t pageCount() const;

    FragmentType fragmentType() const final { return FragmentType::Page; }

    float fragmentHeightForOffset(float offset) const final;
    float fragmentRemainingHeightForOffset(float offset, FragmentBoundaryRule rule) const final;

    Rect pageContentRectAt(uint32_t pageIndex) const;

private:
    template<typename ResourceType>
    RefPtr<ResourceType> fetchResource(const Url& url);
    Element* m_rootElement{nullptr};
    Book* m_book;
    Heap* m_heap;
    ResourceFetcher* m_customResourceFetcher;
    Url m_baseUrl;
    PageBoxList m_pages;
    DocumentElementMap m_idCache;
    DocumentResourceMap m_resourceCache;
    DocumentFontMap m_fontCache;
    DocumentCounterMap m_counterCache;
    DocumentRunningStyleMap m_runningStyles;
    CSSStyleSheet m_styleSheet;

    float m_containerWidth{0};
    float m_containerHeight{0};
};

inline bool Node::isRootNode() const
{
    return this == m_document->rootElement();
}

inline bool Node::inHTMLDocument() const { return m_document->isHTMLDocument(); }
inline bool Node::inSVGDocument() const  { return m_document->isSVGDocument(); }
inline bool Node::inXMLDocument() const  { return m_document->isXMLDocument(); }

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
