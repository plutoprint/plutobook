#include "contentbox.h"
#include "replacedbox.h"
#include "imageresource.h"
#include "htmldocument.h"
#include "cssrule.h"
#include "counters.h"
#include "qrcodegen.h"

#include <sstream>

namespace plutobook {

ContentBox::ContentBox(const RefPtr<BoxStyle>& style)
    : TextBox(nullptr, style)
{
}

LeaderBox::LeaderBox(const RefPtr<BoxStyle>& style)
    : ContentBox(style)
{
}

TargetCounterBox::TargetCounterBox(const RefPtr<BoxStyle>& style, const HeapString& fragment, const GlobalString& identifier, const HeapString& seperator, const GlobalString& listStyle)
    : ContentBox(style)
    , m_fragment(fragment)
    , m_identifier(identifier)
    , m_seperator(seperator)
    , m_listStyle(listStyle)
{
}

void TargetCounterBox::build()
{
    setText(document()->getTargetCounterText(m_fragment, m_identifier, m_listStyle, m_seperator));
}

PageCounterBox::PageCounterBox(const RefPtr<BoxStyle>& style)
    : ContentBox(style)
{
}

uint32_t PageCounterBox::pageNumber() const
{
    return 0;
}

PagesCounterBox::PagesCounterBox(const RefPtr<BoxStyle>& style)
    : PageCounterBox(style)
{
}

uint32_t PagesCounterBox::pageNumber() const
{
    return document()->pageCount();
}

TargetPageCounterBox::TargetPageCounterBox(const RefPtr<BoxStyle>& style, const Element* element)
    : PageCounterBox(style)
    , m_element(element)
{
}

uint32_t TargetPageCounterBox::pageNumber() const
{
    return 0;
}

ContentBoxBuilder::ContentBoxBuilder(Counters& counters, Element* element, Box* parent)
    : m_counters(counters)
    , m_element(element)
    , m_parentBox(parent)
    , m_parentStyle(parent->style())
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

    auto newBox = new (m_parentStyle->heap()) TextBox(nullptr, m_parentStyle);
    newBox->setText(text);
    m_parentBox->addChild(newBox);
    m_lastTextBox = newBox;
}

void ContentBoxBuilder::addLeaderText(const HeapString& text)
{
    if(text.empty())
        return;
    auto newBox = new (m_parentStyle->heap()) LeaderBox(m_parentStyle);
    newBox->setText(text);
    m_parentBox->addChild(newBox);
    m_lastTextBox = nullptr;
}

void ContentBoxBuilder::addLeader(const CSSValue& value)
{
    static const GlobalString dotted(".");
    static const GlobalString solid("_");
    static const GlobalString space(" ");
    if(is<CSSStringValue>(value)) {
        addLeaderText(to<CSSStringValue>(value).value());
        return;
    }

    const auto& ident = to<CSSIdentValue>(value);
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

void ContentBoxBuilder::addElement(const CSSValue& value)
{
    if(!m_parentBox->isPageMarginBox())
        return;
    const auto& name = to<CSSCustomIdentValue>(value).value();
    auto style = m_parentStyle->document()->getRunningStyle(name);
    if(style == nullptr)
        return;
    auto& element = to<HTMLElement>(*style->node());
    auto newBox = element.createBox(style);
    if(newBox == nullptr)
        return;
    m_parentBox->addChild(newBox);
    element.buildElementBox(m_counters, newBox);
    m_lastTextBox = nullptr;
}

void ContentBoxBuilder::addCounter(const CSSCounterValue& counter)
{
    addText(m_counters.counterText(counter.identifier(), counter.listStyle(), counter.separator()));
}

void ContentBoxBuilder::addTargetCounter(const CSSFunctionValue& function)
{
    HeapString fragment;
    GlobalString identifier;
    HeapString seperator;
    GlobalString listStyle;

    size_t index = 0;

    if(auto value = to<CSSLocalUrlValue>(function.at(index))) {
        fragment = to<CSSLocalUrlValue>(*value).value();
    } else {
        fragment = resolveAttr(to<CSSAttrValue>(*function.at(index)));
    }

    ++index;

    identifier = to<CSSCustomIdentValue>(*function.at(index++)).value();
    if(function.id() == CSSFunctionID::TargetCounters)
        seperator = to<CSSStringValue>(*function.at(index++)).value();
    if(index < function.size()) {
        listStyle = to<CSSCustomIdentValue>(*function.at(index++)).value();
    }

    assert(index == function.size());

    if(m_parentBox->isPageMarginBox()) {
        addText(m_parentStyle->document()->getTargetCounterText(fragment, identifier, listStyle, seperator));
        return;
    }

    auto newStyle = BoxStyle::create(m_parentStyle, Display::Inline);
    auto newBox = new (m_parentStyle->heap()) TargetCounterBox(newStyle, fragment, identifier, seperator, listStyle);
    m_parentBox->addChild(newBox);
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

void ContentBoxBuilder::addQrCode(const CSSFunctionValue& function)
{
    std::string text(to<CSSStringValue>(*function.at(0)).value());

    char fill[16] = "black";
    if(function.size() == 2) {
        const auto& color = to<CSSColorValue>(*function.at(1)).value();
        if(color.alpha() == 255) {
            std::snprintf(fill, sizeof(fill), "#%02X%02X%02X", color.red(), color.green(), color.blue());
        } else {
            std::snprintf(fill, sizeof(fill), "#%02X%02X%02X%02X", color.red(), color.green(), color.blue(), color.alpha());
        }
    }

    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

    if(qrcodegen_encodeText(text.data(), tempBuffer, qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true)) {
        auto size = qrcodegen_getSize(qrcode);

        std::ostringstream ss;
        ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 " << size << ' ' << size << "\">";
        ss << "<path d=\"";
        for(int y = 0; y < size; y++) {
            for(int x = 0; x < size; x++) {
                if(qrcodegen_getModule(qrcode, x, y)) {
                    ss << 'M' << x << ',' << y << "h1v1h-1z";
                }
            }
        }

        ss << "\" fill=\"" << fill << "\"/>";
        ss << "</svg>";

        addImage(SVGImage::create(ss.view(), emptyGlo, nullptr));
    }
}

void ContentBoxBuilder::addImage(RefPtr<Image> image)
{
    if(image == nullptr)
        return;
    auto newStyle = BoxStyle::create(m_parentStyle, Display::Inline);
    auto newBox = new (m_parentStyle->heap()) ImageBox(nullptr, newStyle);
    newBox->setImage(std::move(image));
    m_parentBox->addChild(newBox);
    m_lastTextBox = nullptr;
}

const HeapString& ContentBoxBuilder::resolveAttr(const CSSAttrValue& attr) const
{
    if(m_element == nullptr)
        return emptyGlo;
    auto attribute = m_element->findAttributePossiblyIgnoringCase(attr.name());
    if(attribute == nullptr)
        return attr.fallback();
    return attribute->value();
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

        const auto& listStyle = to<CSSCustomIdentValue>(*listStyleType);
        addText(m_counters.markerText(listStyle.value()));
        return;
    }

    for(const auto& value : to<CSSListValue>(*content)) {
        if(auto string = to<CSSStringValue>(value)) {
            addText(string->value());
        } else if(auto image = to<CSSImageValue>(value)) {
            addImage(image->fetch(m_parentStyle->document()));
        } else if(auto counter = to<CSSCounterValue>(value)) {
            addCounter(*counter);
        } else if(auto ident = to<CSSIdentValue>(value)) {
            addQuote(ident->value());
        } else if(auto attr = to<CSSAttrValue>(value)) {
            addText(resolveAttr(*attr));
        } else {
            if(is<CSSFunctionValue>(value)) {
                const auto& function = to<CSSFunctionValue>(*value);
                if(function.id() == CSSFunctionID::TargetCounter
                    || function.id() == CSSFunctionID::TargetCounters) {
                    addTargetCounter(function);
                } else {
                    assert(function.id() == CSSFunctionID::Qrcode);
                    addQrCode(function);
                }
            } else {
                const auto& function = to<CSSUnaryFunctionValue>(*value);
                if(function.id() == CSSFunctionID::Leader) {
                    addLeader(*function.value());
                } else {
                    assert(function.id() == CSSFunctionID::Element);
                    addElement(*function.value());
                }
            }
        }
    }
}

} // namespace plutobook
