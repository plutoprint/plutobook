#include "contentbox.h"
#include "replacedbox.h"
#include "imageresource.h"
#include "document.h"
#include "cssrule.h"
#include "counters.h"

namespace plutobook {

ContentBox::ContentBox(const RefPtr<BoxStyle>& style)
    : TextBox(nullptr, style)
{
}

LeaderBox::LeaderBox(const RefPtr<BoxStyle>& style)
    : ContentBox(style)
{
}

TargetCounterBox::TargetCounterBox(const RefPtr<BoxStyle>& style)
    : ContentBox(style)
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

PagesCounterBox::PagesCounterBox(const RefPtr<BoxStyle>& style)
    : ContentBox(style)
{
}

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
    auto box = new (m_element->heap()) LeaderBox(m_parentStyle);
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
    assert(function->id() == CSSValueID::TargetCounter || function->id() == CSSValueID::TargetCounters);
    auto style = BoxStyle::create(*m_parentStyle, Display::Inline);
    auto box = new (m_element->heap()) TargetCounterBox(style);

    size_t index = 0;

    if(auto attr = to<CSSUnaryFunctionValue>(function->at(index))) {
        assert(attr->id() == CSSValueID::Attr);
        box->setFragment(m_element->getAttribute(to<CSSCustomIdentValue>(*attr->value()).value()));
    } else {
        box->setFragment(to<CSSLocalUrlValue>(*function->at(index)).value());
    }

    ++index;

    box->setIdentifier(to<CSSCustomIdentValue>(*function->at(index++)).value());
    if(function->id() == CSSValueID::TargetCounters)
        box->setSeperator(to<CSSStringValue>(*function->at(index++)).value());
    if(index < function->size()) {
        box->setListStyle(to<CSSCustomIdentValue>(*function->at(index++)).value());
        assert(index == function->size());
    }

    m_parentBox->addChild(box);
    m_lastTextBox = nullptr;
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
        } else if(auto function = to<CSSFunctionValue>(value)) {
            addTargetCounter(function);
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

} // namespace plutobook
