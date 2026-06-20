/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "cssproperty.h"
#include "cssparser.h"
#include "document.h"
#include "boxstyle.h"
#include "imageresource.h"
#include "stringutils.h"

namespace plutobook {

CSSLengthResolver::CSSLengthResolver(const Document* document)
    : m_style(nullptr), m_document(document)
{
}

CSSLengthResolver::CSSLengthResolver(const BoxStyle* style)
    : m_style(style), m_document(style->document())
{
}

float CSSLengthResolver::resolveLength(const CSSValue& value) const
{
    if(is<CSSLengthValue>(value))
        return resolveLength(to<CSSLengthValue>(value));
    if(auto result = to<CSSCalcValue>(value).resolve(*this))
        return result->value;
    return 0.f;
}

float CSSLengthResolver::resolveLength(const CSSLengthValue& length) const
{
    return resolveLength(length.value(), length.units());
}

float CSSLengthResolver::resolveLength(float value, CSSLengthUnits units) const
{
    constexpr auto dpi = 96.f;
    switch(units) {
    case CSSLengthUnits::None:
    case CSSLengthUnits::Pixels:
        return value;
    case CSSLengthUnits::Points:
        return value * dpi / 72.f;
    case CSSLengthUnits::Picas:
        return value * dpi / 6.f;
    case CSSLengthUnits::Inches:
        return value * dpi;
    case CSSLengthUnits::Centimeters:
        return value * dpi / 2.54f;
    case CSSLengthUnits::Millimeters:
        return value * dpi / 25.4f;
    case CSSLengthUnits::ViewportWidth:
        return value * viewportWidth() / 100.f;
    case CSSLengthUnits::ViewportHeight:
        return value * viewportHeight() / 100.f;
    case CSSLengthUnits::ViewportMin:
        return value * viewportMin() / 100.f;
    case CSSLengthUnits::ViewportMax:
        return value * viewportMax() / 100.f;
    case CSSLengthUnits::Ems:
        return value * emFontSize();
    case CSSLengthUnits::Rems:
        return value * remFontSize();
    case CSSLengthUnits::Exs:
        return value * exFontSize();
    case CSSLengthUnits::Rexs:
        return value * rexFontSize();
    case CSSLengthUnits::Chs:
        return value * chFontSize();
    case CSSLengthUnits::Rchs:
        return value * rchFontSize();
    case CSSLengthUnits::Ics:
        return value * icFontSize();
    case CSSLengthUnits::Rics:
        return value * ricFontSize();
    case CSSLengthUnits::Caps:
        return value * capFontSize();
    case CSSLengthUnits::Rcaps:
        return value * rcapFontSize();
    case CSSLengthUnits::Lhs:
        return value * lineHeight();
    case CSSLengthUnits::Rlhs:
        return value * rootLineHeight();
    default:
        assert(false);
    }

    return 0.f;
}

float CSSLengthResolver::emFontSize() const
{
    if(m_style)
        return m_style->fontSize();
    return 0.f;
}

float CSSLengthResolver::remFontSize() const
{
    if(auto style = m_document->rootStyle())
        return style->fontSize();
    return 0.f;
}

float CSSLengthResolver::exFontSize() const
{
    if(m_style)
        return m_style->exFontSize();
    return 0.f;
}

float CSSLengthResolver::rexFontSize() const
{
    if(auto style = m_document->rootStyle())
        return style->exFontSize();
    return 0.f;
}

float CSSLengthResolver::chFontSize() const
{
    if(m_style)
        return m_style->chFontSize();
    return 0.f;
}

float CSSLengthResolver::rchFontSize() const
{
    if(auto style = m_document->rootStyle())
        return style->chFontSize();
    return 0.f;
}

float CSSLengthResolver::icFontSize() const
{
    if(m_style)
        return m_style->icFontSize();
    return 0.f;
}

float CSSLengthResolver::ricFontSize() const
{
    if(auto style = m_document->rootStyle())
        return style->icFontSize();
    return 0.f;
}

float CSSLengthResolver::capFontSize() const
{
    if(m_style)
        return m_style->capFontSize();
    return 0.f;
}

float CSSLengthResolver::rcapFontSize() const
{
    if(auto style = m_document->rootStyle())
        return style->capFontSize();
    return 0.f;
}

float CSSLengthResolver::lineHeight() const
{
    if(m_style)
        return m_style->lineHeightValue();
    return 0.f;
}

float CSSLengthResolver::rootLineHeight() const
{
    if(auto style = m_document->rootStyle())
        return style->lineHeightValue();
    return 0.f;
}

float CSSLengthResolver::viewportWidth() const
{
    return m_document->viewportWidth();
}

float CSSLengthResolver::viewportHeight() const
{
    return m_document->viewportHeight();
}

float CSSLengthResolver::viewportMin() const
{
    return std::min(m_document->viewportWidth(), m_document->viewportHeight());
}

float CSSLengthResolver::viewportMax() const
{
    return std::max(m_document->viewportWidth(), m_document->viewportHeight());
}

std::optional<CSSCalc> CSSCalcValue::resolve(const CSSLengthResolver& resolver) const
{
    std::vector<CSSCalc> stack;
    for(const auto& item : m_values) {
        if(item.op == CSSCalcOperator::None) {
            if(item.units == CSSLengthUnits::None) {
                stack.push_back(item);
            } else {
                auto value = resolver.resolveLength(item.value, item.units);
                stack.emplace_back(value, CSSLengthUnits::Pixels);
            }
        } else {
            if(stack.size() < 2)
                return std::nullopt;
            auto right = stack.back();
            stack.pop_back();
            auto left = stack.back();
            stack.pop_back();

            switch(item.op) {
            case CSSCalcOperator::Add:
                if(right.units != left.units)
                    return std::nullopt;
                stack.emplace_back(left.value + right.value, right.units);
                break;
            case CSSCalcOperator::Sub:
                if(right.units != left.units)
                    return std::nullopt;
                stack.emplace_back(left.value - right.value, right.units);
                break;
            case CSSCalcOperator::Mul:
                if(right.units == CSSLengthUnits::Pixels && left.units == CSSLengthUnits::Pixels)
                    return std::nullopt;
                stack.emplace_back(left.value * right.value, std::max(left.units, right.units));
                break;
            case CSSCalcOperator::Div:
                if(right.units == CSSLengthUnits::Pixels || right.value == 0)
                    return std::nullopt;
                stack.emplace_back(left.value / right.value, left.units);
                break;
            case CSSCalcOperator::Min:
                if(right.units != left.units)
                    return std::nullopt;
                stack.emplace_back(std::min(left.value, right.value), right.units);
                break;
            case CSSCalcOperator::Max:
                if(right.units != left.units)
                    return std::nullopt;
                stack.emplace_back(std::max(left.value, right.value), right.units);
                break;
            default:
                assert(false);
            }
        }
    }

    if(stack.size() == 1) {
        const auto& result = stack.back();
        if(result.value < 0 && !m_negative)
            return std::nullopt;
        if(result.units == CSSLengthUnits::None && !m_unitless)
            return std::nullopt;
        return result;
    }

    return std::nullopt;
}

using CSSIdentValueList = std::pmr::vector<RefPtr<CSSIdentValue>>;

class CSSValuePool {
public:
    CSSValuePool();

    RefPtr<CSSInitialValue> initialValue() const { return m_initialValue; }
    RefPtr<CSSInheritValue> inheritValue() const { return m_inheritValue; }
    RefPtr<CSSUnsetValue> unsetValue() const { return m_unsetValue; }

    RefPtr<CSSIdentValue> identValue(CSSValueID id) const;

private:
    Heap m_heap;
    RefPtr<CSSInitialValue> m_initialValue;
    RefPtr<CSSInheritValue> m_inheritValue;
    RefPtr<CSSUnsetValue> m_unsetValue;
    CSSIdentValueList m_identValues;
};

CSSValuePool::CSSValuePool()
    : m_heap(1024 * 8)
    , m_initialValue(adoptPtr(new (&m_heap) CSSInitialValue))
    , m_inheritValue(adoptPtr(new (&m_heap) CSSInheritValue))
    , m_unsetValue(adoptPtr(new (&m_heap) CSSUnsetValue))
    , m_identValues(kNumCSSValueIDs, &m_heap)
{
    assert(CSSValueID::Unknown == static_cast<CSSValueID>(0));
    for(int i = 1; i < kNumCSSValueIDs; ++i) {
        const auto id = static_cast<CSSValueID>(i);
        m_identValues[i] = adoptPtr(new (&m_heap) CSSIdentValue(id));
    }
}

RefPtr<CSSIdentValue> CSSValuePool::identValue(CSSValueID id) const
{
    return m_identValues[static_cast<int>(id)];
}

static CSSValuePool* cssValuePool()
{
    static CSSValuePool valuePool;
    return &valuePool;
}

RefPtr<CSSInitialValue> CSSInitialValue::create()
{
    return cssValuePool()->initialValue();
}

RefPtr<CSSInheritValue> CSSInheritValue::create()
{
    return cssValuePool()->inheritValue();
}

RefPtr<CSSUnsetValue> CSSUnsetValue::create()
{
    return cssValuePool()->unsetValue();
}

RefPtr<CSSIdentValue> CSSIdentValue::create(CSSValueID value)
{
    return cssValuePool()->identValue(value);
}

RefPtr<CSSVariableData> CSSVariableData::create(Heap* heap, const CSSTokenStream& value)
{
    return adoptPtr(new (heap) CSSVariableData(heap, value));
}

CSSVariableData::CSSVariableData(Heap* heap, const CSSTokenStream& value)
    : m_tokens(heap)
{
    m_tokens.assign(value.begin(), value.end());
    for(auto& token : m_tokens) {
        if(!token.m_data.empty()) {
            token.m_data = heap->createString(token.data());
        }
    }
}

bool CSSVariableData::resolve(const BoxStyle* style, CSSTokenList& tokens, std::set<CSSVariableData*>& references) const
{
    CSSTokenStream input(m_tokens.data(), m_tokens.size());
    return resolve(input, style, tokens, references);
}

bool CSSVariableData::resolve(CSSTokenStream input, const BoxStyle* style, CSSTokenList& tokens, std::set<CSSVariableData*>& references) const
{
    while(!input.empty()) {
        if(input->type() == CSSToken::Type::Function && equalsIgnoringCase("var", input->data())) {
            auto block = input.consumeBlock();
            if(!resolveVar(block, style, tokens, references))
                return false;
            continue;
        }

        tokens.push_back(input.get());
        input.consume();
    }

    return true;
}

bool CSSVariableData::resolveVar(CSSTokenStream input, const BoxStyle* style, CSSTokenList& tokens, std::set<CSSVariableData*>& references) const
{
    input.consumeWhitespace();
    if(input->type() != CSSToken::Type::Ident)
        return false;
    auto data = style->getCustom(input->data());
    input.consumeIncludingWhitespace();
    if(!input.empty() && input->type() != CSSToken::Type::Comma)
        return false;
    if(data == nullptr) {
        if(!input.consumeCommaIncludingWhitespace())
            return false;
        return resolve(input, style, tokens, references);
    }

    if(references.contains(data))
        return false;
    references.insert(data);
    return data->resolve(style, tokens, references);
}

RefPtr<CSSCustomPropertyValue> CSSCustomPropertyValue::create(Heap* heap, const HeapString& name, RefPtr<CSSVariableData> value)
{
    return adoptPtr(new (heap) CSSCustomPropertyValue(name, std::move(value)));
}

CSSCustomPropertyValue::CSSCustomPropertyValue(const HeapString& name, RefPtr<CSSVariableData> value)
    : m_name(name), m_value(std::move(value))
{
}

CSSParserContext::CSSParserContext(const Node* node, CSSStyleOrigin origin, Url baseUrl)
    : m_inHTMLDocument(node && node->isHTMLDocument())
    , m_inSVGElement(node && node->isSVGElement())
    , m_origin(origin)
    , m_baseUrl(std::move(baseUrl))
{
}

RefPtr<CSSVariableReferenceValue> CSSVariableReferenceValue::create(Heap* heap, const CSSParserContext& context, CSSPropertyID id, bool important, RefPtr<CSSVariableData> value)
{
    return adoptPtr(new (heap) CSSVariableReferenceValue(context, id, important, std::move(value)));
}

CSSPropertyList CSSVariableReferenceValue::resolve(const BoxStyle* style) const
{
    CSSTokenList tokens;
    std::set<CSSVariableData*> references;
    if(!m_value->resolve(style, tokens, references))
        return CSSPropertyList();
    CSSTokenStream input(tokens.data(), tokens.size());
    CSSParser parser(m_context, style->heap());
    return parser.parsePropertyValue(input, m_id, m_important);
}

CSSVariableReferenceValue::CSSVariableReferenceValue(const CSSParserContext& context, CSSPropertyID id, bool important, RefPtr<CSSVariableData> value)
    : m_context(context), m_id(id), m_important(important), m_value(std::move(value))
{
}

RefPtr<CSSImageValue> CSSImageValue::create(Heap* heap, Url value)
{
    return adoptPtr(new (heap) CSSImageValue(std::move(value)));
}

const RefPtr<Image>& CSSImageValue::fetch(Document* document) const
{
    if(m_image == nullptr) {
        if(auto resource = document->fetchImageResource(m_value)) {
            m_image = resource->image();
        }
    }

    return m_image;
}

CSSImageValue::CSSImageValue(Url value)
    : m_value(std::move(value))
{
}

CSSShorthand CSSShorthand::longhand(CSSPropertyID id)
{
    switch(id) {
    case CSSPropertyID::BorderColor: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopColor,
            CSSPropertyID::BorderRightColor,
            CSSPropertyID::BorderBottomColor,
            CSSPropertyID::BorderLeftColor
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderInlineColor: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderInlineStartColor,
            CSSPropertyID::BorderInlineEndColor
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderBlockColor: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderBlockStartColor,
            CSSPropertyID::BorderBlockEndColor
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderStyle: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopStyle,
            CSSPropertyID::BorderRightStyle,
            CSSPropertyID::BorderBottomStyle,
            CSSPropertyID::BorderLeftStyle
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderInlineStyle: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderInlineStartStyle,
            CSSPropertyID::BorderInlineEndStyle
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderBlockStyle: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderBlockStartStyle,
            CSSPropertyID::BorderBlockEndStyle
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderWidth: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopWidth,
            CSSPropertyID::BorderRightWidth,
            CSSPropertyID::BorderBottomWidth,
            CSSPropertyID::BorderLeftWidth
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderInlineWidth: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderInlineStartWidth,
            CSSPropertyID::BorderInlineEndWidth
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderBlockWidth: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderBlockStartWidth,
            CSSPropertyID::BorderBlockEndWidth
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderTop: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopColor,
            CSSPropertyID::BorderTopStyle,
            CSSPropertyID::BorderTopWidth
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderRight: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderRightColor,
            CSSPropertyID::BorderRightStyle,
            CSSPropertyID::BorderRightWidth
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderBottom: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderBottomColor,
            CSSPropertyID::BorderBottomStyle,
            CSSPropertyID::BorderBottomWidth
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderLeft: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderLeftColor,
            CSSPropertyID::BorderLeftStyle,
            CSSPropertyID::BorderLeftWidth
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::Border: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopWidth,
            CSSPropertyID::BorderRightWidth,
            CSSPropertyID::BorderBottomWidth,
            CSSPropertyID::BorderLeftWidth,
            CSSPropertyID::BorderTopStyle,
            CSSPropertyID::BorderRightStyle,
            CSSPropertyID::BorderBottomStyle,
            CSSPropertyID::BorderLeftStyle,
            CSSPropertyID::BorderTopColor,
            CSSPropertyID::BorderRightColor,
            CSSPropertyID::BorderBottomColor,
            CSSPropertyID::BorderLeftColor
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderInlineStart: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderInlineStartWidth,
            CSSPropertyID::BorderInlineStartStyle,
            CSSPropertyID::BorderInlineStartColor
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderInlineEnd: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderInlineEndWidth,
            CSSPropertyID::BorderInlineEndStyle,
            CSSPropertyID::BorderInlineEndColor
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderBlockStart: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderBlockStartWidth,
            CSSPropertyID::BorderBlockStartStyle,
            CSSPropertyID::BorderBlockStartColor
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderBlockEnd: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderBlockEndWidth,
            CSSPropertyID::BorderBlockEndStyle,
            CSSPropertyID::BorderBlockEndColor
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderInline: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderInlineStartColor,
            CSSPropertyID::BorderInlineStartStyle,
            CSSPropertyID::BorderInlineStartWidth,
            CSSPropertyID::BorderInlineEndColor,
            CSSPropertyID::BorderInlineEndStyle,
            CSSPropertyID::BorderInlineEndWidth
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderBlock: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderBlockStartColor,
            CSSPropertyID::BorderBlockStartStyle,
            CSSPropertyID::BorderBlockStartWidth,
            CSSPropertyID::BorderBlockEndColor,
            CSSPropertyID::BorderBlockEndStyle,
            CSSPropertyID::BorderBlockEndWidth
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderRadius: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopLeftRadius,
            CSSPropertyID::BorderTopRightRadius,
            CSSPropertyID::BorderBottomRightRadius,
            CSSPropertyID::BorderBottomLeftRadius
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::BorderSpacing: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderHorizontalSpacing,
            CSSPropertyID::BorderVerticalSpacing
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::Padding: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::PaddingTop,
            CSSPropertyID::PaddingRight,
            CSSPropertyID::PaddingBottom,
            CSSPropertyID::PaddingLeft
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::PaddingInline: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::PaddingInlineStart,
            CSSPropertyID::PaddingInlineEnd
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::PaddingBlock: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::PaddingBlockStart,
            CSSPropertyID::PaddingBlockEnd
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::Margin: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::MarginTop,
            CSSPropertyID::MarginRight,
            CSSPropertyID::MarginBottom,
            CSSPropertyID::MarginLeft
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::MarginInline: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::MarginInlineStart,
            CSSPropertyID::MarginInlineEnd
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::MarginBlock: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::MarginBlockStart,
            CSSPropertyID::MarginBlockEnd
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::Inset: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::Top,
            CSSPropertyID::Right,
            CSSPropertyID::Bottom,
            CSSPropertyID::Left
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::InsetInline: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::InsetInlineStart,
            CSSPropertyID::InsetInlineEnd
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::InsetBlock: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::InsetBlockStart,
            CSSPropertyID::InsetBlockEnd
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::Outline: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::OutlineColor,
            CSSPropertyID::OutlineStyle,
            CSSPropertyID::OutlineWidth
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::Background: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BackgroundColor,
            CSSPropertyID::BackgroundImage,
            CSSPropertyID::BackgroundRepeat,
            CSSPropertyID::BackgroundAttachment,
            CSSPropertyID::BackgroundOrigin,
            CSSPropertyID::BackgroundClip,
            CSSPropertyID::BackgroundPosition,
            CSSPropertyID::BackgroundSize
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::ListStyle: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::ListStyleType,
            CSSPropertyID::ListStylePosition,
            CSSPropertyID::ListStyleImage
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::ColumnRule: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::ColumnRuleColor,
            CSSPropertyID::ColumnRuleStyle,
            CSSPropertyID::ColumnRuleWidth
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::FlexFlow: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::FlexDirection,
            CSSPropertyID::FlexWrap
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::Flex: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::FlexGrow,
            CSSPropertyID::FlexShrink,
            CSSPropertyID::FlexBasis
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::Gap: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::RowGap,
            CSSPropertyID::ColumnGap
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::Columns: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::ColumnWidth,
            CSSPropertyID::ColumnCount
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::Font: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::FontStyle,
            CSSPropertyID::FontWeight,
            CSSPropertyID::FontVariantCaps,
            CSSPropertyID::FontStretch,
            CSSPropertyID::FontSize,
            CSSPropertyID::LineHeight,
            CSSPropertyID::FontFamily
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::FontVariant: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::FontVariantCaps,
            CSSPropertyID::FontVariantEastAsian,
            CSSPropertyID::FontVariantEmoji,
            CSSPropertyID::FontVariantLigatures,
            CSSPropertyID::FontVariantNumeric,
            CSSPropertyID::FontVariantPosition
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::TextDecoration: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::TextDecorationLine,
            CSSPropertyID::TextDecorationStyle,
            CSSPropertyID::TextDecorationColor
        };

        return CSSShorthand(data);
    }

    case CSSPropertyID::Marker: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::MarkerStart,
            CSSPropertyID::MarkerMid,
            CSSPropertyID::MarkerEnd
        };

        return CSSShorthand(data);
    }

    default:
        return CSSShorthand();
    }
}

constexpr bool isCustomPropertyName(std::string_view name)
{
    return name.length() > 2 && name[0] == '-' && name[1] == '-';
}

CSSPropertyID CSSProperty::id(std::string_view name)
{
    if(isCustomPropertyName(name))
        return CSSPropertyID::Custom;
    static const struct {
        std::string_view name;
        CSSPropertyID value;
    } table[] = {
        {"-pluto-lang", CSSPropertyID::Lang},
        {"-pluto-page-scale", CSSPropertyID::PageScale},
        {"additive-symbols", CSSPropertyID::AdditiveSymbols},
        {"align-content", CSSPropertyID::AlignContent},
        {"align-items", CSSPropertyID::AlignItems},
        {"align-self", CSSPropertyID::AlignSelf},
        {"alignment-baseline", CSSPropertyID::AlignmentBaseline},
        {"background", CSSPropertyID::Background},
        {"background-attachment", CSSPropertyID::BackgroundAttachment},
        {"background-clip", CSSPropertyID::BackgroundClip},
        {"background-color", CSSPropertyID::BackgroundColor},
        {"background-image", CSSPropertyID::BackgroundImage},
        {"background-origin", CSSPropertyID::BackgroundOrigin},
        {"background-position", CSSPropertyID::BackgroundPosition},
        {"background-repeat", CSSPropertyID::BackgroundRepeat},
        {"background-size", CSSPropertyID::BackgroundSize},
        {"baseline-shift", CSSPropertyID::BaselineShift},
        {"block-size", CSSPropertyID::BlockSize},
        {"border", CSSPropertyID::Border},
        {"border-block", CSSPropertyID::BorderBlock},
        {"border-block-color", CSSPropertyID::BorderBlockColor},
        {"border-block-end", CSSPropertyID::BorderBlockEnd},
        {"border-block-end-color", CSSPropertyID::BorderBlockEndColor},
        {"border-block-end-style", CSSPropertyID::BorderBlockEndStyle},
        {"border-block-end-width", CSSPropertyID::BorderBlockEndWidth},
        {"border-block-start", CSSPropertyID::BorderBlockStart},
        {"border-block-start-color", CSSPropertyID::BorderBlockStartColor},
        {"border-block-start-style", CSSPropertyID::BorderBlockStartStyle},
        {"border-block-start-width", CSSPropertyID::BorderBlockStartWidth},
        {"border-block-style", CSSPropertyID::BorderBlockStyle},
        {"border-block-width", CSSPropertyID::BorderBlockWidth},
        {"border-bottom", CSSPropertyID::BorderBottom},
        {"border-bottom-color", CSSPropertyID::BorderBottomColor},
        {"border-bottom-left-radius", CSSPropertyID::BorderBottomLeftRadius},
        {"border-bottom-right-radius", CSSPropertyID::BorderBottomRightRadius},
        {"border-bottom-style", CSSPropertyID::BorderBottomStyle},
        {"border-bottom-width", CSSPropertyID::BorderBottomWidth},
        {"border-collapse", CSSPropertyID::BorderCollapse},
        {"border-color", CSSPropertyID::BorderColor},
        {"border-end-end-radius", CSSPropertyID::BorderEndEndRadius},
        {"border-end-start-radius", CSSPropertyID::BorderEndStartRadius},
        {"border-horizontal-spacing", CSSPropertyID::BorderHorizontalSpacing},
        {"border-inline", CSSPropertyID::BorderInline},
        {"border-inline-color", CSSPropertyID::BorderInlineColor},
        {"border-inline-end", CSSPropertyID::BorderInlineEnd},
        {"border-inline-end-color", CSSPropertyID::BorderInlineEndColor},
        {"border-inline-end-style", CSSPropertyID::BorderInlineEndStyle},
        {"border-inline-end-width", CSSPropertyID::BorderInlineEndWidth},
        {"border-inline-start", CSSPropertyID::BorderInlineStart},
        {"border-inline-start-color", CSSPropertyID::BorderInlineStartColor},
        {"border-inline-start-style", CSSPropertyID::BorderInlineStartStyle},
        {"border-inline-start-width", CSSPropertyID::BorderInlineStartWidth},
        {"border-inline-style", CSSPropertyID::BorderInlineStyle},
        {"border-inline-width", CSSPropertyID::BorderInlineWidth},
        {"border-left", CSSPropertyID::BorderLeft},
        {"border-left-color", CSSPropertyID::BorderLeftColor},
        {"border-left-style", CSSPropertyID::BorderLeftStyle},
        {"border-left-width", CSSPropertyID::BorderLeftWidth},
        {"border-radius", CSSPropertyID::BorderRadius},
        {"border-right", CSSPropertyID::BorderRight},
        {"border-right-color", CSSPropertyID::BorderRightColor},
        {"border-right-style", CSSPropertyID::BorderRightStyle},
        {"border-right-width", CSSPropertyID::BorderRightWidth},
        {"border-spacing", CSSPropertyID::BorderSpacing},
        {"border-start-end-radius", CSSPropertyID::BorderStartEndRadius},
        {"border-start-start-radius", CSSPropertyID::BorderStartStartRadius},
        {"border-style", CSSPropertyID::BorderStyle},
        {"border-top", CSSPropertyID::BorderTop},
        {"border-top-color", CSSPropertyID::BorderTopColor},
        {"border-top-left-radius", CSSPropertyID::BorderTopLeftRadius},
        {"border-top-right-radius", CSSPropertyID::BorderTopRightRadius},
        {"border-top-style", CSSPropertyID::BorderTopStyle},
        {"border-top-width", CSSPropertyID::BorderTopWidth},
        {"border-vertical-spacing", CSSPropertyID::BorderVerticalSpacing},
        {"border-width", CSSPropertyID::BorderWidth},
        {"bottom", CSSPropertyID::Bottom},
        {"box-sizing", CSSPropertyID::BoxSizing},
        {"break-after", CSSPropertyID::BreakAfter},
        {"break-before", CSSPropertyID::BreakBefore},
        {"break-inside", CSSPropertyID::BreakInside},
        {"caption-side", CSSPropertyID::CaptionSide},
        {"clear", CSSPropertyID::Clear},
        {"clip", CSSPropertyID::Clip},
        {"clip-path", CSSPropertyID::ClipPath},
        {"clip-rule", CSSPropertyID::ClipRule},
        {"color", CSSPropertyID::Color},
        {"column-break-after", CSSPropertyID::ColumnBreakAfter},
        {"column-break-before", CSSPropertyID::ColumnBreakBefore},
        {"column-break-inside", CSSPropertyID::ColumnBreakInside},
        {"column-count", CSSPropertyID::ColumnCount},
        {"column-fill", CSSPropertyID::ColumnFill},
        {"column-gap", CSSPropertyID::ColumnGap},
        {"column-rule", CSSPropertyID::ColumnRule},
        {"column-rule-color", CSSPropertyID::ColumnRuleColor},
        {"column-rule-style", CSSPropertyID::ColumnRuleStyle},
        {"column-rule-width", CSSPropertyID::ColumnRuleWidth},
        {"column-span", CSSPropertyID::ColumnSpan},
        {"column-width", CSSPropertyID::ColumnWidth},
        {"columns", CSSPropertyID::Columns},
        {"content", CSSPropertyID::Content},
        {"counter-increment", CSSPropertyID::CounterIncrement},
        {"counter-reset", CSSPropertyID::CounterReset},
        {"counter-set", CSSPropertyID::CounterSet},
        {"cx", CSSPropertyID::Cx},
        {"cy", CSSPropertyID::Cy},
        {"direction", CSSPropertyID::Direction},
        {"display", CSSPropertyID::Display},
        {"dominant-baseline", CSSPropertyID::DominantBaseline},
        {"empty-cells", CSSPropertyID::EmptyCells},
        {"fallback", CSSPropertyID::Fallback},
        {"fill", CSSPropertyID::Fill},
        {"fill-opacity", CSSPropertyID::FillOpacity},
        {"fill-rule", CSSPropertyID::FillRule},
        {"flex", CSSPropertyID::Flex},
        {"flex-basis", CSSPropertyID::FlexBasis},
        {"flex-direction", CSSPropertyID::FlexDirection},
        {"flex-flow", CSSPropertyID::FlexFlow},
        {"flex-grow", CSSPropertyID::FlexGrow},
        {"flex-shrink", CSSPropertyID::FlexShrink},
        {"flex-wrap", CSSPropertyID::FlexWrap},
        {"float", CSSPropertyID::Float},
        {"font", CSSPropertyID::Font},
        {"font-family", CSSPropertyID::FontFamily},
        {"font-feature-settings", CSSPropertyID::FontFeatureSettings},
        {"font-kerning", CSSPropertyID::FontKerning},
        {"font-size", CSSPropertyID::FontSize},
        {"font-stretch", CSSPropertyID::FontStretch},
        {"font-style", CSSPropertyID::FontStyle},
        {"font-variant", CSSPropertyID::FontVariant},
        {"font-variant-caps", CSSPropertyID::FontVariantCaps},
        {"font-variant-east-asian", CSSPropertyID::FontVariantEastAsian},
        {"font-variant-emoji", CSSPropertyID::FontVariantEmoji},
        {"font-variant-ligatures", CSSPropertyID::FontVariantLigatures},
        {"font-variant-numeric", CSSPropertyID::FontVariantNumeric},
        {"font-variant-position", CSSPropertyID::FontVariantPosition},
        {"font-variation-settings", CSSPropertyID::FontVariationSettings},
        {"font-weight", CSSPropertyID::FontWeight},
        {"gap", CSSPropertyID::Gap},
        {"height", CSSPropertyID::Height},
        {"hyphens", CSSPropertyID::Hyphens},
        {"inline-size", CSSPropertyID::InlineSize},
        {"inset", CSSPropertyID::Inset},
        {"inset-block", CSSPropertyID::InsetBlock},
        {"inset-block-end", CSSPropertyID::InsetBlockEnd},
        {"inset-block-start", CSSPropertyID::InsetBlockStart},
        {"inset-inline", CSSPropertyID::InsetInline},
        {"inset-inline-end", CSSPropertyID::InsetInlineEnd},
        {"inset-inline-start", CSSPropertyID::InsetInlineStart},
        {"justify-content", CSSPropertyID::JustifyContent},
        {"left", CSSPropertyID::Left},
        {"letter-spacing", CSSPropertyID::LetterSpacing},
        {"line-height", CSSPropertyID::LineHeight},
        {"list-style", CSSPropertyID::ListStyle},
        {"list-style-image", CSSPropertyID::ListStyleImage},
        {"list-style-position", CSSPropertyID::ListStylePosition},
        {"list-style-type", CSSPropertyID::ListStyleType},
        {"margin", CSSPropertyID::Margin},
        {"margin-block", CSSPropertyID::MarginBlock},
        {"margin-block-end", CSSPropertyID::MarginBlockEnd},
        {"margin-block-start", CSSPropertyID::MarginBlockStart},
        {"margin-bottom", CSSPropertyID::MarginBottom},
        {"margin-inline", CSSPropertyID::MarginInline},
        {"margin-inline-end", CSSPropertyID::MarginInlineEnd},
        {"margin-inline-start", CSSPropertyID::MarginInlineStart},
        {"margin-left", CSSPropertyID::MarginLeft},
        {"margin-right", CSSPropertyID::MarginRight},
        {"margin-top", CSSPropertyID::MarginTop},
        {"marker", CSSPropertyID::Marker},
        {"marker-end", CSSPropertyID::MarkerEnd},
        {"marker-mid", CSSPropertyID::MarkerMid},
        {"marker-start", CSSPropertyID::MarkerStart},
        {"mask", CSSPropertyID::Mask},
        {"mask-type", CSSPropertyID::MaskType},
        {"max-block-size", CSSPropertyID::MaxBlockSize},
        {"max-height", CSSPropertyID::MaxHeight},
        {"max-inline-size", CSSPropertyID::MaxInlineSize},
        {"max-width", CSSPropertyID::MaxWidth},
        {"min-block-size", CSSPropertyID::MinBlockSize},
        {"min-height", CSSPropertyID::MinHeight},
        {"min-inline-size", CSSPropertyID::MinInlineSize},
        {"min-width", CSSPropertyID::MinWidth},
        {"mix-blend-mode", CSSPropertyID::MixBlendMode},
        {"negative", CSSPropertyID::Negative},
        {"object-fit", CSSPropertyID::ObjectFit},
        {"object-position", CSSPropertyID::ObjectPosition},
        {"opacity", CSSPropertyID::Opacity},
        {"order", CSSPropertyID::Order},
        {"orphans", CSSPropertyID::Orphans},
        {"outline", CSSPropertyID::Outline},
        {"outline-color", CSSPropertyID::OutlineColor},
        {"outline-offset", CSSPropertyID::OutlineOffset},
        {"outline-style", CSSPropertyID::OutlineStyle},
        {"outline-width", CSSPropertyID::OutlineWidth},
        {"overflow", CSSPropertyID::Overflow},
        {"overflow-wrap", CSSPropertyID::OverflowWrap},
        {"pad", CSSPropertyID::Pad},
        {"padding", CSSPropertyID::Padding},
        {"padding-block", CSSPropertyID::PaddingBlock},
        {"padding-block-end", CSSPropertyID::PaddingBlockEnd},
        {"padding-block-start", CSSPropertyID::PaddingBlockStart},
        {"padding-bottom", CSSPropertyID::PaddingBottom},
        {"padding-inline", CSSPropertyID::PaddingInline},
        {"padding-inline-end", CSSPropertyID::PaddingInlineEnd},
        {"padding-inline-start", CSSPropertyID::PaddingInlineStart},
        {"padding-left", CSSPropertyID::PaddingLeft},
        {"padding-right", CSSPropertyID::PaddingRight},
        {"padding-top", CSSPropertyID::PaddingTop},
        {"page", CSSPropertyID::Page},
        {"page-break-after", CSSPropertyID::PageBreakAfter},
        {"page-break-before", CSSPropertyID::PageBreakBefore},
        {"page-break-inside", CSSPropertyID::PageBreakInside},
        {"paint-order", CSSPropertyID::PaintOrder},
        {"position", CSSPropertyID::Position},
        {"prefix", CSSPropertyID::Prefix},
        {"quotes", CSSPropertyID::Quotes},
        {"r", CSSPropertyID::R},
        {"range", CSSPropertyID::Range},
        {"right", CSSPropertyID::Right},
        {"row-gap", CSSPropertyID::RowGap},
        {"rx", CSSPropertyID::Rx},
        {"ry", CSSPropertyID::Ry},
        {"size", CSSPropertyID::Size},
        {"src", CSSPropertyID::Src},
        {"stop-color", CSSPropertyID::StopColor},
        {"stop-opacity", CSSPropertyID::StopOpacity},
        {"stroke", CSSPropertyID::Stroke},
        {"stroke-dasharray", CSSPropertyID::StrokeDasharray},
        {"stroke-dashoffset", CSSPropertyID::StrokeDashoffset},
        {"stroke-linecap", CSSPropertyID::StrokeLinecap},
        {"stroke-linejoin", CSSPropertyID::StrokeLinejoin},
        {"stroke-miterlimit", CSSPropertyID::StrokeMiterlimit},
        {"stroke-opacity", CSSPropertyID::StrokeOpacity},
        {"stroke-width", CSSPropertyID::StrokeWidth},
        {"suffix", CSSPropertyID::Suffix},
        {"symbols", CSSPropertyID::Symbols},
        {"system", CSSPropertyID::System},
        {"tab-size", CSSPropertyID::TabSize},
        {"table-layout", CSSPropertyID::TableLayout},
        {"text-align", CSSPropertyID::TextAlign},
        {"text-anchor", CSSPropertyID::TextAnchor},
        {"text-decoration", CSSPropertyID::TextDecoration},
        {"text-decoration-color", CSSPropertyID::TextDecorationColor},
        {"text-decoration-line", CSSPropertyID::TextDecorationLine},
        {"text-decoration-style", CSSPropertyID::TextDecorationStyle},
        {"text-indent", CSSPropertyID::TextIndent},
        {"text-orientation", CSSPropertyID::TextOrientation},
        {"text-overflow", CSSPropertyID::TextOverflow},
        {"text-transform", CSSPropertyID::TextTransform},
        {"top", CSSPropertyID::Top},
        {"transform", CSSPropertyID::Transform},
        {"transform-origin", CSSPropertyID::TransformOrigin},
        {"unicode-bidi", CSSPropertyID::UnicodeBidi},
        {"unicode-range", CSSPropertyID::UnicodeRange},
        {"vector-effect", CSSPropertyID::VectorEffect},
        {"vertical-align", CSSPropertyID::VerticalAlign},
        {"visibility", CSSPropertyID::Visibility},
        {"white-space", CSSPropertyID::WhiteSpace},
        {"widows", CSSPropertyID::Widows},
        {"width", CSSPropertyID::Width},
        {"word-break", CSSPropertyID::WordBreak},
        {"word-spacing", CSSPropertyID::WordSpacing},
        {"writing-mode", CSSPropertyID::WritingMode},
        {"x", CSSPropertyID::X},
        {"y", CSSPropertyID::Y},
        {"z-index", CSSPropertyID::ZIndex}
    };

    char buffer[32];
    if(name.length() > sizeof(buffer))
        return CSSPropertyID::Unknown;
    for(size_t i = 0; i < name.length(); ++i) {
        buffer[i] = toLower(name[i]);
    }

    std::string_view lowerName(buffer, name.length());
    auto it = std::lower_bound(table, std::end(table), lowerName, [](const auto& item, const auto& name) { return item.name < name; });
    if(it != std::end(table) && it->name == lowerName)
        return it->value;
    return CSSPropertyID::Unknown;
}

} // namespace plutobook
