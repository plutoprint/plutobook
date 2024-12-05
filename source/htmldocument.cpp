#include "htmldocument.h"
#include "htmlparser.h"
#include "plutobook.hpp"
#include "cssrule.h"
#include "counters.h"
#include "textbox.h"
#include "replacedbox.h"
#include "tablebox.h"
#include "formcontrolbox.h"
#include "textresource.h"
#include "imageresource.h"
#include "stringutils.h"

#include <unicode/uchar.h>
#include <unicode/uiter.h>

namespace plutobook {

HTMLElement::HTMLElement(Document* document, const GlobalString& tagName)
    : Element(document, xhtmlNs, tagName)
{
}

inline bool isFirstLetterPunctuation(UChar32 cc)
{
    switch(u_charType(cc)) {
    case U_START_PUNCTUATION:
    case U_END_PUNCTUATION:
    case U_INITIAL_PUNCTUATION:
    case U_FINAL_PUNCTUATION:
    case U_OTHER_PUNCTUATION:
        return true;
    default:
        return false;
    }
}

static size_t firstLetterTextLength(const HeapString& text)
{
    UCharIterator it;
    uiter_setUTF8(&it, text.data(), text.size());

    auto hasLetter = false;
    auto hasPunctuation = false;
    size_t textLength = 0;
    while(it.hasNext(&it)) {
        auto cc = uiter_next32(&it);
        if(!isSpace(cc)) {
            if(!isFirstLetterPunctuation(cc)) {
                if(hasLetter)
                    break;
                hasLetter = true;
            } else {
                hasPunctuation = true;
            }
        }

        textLength += U8_LENGTH(cc);
    }

    if(!hasLetter && !hasPunctuation)
        return 0;
    return textLength;
}

void HTMLElement::buildFirstLetterPseudoBox(Box* parent)
{
    if(!parent->isBlockFlowBox())
        return;
    auto style = document()->pseudoStyleForElement(this, PseudoType::FirstLetter, *parent->style());
    if(style == nullptr || style->display() == Display::None)
        return;
    auto child = parent->firstChild();
    while(child) {
        if(child->pseudoType() == PseudoType::FirstLetter || child->isReplaced()
            || child->isLineBreakBox() || child->isWordBreakBox()) {
            return;
        }

        if(auto textBox = to<TextBox>(child)) {
            auto& text = textBox->text();
            if(auto length = firstLetterTextLength(text)) {
                auto newTextBox = new (heap()) TextBox(nullptr, style);
                newTextBox->setText(text.substring(0, length));
                textBox->setText(text.substring(length));

                auto letterBox = Box::create(nullptr, style);
                letterBox->addChild(newTextBox);
                textBox->parentBox()->insertChild(letterBox, textBox);
                break;
            }
        }

        if(!child->isFloatingOrPositioned() && !child->isListMarkerBox()
            && !child->isTableBox() && !child->isFlexibleBox()) {
            if(child->firstChild()) {
                child = child->firstChild();
                continue;
            }
        }

        while(true) {
            if(child->nextSibling()) {
                child = child->nextSibling();
                break;
            }

            child = child->parentBox();
            if(child == parent) {
                return;
            }
        }
    }
}

class ContentBoxBuilder {
public:
    ContentBoxBuilder(Counters& counters, Element* element, Box* box);

    void build();

private:
    void addText(const HeapString& text);
    void addLeaderText(const HeapString& text);
    void addLeader(const RefPtr<CSSValue>& value);
    void addTargetCounter(const RefPtr<CSSFunctionValue>& function);
    void addImage(RefPtr<Image> image);
    void addQuote(CSSValueID value);

    Counters& m_counters;
    Element* m_element;
    Box* m_parentBox;
    BoxStyle* m_parentStyle;
    TextBox* m_lastTextBox{nullptr};
};

ContentBoxBuilder::ContentBoxBuilder(Counters& counters, Element* element, Box* box)
    : m_counters(counters)
    , m_element(element)
    , m_parentBox(box)
    , m_parentStyle(box->style())
{
}

void ContentBoxBuilder::addText(const HeapString& text)
{
    if(text.empty())
        return;
    if(m_lastTextBox) {
        m_lastTextBox->appendText(text);
        return;
    }

    auto box = new (m_element->heap()) TextBox(nullptr, m_parentStyle);
    box->setText(text);
    m_parentBox->addChild(box);
    m_lastTextBox = box;
}

void ContentBoxBuilder::addLeaderText(const HeapString& text)
{
    if(text.empty())
        return;
    auto box = new (m_element->heap()) LeaderTextBox(m_parentStyle);
    box->setText(text);
    m_parentBox->addChild(box);
    m_lastTextBox = nullptr;
}

void ContentBoxBuilder::addLeader(const RefPtr<CSSValue>& value)
{
    static const GlobalString dotted(".");
    static const GlobalString solid("_");
    static const GlobalString space(" ");
    if(auto string = to<CSSStringValue>(value)) {
        addLeaderText(string->value());
        return;
    }

    auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Dotted:
        addLeaderText(dotted);
        break;
    case CSSValueID::Solid:
        addLeaderText(solid);
        break;
    case CSSValueID::Space:
        addLeaderText(space);
        break;
    default:
        assert(false);
    }
}

void ContentBoxBuilder::addTargetCounter(const RefPtr<CSSFunctionValue>& function)
{
    HeapString fragment;
    GlobalString identifier;
    HeapString seperator;
    GlobalString listStyle;

    size_t index = 0;

    assert(function->id() == CSSValueID::TargetCounter || function->id() == CSSValueID::TargetCounters);
    if(auto attr = to<CSSUnaryFunctionValue>(function->at(index))) {
        assert(attr->id() == CSSValueID::Attr);
        fragment = m_element->getAttribute(to<CSSCustomIdentValue>(*attr->value()).value());
    } else {
        fragment = to<CSSLocalUrlValue>(*function->at(index)).value();
    }

    ++index;

    identifier = to<CSSCustomIdentValue>(*function->at(index++)).value();
    if(function->id() == CSSValueID::TargetCounters)
        seperator = to<CSSStringValue>(*function->at(index++)).value();
    if(index < function->size()) {
        listStyle = to<CSSCustomIdentValue>(*function->at(index++)).value();
        assert(index == function->size());
    }

    addText(m_element->document()->getTargetCounterText(fragment, identifier, listStyle, seperator));
}

void ContentBoxBuilder::addImage(RefPtr<Image> image)
{
    if(image == nullptr)
        return;
    auto style = BoxStyle::create(*m_parentStyle, Display::Inline);
    auto box = new (m_element->heap()) ImageBox(nullptr, style);
    box->setImage(std::move(image));
    m_parentBox->addChild(box);
    m_lastTextBox = nullptr;
}

void ContentBoxBuilder::addQuote(CSSValueID value)
{
    assert(value == CSSValueID::OpenQuote || value == CSSValueID::CloseQuote || value == CSSValueID::NoOpenQuote || value == CSSValueID::NoCloseQuote);
    auto openquote = (value == CSSValueID::OpenQuote || value == CSSValueID::NoOpenQuote);
    auto closequote = (value == CSSValueID::CloseQuote || value == CSSValueID::NoCloseQuote);
    auto usequote = (value == CSSValueID::OpenQuote || value == CSSValueID::CloseQuote);
    if(closequote && m_counters.quoteDepth())
        m_counters.decreaseQuoteDepth();
    if(usequote)
        addText(m_parentStyle->getQuote(openquote, m_counters.quoteDepth()));
    if(openquote) {
        m_counters.increaseQuoteDepth();
    }
}

void ContentBoxBuilder::build()
{
    auto content = m_parentStyle->get(CSSPropertyID::Content);
    if(content && content->id() == CSSValueID::None)
        return;
    if(content == nullptr || content->id() == CSSValueID::Normal) {
        if(m_parentStyle->pseudoType() != PseudoType::Marker)
            return;
        if(auto image = m_parentStyle->listStyleImage()) {
            addImage(std::move(image));
            return;
        }

        auto listStyleType = m_parentStyle->get(CSSPropertyID::ListStyleType);
        if(listStyleType == nullptr) {
            static const GlobalString disc("disc");
            addText(m_counters.markerText(disc));
            return;
        }

        if(listStyleType->id() == CSSValueID::None)
            return;
        if(auto listStyle = to<CSSStringValue>(listStyleType)) {
            addText(listStyle->value());
            return;
        }

        auto& listStyle = to<CSSCustomIdentValue>(*listStyleType);
        addText(m_counters.markerText(listStyle.value()));
        return;
    }

    for(auto& value : to<CSSListValue>(*content)) {
        if(auto string = to<CSSStringValue>(value)) {
            addText(string->value());
        } else if(auto image = to<CSSImageValue>(value)) {
            addImage(image->fetch(m_element->document()));
        } else if(auto counter = to<CSSCounterValue>(value)) {
            addText(m_counters.counterText(counter->identifier(), counter->listStyle(), counter->separator()));
        } else if(auto targetCounter = to<CSSFunctionValue>(value)) {
            addTargetCounter(targetCounter);
        } else if(auto ident = to<CSSIdentValue>(value)) {
            addQuote(ident->value());
        } else {
            auto& function = to<CSSUnaryFunctionValue>(*value);
            if(function.id() == CSSValueID::Attr) {
                auto& name = to<CSSCustomIdentValue>(*function.value());
                addText(m_element->getAttribute(name.value()));
            } else {
                assert(function.id() == CSSValueID::Leader);
                addLeader(function.value());
            }
        }
    }
}

void HTMLElement::buildPseudoBox(Counters& counters, Box* parent, PseudoType pseudoType)
{
    if(pseudoType == PseudoType::Marker && !parent->isListItemBox())
        return;
    auto style = document()->pseudoStyleForElement(this, pseudoType, *parent->style());
    if(style == nullptr || style->display() == Display::None)
        return;
    auto box = Box::create(nullptr, style);
    parent->addChild(box);
    if(pseudoType == PseudoType::Before || pseudoType == PseudoType::After) {
        counters.update(box);
        buildPseudoBox(counters, box, PseudoType::Marker);
    }

    ContentBoxBuilder(counters, this, box).build();
}

void HTMLElement::buildBox(Counters& counters, Box* parent)
{
    auto style = document()->styleForElement(this, *parent->style());
    if(style == nullptr || style->display() == Display::None)
        return;
    auto box = createBox(style);
    if(box == nullptr)
        return;
    parent->addChild(box);
    counters.update(box);
    counters.push();
    buildPseudoBox(counters, box, PseudoType::Marker);
    buildPseudoBox(counters, box, PseudoType::Before);
    ContainerNode::buildBox(counters, box);
    buildPseudoBox(counters, box, PseudoType::After);
    buildFirstLetterPseudoBox(box);
    counters.pop();
}

template<typename T>
static bool parseHTMLInteger(T& output, std::string_view input)
{
    constexpr auto isSigned = std::numeric_limits<T>::is_signed;
    stripLeadingAndTrailingSpaces(input);
    bool isNegative = false;
    if(!input.empty() && input.front() == '+')
        input.remove_prefix(1);
    else if(!input.empty() && isSigned && input.front() == '-') {
        input.remove_prefix(1);
        isNegative = true;
    }

    if(input.empty() || !isDigit(input.front()))
        return false;
    T integer = 0;
    do {
        integer = integer * 10 + input.front() - '0';
        input.remove_prefix(1);
    } while(!input.empty() && isDigit(input.front()));

    using SignedType = typename std::make_signed<T>::type;
    if(isNegative)
        output = -static_cast<SignedType>(integer);
    else
        output = integer;
    return true;
}

static void addHTMLAttributeStyle(std::string& output, const std::string_view& name, const std::string_view& value)
{
    if(value.empty())
        return;
    output += name;
    output += ':';
    output += value;
    output += ';';
}

static void addHTMLLengthAttributeStyle(std::string& output, const std::string_view& name, const std::string_view& value)
{
    if(value.empty())
        return;
    output += name;
    output += ':';
    output += value;
    output += "px;";
}

static void addHTMLUrlAttributeStyle(std::string& output, const std::string_view& name, const std::string_view& value)
{
    if(value.empty())
        return;
    output += name;
    output += ':';
    output += "url(";
    output += value;
    output += ");";
}

HTMLBodyElement::HTMLBodyElement(Document* document)
    : HTMLElement(document, bodyTag)
{
}

void HTMLBodyElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == textAttr) {
        addHTMLAttributeStyle(output, "color", value);
    } else if(name == bgcolorAttr) {
        addHTMLAttributeStyle(output, "background-color", value);
    } else if(name == backgroundAttr) {
        addHTMLUrlAttributeStyle(output, "background-image", value);
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLFontElement::HTMLFontElement(Document* document)
    : HTMLElement(document, fontTag)
{
}

static void addHTMLFontSizeAttributeStyle(std::string& output, std::string_view input)
{
    bool hasPlusSign = false;
    bool hasMinusSign = false;
    stripLeadingAndTrailingSpaces(input);
    if(!input.empty() && input.front() == '+') {
        input.remove_prefix(1);
        hasPlusSign = true;
    } else if(!input.empty() && input.front() == '-') {
        input.remove_prefix(1);
        hasMinusSign = true;
    }

    if(input.empty() || !isDigit(input.front()))
        return;
    int value = 0;
    do {
        value = value * 10 + input.front() - '0';
        input.remove_prefix(1);
    } while(!input.empty() && isDigit(input.front()));

    if(hasPlusSign)
        value += 3;
    else if(hasMinusSign) {
        value = 3 - value;
    }

    if(value > 7)
        value = 7;
    if(value < 1) {
        value = 1;
    }

    output += "font-size:";
    switch(value) {
    case 1:
        output += "x-small;";
        break;
    case 2:
        output += "small;";
        break;
    case 3:
        output += "medium;";
        break;
    case 4:
        output += "large;";
        break;
    case 5:
        output += "x-large;";
        break;
    case 6:
        output += "xx-large;";
        break;
    case 7:
        output += "xxx-large;";
        break;
    default:
        assert(false);
    }
}

void HTMLFontElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == sizeAttr) {
        addHTMLFontSizeAttributeStyle(output, value);
    } else if(name == faceAttr) {
        addHTMLAttributeStyle(output, "font-family", value);
    } else if(name == colorAttr) {
        addHTMLAttributeStyle(output, "color", value);
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLImageElement::HTMLImageElement(Document* document)
    : HTMLElement(document, imgTag)
{
}

void HTMLImageElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        addHTMLLengthAttributeStyle(output, "width", value);
    } else if(name == heightAttr) {
        addHTMLLengthAttributeStyle(output, "height", value);
    } else if(name == hspaceAttr) {
        addHTMLLengthAttributeStyle(output, "margin-left", value);
        addHTMLLengthAttributeStyle(output, "margin-right", value);
    } else if(name == vspaceAttr) {
        addHTMLLengthAttributeStyle(output, "margin-top", value);
        addHTMLLengthAttributeStyle(output, "margin-bottom", value);
    } else if(name == borderAttr) {
        addHTMLLengthAttributeStyle(output, "border-width", value);
        addHTMLAttributeStyle(output, "border-style", "solid");
    } else if(name == valignAttr) {
        addHTMLAttributeStyle(output, "vertical-align", value);
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

const HeapString& HTMLImageElement::src() const
{
    return getAttribute(srcAttr);
}

const HeapString& HTMLImageElement::altText() const
{
    return getAttribute(altAttr);
}

RefPtr<Image> HTMLImageElement::srcImage() const
{
    auto url = document()->completeUrl(src());
    if(auto resource = document()->fetchImageResource(url))
        return resource->image();
    return nullptr;
}

Box* HTMLImageElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto image = srcImage();
    auto text = altText();
    if(!image && !text.empty()) {
        auto container = Box::create(this, style);
        auto box = new (heap()) TextBox(nullptr, style);
        box->setText(text);
        container->addChild(box);
        return container;
    }

    auto box = new (heap()) ImageBox(this, style);
    box->setImage(std::move(image));
    return box;
}

HTMLBRElement::HTMLBRElement(Document* document)
    : HTMLElement(document, brTag)
{
}

Box* HTMLBRElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) LineBreakBox(this, style);
}

HTMLWBRElement::HTMLWBRElement(Document* document)
    : HTMLElement(document, wbrTag)
{
}

Box* HTMLWBRElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) WordBreakBox(this, style);
}

HTMLLIElement::HTMLLIElement(Document* document)
    : HTMLElement(document, liTag)
{
}

std::optional<int> HTMLLIElement::value() const
{
    int value;
    if(!parseHTMLInteger(value, getAttribute(valueAttr)))
        return std::nullopt;
    return value;
}

HTMLOLElement::HTMLOLElement(Document* document)
    : HTMLElement(document, olTag)
{
}

int HTMLOLElement::start() const
{
    int value;
    if(!parseHTMLInteger(value, getAttribute(startAttr)))
        return 1;
    return value;
}

HTMLTableElement::HTMLTableElement(Document* document)
    : HTMLElement(document, tableTag)
{
}

void HTMLTableElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        addHTMLLengthAttributeStyle(output, "width", value);
    } else if(name == heightAttr) {
        addHTMLLengthAttributeStyle(output, "height", value);
    } else if(name == valignAttr) {
        addHTMLAttributeStyle(output, "vertical-align", value);
    } else if(name == alignAttr) {
        addHTMLAttributeStyle(output, "text-align", value);
    } else if(name == cellspacingAttr) {
        addHTMLLengthAttributeStyle(output, "border-spacing", value);
    } else if(name == borderAttr) {
        addHTMLLengthAttributeStyle(output, "border-width", value);
    } else if(name == bordercolorAttr) {
        addHTMLAttributeStyle(output, "border-color", value);
    } else if(name == bgcolorAttr) {
        addHTMLAttributeStyle(output, "background-color", value);
    } else if(name == backgroundAttr) {
        addHTMLUrlAttributeStyle(output, "background-image", value);
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLTablePartElement::HTMLTablePartElement(Document* document, const GlobalString& tagName)
    : HTMLElement(document, tagName)
{
}

void HTMLTablePartElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == heightAttr) {
        addHTMLLengthAttributeStyle(output, "height", value);
    } else if(name == valignAttr) {
        addHTMLAttributeStyle(output, "vertical-align", value);
    } else if(name == alignAttr) {
        addHTMLAttributeStyle(output, "text-align", value);
    } else if(name == bgcolorAttr) {
        addHTMLAttributeStyle(output, "background-color", value);
    } else if(name == backgroundAttr) {
        addHTMLUrlAttributeStyle(output, "background-image", value);
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLTableSectionElement::HTMLTableSectionElement(Document* document, const GlobalString& tagName)
    : HTMLTablePartElement(document, tagName)
{
}

HTMLTableRowElement::HTMLTableRowElement(Document* document)
    : HTMLTablePartElement(document, trTag)
{
}

HTMLTableColElement::HTMLTableColElement(Document* document, const GlobalString& tagName)
    : HTMLTablePartElement(document, tagName)
{
}

unsigned HTMLTableColElement::span() const
{
    unsigned value;
    if(!parseHTMLInteger(value, getAttribute(spanAttr)))
        return 1;
    return std::max(1u, value);
}

Box* HTMLTableColElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto box = HTMLElement::createBox(style);
    if(auto column = to<TableColumnBox>(box))
        column->setSpan(span());
    return box;
}

HTMLTableCellElement::HTMLTableCellElement(Document* document, const GlobalString& tagName)
    : HTMLTablePartElement(document, tagName)
{
}

unsigned HTMLTableCellElement::colSpan() const
{
    unsigned value;
    if(!parseHTMLInteger(value, getAttribute(colspanAttr)))
        return 1;
    return std::max(1u, value);
}

unsigned HTMLTableCellElement::rowSpan() const
{
    unsigned value;
    if(!parseHTMLInteger(value, getAttribute(rowspanAttr)))
        return 1;
    return value;
}

Box* HTMLTableCellElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto box = HTMLElement::createBox(style);
    if(auto cell = to<TableCellBox>(box)) {
        cell->setColSpan(colSpan());
        cell->setRowSpan(rowSpan());
    }

    return box;
}

HTMLInputElement::HTMLInputElement(Document* document)
    : HTMLElement(document, inputTag)
{
}

unsigned HTMLInputElement::size() const
{
    unsigned value;
    if(!parseHTMLInteger(value, getAttribute(sizeAttr)))
        return 20;
    return std::max(1u, value);
}

Box* HTMLInputElement::createBox(const RefPtr<BoxStyle>& style)
{
    const auto& type = getAttribute(typeAttr);
    if(!equals(type, "text", false)
        && !equals(type, "search", false)
        && !equals(type, "url", false)
        && !equals(type, "tel", false)
        && !equals(type, "email", false)
        && !equals(type, "password", false)) {
        return HTMLElement::createBox(style);
    }

    auto box = new (heap()) TextInputBox(this, style);
    box->setCols(size());
    return box;
}

HTMLTextAreaElement::HTMLTextAreaElement(Document* document)
    : HTMLElement(document, textareaTag)
{
}

unsigned HTMLTextAreaElement::rows() const
{
    unsigned value;
    if(!parseHTMLInteger(value, getAttribute(rowsAttr)))
        return 2;
    return std::max(1u, value);
}

unsigned HTMLTextAreaElement::cols() const
{
    unsigned value;
    if(!parseHTMLInteger(value, getAttribute(colsAttr)))
        return 20;
    return std::max(1u, value);
}

Box* HTMLTextAreaElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto box = new (heap()) TextInputBox(this, style);
    box->setRows(rows());
    box->setCols(cols());
    return box;
}

HTMLSelectElement::HTMLSelectElement(Document* document)
    : HTMLElement(document, selectTag)
{
}

unsigned HTMLSelectElement::size() const
{
    unsigned value;
    if(!parseHTMLInteger(value, getAttribute(sizeAttr)))
        return hasAttribute(multipleAttr) ? 4 : 1;
    return std::max(1u, value);
}

Box* HTMLSelectElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SelectBox(this, style);
}

HTMLStyleElement::HTMLStyleElement(Document* document)
    : HTMLElement(document, styleTag)
{
}

const HeapString& HTMLStyleElement::type() const
{
    return getAttribute(typeAttr);
}

const HeapString& HTMLStyleElement::media() const
{
    return getAttribute(mediaAttr);
}

void HTMLStyleElement::finishParsingDocument()
{
    if(document()->supportsMedia(type(), media()))
        document()->addAuthorStyleSheet(textFromChildren(), document()->baseUrl());
    HTMLElement::finishParsingDocument();
}

HTMLLinkElement::HTMLLinkElement(Document* document)
    : HTMLElement(document, linkTag)
{
}

const HeapString& HTMLLinkElement::rel() const
{
    return getAttribute(relAttr);
}

const HeapString& HTMLLinkElement::type() const
{
    return getAttribute(typeAttr);
}

const HeapString& HTMLLinkElement::media() const
{
    return getAttribute(mediaAttr);
}

const HeapString& HTMLLinkElement::href() const
{
    return getAttribute(hrefAttr);
}

void HTMLLinkElement::finishParsingDocument()
{
    if(equals(rel(), "stylesheet", false) && document()->supportsMedia(type(), media())) {
        auto url = document()->completeUrl(href());
        if(auto resource = document()->fetchTextResource(url)) {
            document()->addAuthorStyleSheet(resource->text(), std::move(url));
        }
    }

    HTMLElement::finishParsingDocument();
}

HTMLTitleElement::HTMLTitleElement(Document* document)
    : HTMLElement(document, titleTag)
{
}

void HTMLTitleElement::finishParsingDocument()
{
    auto book = document()->book();
    if(book && book->title().empty())
        book->setTitle(textFromChildren());
    HTMLElement::finishParsingDocument();
}

std::unique_ptr<HTMLDocument> HTMLDocument::create(Book* book, Heap* heap, ResourceFetcher* fetcher, Url url)
{
    return std::unique_ptr<HTMLDocument>(new (heap) HTMLDocument(book, heap, fetcher, std::move(url)));
}

bool HTMLDocument::load(const std::string_view& content)
{
    HTMLParser parser(this, content);
    parser.parse();
    return true;
}

HTMLDocument::HTMLDocument(Book* book, Heap* heap, ResourceFetcher* fetcher, Url url)
    : Document(book, heap, fetcher, std::move(url))
{
}

} // namespace plutobook
