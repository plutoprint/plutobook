#include "document.h"
#include "htmldocument.h"
#include "svgdocument.h"
#include "cssparser.h"
#include "counters.h"
#include "textbox.h"
#include "svgtextbox.h"
#include "pagebox.h"
#include "boxview.h"
#include "boxlayer.h"
#include "textresource.h"
#include "imageresource.h"
#include "fontresource.h"
#include "stringutils.h"
#include "fragmentbuilder.h"

#include "plutobook.hpp"

namespace plutobook {

Node::Node(Document* document)
    : m_document(document)
{
}

Node::~Node()
{
    if(m_parentNode)
        m_parentNode->removeChild(this);
    delete m_box;
}

void Node::reparent(ContainerNode* newParent)
{
    if(m_parentNode)
        m_parentNode->removeChild(this);
    newParent->appendChild(this);
}

void Node::remove()
{
    if(m_parentNode) {
        m_parentNode->removeChild(this);
    }
}

Box* Node::nextSiblingBox() const
{
    auto node = nextSibling();
    while(node) {
        if(auto box = node->box())
            return box;
        node = node->nextSibling();
    }

    return nullptr;
}

Box* Node::previousSiblingBox() const
{
    auto node = previousSibling();
    while(node) {
        if(auto box = node->box())
            return box;
        node = node->previousSibling();
    }

    return nullptr;
}

BoxStyle* Node::style() const
{
    if(m_box)
        return m_box->style();
    return nullptr;
}

TextNode::TextNode(Document* document, const HeapString& data)
    : Node(document)
    , m_data(data)
{
}

void TextNode::appendData(const std::string_view& data)
{
    m_data = heap()->concatenateString(m_data, data);
}

bool TextNode::isHidden(const Box* parent) const
{
    if(m_data.empty())
        return true;
    for(auto cc : m_data) {
        if(!isSpace(cc)) {
            return false;
        }
    }

    if(parent->isFlexibleBox() || parent->isTableBox()
        || parent->isTableSectionBox() || parent->isTableRowBox()
        || parent->isTableColumnBox()) {
        return true;
    }

    if(parent->style()->preserveNewline())
        return false;
    if(auto prevBox = previousSiblingBox())
        return !prevBox->isInline() || prevBox->isLineBreakBox();;
    return !parent->isInlineBox();
}

Node* TextNode::cloneNode(bool deep)
{
    return document()->createTextNode(m_data);
}

Box* TextNode::createBox(const RefPtr<BoxStyle>& style)
{
    if(parentNode()->isSVGElement())
        return new (heap()) SVGInlineTextBox(this, style);
    auto box = new (heap()) TextBox(this, style);
    box->setText(m_data);
    return box;
}

void TextNode::buildBox(Counters& counters, Box* parent)
{
    if(isHidden(parent))
        return;
    if(auto box = createBox(parent->style())) {
        parent->addChild(box);
    }
}

void TextNode::serialize(std::ostream& o) const
{
    o << m_data;
}

ContainerNode::ContainerNode(Document* document)
    : Node(document)
{
}

ContainerNode::~ContainerNode()
{
    auto child = m_firstChild;
    while(child) {
        Node* nextChild = child->nextSibling();
        child->setParentNode(nullptr);
        child->setPreviousSibling(nullptr);
        child->setNextSibling(nullptr);
        delete child;
        child = nextChild;
    }
}

void ContainerNode::appendChild(Node* newChild)
{
    assert(newChild->parentNode() == nullptr);
    assert(newChild->previousSibling() == nullptr);
    assert(newChild->nextSibling() == nullptr);
    newChild->setParentNode(this);
    if(m_lastChild == nullptr) {
        assert(m_firstChild == nullptr);
        m_firstChild = m_lastChild = newChild;
        return;
    }

    newChild->setPreviousSibling(m_lastChild);
    m_lastChild->setNextSibling(newChild);
    m_lastChild = newChild;
}

void ContainerNode::insertChild(Node* newChild, Node* nextChild)
{
    if(nextChild == nullptr) {
        appendChild(newChild);
        return;
    }

    assert(nextChild->parentNode() == this);
    assert(newChild->parentNode() == nullptr);
    assert(newChild->previousSibling() == nullptr);
    assert(newChild->nextSibling() == nullptr);

    auto previousChild = nextChild->previousSibling();
    nextChild->setPreviousSibling(newChild);
    assert(m_lastChild != previousChild);
    if(previousChild == nullptr) {
        assert(m_firstChild == nextChild);
        m_firstChild = newChild;
    } else {
        assert(m_firstChild != nextChild);
        previousChild->setNextSibling(newChild);
    }

    newChild->setParentNode(this);
    newChild->setPreviousSibling(previousChild);
    newChild->setNextSibling(nextChild);
}

void ContainerNode::removeChild(Node* child)
{
    assert(child->parentNode() == this);
    auto nextChild = child->nextSibling();
    auto previousChild = child->previousSibling();
    if(nextChild)
        nextChild->setPreviousSibling(previousChild);
    if(previousChild) {
        previousChild->setNextSibling(nextChild);
    }

    if(m_firstChild == child)
        m_firstChild = nextChild;
    if(m_lastChild == child) {
        m_lastChild = previousChild;
    }

    child->setParentNode(nullptr);
    child->setPreviousSibling(nullptr);
    child->setNextSibling(nullptr);
}

void ContainerNode::reparentChildren(ContainerNode* newParent)
{
    while(auto child = firstChild()) {
        child->reparent(newParent);
    }
}

void ContainerNode::cloneChildren(ContainerNode* newParent)
{
    auto child = m_firstChild;
    while(child) {
        newParent->appendChild(child->cloneNode(true));
        child = child->nextSibling();
    }
}

std::string ContainerNode::textFromChildren() const
{
    std::string content;
    auto child = m_firstChild;
    while(child) {
        if(auto node = to<TextNode>(child))
            content += node->data();
        child = child->nextSibling();
    }

    return content;
}

void ContainerNode::buildBox(Counters& counters, Box* parent)
{
    auto child = m_firstChild;
    while(child) {
        child->buildBox(counters, parent);
        child = child->nextSibling();
    }
}

void ContainerNode::serialize(std::ostream& o) const
{
    auto child = m_firstChild;
    while(child) {
        child->serialize(o);
        child = child->nextSibling();
    }
}

void ContainerNode::finishParsingDocument()
{
    auto child = m_firstChild;
    while(child) {
        child->finishParsingDocument();
        child = child->nextSibling();
    }
}

Element::Element(Document* document, const GlobalString& namespaceURI, const GlobalString& tagName)
    : ContainerNode(document)
    , m_namespaceURI(namespaceURI)
    , m_tagName(tagName)
    , m_classNames(document->heap())
    , m_attributes(document->heap())
{
}

const HeapString& Element::lang() const
{
    return getAttribute(langAttr);
}

const Attribute* Element::findAttribute(const GlobalString& name) const
{
    for(auto& attribute : m_attributes) {
        if(name == attribute.name()) {
            return &attribute;
        }
    }

    return nullptr;
}

bool Element::hasAttribute(const GlobalString& name) const
{
    for(auto& attribute : m_attributes) {
        if(name == attribute.name()) {
            return true;
        }
    }

    return false;
}

const HeapString& Element::getAttribute(const GlobalString& name) const
{
    for(auto& attribute : m_attributes) {
        if(name == attribute.name()) {
            return attribute.value();
        }
    }

    return emptyGlo;
}

void Element::setAttributes(const AttributeList& attributes)
{
    assert(m_attributes.empty());
    for(auto& attribute : attributes) {
        setAttribute(attribute);
    }
}

void Element::setAttribute(const Attribute& attribute)
{
    setAttribute(attribute.name(), attribute.value());
}

void Element::setAttribute(const GlobalString& name, const HeapString& value)
{
    parseAttribute(name, value);
    for(auto& attribute : m_attributes) {
        if(name == attribute.name()) {
            attribute.setValue(value);
            return;
        }
    }

    m_attributes.emplace_front(name, value);
}

void Element::removeAttribute(const GlobalString& name)
{
    parseAttribute(name, emptyGlo);
    m_attributes.remove_if([&name](const auto& attribute) {
        return name == attribute.name();
    });
}

void Element::parseAttribute(const GlobalString& name, const HeapString& value)
{
    if(name == idAttr) {
        if(!m_id.empty())
            document()->removeElementById(m_id, this);
        if(!value.empty()) {
            document()->addElementById(value, this);
        }

        m_id = value;
    } else if(name == classAttr) {
        m_classNames.clear();
        if(value.empty())
            return;
        size_t begin = 0;
        while(true) {
            while(begin < value.size() && isSpace(value[begin]))
                ++begin;
            if(begin >= value.size())
                break;
            size_t end = begin + 1;
            while(end < value.size() && !isSpace(value[end]))
                ++end;
            m_classNames.push_front(value.substring(begin, end - begin));
            begin = end + 1;
        }
    }
}

CSSPropertyList Element::inlineStyle()
{
    auto value = getAttribute(styleAttr);
    if(value.empty())
        return CSSPropertyList();
    CSSParser parser(CSSStyleOrigin::Inline, document(), document()->baseUrl());
    return parser.parseStyle(value);
}

CSSPropertyList Element::presentationAttributeStyle()
{
    std::string output;
    for(auto& attribute : attributes()) {
        collectAttributeStyle(output, attribute.name(), attribute.value());
    }

    if(output.empty())
        return CSSPropertyList();
    CSSParser parser(CSSStyleOrigin::PresentationAttribute, this, document()->baseUrl());
    return parser.parseStyle(output);
}

Element* Element::parentElement() const
{
    return to<Element>(parentNode());
}

Element* Element::previousElement() const
{
    auto node = previousSibling();
    while(node) {
        if(auto element = to<Element>(node))
            return element;
        node = node->previousSibling();
    }

    return nullptr;
}

Element* Element::nextElement() const
{
    auto node = nextSibling();
    while(node) {
        if(auto element = to<Element>(node))
            return element;
        node = node->nextSibling();
    }

    return nullptr;
}

Node* Element::cloneNode(bool deep)
{
    auto newElement = document()->createElement(m_namespaceURI, m_tagName);
    newElement->setAttributes(m_attributes);
    if(deep) { cloneChildren(newElement); }
    return newElement;
}

Box* Element::createBox(const RefPtr<BoxStyle>& style)
{
    return Box::create(this, style);
}

void Element::buildBox(Counters& counters, Box* parent)
{
    auto style = document()->styleForElement(this, *parent->style());
    if(style == nullptr || style->display() == Display::None)
        return;
    auto box = createBox(style);
    if(box == nullptr)
        return;
    parent->addChild(box);
    ContainerNode::buildBox(counters, box);
}

void Element::serialize(std::ostream& o) const
{
    o << '<';
    o << m_tagName;
    for(auto& attribute : attributes()) {
        o << ' ';
        o << attribute.name();
        o << '=';
        o << '"';
        o << attribute.value();
        o << '"';
    }

    if(!firstChild()) {
        o << '/';
        o << '>';
    } else {
        o << '>';
        ContainerNode::serialize(o);
        o << '<';
        o << '/';
        o << m_tagName;
        o << '>';
    }
}

Document::Document(Book* book, Heap* heap, ResourceFetcher* fetcher, Url url)
    : ContainerNode(this)
    , m_book(book)
    , m_heap(heap)
    , m_customResourceFetcher(fetcher)
    , m_baseUrl(std::move(url))
    , m_pages(heap)
    , m_idCache(heap)
    , m_resourceCache(heap)
    , m_fontCache(heap)
    , m_counterCache(heap)
    , m_styleSheet(this)
{
}

Document::~Document() = default;

BoxView* Document::box() const
{
    return static_cast<BoxView*>(Node::box());
}

float Document::width() const
{
    return box()->layer()->overflowRight();
}

float Document::height() const
{
    return box()->layer()->overflowBottom();
}

float Document::viewportWidth() const
{
    if(m_book)
        return m_book->viewportWidth();
    return 0.f;
}

float Document::viewportHeight() const
{
    if(m_book)
        return m_book->viewportHeight();
    return 0.f;
}

TextNode* Document::createTextNode(const std::string_view& value)
{
    return new (m_heap) TextNode(this, m_heap->createString(value));
}

Element* Document::createElement(const GlobalString& namespaceURI, const GlobalString& tagName)
{
    if(namespaceURI == xhtmlNs) {
        if(tagName == bodyTag)
            return new (m_heap) HTMLBodyElement(this);
        if(tagName == fontTag)
            return new (m_heap) HTMLFontElement(this);
        if(tagName == imgTag)
            return new (m_heap) HTMLImageElement(this);
        if(tagName == brTag)
            return new (m_heap) HTMLBRElement(this);
        if(tagName == wbrTag)
            return new (m_heap) HTMLWBRElement(this);
        if(tagName == liTag)
            return new (m_heap) HTMLLIElement(this);
        if(tagName == olTag)
            return new (m_heap) HTMLOLElement(this);
        if(tagName == tableTag)
            return new (m_heap) HTMLTableElement(this);
        if(tagName == theadTag || tagName == tbodyTag || tagName == tfootTag)
            return new (m_heap) HTMLTableSectionElement(this, tagName);
        if(tagName == trTag)
            return new (m_heap) HTMLTableRowElement(this);
        if(tagName == colTag || tagName == colgroupTag)
            return new (m_heap) HTMLTableColElement(this, tagName);
        if(tagName == tdTag || tagName == thTag)
            return new (m_heap) HTMLTableCellElement(this, tagName);
        if(tagName == inputTag)
            return new (m_heap) HTMLInputElement(this);
        if(tagName == textareaTag)
            return new (m_heap) HTMLTextAreaElement(this);
        if(tagName == selectTag)
            return new (m_heap) HTMLSelectElement(this);
        if(tagName == styleTag)
            return new (m_heap) HTMLStyleElement(this);
        if(tagName == linkTag)
            return new (m_heap) HTMLLinkElement(this);
        if(tagName == titleTag)
            return new (m_heap) HTMLTitleElement(this);
        return new (m_heap) HTMLElement(this, tagName);
    }

    if(namespaceURI == svgNs) {
        if(tagName == svgTag)
            return new (m_heap) SVGSVGElement(this);
        if(tagName == useTag)
            return new (m_heap) SVGUseElement(this);
        if(tagName == imageTag)
            return new (m_heap) SVGImageElement(this);
        if(tagName == symbolTag)
            return new (m_heap) SVGSymbolElement(this);
        if(tagName == aTag)
            return new (m_heap) SVGAElement(this);
        if(tagName == gTag)
            return new (m_heap) SVGGElement(this);
        if(tagName == defsTag)
            return new (m_heap) SVGDefsElement(this);
        if(tagName == lineTag)
            return new (m_heap) SVGLineElement(this);
        if(tagName == rectTag)
            return new (m_heap) SVGRectElement(this);
        if(tagName == circleTag)
            return new (m_heap) SVGCircleElement(this);
        if(tagName == ellipseTag)
            return new (m_heap) SVGEllipseElement(this);
        if(tagName == polylineTag)
            return new (m_heap) SVGPolylineElement(this);
        if(tagName == polygonTag)
            return new (m_heap) SVGPolygonElement(this);
        if(tagName == pathTag)
            return new (m_heap) SVGPathElement(this);
        if(tagName == tspanTag)
            return new (m_heap) SVGTSpanElement(this);
        if(tagName == textTag)
            return new (m_heap) SVGTextElement(this);
        if(tagName == markerTag)
            return new (m_heap) SVGMarkerElement(this);
        if(tagName == clipPathTag)
            return new (m_heap) SVGClipPathElement(this);
        if(tagName == maskTag)
            return new (m_heap) SVGMaskElement(this);
        if(tagName == patternTag)
            return new (m_heap) SVGPatternElement(this);
        if(tagName == stopTag)
            return new (m_heap) SVGStopElement(this);
        if(tagName == linearGradientTag)
            return new (m_heap) SVGLinearGradientElement(this);
        if(tagName == radialGradientTag)
            return new (m_heap) SVGRadialGradientElement(this);
        if(tagName == styleTag)
            return new (m_heap) SVGStyleElement(this);
        return new (m_heap) SVGElement(this, tagName);
    }

    return new (m_heap) Element(this, namespaceURI, tagName);
}

Element* Document::bodyElement() const
{
    auto element = to<HTMLElement>(m_rootElement);
    if(element && element->tagName() == htmlTag) {
        auto child = element->firstChild();
        while(child) {
            auto element = to<HTMLElement>(child);
            if(element && element->tagName() == bodyTag)
                return element;
            child = child->nextSibling();
        }
    }

    return nullptr;
}

BoxStyle* Document::bodyStyle() const
{
    if(auto element = bodyElement())
        return element->style();
    return nullptr;
}

BoxStyle* Document::rootStyle() const
{
    if(m_rootElement) {
        if(auto rootStyle = m_rootElement->style()) {
            return rootStyle;
        }
    }

    return style();
}

BoxStyle* Document::backgroundStyle() const
{
    return box()->backgroundStyle();
}

Element* Document::getElementById(const std::string_view& id) const
{
    auto it = m_idCache.find(id);
    if(it == m_idCache.end())
        return nullptr;
    return it->second;
}

void Document::addElementById(const HeapString& id, Element* element)
{
    m_idCache.emplace(id, element);
}

void Document::removeElementById(const HeapString& id, Element* element)
{
    auto range = m_idCache.equal_range(id);
    for(auto it = range.first; it != range.second; ++it) {
        if(it->second == element) {
            m_idCache.erase(it);
            break;
        }
    }
}

void Document::addTargetCounters(const HeapString& id, const CounterMap& counters)
{
    m_counterCache.emplace(id, counters);
}

HeapString Document::getTargetCounterText(const HeapString& fragment, const GlobalString& name, const GlobalString& listStyle, const HeapString& separator)
{
    if(fragment.empty() || fragment.front() != '#')
        return emptyGlo;
    auto it = m_counterCache.find(fragment.substring(1));
    if(it == m_counterCache.end())
        return emptyGlo;
    return getCountersText(it->second, name, listStyle, separator);
}

HeapString Document::getCountersText(const CounterMap& counters, const GlobalString& name, const GlobalString& listStyle, const HeapString& separator)
{
    auto it = counters.find(name);
    if(it == counters.end())
        return m_heap->createString(getCounterText(0, listStyle));
    if(separator.empty()) {
        int value = 0;
        if(!it->second.empty())
            value = it->second.back();
        return m_heap->createString(getCounterText(value, listStyle));
    }

    std::string text;
    for(auto value : it->second) {
        if(!text.empty())
            text += separator.value();
        text += getCounterText(value, listStyle);
    }

    return m_heap->createString(text);
}

void Document::addAuthorJavaScript(const std::string_view& content)
{
}

void Document::addUserJavaScript(const std::string_view& content)
{
}

void Document::addAuthorStyleSheet(const std::string_view& content, Url baseUrl)
{
    m_styleSheet.parseStyle(content, CSSStyleOrigin::Author, std::move(baseUrl));
}

void Document::addUserStyleSheet(const std::string_view& content)
{
    m_styleSheet.parseStyle(content, CSSStyleOrigin::User, m_baseUrl);
}

bool Document::supportsMediaType(CSSMediaType mediaType) const
{
    if(m_book == nullptr || mediaType == CSSMediaType::All)
        return true;
    if(mediaType == CSSMediaType::Print)
        return m_book->mediaType() == MediaType::Print;
    return m_book->mediaType() == MediaType::Screen;
}

bool Document::supportsMediaQueries(const CSSMediaList& queries) const
{
    if(queries.empty())
        return true;
    for(auto& mediaType : queries) {
        if(supportsMediaType(mediaType)) {
            return true;
        }
    }

    return false;
}

bool Document::supportsMedia(const std::string_view& type, const std::string_view& media) const
{
    if(type.empty() || equals(type, "text/css", isXMLDocument())) {
        CSSMediaList queries(heap());
        if(CSSParser::parseMediaQueries(queries, media)) {
            return supportsMediaQueries(queries);
        }
    }

    return false;
}

RefPtr<BoxStyle> Document::styleForElement(Element* element, const BoxStyle& parentStyle) const
{
    return m_styleSheet.styleForElement(element, parentStyle);
}

RefPtr<BoxStyle> Document::pseudoStyleForElement(Element* element, PseudoType pseudoType, const BoxStyle& parentStyle) const
{
    return m_styleSheet.pseudoStyleForElement(element, pseudoType, parentStyle);
}

RefPtr<BoxStyle> Document::styleForPage(const GlobalString& pageName, size_t pageIndex, PseudoType pseudoType) const
{
    return m_styleSheet.styleForPage(pageName, pageIndex, pseudoType);
}

RefPtr<BoxStyle> Document::styleForPageMargin(const GlobalString& pageName, size_t pageIndex, PageMarginType marginType, const BoxStyle& pageStyle) const
{
    return m_styleSheet.styleForPageMargin(pageName, pageIndex, marginType, pageStyle);
}

std::string Document::getCounterText(int value, const GlobalString& listType)
{
    return m_styleSheet.getCounterText(value, listType);
}

std::string Document::getMarkerText(int value, const GlobalString& listType)
{
    return m_styleSheet.getMarkerText(value, listType);
}

RefPtr<FontData> Document::getFontData(const GlobalString& family, const FontDataDescription& description)
{
    return m_styleSheet.getFontData(family, description);
}

RefPtr<Font> Document::createFont(const FontDescription& description)
{
    auto& font = m_fontCache[description];
    if(font == nullptr)
        font = Font::create(this, description);
    return font;
}

RefPtr<TextResource> Document::fetchTextResource(const Url& url)
{
    return fetchResource<TextResource>(url);
}

RefPtr<ImageResource> Document::fetchImageResource(const Url& url)
{
    return fetchResource<ImageResource>(url);
}

RefPtr<FontResource> Document::fetchFontResource(const Url& url)
{
    return fetchResource<FontResource>(url);
}

Node* Document::cloneNode(bool deep)
{
    return nullptr;
}

Box* Document::createBox(const RefPtr<BoxStyle>& style)
{
    return new (m_heap) BoxView(this, style);
}

void Document::finishParsingDocument()
{
    assert(m_rootElement == nullptr);
    auto child = firstChild();
    while(child) {
        if(m_rootElement == nullptr)
            m_rootElement = to<Element>(child);
        child->finishParsingDocument();
        child = child->nextSibling();
    }
}

void Document::buildBox(Counters& counters, Box* parent)
{
    auto rootStyle = BoxStyle::create(this, PseudoType::None, Display::Block);
    rootStyle->setPosition(Position::Absolute);
    rootStyle->setFontDescription(FontDescription());

    auto rootBox = createBox(rootStyle);
    counters.push();
    ContainerNode::buildBox(counters, rootBox);
    counters.pop();
    rootBox->build();
}

void Document::build()
{
    Counters counters(this);
    buildBox(counters, nullptr);
}

void Document::layout(PageBuilder* paginator)
{
    box()->layout(paginator);
}

void Document::render(GraphicsContext& context, const Rect& rect)
{
    box()->layer()->paint(context, rect);
}

void Document::renderPage(GraphicsContext& context, uint32_t pageIndex)
{
    if(pageIndex >= m_pages.size())
        return;
    const auto& page = m_pages[pageIndex];
    Rect pageRect(0, page->pageTop(), page->width() - page->marginWidth(), page->height() - page->marginHeight());
    if(pageRect.isEmpty())
        return;
    context.save();
    context.translate(page->marginLeft(), page->marginTop());
    context.translate(-pageRect.x, -pageRect.y);
    context.clipRect(pageRect);
    box()->setCurrentPage(page.get());
    box()->layer()->paint(context, pageRect);
    context.restore();
}

PageSize Document::pageSizeAt(uint32_t pageIndex) const
{
    if(pageIndex < m_pages.size())
        return m_pages[pageIndex]->pageSize();
    return PageSize();
}

uint32_t Document::pageCount() const
{
    return m_pages.size();
}

template<typename ResourceType>
RefPtr<ResourceType> Document::fetchResource(const Url& url)
{
    if(url.isEmpty())
        return nullptr;
    auto it = m_resourceCache.find(url);
    if(it != m_resourceCache.end())
        return to<ResourceType>(it->second);
    auto resource = ResourceType::create(m_customResourceFetcher, url);
    if(!url.protocolIs("data"))
        m_resourceCache.emplace(url, resource);
    return resource;
}

} // namespace plutobook
