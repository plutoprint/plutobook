/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "cssstylesheet.h"
#include "uastylesheet.h"
#include "cssparser.h"
#include "textresource.h"
#include "fontresource.h"
#include "document.h"
#include "boxstyle.h"

#include "plutobook.hpp"

namespace plutobook {

CSSFontFaceCache::CSSFontFaceCache(Heap* heap)
    : m_table(heap)
{
}

RefPtr<FontData> CSSFontFaceCache::get(const GlobalString& family, const FontDataDescription& description) const
{
    auto it = m_table.find(family);
    if(it == m_table.end())
        return fontDataCache()->getFontData(family, description);
    FontSelectionAlgorithm algorithm(description.request);
    for(const auto& item : it->second) {
        algorithm.addCandidate(item.first);
    }

    RefPtr<SegmentedFontFace> face;
    for(const auto& item : it->second) {
        if(face == nullptr || algorithm.isCandidateBetter(item.first, face->description())) {
            face = item.second;
        }
    }

    return face->getFontData(description);
}

void CSSFontFaceCache::add(const GlobalString& family, const FontSelectionDescription& description, RefPtr<FontFace> face)
{
    auto& fontFace = m_table[family][description];
    if(fontFace == nullptr)
        fontFace = SegmentedFontFace::create(description);
    fontFace->add(std::move(face));
}

class CSSPropertyData {
public:
    CSSPropertyData(uint32_t specificity, uint32_t position, const CSSProperty& property)
        : m_id(property.id()), m_origin(property.origin()), m_important(property.important())
        , m_specificity(specificity), m_position(position), m_value(property.value())
    {}

    CSSPropertyID id() const { return m_id; }
    CSSStyleOrigin origin() const { return m_origin; }
    bool important() const { return m_important; }
    uint32_t specificity() const { return m_specificity; }
    uint32_t position() const { return m_position; }
    const RefPtr<CSSValue>& value() const { return m_value; }

    bool isLessThan(const CSSPropertyData& data) const;

private:
    CSSPropertyID m_id;
    CSSStyleOrigin m_origin;
    bool m_important;
    uint32_t m_specificity;
    uint32_t m_position;
    RefPtr<CSSValue> m_value;
};

inline bool CSSPropertyData::isLessThan(const CSSPropertyData& data) const
{
    if(m_important != data.important())
        return m_important < data.important();
    if(m_origin != data.origin())
        return m_origin < data.origin();
    if(m_specificity != data.specificity())
        return m_specificity < data.specificity();
    return m_position < data.position();
}

using CSSPropertyDataList = std::vector<CSSPropertyData>;

class FontDescriptionBuilder {
public:
    FontDescriptionBuilder(const BoxStyle* parentStyle, const CSSPropertyDataList& properties);

    FontFamilyList family() const;
    FontSelectionValue size() const;
    FontSelectionValue weight() const;
    FontSelectionValue stretch() const;
    FontSelectionValue style() const;
    FontVariationList variationSettings() const;

    FontDescription build() const;

private:
    const BoxStyle* m_parentStyle;
    RefPtr<CSSValue> m_family;
    RefPtr<CSSValue> m_size;
    RefPtr<CSSValue> m_weight;
    RefPtr<CSSValue> m_stretch;
    RefPtr<CSSValue> m_style;
    RefPtr<CSSValue> m_variationSettings;
};

FontDescriptionBuilder::FontDescriptionBuilder(const BoxStyle* parentStyle, const CSSPropertyDataList& properties)
    : m_parentStyle(parentStyle)
{
    for(const auto& property : properties) {
        if(is<CSSInheritValue>(*property.value())
            || is<CSSUnsetValue>(*property.value())
            || is<CSSVariableReferenceValue>(*property.value())) {
            continue;
        }

        switch(property.id()) {
        case CSSPropertyID::FontFamily:
            m_family = property.value();
            break;
        case CSSPropertyID::FontSize:
            m_size = property.value();
            break;
        case CSSPropertyID::FontWeight:
            m_weight = property.value();
            break;
        case CSSPropertyID::FontStretch:
            m_stretch = property.value();
            break;
        case CSSPropertyID::FontStyle:
            m_style = property.value();
            break;
        case CSSPropertyID::FontVariationSettings:
            m_variationSettings = property.value();
            break;
        default:
            break;
        }
    }
}

FontFamilyList FontDescriptionBuilder::family() const
{
    if(m_family == nullptr)
        return m_parentStyle->fontFamily();
    if(is<CSSInitialValue>(*m_family))
        return FontFamilyList();
    FontFamilyList families;
    for(const auto& family : to<CSSListValue>(*m_family)) {
        const auto& name = to<CSSCustomIdentValue>(*family);
        families.push_front(name.value());
    }

    families.reverse();
    return families;
}

FontSelectionValue FontDescriptionBuilder::size() const
{
    if(m_size == nullptr)
        return m_parentStyle->fontSize();
    if(is<CSSInitialValue>(*m_size))
        return kMediumFontSize;
    if(auto ident = to<CSSIdentValue>(m_size)) {
        switch(ident->value()) {
        case CSSValueID::XxSmall:
            return kMediumFontSize * 0.6;
        case CSSValueID::XSmall:
            return kMediumFontSize * 0.75;
        case CSSValueID::Small:
            return kMediumFontSize * 0.89;
        case CSSValueID::Medium:
            return kMediumFontSize * 1.0;
        case CSSValueID::Large:
            return kMediumFontSize * 1.2;
        case CSSValueID::XLarge:
            return kMediumFontSize * 1.5;
        case CSSValueID::XxLarge:
            return kMediumFontSize * 2.0;
        case CSSValueID::XxxLarge:
            return kMediumFontSize * 3.0;
        case CSSValueID::Smaller:
            return m_parentStyle->fontSize() / 1.2;
        case CSSValueID::Larger:
            return m_parentStyle->fontSize() * 1.2;
        default:
            assert(false);
        }
    }

    if(auto percent = to<CSSPercentValue>(m_size))
        return percent->value() * m_parentStyle->fontSize() / 100.0;
    return m_parentStyle->convertLengthValue(*m_size);
}

constexpr FontSelectionValue lighterFontWeight(FontSelectionValue weight)
{
    assert(weight >= kMinFontWeight && weight <= kMaxFontWeight);
    if(weight < FontSelectionValue(100))
        return weight;
    if(weight < FontSelectionValue(550))
        return FontSelectionValue(100);
    if(weight < FontSelectionValue(750))
        return FontSelectionValue(400);
    return FontSelectionValue(700);
}

constexpr FontSelectionValue bolderFontWeight(FontSelectionValue weight)
{
    assert(weight >= kMinFontWeight && weight <= kMaxFontWeight);
    if(weight < FontSelectionValue(350))
        return FontSelectionValue(400);
    if(weight < FontSelectionValue(550))
        return FontSelectionValue(700);
    if(weight < FontSelectionValue(900))
        return FontSelectionValue(900);
    return weight;
}

static FontSelectionValue convertFontWeightNumber(const CSSValue& value)
{
    return std::clamp<FontSelectionValue>(to<CSSNumberValue>(value).value(), kMinFontWeight, kMaxFontWeight);
}

FontSelectionValue FontDescriptionBuilder::weight() const
{
    if(m_weight == nullptr)
        return m_parentStyle->fontWeight();
    if(is<CSSInitialValue>(*m_weight))
        return kNormalFontWeight;
    if(auto ident = to<CSSIdentValue>(m_weight)) {
        switch(ident->value()) {
        case CSSValueID::Normal:
            return kNormalFontWeight;
        case CSSValueID::Bold:
            return kBoldFontWeight;
        case CSSValueID::Lighter:
            return lighterFontWeight(m_parentStyle->fontWeight());
        case CSSValueID::Bolder:
            return bolderFontWeight(m_parentStyle->fontWeight());
        default:
            assert(false);
        }
    }

    return convertFontWeightNumber(*m_weight);
}

static FontSelectionValue convertFontStretchIdent(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::UltraCondensed:
        return kUltraCondensedFontWidth;
    case CSSValueID::ExtraCondensed:
        return kExtraCondensedFontWidth;
    case CSSValueID::Condensed:
        return kCondensedFontWidth;
    case CSSValueID::SemiCondensed:
        return kSemiCondensedFontWidth;
    case CSSValueID::Normal:
        return kNormalFontWidth;
    case CSSValueID::SemiExpanded:
        return kSemiExpandedFontWidth;
    case CSSValueID::Expanded:
        return kExpandedFontWidth;
    case CSSValueID::ExtraExpanded:
        return kExtraExpandedFontWidth;
    case CSSValueID::UltraExpanded:
        return kUltraExpandedFontWidth;
    default:
        assert(false);
    }

    return kNormalFontWidth;
}

FontSelectionValue FontDescriptionBuilder::stretch() const
{
    if(m_stretch == nullptr)
        return m_parentStyle->fontStretch();
    if(is<CSSInitialValue>(*m_stretch))
        return kNormalFontWidth;
    if(auto percent = to<CSSPercentValue>(m_stretch))
        return percent->value();
    return convertFontStretchIdent(*m_stretch);
}

static FontSelectionValue convertFontStyleIdent(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Normal:
        return kNormalFontSlope;
    case CSSValueID::Italic:
        return kItalicFontSlope;
    case CSSValueID::Oblique:
        return kObliqueFontSlope;
    default:
        assert(false);
    }

    return kNormalFontSlope;
}

static FontSelectionValue convertFontStyleAngle(const CSSValue& value)
{
    return std::clamp<FontSelectionValue>(to<CSSAngleValue>(value).valueInDegrees(), kMinFontSlope, kMaxFontSlope);
}

FontSelectionValue FontDescriptionBuilder::style() const
{
    if(m_style == nullptr)
        return m_parentStyle->fontStyle();
    if(is<CSSInitialValue>(*m_style))
        return kNormalFontSlope;
    if(auto ident = to<CSSIdentValue>(m_style))
        return convertFontStyleIdent(*ident);
    const auto& pair = to<CSSPairValue>(*m_style);
    const auto& ident = to<CSSIdentValue>(*pair.first());
    assert(ident.value() == CSSValueID::Oblique);
    return convertFontStyleAngle(*pair.second());
}

FontVariationList FontDescriptionBuilder::variationSettings() const
{
    if(m_variationSettings == nullptr)
        return m_parentStyle->fontVariationSettings();
    if(is<CSSInitialValue>(*m_variationSettings))
        return FontVariationList();
    FontVariationList variationSettings;
    if(auto ident = to<CSSIdentValue>(m_variationSettings)) {
        assert(ident->value() == CSSValueID::Normal);
        return variationSettings;
    }

    for(const auto& value : to<CSSListValue>(*m_variationSettings)) {
        const auto& variation = to<CSSFontVariationValue>(*value);
        variationSettings.emplace_front(variation.tag(), variation.value());
    }

    variationSettings.sort();
    variationSettings.unique();
    return variationSettings;
}

FontDescription FontDescriptionBuilder::build() const
{
    FontDescription description;
    description.families = family();
    description.data.size = size();
    description.data.request.weight = weight();
    description.data.request.width = stretch();
    description.data.request.slope = style();
    description.data.variations = variationSettings();
    return description;
}

class StyleBuilder {
protected:
    StyleBuilder(const BoxStyle* parentStyle, PseudoType pseudoType)
        : m_parentStyle(parentStyle), m_pseudoType(pseudoType)
    {}

    FontDescription fontDescription() const;
    void merge(uint32_t specificity, uint32_t position, const CSSPropertyList& properties);
    void buildStyle(BoxStyle* newStyle);

    CSSPropertyDataList m_properties;
    const BoxStyle* m_parentStyle;
    PseudoType m_pseudoType;
};

FontDescription StyleBuilder::fontDescription() const
{
    return FontDescriptionBuilder(m_parentStyle, m_properties).build();
}

void StyleBuilder::merge(uint32_t specificity, uint32_t position, const CSSPropertyList& properties)
{
    for(const auto& property : properties) {
        CSSPropertyData data(specificity, position, property);
        auto predicate_func = [&property](const CSSPropertyData& item) {
            if(property.id() == CSSPropertyID::Custom && item.id() == CSSPropertyID::Custom) {
                const auto& a = to<CSSCustomPropertyValue>(*property.value());
                const auto& b = to<CSSCustomPropertyValue>(*item.value());
                return a.name() == b.name();
            }

            return property.id() == item.id();
        };

        auto it = std::find_if(m_properties.begin(), m_properties.end(), predicate_func);
        if(it == m_properties.end()) {
            m_properties.push_back(std::move(data));
        } else if(!data.isLessThan(*it)) {
            *it = std::move(data);
        }
    }
}

void StyleBuilder::buildStyle(BoxStyle* newStyle)
{
    CSSPropertyDataList variables;
    for(const auto& property : m_properties) {
        if(is<CSSVariableReferenceValue>(*property.value())) {
            variables.push_back(property);
        } else if(property.id() == CSSPropertyID::Custom) {
            const auto& custom = to<CSSCustomPropertyValue>(*property.value());
            newStyle->setCustom(custom.name(), custom.value());
        }
    }

    for(const auto& variable : variables) {
        const auto& value = to<CSSVariableReferenceValue>(*variable.value());
        merge(variable.specificity(), variable.position(), value.resolve(newStyle));
    }

    newStyle->setFontDescription(fontDescription());

    for(const auto& property : m_properties) {
        const auto id = property.id();
        switch(id) {
        case CSSPropertyID::Custom:
        case CSSPropertyID::FontFamily:
        case CSSPropertyID::FontSize:
        case CSSPropertyID::FontWeight:
        case CSSPropertyID::FontStretch:
        case CSSPropertyID::FontStyle:
        case CSSPropertyID::FontVariationSettings:
            continue;
        default:
            break;
        }

        auto value = property.value();
        if(is<CSSUnsetValue>(*value) || is<CSSVariableReferenceValue>(*value))
            continue;
        if(is<CSSInitialValue>(*value)) {
            newStyle->reset(id);
            continue;
        }

        if(is<CSSInheritValue>(*value) && !(value = m_parentStyle->get(id)))
            continue;
        if(is<CSSLengthValue>(*value) || is<CSSCalcValue>(*value))
            value = newStyle->resolveLength(value);
        newStyle->set(id, std::move(value));
    }
}

class ElementStyleBuilder final : public StyleBuilder {
public:
    ElementStyleBuilder(Element* element, PseudoType pseudoType, const BoxStyle* parentStyle);

    void add(const CSSRuleDataList* rules);
    RefPtr<BoxStyle> build();

private:
    Element* m_element;
};

ElementStyleBuilder::ElementStyleBuilder(Element* element, PseudoType pseudoType, const BoxStyle* parentStyle)
    : StyleBuilder(parentStyle, pseudoType)
    , m_element(element)
{
}

void ElementStyleBuilder::add(const CSSRuleDataList* rules)
{
    if(rules == nullptr)
        return;
    for(const auto& rule : *rules) {
        if(rule.match(m_element, m_pseudoType)) {
            merge(rule.specificity(), rule.position(), rule.properties());
        }
    }
}

RefPtr<BoxStyle> ElementStyleBuilder::build()
{
    if(m_pseudoType == PseudoType::None) {
        merge(0, 0, m_element->presentationAttributeStyle());
        merge(0, 0, m_element->inlineStyle());
    }

    if(m_properties.empty()) {
        if(m_pseudoType == PseudoType::None) {
            if(m_element->isRootNode() || m_parentStyle->isDisplayFlex())
                return BoxStyle::create(m_element, m_parentStyle, m_pseudoType, Display::Block);
            return BoxStyle::create(m_element, m_parentStyle, m_pseudoType, Display::Inline);
        }

        if(m_pseudoType == PseudoType::Marker)
            return BoxStyle::create(m_element, m_parentStyle, m_pseudoType, Display::Inline);
        return nullptr;
    }

    auto newStyle = BoxStyle::create(m_element, m_parentStyle, m_pseudoType, Display::Inline);
    buildStyle(newStyle.get());
    if(newStyle->display() == Display::None)
        return newStyle;
    if(newStyle->position() == Position::Static && !m_parentStyle->isDisplayFlex())
        newStyle->reset(CSSPropertyID::ZIndex);
    if(m_pseudoType == PseudoType::FirstLetter) {
        newStyle->setPosition(Position::Static);
        if(newStyle->isFloating()) {
            newStyle->setDisplay(Display::Block);
        } else {
            newStyle->setDisplay(Display::Inline);
        }
    }

    if(newStyle->isFloating() || newStyle->isPositioned() || m_element->isRootNode() || m_parentStyle->isDisplayFlex()) {
        switch(newStyle->display()) {
        case Display::Inline:
        case Display::InlineBlock:
            newStyle->setDisplay(Display::Block);
            break;
        case Display::InlineTable:
            newStyle->setDisplay(Display::Table);
            break;
        case Display::InlineFlex:
            newStyle->setDisplay(Display::Flex);
            break;
        case Display::TableCaption:
        case Display::TableCell:
        case Display::TableColumn:
        case Display::TableColumnGroup:
        case Display::TableFooterGroup:
        case Display::TableHeaderGroup:
        case Display::TableRow:
        case Display::TableRowGroup:
            newStyle->setDisplay(Display::Block);
            break;
        default:
            break;
        }
    }

    if(newStyle->isPositioned() || m_parentStyle->isDisplayFlex())
        newStyle->setFloating(Float::None);
    return newStyle;
}

class PageStyleBuilder final : public StyleBuilder {
public:
    PageStyleBuilder(const GlobalString& pageName, uint32_t pageIndex, PageMarginType marginType, PseudoType pseudoType, const BoxStyle* parentStyle);

    void add(const CSSPageRuleDataList& rules);
    RefPtr<BoxStyle> build();

private:
    GlobalString m_pageName;
    uint32_t m_pageIndex;
    PageMarginType m_marginType;
};

PageStyleBuilder::PageStyleBuilder(const GlobalString& pageName, uint32_t pageIndex, PageMarginType marginType, PseudoType pseudoType, const BoxStyle* parentStyle)
    : StyleBuilder(parentStyle, pseudoType)
    , m_pageName(pageName)
    , m_pageIndex(pageIndex)
    , m_marginType(marginType)
{
}

void PageStyleBuilder::add(const CSSPageRuleDataList& rules)
{
    for(const auto& rule : rules) {
        if(rule.match(m_pageName, m_pageIndex, m_pseudoType)) {
            if(m_marginType == PageMarginType::None) {
                merge(rule.specificity(), rule.position(), rule.properties());
            } else {
                for(const auto& margin : rule.margins()) {
                    if(m_marginType == margin->marginType()) {
                        merge(rule.specificity(), rule.position(), margin->properties());
                    }
                }
            }
        }
    }
}

RefPtr<BoxStyle> PageStyleBuilder::build()
{
    if(m_properties.empty()) {
        if(m_marginType == PageMarginType::None)
            return BoxStyle::create(m_parentStyle, m_pseudoType, Display::Block);
        return nullptr;
    }

    auto newStyle = BoxStyle::create(m_parentStyle, m_pseudoType, Display::Block);
    switch(m_marginType) {
    case PageMarginType::TopLeftCorner:
        newStyle->setTextAlign(TextAlign::Right);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::TopLeft:
        newStyle->setTextAlign(TextAlign::Left);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::TopCenter:
        newStyle->setTextAlign(TextAlign::Center);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::TopRight:
        newStyle->setTextAlign(TextAlign::Right);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::TopRightCorner:
        newStyle->setTextAlign(TextAlign::Left);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::RightTop:
        newStyle->setTextAlign(TextAlign::Center);
        newStyle->setVerticalAlignType(VerticalAlignType::Top);
        break;
    case PageMarginType::RightMiddle:
        newStyle->setTextAlign(TextAlign::Center);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::RightBottom:
        newStyle->setTextAlign(TextAlign::Center);
        newStyle->setVerticalAlignType(VerticalAlignType::Bottom);
        break;
    case PageMarginType::BottomRightCorner:
        newStyle->setTextAlign(TextAlign::Left);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::BottomRight:
        newStyle->setTextAlign(TextAlign::Right);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::BottomCenter:
        newStyle->setTextAlign(TextAlign::Center);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::BottomLeft:
        newStyle->setTextAlign(TextAlign::Left);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::BottomLeftCorner:
        newStyle->setTextAlign(TextAlign::Right);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::LeftBottom:
        newStyle->setTextAlign(TextAlign::Center);
        newStyle->setVerticalAlignType(VerticalAlignType::Bottom);
        break;
    case PageMarginType::LeftMiddle:
        newStyle->setTextAlign(TextAlign::Center);
        newStyle->setVerticalAlignType(VerticalAlignType::Middle);
        break;
    case PageMarginType::LeftTop:
        newStyle->setTextAlign(TextAlign::Center);
        newStyle->setVerticalAlignType(VerticalAlignType::Top);
        break;
    case PageMarginType::None:
        break;
    }

    buildStyle(newStyle.get());
    newStyle->setPosition(Position::Static);
    newStyle->setDisplay(Display::Block);
    newStyle->setFloating(Float::None);
    return newStyle;
}

static const CSSRuleList& userAgentRules()
{
    static CSSRuleList rules = []() {
        static Heap heap(1024 * 96);
        CSSParserContext context(nullptr, CSSStyleOrigin::UserAgent, ResourceLoader::baseUrl());
        CSSParser parser(context, &heap);
        return parser.parseSheet(kUserAgentStyle);
    }();

    return rules;
}

CSSStyleSheet::CSSStyleSheet(Document* document)
    : m_document(document)
    , m_idRules(document->heap())
    , m_classRules(document->heap())
    , m_tagRules(document->heap())
    , m_attributeRules(document->heap())
    , m_pseudoRules(document->heap())
    , m_universalRules(document->heap())
    , m_pageRules(document->heap())
    , m_counterStyleRules(document->heap())
    , m_fontFaceCache(document->heap())
{
    if(document->book()) {
        addRuleList(userAgentRules());
    }
}

RefPtr<BoxStyle> CSSStyleSheet::styleForElement(Element* element, const BoxStyle* parentStyle) const
{
    ElementStyleBuilder builder(element, PseudoType::None, parentStyle);
    for(const auto& className : element->classNames())
        builder.add(m_classRules.get(className));
    for(const auto& attribute : element->attributes())
        builder.add(m_attributeRules.get(element->foldCase(attribute.name())));
    builder.add(m_tagRules.get(element->foldTagNameCase()));
    builder.add(m_idRules.get(element->id()));
    builder.add(&m_universalRules);
    return builder.build();
}

RefPtr<BoxStyle> CSSStyleSheet::pseudoStyleForElement(Element* element, PseudoType pseudoType, const BoxStyle* parentStyle) const
{
    ElementStyleBuilder builder(element, pseudoType, parentStyle);
    builder.add(m_pseudoRules.get(pseudoType));
    return builder.build();
}

RefPtr<BoxStyle> CSSStyleSheet::styleForPage(const GlobalString& pageName, uint32_t pageIndex, PseudoType pseudoType) const
{
    PageStyleBuilder builder(pageName, pageIndex, PageMarginType::None, pseudoType, m_document->rootStyle());
    builder.add(m_pageRules);
    return builder.build();
}

RefPtr<BoxStyle> CSSStyleSheet::styleForPageMargin(const GlobalString& pageName, uint32_t pageIndex, PageMarginType marginType, const BoxStyle* pageStyle) const
{
    PageStyleBuilder builder(pageName, pageIndex, marginType, pageStyle->pseudoType(), pageStyle);
    builder.add(m_pageRules);
    return builder.build();
}

RefPtr<FontData> CSSStyleSheet::getFontData(const GlobalString& family, const FontDataDescription& description) const
{
    return m_fontFaceCache.get(family, description);
}

const CSSCounterStyle& CSSStyleSheet::getCounterStyle(const GlobalString& name)
{
    auto counterStyleMap = userAgentCounterStyleMap();
    if(!m_counterStyleRules.empty()) {
        if(m_counterStyleMap == nullptr)
            m_counterStyleMap = CSSCounterStyleMap::create(m_document->heap(), m_counterStyleRules, counterStyleMap);
        counterStyleMap = m_counterStyleMap.get();
    }

    if(auto counterStyle = counterStyleMap->findCounterStyle(name))
        return *counterStyle;
    return CSSCounterStyle::defaultStyle();
}

std::string CSSStyleSheet::getCounterText(int value, const GlobalString& listType)
{
    return getCounterStyle(listType).generateRepresentation(value);
}

std::string CSSStyleSheet::getMarkerText(int value, const GlobalString& listType)
{
    const auto& counterStyle = getCounterStyle(listType);
    std::string representation(counterStyle.prefix());
    representation += counterStyle.generateRepresentation(value);
    representation += counterStyle.suffix();
    return representation;
}

void CSSStyleSheet::parseStyle(const std::string_view& content, CSSStyleOrigin origin, Url baseUrl)
{
    CSSParserContext context(m_document, origin, std::move(baseUrl));
    CSSParser parser(context, m_document->heap());
    addRuleList(parser.parseSheet(content));
}

void CSSStyleSheet::addRuleList(const CSSRuleList& rules)
{
    for(const auto& rule : rules) {
        switch(rule->type()) {
        case CSSRuleType::Style:
            addStyleRule(to<CSSStyleRule>(rule));
            break;
        case CSSRuleType::Import:
            addImportRule(to<CSSImportRule>(rule));
            break;
        case CSSRuleType::Page:
            addPageRule(to<CSSPageRule>(rule));
            break;
        case CSSRuleType::FontFace:
            addFontFaceRule(to<CSSFontFaceRule>(rule));
            break;
        case CSSRuleType::CounterStyle:
            addCounterStyleRule(to<CSSCounterStyleRule>(rule));
            break;
        case CSSRuleType::Media:
            addMediaRule(to<CSSMediaRule>(rule));
            break;
        default:
            break;
        }

        m_position += 1;
    }
}

void CSSStyleSheet::addStyleRule(const RefPtr<CSSStyleRule>& rule)
{
    for(const auto& selector : rule->selectors()) {
        uint32_t specificity = 0;
        for(const auto& complexSelector : selector) {
            for(const auto& simpleSelector : complexSelector.compoundSelector()) {
                specificity += simpleSelector.specificity();
            }
        }

        HeapString idName;
        HeapString className;
        GlobalString tagName;
        GlobalString attrName;
        PseudoType pseudoType = PseudoType::None;
        const auto& lastComplexSelector = selector.front();
        for(const auto& simpleSelector : lastComplexSelector.compoundSelector()) {
            switch(simpleSelector.matchType()) {
            case CSSSimpleSelector::MatchType::Id:
                idName = simpleSelector.value();
                break;
            case CSSSimpleSelector::MatchType::Class:
                className = simpleSelector.value();
                break;
            case CSSSimpleSelector::MatchType::Tag:
                tagName = simpleSelector.name();
                break;
            case CSSSimpleSelector::MatchType::AttributeContains:
            case CSSSimpleSelector::MatchType::AttributeDashEquals:
            case CSSSimpleSelector::MatchType::AttributeEndsWith:
            case CSSSimpleSelector::MatchType::AttributeEquals:
            case CSSSimpleSelector::MatchType::AttributeHas:
            case CSSSimpleSelector::MatchType::AttributeIncludes:
            case CSSSimpleSelector::MatchType::AttributeStartsWith:
                attrName = simpleSelector.name();
                break;
            case CSSSimpleSelector::MatchType::PseudoElementBefore:
            case CSSSimpleSelector::MatchType::PseudoElementAfter:
            case CSSSimpleSelector::MatchType::PseudoElementMarker:
            case CSSSimpleSelector::MatchType::PseudoElementFirstLetter:
            case CSSSimpleSelector::MatchType::PseudoElementFirstLine:
                pseudoType = simpleSelector.pseudoType();
                break;
            default:
                break;
            }
        }

        CSSRuleData ruleData(rule, selector, specificity, m_position);
        if(pseudoType > PseudoType::None) {
            m_pseudoRules.add(pseudoType, std::move(ruleData));
        } else if(!idName.empty()) {
            m_idRules.add(idName, std::move(ruleData));
        } else if(!className.empty()) {
            m_classRules.add(className, std::move(ruleData));
        } else if(!attrName.isEmpty()) {
            m_attributeRules.add(attrName, std::move(ruleData));
        } else if(!tagName.isEmpty()) {
            m_tagRules.add(tagName, std::move(ruleData));
        } else {
            m_universalRules.push_back(std::move(ruleData));
        }
    }
}

void CSSStyleSheet::addImportRule(const RefPtr<CSSImportRule>& rule)
{
    constexpr auto kMaxImportDepth = 256;
    if(m_importDepth < kMaxImportDepth && m_document->supportsMediaQueries(rule->queries())) {
        if(auto resource = m_document->fetchTextResource(rule->href())) {
            m_importDepth++;
            parseStyle(resource->text(), rule->origin(), rule->href());
            m_importDepth--;
        }
    }
}

void CSSStyleSheet::addPageRule(const RefPtr<CSSPageRule>& rule)
{
    const auto& selectors = rule->selectors();
    if(selectors.empty()) {
        m_pageRules.emplace_back(rule, nullptr, 0, m_position);
        return;
    }

    for(const auto& selector : selectors) {
        uint32_t specificity = 0;
        for(const auto& sel : selector) {
            switch(sel.matchType()) {
            case CSSSimpleSelector::MatchType::PseudoPageName:
                specificity += 0x10000;
                break;
            case CSSSimpleSelector::MatchType::PseudoPageFirst:
            case CSSSimpleSelector::MatchType::PseudoPageBlank:
                specificity += 0x100;
                break;
            case CSSSimpleSelector::MatchType::PseudoPageLeft:
            case CSSSimpleSelector::MatchType::PseudoPageRight:
            case CSSSimpleSelector::MatchType::PseudoPageNth:
                specificity += 0x1;
                break;
            default:
                assert(false);
            }
        }

        m_pageRules.emplace_back(rule, &selector, specificity, m_position);
    }
}

class CSSFontFaceBuilder {
public:
    explicit CSSFontFaceBuilder(const CSSPropertyList& properties);

    FontSelectionRange weight() const;
    FontSelectionRange stretch() const;
    FontSelectionRange style() const;

    FontFeatureList featureSettings() const;
    FontVariationList variationSettings() const;
    UnicodeRangeList unicodeRanges() const;

    GlobalString family() const;
    FontSelectionDescription description() const;

    RefPtr<FontFace> build(Document* document) const;

private:
    RefPtr<CSSValue> m_src;
    RefPtr<CSSValue> m_family;
    RefPtr<CSSValue> m_weight;
    RefPtr<CSSValue> m_stretch;
    RefPtr<CSSValue> m_style;
    RefPtr<CSSValue> m_featureSettings;
    RefPtr<CSSValue> m_variationSettings;
    RefPtr<CSSValue> m_unicodeRange;
};

CSSFontFaceBuilder::CSSFontFaceBuilder(const CSSPropertyList& properties)
{
    for(const auto& property : properties) {
        switch(property.id()) {
        case CSSPropertyID::Src:
            m_src = property.value();
            break;
        case CSSPropertyID::FontFamily:
            m_family = property.value();
            break;
        case CSSPropertyID::FontWeight:
            m_weight = property.value();
            break;
        case CSSPropertyID::FontStretch:
            m_stretch = property.value();
            break;
        case CSSPropertyID::FontStyle:
            m_style = property.value();
            break;
        case CSSPropertyID::UnicodeRange:
            m_unicodeRange = property.value();
            break;
        case CSSPropertyID::FontFeatureSettings:
            m_featureSettings = property.value();
            break;
        case CSSPropertyID::FontVariationSettings:
            m_variationSettings = property.value();
            break;
        default:
            assert(false);
        }
    }
}

FontSelectionRange CSSFontFaceBuilder::weight() const
{
    if(m_weight == nullptr)
        return FontSelectionRange(kNormalFontWeight);
    if(auto ident = to<CSSIdentValue>(m_weight)) {
        switch(ident->value()) {
        case CSSValueID::Normal:
            return FontSelectionRange(kNormalFontWeight);
        case CSSValueID::Bold:
            return FontSelectionRange(kBoldFontWeight);
        default:
            assert(false);
        }
    }

    const auto& pair = to<CSSPairValue>(*m_weight);
    auto startWeight = convertFontWeightNumber(*pair.first());
    auto endWeight = convertFontWeightNumber(*pair.second());
    if(startWeight > endWeight)
        return FontSelectionRange(endWeight, startWeight);
    return FontSelectionRange(startWeight, endWeight);
}

FontSelectionRange CSSFontFaceBuilder::stretch() const
{
    if(m_stretch == nullptr)
        return FontSelectionRange(kNormalFontWidth);
    if(auto ident = to<CSSIdentValue>(m_stretch))
        return FontSelectionRange(convertFontStretchIdent(*ident));
    const auto& pair = to<CSSPairValue>(*m_stretch);
    const auto& startPercent = to<CSSPercentValue>(*pair.first());
    const auto& endPercent = to<CSSPercentValue>(*pair.second());
    if(startPercent.value() > endPercent.value())
        return FontSelectionRange(endPercent.value(), startPercent.value());
    return FontSelectionRange(startPercent.value(), endPercent.value());
}

FontSelectionRange CSSFontFaceBuilder::style() const
{
    if(m_style == nullptr)
        return FontSelectionRange(kNormalFontSlope);
    if(auto ident = to<CSSIdentValue>(m_style))
        return FontSelectionRange(convertFontStyleIdent(*ident));
    const auto& list = to<CSSListValue>(*m_style);
    const auto& ident = to<CSSIdentValue>(*list.at(0));
    assert(list.size() == 3 && ident.value() == CSSValueID::Oblique);

    auto startAngle = convertFontStyleAngle(*list.at(1));
    auto endAngle = convertFontStyleAngle(*list.at(2));
    if(startAngle > endAngle)
        return FontSelectionRange(endAngle, startAngle);
    return FontSelectionRange(startAngle, endAngle);
}

FontFeatureList CSSFontFaceBuilder::featureSettings() const
{
    FontFeatureList featureSettings;
    if(m_featureSettings == nullptr)
        return featureSettings;
    if(auto ident = to<CSSIdentValue>(m_featureSettings)) {
        assert(ident->value() == CSSValueID::Normal);
        return featureSettings;
    }

    for(const auto& value : to<CSSListValue>(*m_featureSettings)) {
        const auto& feature = to<CSSFontFeatureValue>(*value);
        featureSettings.emplace_front(feature.tag(), feature.value());
    }

    return featureSettings;
}

FontVariationList CSSFontFaceBuilder::variationSettings() const
{
    FontVariationList variationSettings;
    if(m_variationSettings == nullptr)
        return variationSettings;
    if(auto ident = to<CSSIdentValue>(m_variationSettings)) {
        assert(ident->value() == CSSValueID::Normal);
        return variationSettings;
    }

    for(const auto& value : to<CSSListValue>(*m_variationSettings)) {
        const auto& variation = to<CSSFontVariationValue>(*value);
        variationSettings.emplace_front(variation.tag(), variation.value());
    }

    return variationSettings;
}

UnicodeRangeList CSSFontFaceBuilder::unicodeRanges() const
{
    UnicodeRangeList unicodeRanges;
    if(m_unicodeRange == nullptr)
        return unicodeRanges;
    for(const auto& value : to<CSSListValue>(*m_unicodeRange)) {
        const auto& range = to<CSSUnicodeRangeValue>(*value);
        unicodeRanges.emplace_front(range.from(), range.to());
    }

    return unicodeRanges;
}

GlobalString CSSFontFaceBuilder::family() const
{
    if(auto family = to<CSSCustomIdentValue>(m_family))
        return family->value();
    return nullGlo;
}

FontSelectionDescription CSSFontFaceBuilder::description() const
{
    return FontSelectionDescription(weight(), stretch(), style());
}

static const HeapString& convertStringOrCustomIdent(const CSSValue& value)
{
    if(is<CSSStringValue>(value))
        return to<CSSStringValue>(value).value();
    return to<CSSCustomIdentValue>(value).value();
}

RefPtr<FontFace> CSSFontFaceBuilder::build(Document* document) const
{
    if(m_src == nullptr)
        return nullptr;
    for(const auto& value : to<CSSListValue>(*m_src)) {
        const auto& list = to<CSSListValue>(*value);
        if(auto function = to<CSSUnaryFunctionValue>(list.at(0))) {
            assert(function->id() == CSSFunctionID::Local);
            const auto& family = to<CSSCustomIdentValue>(*function->value());
            if(!fontDataCache()->isFamilyAvailable(family.value()))
                continue;
            return LocalFontFace::create(family.value(), featureSettings(), variationSettings(), unicodeRanges());
        }

        const auto& url = to<CSSUrlValue>(*list.at(0));
        if(list.size() == 2) {
            const auto& function = to<CSSUnaryFunctionValue>(*list.at(1));
            assert(function.id() == CSSFunctionID::Format);
            const auto& format = convertStringOrCustomIdent(*function.value());
            if(!FontResource::supportsFormat(format.value())) {
                continue;
            }
        }

        if(auto fontResource = document->fetchFontResource(url.value())) {
            return RemoteFontFace::create(featureSettings(), variationSettings(), unicodeRanges(), std::move(fontResource));
        }
    }

    return nullptr;
}

void CSSStyleSheet::addFontFaceRule(const RefPtr<CSSFontFaceRule>& rule)
{
    CSSFontFaceBuilder builder(rule->properties());
    if(auto face = builder.build(m_document)) {
        m_fontFaceCache.add(builder.family(), builder.description(), std::move(face));
    }
}

void CSSStyleSheet::addCounterStyleRule(const RefPtr<CSSCounterStyleRule>& rule)
{
    assert(m_counterStyleMap == nullptr);
    m_counterStyleRules.push_back(rule);
}

void CSSStyleSheet::addMediaRule(const RefPtr<CSSMediaRule>& rule)
{
    if(m_document->supportsMediaQueries(rule->queries())) {
        addRuleList(rule->rules());
    }
}

} // namespace plutobook
