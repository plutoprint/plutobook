/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "contentbox.h"
#include "replacedbox.h"
#include "imageresource.h"
#include "htmldocument.h"
#include "cssrule.h"
#include "counters.h"
#include "qrcodegen.h"

#include <sstream>
#include <cstdio>

namespace plutobook {

ContentBox::ContentBox(const RefPtr<BoxStyle>& style)
    : TextBox(nullptr, style)
{
}

LeaderBox::LeaderBox(const RefPtr<BoxStyle>& style)
    : ContentBox(style)
{
}

ContentBoxBuilder::ContentBoxBuilder(Counters& counters, Element* element, Box* box)
    : m_counters(counters)
    , m_element(element)
    , m_box(box)
    , m_style(box->style())
{
}

void ContentBoxBuilder::build(const CSSValue& content)
{
    if(content.id() == CSSValueID::None)
        return;
    if(content.id() == CSSValueID::Normal) {
        if(m_style->pseudoType() == PseudoType::Marker)
            addDefaultListMarker();
        return;
    }

    for(const auto& value : to<CSSListValue>(content)) {
        addValue(*value);
    }
}

void ContentBoxBuilder::addValue(const CSSValue& value)
{
    switch(value.type()) {
    case CSSValueType::String:
        addText(to<CSSStringValue>(value).value());
        break;
    case CSSValueType::Image:
        addImage(to<CSSImageValue>(value).fetch(m_style->document()));
        break;
    case CSSValueType::Counter:
        addCounter(to<CSSCounterValue>(value));
        break;
    case CSSValueType::Ident:
        addQuote(to<CSSIdentValue>(value).value());
        break;
    case CSSValueType::Attr:
        addText(resolveAttr(to<CSSAttrValue>(value)));
        break;
    case CSSValueType::Function:
        addQrCode(to<CSSFunctionValue>(value));
        break;
    case CSSValueType::UnaryFunction:
        addFunction(to<CSSUnaryFunctionValue>(value));
        break;
    default:
        assert(false);
    }
}

void ContentBoxBuilder::addText(const HeapString& text)
{
    if(text.empty())
        return;
    if(m_lastTextBox) {
        m_lastTextBox->appendText(text);
        return;
    }

    auto newBox = new (m_style->heap()) TextBox(nullptr, m_style);
    newBox->setText(text);
    m_box->addChild(newBox);
    m_lastTextBox = newBox;
}

void ContentBoxBuilder::addLeaderText(const HeapString& text)
{
    if(text.empty())
        return;
    auto newBox = new (m_style->heap()) LeaderBox(m_style);
    newBox->setText(text);
    m_box->addChild(newBox);
    m_lastTextBox = nullptr;
}

void ContentBoxBuilder::addLeader(const CSSValue& value)
{
    if(is<CSSStringValue>(value)) {
        addLeaderText(to<CSSStringValue>(value).value());
        return;
    }

    static const GlobalString dotted(".");
    static const GlobalString solid("_");
    static const GlobalString space(" ");

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
    if(!m_box->isPageMarginBox())
        return;
    const auto& name = to<CSSCustomIdentValue>(value);
    auto style = m_style->document()->getRunningStyle(name.value());
    if(style == nullptr)
        return;
    auto element = to<HTMLElement>(style->node());
    auto newBox = element->createBox(style);
    if(newBox == nullptr)
        return;
    m_box->addChild(newBox);

    SelectorFilter selectorFilter;
    element->buildElementBox(m_counters, selectorFilter, newBox);
    newBox->setIsRunning(true);
    m_lastTextBox = nullptr;
}

void ContentBoxBuilder::addFunction(const CSSUnaryFunctionValue& function)
{
    switch(function.id()) {
    case CSSFunctionID::Leader:
        addLeader(*function.value());
        break;
    case CSSFunctionID::Element:
        addElement(*function.value());
        break;
    default:
        assert(false);
    }
}

void ContentBoxBuilder::addCounter(const CSSCounterValue& counter)
{
    addText(m_counters.counterText(counter.identifier(), counter.listStyle(), counter.separator()));
}

void ContentBoxBuilder::addQuote(CSSValueID value)
{
    switch(value) {
    case CSSValueID::OpenQuote:
        addText(m_style->getQuote(true, m_counters.quoteDepth()));
        m_counters.increaseQuoteDepth();
        break;
    case CSSValueID::CloseQuote:
        if(m_counters.quoteDepth())
            m_counters.decreaseQuoteDepth();
        addText(m_style->getQuote(false, m_counters.quoteDepth()));
        break;
    case CSSValueID::NoOpenQuote:
        m_counters.increaseQuoteDepth();
        break;
    case CSSValueID::NoCloseQuote:
        if(m_counters.quoteDepth())
            m_counters.decreaseQuoteDepth();
        break;
    default:
        assert(false);
    }
}

void ContentBoxBuilder::addQrCode(const CSSFunctionValue& function)
{
    assert(function.id() == CSSFunctionID::Qrcode);
    std::string text(to<CSSStringValue>(*function.at(0)).value());

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

        char fill[16] = "black";
        if(function.size() == 2) {
            const auto color = m_style->convertColor(*function.at(1));
            if(color.alpha() == 255) {
                std::snprintf(fill, sizeof(fill), "#%02X%02X%02X", color.red(), color.green(), color.blue());
            } else {
                std::snprintf(fill, sizeof(fill), "#%02X%02X%02X%02X", color.red(), color.green(), color.blue(), color.alpha());
            }
        }

        ss << "\" fill=\"" << fill << "\"/>";
        ss << "</svg>";

        addImage(SVGImage::create(m_style->book(), ss.str(), emptyGlo));
    }
}

void ContentBoxBuilder::addImage(RefPtr<Image> image)
{
    if(image == nullptr)
        return;
    auto newStyle = BoxStyle::create(m_style, Display::Inline);
    auto newBox = new (m_style->heap()) ImageBox(nullptr, newStyle);
    newBox->setImage(std::move(image));
    m_box->addChild(newBox);
    m_lastTextBox = nullptr;
}

void ContentBoxBuilder::addDefaultListMarker()
{
    if(auto image = m_style->listStyleImage()) {
        addImage(std::move(image));
        return;
    }

    auto listStyleType = m_style->get(CSSPropertyID::ListStyleType);
    if(listStyleType == nullptr) {
        addText(markerText(CSSValueID::Disc));
        return;
    }

    if(auto ident = to<CSSIdentValue>(listStyleType)) {
        addText(markerText(ident->value()));
        return;
    }

    if(auto string = to<CSSStringValue>(listStyleType)) {
        addText(string->value());
        return;
    }

    const auto& listStyle = to<CSSCustomIdentValue>(*listStyleType);
    addText(m_counters.markerText(listStyle.value()));
}

const GlobalString& ContentBoxBuilder::markerText(CSSValueID listStyleType) const
{
    static const GlobalString disc("\u2022 ");
    static const GlobalString circle("\u25E6 ");
    static const GlobalString square("\u25AA ");

    static const GlobalString disclosureOpen("\u25BE ");
    static const GlobalString disclosureClosedLtr("\u25B8 ");
    static const GlobalString disclosureClosedRtl("\u25C2 ");

    switch(listStyleType) {
    case CSSValueID::None:
        return emptyGlo;
    case CSSValueID::Disc:
        return disc;
    case CSSValueID::Circle:
        return circle;
    case CSSValueID::Square:
        return square;
    case CSSValueID::DisclosureOpen:
        return disclosureOpen;
    case CSSValueID::DisclosureClosed:
        return m_style->direction() == Direction::Rtl ? disclosureClosedRtl : disclosureClosedLtr;
    default:
        assert(false);
    }

    return emptyGlo;
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

} // namespace plutobook
