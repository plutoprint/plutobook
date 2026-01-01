/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "cssrule.h"
#include "cssparser.h"
#include "document.h"
#include "fontresource.h"
#include "imageresource.h"
#include "boxstyle.h"
#include "uastylesheet.h"
#include "stringutils.h"

#include <unicode/uiter.h>

namespace plutobook {

CSSLengthResolver::CSSLengthResolver(const Document* document, const Font* font)
    : m_document(document), m_font(font)
{
}

float CSSLengthResolver::resolveLength(const CSSValue& value) const
{
    if(is<CSSLengthValue>(value))
        return resolveLength(to<CSSLengthValue>(value));
    return to<CSSCalcValue>(value).resolve(*this);
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
    case CSSLengthUnits::Inches:
        return value * dpi;
    case CSSLengthUnits::Centimeters:
        return value * dpi / 2.54f;
    case CSSLengthUnits::Millimeters:
        return value * dpi / 25.4f;
    case CSSLengthUnits::Points:
        return value * dpi / 72.f;
    case CSSLengthUnits::Picas:
        return value * dpi / 6.f;
    case CSSLengthUnits::Ems:
        return value * emFontSize();
    case CSSLengthUnits::Exs:
        return value * exFontSize();
    case CSSLengthUnits::Rems:
        return value * remFontSize();
    case CSSLengthUnits::Chs:
        return value * chFontSize();
    case CSSLengthUnits::ViewportWidth:
        return value * viewportWidth() / 100.f;
    case CSSLengthUnits::ViewportHeight:
        return value * viewportHeight() / 100.f;
    case CSSLengthUnits::ViewportMin:
        return value * viewportMin() / 100.f;
    case CSSLengthUnits::ViewportMax:
        return value * viewportMax() / 100.f;
    default:
        assert(false);
    }

    return 0.f;
}

float CSSLengthResolver::emFontSize() const
{
    if(m_font == nullptr)
        return kMediumFontSize;
    return m_font->size();
}

float CSSLengthResolver::exFontSize() const
{
    if(m_font == nullptr)
        return kMediumFontSize / 2.f;
    if(auto fontData = m_font->primaryFont())
        return fontData->xHeight();
    return m_font->size() / 2.f;
}

float CSSLengthResolver::chFontSize() const
{
    if(m_font == nullptr)
        return kMediumFontSize / 2.f;
    if(auto fontData = m_font->primaryFont())
        return fontData->zeroWidth();
    return m_font->size() / 2.f;
}

float CSSLengthResolver::remFontSize() const
{
    if(m_document == nullptr)
        return kMediumFontSize;
    if(auto style = m_document->rootStyle())
        return style->fontSize();
    return kMediumFontSize;
}

float CSSLengthResolver::viewportWidth() const
{
    if(m_document)
        return m_document->viewportWidth();
    return 0.f;
}

float CSSLengthResolver::viewportHeight() const
{
    if(m_document)
        return m_document->viewportHeight();
    return 0.f;
}

float CSSLengthResolver::viewportMin() const
{
    if(m_document)
        return std::min(m_document->viewportWidth(), m_document->viewportHeight());
    return 0.f;
}

float CSSLengthResolver::viewportMax() const
{
    if(m_document)
        return std::max(m_document->viewportWidth(), m_document->viewportHeight());
    return 0.f;
}

float CSSCalcValue::resolve(const CSSLengthResolver& resolver) const
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
                return 0;
            auto right = stack.back();
            stack.pop_back();
            auto left = stack.back();
            stack.pop_back();

            switch(item.op) {
            case CSSCalcOperator::Add:
                if(right.units != left.units)
                    return 0;
                stack.emplace_back(left.value + right.value, right.units);
                break;
            case CSSCalcOperator::Sub:
                if(right.units != left.units)
                    return 0;
                stack.emplace_back(left.value - right.value, right.units);
                break;
            case CSSCalcOperator::Mul:
                if(right.units == CSSLengthUnits::Pixels && left.units == CSSLengthUnits::Pixels)
                    return 0;
                stack.emplace_back(left.value * right.value, std::max(left.units, right.units));
                break;
            case CSSCalcOperator::Div:
                if(right.units == CSSLengthUnits::Pixels || right.value == 0)
                    return 0;
                stack.emplace_back(left.value / right.value, left.units);
                break;
            case CSSCalcOperator::Min:
                if(right.units != left.units)
                    return 0;
                stack.emplace_back(std::min(left.value, right.value), right.units);
                break;
            case CSSCalcOperator::Max:
                if(right.units != left.units)
                    return 0;
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
            return 0;
        if(result.units == CSSLengthUnits::None && !m_unitless)
            return 0;
        return result.value;
    }

    return 0;
}

class CSSValuePool {
public:
    CSSValuePool();

    CSSInitialValue* initialValue() const { return m_initialValue; }
    CSSInheritValue* inheritValue() const { return m_inheritValue; }
    CSSUnsetValue* unsetValue() const { return m_unsetValue; }

    CSSIdentValue* identValue(CSSValueID id) const;

private:
    using CSSIdentValueList = std::pmr::vector<CSSIdentValue*>;
    Heap m_heap;
    CSSInitialValue* m_initialValue;
    CSSInheritValue* m_inheritValue;
    CSSUnsetValue* m_unsetValue;
    CSSIdentValueList m_identValues;
};

CSSValuePool::CSSValuePool()
    : m_heap(1024 * 8)
    , m_initialValue(new (&m_heap) CSSInitialValue)
    , m_inheritValue(new (&m_heap) CSSInheritValue)
    , m_unsetValue(new (&m_heap) CSSUnsetValue)
    , m_identValues(kNumCSSValueIDs, &m_heap)
{
    assert(CSSValueID::Unknown == static_cast<CSSValueID>(0));
    for(int i = 1; i < kNumCSSValueIDs; ++i) {
        const auto id = static_cast<CSSValueID>(i);
        m_identValues[i] = new (&m_heap) CSSIdentValue(id);
    }
}

CSSIdentValue* CSSValuePool::identValue(CSSValueID id) const
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

RefPtr<CSSCustomPropertyValue> CSSCustomPropertyValue::create(Heap* heap, const GlobalString& name, RefPtr<CSSVariableData> value)
{
    return adoptPtr(new (heap) CSSCustomPropertyValue(name, std::move(value)));
}

CSSCustomPropertyValue::CSSCustomPropertyValue(const GlobalString& name, RefPtr<CSSVariableData> value)
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
        if(auto imageResource = document->fetchImageResource(m_value)) {
            m_image = imageResource->image();
        }
    }

    return m_image;
}

CSSImageValue::CSSImageValue(Url value)
    : m_value(std::move(value))
{
}

bool CSSSimpleSelector::matchnth(int count) const
{
    auto [a, b] = m_matchPattern;
    if(a > 0)
        return count >= b && !((count - b) % a);
    if(a < 0)
        return count <= b && !((b - count) % -a);
    return count == b;
}

PseudoType CSSSimpleSelector::pseudoType() const
{
    switch(m_matchType) {
    case MatchType::PseudoElementBefore:
        return PseudoType::Before;
    case MatchType::PseudoElementAfter:
        return PseudoType::After;
    case MatchType::PseudoElementMarker:
        return PseudoType::Marker;
    case MatchType::PseudoElementFirstLetter:
        return PseudoType::FirstLetter;
    case MatchType::PseudoElementFirstLine:
        return PseudoType::FirstLine;
    case MatchType::PseudoPageFirst:
        return PseudoType::FirstPage;
    case MatchType::PseudoPageLeft:
        return PseudoType::LeftPage;
    case MatchType::PseudoPageRight:
        return PseudoType::RightPage;
    case MatchType::PseudoPageBlank:
        return PseudoType::BlankPage;
    default:
        return PseudoType::None;
    }
}

uint32_t CSSSimpleSelector::specificity() const
{
    switch(m_matchType) {
    case MatchType::Id:
        return 0x10000;
    case MatchType::Class:
    case MatchType::AttributeContains:
    case MatchType::AttributeDashEquals:
    case MatchType::AttributeEndsWith:
    case MatchType::AttributeEquals:
    case MatchType::AttributeHas:
    case MatchType::AttributeIncludes:
    case MatchType::AttributeStartsWith:
    case MatchType::PseudoClassActive:
    case MatchType::PseudoClassAnyLink:
    case MatchType::PseudoClassChecked:
    case MatchType::PseudoClassDisabled:
    case MatchType::PseudoClassEmpty:
    case MatchType::PseudoClassEnabled:
    case MatchType::PseudoClassFirstChild:
    case MatchType::PseudoClassFirstOfType:
    case MatchType::PseudoClassFocus:
    case MatchType::PseudoClassFocusVisible:
    case MatchType::PseudoClassFocusWithin:
    case MatchType::PseudoClassHover:
    case MatchType::PseudoClassLang:
    case MatchType::PseudoClassLastChild:
    case MatchType::PseudoClassLastOfType:
    case MatchType::PseudoClassLink:
    case MatchType::PseudoClassLocalLink:
    case MatchType::PseudoClassNthChild:
    case MatchType::PseudoClassNthLastChild:
    case MatchType::PseudoClassNthLastOfType:
    case MatchType::PseudoClassNthOfType:
    case MatchType::PseudoClassOnlyChild:
    case MatchType::PseudoClassOnlyOfType:
    case MatchType::PseudoClassRoot:
    case MatchType::PseudoClassScope:
    case MatchType::PseudoClassTarget:
    case MatchType::PseudoClassTargetWithin:
    case MatchType::PseudoClassVisited:
        return 0x100;
    case MatchType::Tag:
    case MatchType::PseudoElementAfter:
    case MatchType::PseudoElementBefore:
    case MatchType::PseudoElementFirstLetter:
    case MatchType::PseudoElementFirstLine:
    case MatchType::PseudoElementMarker:
        return 0x1;
    case MatchType::PseudoClassIs:
    case MatchType::PseudoClassNot:
    case MatchType::PseudoClassHas: {
        uint32_t maxSpecificity = 0;
        for(const auto& subSelector : m_subSelectors) {
            uint32_t specificity = 0x0;
            for(const auto& complexSelector : subSelector) {
                for(const auto& simpleSelector : complexSelector.compoundSelector()) {
                    specificity += simpleSelector.specificity();
                }
            }

            maxSpecificity = std::max(specificity, maxSpecificity);
        }

        return maxSpecificity;
    }

    default:
        return 0x0;
    }
}

bool CSSRuleData::match(const Element* element, PseudoType pseudoType) const
{
    return matchSelector(element, pseudoType, *m_selector);
}

bool CSSRuleData::matchSelector(const Element* element, PseudoType pseudoType, const CSSSelector& selector)
{
    assert(!selector.empty());
    auto it = selector.begin();
    auto end = selector.end();
    if(!matchCompoundSelector(element, pseudoType, it->compoundSelector())) {
        return false;
    }

    auto combinator = it->combinator();
    ++it;

    while(it != end) {
        switch(combinator) {
        case CSSComplexSelector::Combinator::Descendant:
        case CSSComplexSelector::Combinator::Child:
            element = element->parentElement();
            break;
        case CSSComplexSelector::Combinator::DirectAdjacent:
        case CSSComplexSelector::Combinator::InDirectAdjacent:
            element = element->previousSiblingElement();
            break;
        case CSSComplexSelector::Combinator::None:
            assert(false);
        }

        if(element == nullptr)
            return false;
        if(matchCompoundSelector(element, PseudoType::None, it->compoundSelector())) {
            combinator = it->combinator();
            ++it;
        } else if(combinator != CSSComplexSelector::Combinator::Descendant
            && combinator != CSSComplexSelector::Combinator::InDirectAdjacent) {
            return false;
        }
    }

    return true;
}

bool CSSRuleData::matchCompoundSelector(const Element* element, PseudoType pseudoType, const CSSCompoundSelector& selector)
{
    assert(!selector.empty());
    auto it = selector.begin();
    auto end = selector.end();
    if(pseudoType != PseudoType::None) {
        if(pseudoType != it->pseudoType())
            return false;
        ++it;
    }

    for(; it != end; ++it) {
        if(!matchSimpleSelector(element, *it)) {
            return false;
        }
    }

    return true;
}

bool CSSRuleData::matchSimpleSelector(const Element* element, const CSSSimpleSelector& selector)
{
    switch(selector.matchType()) {
    case CSSSimpleSelector::MatchType::Universal:
        return true;
    case CSSSimpleSelector::MatchType::Namespace:
        return matchNamespaceSelector(element, selector);
    case CSSSimpleSelector::MatchType::Tag:
        return matchTagSelector(element, selector);
    case CSSSimpleSelector::MatchType::Id:
        return matchIdSelector(element, selector);
    case CSSSimpleSelector::MatchType::Class:
        return matchClassSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeHas:
        return matchAttributeHasSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeEquals:
        return matchAttributeEqualsSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeIncludes:
        return matchAttributeIncludesSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeContains:
        return matchAttributeContainsSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeDashEquals:
        return matchAttributeDashEqualsSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeStartsWith:
        return matchAttributeStartsWithSelector(element, selector);
    case CSSSimpleSelector::MatchType::AttributeEndsWith:
        return matchAttributeEndsWithSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassIs:
    case CSSSimpleSelector::MatchType::PseudoClassWhere:
        return matchPseudoClassIsSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassNot:
        return matchPseudoClassNotSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassHas:
        return matchPseudoClassHasSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassLink:
    case CSSSimpleSelector::MatchType::PseudoClassAnyLink:
        return matchPseudoClassLinkSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassLocalLink:
        return matchPseudoClassLocalLinkSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassEnabled:
        return matchPseudoClassEnabledSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassDisabled:
        return matchPseudoClassDisabledSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassChecked:
        return matchPseudoClassCheckedSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassLang:
        return matchPseudoClassLangSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassRoot:
    case CSSSimpleSelector::MatchType::PseudoClassScope:
        return matchPseudoClassRootSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassEmpty:
        return matchPseudoClassEmptySelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassFirstChild:
        return matchPseudoClassFirstChildSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassLastChild:
        return matchPseudoClassLastChildSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassOnlyChild:
        return matchPseudoClassOnlyChildSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassFirstOfType:
        return matchPseudoClassFirstOfTypeSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassLastOfType:
        return matchPseudoClassLastOfTypeSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassOnlyOfType:
        return matchPseudoClassOnlyOfTypeSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassNthChild:
        return matchPseudoClassNthChildSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassNthLastChild:
        return matchPseudoClassNthLastChildSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassNthOfType:
        return matchPseudoClassNthOfTypeSelector(element, selector);
    case CSSSimpleSelector::MatchType::PseudoClassNthLastOfType:
        return matchPseudoClassNthLastOfTypeSelector(element, selector);
    default:
        return false;
    }
}

bool CSSRuleData::matchNamespaceSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return selector.name() == starGlo || element->namespaceURI() == selector.name();
}

bool CSSRuleData::matchTagSelector(const Element* element, const CSSSimpleSelector& selector)
{
    if(element->isCaseSensitive())
        return element->tagName() == selector.name();
    return equalsIgnoringCase(element->tagName(), selector.name());
}

bool CSSRuleData::matchIdSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->id() == selector.value();
}

bool CSSRuleData::matchClassSelector(const Element* element, const CSSSimpleSelector& selector)
{
    for(const auto& name : element->classNames()) {
        if(name == selector.value()) {
            return true;
        }
    }

    return false;
}

bool CSSRuleData::matchAttributeHasSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->findAttributePossiblyIgnoringCase(selector.name());
}

bool CSSRuleData::matchAttributeEqualsSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttributePossiblyIgnoringCase(selector.name());
    if(attribute == nullptr)
        return false;
    return equals(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchAttributeIncludesSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttributePossiblyIgnoringCase(selector.name());
    if(attribute == nullptr)
        return false;
    return includes(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchAttributeContainsSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttributePossiblyIgnoringCase(selector.name());
    if(attribute == nullptr)
        return false;
    return contains(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchAttributeDashEqualsSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttributePossiblyIgnoringCase(selector.name());
    if(attribute == nullptr)
        return false;
    return dashequals(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchAttributeStartsWithSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttributePossiblyIgnoringCase(selector.name());
    if(attribute == nullptr)
        return false;
    return startswith(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchAttributeEndsWithSelector(const Element* element, const CSSSimpleSelector& selector)
{
    auto attribute = element->findAttributePossiblyIgnoringCase(selector.name());
    if(attribute == nullptr)
        return false;
    return endswith(attribute->value(), selector.value(), selector.isCaseSensitive());
}

bool CSSRuleData::matchPseudoClassIsSelector(const Element* element, const CSSSimpleSelector& selector)
{
    for(const auto& subSelector : selector.subSelectors()) {
        if(matchSelector(element, PseudoType::None, subSelector)) {
            return true;
        }
    }

    return false;
}

bool CSSRuleData::matchPseudoClassNotSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return !matchPseudoClassIsSelector(element, selector);
}

bool CSSRuleData::matchPseudoClassHasSelector(const Element* element, const CSSSimpleSelector& selector)
{
    for(const auto& subSelector : selector.subSelectors()) {
        int maxDepth = 0;
        auto combinator = CSSComplexSelector::Combinator::None;
        for(const auto& selector : subSelector) {
            combinator = selector.combinator();
            ++maxDepth;
        }

        if(combinator == CSSComplexSelector::Combinator::None)
            combinator = CSSComplexSelector::Combinator::Descendant;
        auto checkDescendants = [&](const Element* descendant) {
            int depth = 0;
            do {
                if(matchSelector(descendant, PseudoType::None, subSelector))
                    return true;
                if((combinator == CSSComplexSelector::Combinator::Descendant || depth < maxDepth - 1)
                    && descendant->firstChildElement()) {
                    descendant = descendant->firstChildElement();
                    ++depth;
                    continue;
                }

                while(depth > 0) {
                    if(descendant->nextSiblingElement()) {
                        descendant = descendant->nextSiblingElement();
                        break;
                    }

                    descendant = descendant->parentElement();
                    --depth;
                }
            } while(descendant && depth > 0);
            return false;
        };

        switch(combinator) {
        case CSSComplexSelector::Combinator::Descendant:
        case CSSComplexSelector::Combinator::Child:
            for(auto child = element->firstChildElement(); child; child = child->nextSiblingElement()) {
                if(checkDescendants(child)) {
                    return true;
                }
            }

            break;
        case CSSComplexSelector::Combinator::DirectAdjacent:
        case CSSComplexSelector::Combinator::InDirectAdjacent:
            for(auto sibling = element->nextSiblingElement(); sibling; sibling = sibling->nextSiblingElement()) {
                if(checkDescendants(sibling))
                    return true;
                if(combinator == CSSComplexSelector::Combinator::DirectAdjacent) {
                    break;
                }
            }

            break;
        case CSSComplexSelector::Combinator::None:
            assert(false);
        }
    }

    return false;
}

bool CSSRuleData::matchPseudoClassLinkSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == aTag && element->hasAttribute(hrefAttr);
}

bool CSSRuleData::matchPseudoClassLocalLinkSelector(const Element* element, const CSSSimpleSelector& selector)
{
    if(matchPseudoClassLinkSelector(element, selector)) {
        const auto& baseUrl = element->document()->baseUrl();
        auto completeUrl = element->getUrlAttribute(hrefAttr);
        return baseUrl == completeUrl.base();
    }

    return false;
}

bool CSSRuleData::matchPseudoClassEnabledSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == inputTag && element->hasAttribute(enabledAttr);
}

bool CSSRuleData::matchPseudoClassDisabledSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == inputTag && element->hasAttribute(disabledAttr);
}

bool CSSRuleData::matchPseudoClassCheckedSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return element->tagName() == inputTag && element->hasAttribute(checkedAttr);
}

bool CSSRuleData::matchPseudoClassLangSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return dashequals(element->lang(), selector.value(), false);
}

bool CSSRuleData::matchPseudoClassRootSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return !element->parentElement();
}

bool CSSRuleData::matchPseudoClassEmptySelector(const Element* element, const CSSSimpleSelector& selector)
{
    return !element->firstChild();
}

bool CSSRuleData::matchPseudoClassFirstChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return !element->previousSiblingElement();
}

bool CSSRuleData::matchPseudoClassLastChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return !element->nextSiblingElement();
}

bool CSSRuleData::matchPseudoClassOnlyChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return matchPseudoClassFirstChildSelector(element, selector) && matchPseudoClassLastChildSelector(element, selector);
}

bool CSSRuleData::matchPseudoClassFirstOfTypeSelector(const Element* element, const CSSSimpleSelector& selector)
{
    for(auto sibling = element->previousSiblingElement(); sibling; sibling = sibling->previousSiblingElement()) {
        if(sibling->isOfType(element->namespaceURI(), element->tagName())) {
            return false;
        }
    }

    return true;
}

bool CSSRuleData::matchPseudoClassLastOfTypeSelector(const Element* element, const CSSSimpleSelector& selector)
{
    for(auto sibling = element->nextSiblingElement(); sibling; sibling = sibling->nextSiblingElement()) {
        if(sibling->isOfType(element->namespaceURI(), element->tagName())) {
            return false;
        }
    }

    return true;
}

bool CSSRuleData::matchPseudoClassOnlyOfTypeSelector(const Element* element, const CSSSimpleSelector& selector)
{
    return matchPseudoClassFirstOfTypeSelector(element, selector) && matchPseudoClassLastOfTypeSelector(element, selector);
}

bool CSSRuleData::matchPseudoClassNthChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    int index = 0;
    for(auto sibling = element->previousSiblingElement(); sibling; sibling = sibling->previousSiblingElement())
        ++index;
    return selector.matchnth(index + 1);
}

bool CSSRuleData::matchPseudoClassNthLastChildSelector(const Element* element, const CSSSimpleSelector& selector)
{
    int index = 0;
    for(auto sibling = element->nextSiblingElement(); sibling; sibling = sibling->nextSiblingElement())
        ++index;
    return selector.matchnth(index + 1);
}

bool CSSRuleData::matchPseudoClassNthOfTypeSelector(const Element* element, const CSSSimpleSelector& selector)
{
    int index = 0;
    for(auto sibling = element->previousSiblingElement(); sibling; sibling = sibling->previousSiblingElement()) {
        if(sibling->isOfType(element->namespaceURI(), element->tagName())) {
            ++index;
        }
    }

    return selector.matchnth(index + 1);
}

bool CSSRuleData::matchPseudoClassNthLastOfTypeSelector(const Element* element, const CSSSimpleSelector& selector)
{
    int index = 0;
    for(auto sibling = element->nextSiblingElement(); sibling; sibling = sibling->nextSiblingElement()) {
        if(sibling->isOfType(element->namespaceURI(), element->tagName())) {
            ++index;
        }
    }

    return selector.matchnth(index + 1);
}

bool CSSPageRuleData::match(const GlobalString& pageName, uint32_t pageIndex, PseudoType pseudoType) const
{
    if(m_selector) {
        for(const auto& sel : *m_selector) {
            if(!matchSelector(pageName, pageIndex, pseudoType, sel)) {
                return false;
            }
        }
    }

    return true;
}

bool CSSPageRuleData::matchSelector(const GlobalString& pageName, uint32_t pageIndex, PseudoType pseudoType, const CSSSimpleSelector& selector)
{
    switch(selector.matchType()) {
    case CSSSimpleSelector::MatchType::PseudoPageName:
        return pageName == selector.name();
    case CSSSimpleSelector::MatchType::PseudoPageFirst:
        return pseudoType == PseudoType::FirstPage;
    case CSSSimpleSelector::MatchType::PseudoPageLeft:
        return pseudoType == PseudoType::LeftPage;
    case CSSSimpleSelector::MatchType::PseudoPageRight:
        return pseudoType == PseudoType::RightPage;
    case CSSSimpleSelector::MatchType::PseudoPageBlank:
        return pseudoType == PseudoType::BlankPage;
    case CSSSimpleSelector::MatchType::PseudoPageNth:
        return selector.matchnth(pageIndex + 1);
    default:
        assert(false);
    }

    return false;
}

RefPtr<CSSCounterStyle> CSSCounterStyle::create(Heap* heap, RefPtr<CSSCounterStyleRule> rule)
{
    return adoptPtr(new (heap) CSSCounterStyle(std::move(rule)));
}

static void cyclicAlgorithm(int value, size_t numSymbols, std::vector<size_t>& indexes)
{
    assert(numSymbols > 0);
    value %= numSymbols;
    value -= 1;
    if(value < 0) {
        value += numSymbols;
    }

    indexes.push_back(value);
}

static void fixedAlgorithm(int value, int firstSymbolValue, size_t numSymbols, std::vector<size_t>& indexes)
{
    assert(numSymbols > 0);
    if(value < firstSymbolValue || value - firstSymbolValue >= numSymbols)
        return;
    indexes.push_back(value - firstSymbolValue);
}

static void symbolicAlgorithm(unsigned value, size_t numSymbols, std::vector<size_t>& indexes)
{
    assert(numSymbols > 0);
    if(value == 0)
        return;
    size_t index = (value - 1) % numSymbols;
    size_t repetitions = (value + numSymbols - 1) / numSymbols;
    for(size_t i = 0; i < repetitions; ++i) {
        indexes.push_back(index);
    }
}

static void alphabeticAlgorithm(unsigned value, size_t numSymbols, std::vector<size_t>& indexes)
{
    assert(numSymbols > 0);
    if(value == 0 || numSymbols == 1)
        return;
    while(value > 0) {
        value -= 1;
        indexes.push_back(value % numSymbols);
        value /= numSymbols;
    }

    std::reverse(indexes.begin(), indexes.end());
}

static void numericAlgorithm(unsigned value, size_t numSymbols, std::vector<size_t>& indexes)
{
    assert(numSymbols > 0);
    if(numSymbols == 1)
        return;
    if(value == 0) {
        indexes.push_back(0);
        return;
    }

    while(value > 0) {
        indexes.push_back(value % numSymbols);
        value /= numSymbols;
    }

    std::reverse(indexes.begin(), indexes.end());
}

static const HeapString& counterStyleSymbol(const CSSValue& value)
{
    if(is<CSSStringValue>(value))
        return to<CSSStringValue>(value).value();
    if(is<CSSCustomIdentValue>(value))
        return to<CSSCustomIdentValue>(value).value();
    return emptyGlo;
}

std::string CSSCounterStyle::generateInitialRepresentation(int value) const
{
    std::string representation;
    if(system() == CSSValueID::Additive) {
        if(m_additiveSymbols == nullptr)
            return representation;
        if(value == 0) {
            for(const auto& symbol : *m_additiveSymbols) {
                const auto& pair = to<CSSPairValue>(*symbol);
                const auto& weight = to<CSSIntegerValue>(*pair.first());
                if(weight.value() == 0) {
                    representation += counterStyleSymbol(*pair.second());
                    break;
                }
            }
        } else {
            for(const auto& symbol : *m_additiveSymbols) {
                const auto& pair = to<CSSPairValue>(*symbol);
                const auto& weight = to<CSSIntegerValue>(*pair.first());
                if(weight.value() == 0)
                    continue;
                size_t repetitions = value / weight.value();
                for(size_t i = 0; i < repetitions; ++i)
                    representation += counterStyleSymbol(*pair.second());
                value -= repetitions * weight.value();
                if(value == 0) {
                    break;
                }
            }

            if(value > 0) {
                representation.clear();
            }
        }

        return representation;
    }

    if(m_symbols == nullptr)
        return representation;
    std::vector<size_t> indexes;
    switch(system()) {
    case CSSValueID::Cyclic:
        cyclicAlgorithm(value, m_symbols->size(), indexes);
        break;
    case CSSValueID::Fixed:
        fixedAlgorithm(value, m_fixed->value(), m_symbols->size(), indexes);
        break;
    case CSSValueID::Numeric:
        numericAlgorithm(value, m_symbols->size(), indexes);
        break;
    case CSSValueID::Symbolic:
        symbolicAlgorithm(value, m_symbols->size(), indexes);
        break;
    case CSSValueID::Alphabetic:
        alphabeticAlgorithm(value, m_symbols->size(), indexes);
        break;
    default:
        assert(false);
    }

    for(auto index : indexes) {
        representation += counterStyleSymbol(*m_symbols->at(index));
    }

    return representation;
}

std::string CSSCounterStyle::generateFallbackRepresentation(int value) const
{
    if(m_fallbackStyle == nullptr)
        return defaultStyle().generateRepresentation(value);
    auto fallbackStyle = std::move(m_fallbackStyle);
    auto representation = fallbackStyle->generateRepresentation(value);
    m_fallbackStyle = std::move(fallbackStyle);
    return representation;
}

static size_t counterStyleSymbolLength(const std::string_view& value)
{
    UCharIterator it;
    uiter_setUTF8(&it, value.data(), value.length());

    size_t count = 0;
    while(it.hasNext(&it)) {
        uiter_next32(&it);
        ++count;
    }

    return count;
}

std::string CSSCounterStyle::generateRepresentation(int value) const
{
    if(!rangeContains(value))
        return generateFallbackRepresentation(value);
    auto initialRepresentation = generateInitialRepresentation(std::abs(value));
    if(initialRepresentation.empty()) {
        return generateFallbackRepresentation(value);
    }

    std::string_view negativePrefix("-");
    std::string_view negativeSuffix;
    if(m_negative && needsNegativeSign(value)) {
        if(auto pair = to<CSSPairValue>(m_negative)) {
            negativePrefix = counterStyleSymbol(*pair->first());
            negativeSuffix = counterStyleSymbol(*pair->second());
        } else {
            negativePrefix = counterStyleSymbol(*m_negative);
        }
    }

    size_t padLength = 0;
    std::string_view padSymbol;
    if(m_pad) {
        padLength = to<CSSIntegerValue>(*m_pad->first()).value();
        padSymbol = counterStyleSymbol(*m_pad->second());
    }

    auto initialLength = counterStyleSymbolLength(initialRepresentation);
    if(needsNegativeSign(value)) {
        initialLength += counterStyleSymbolLength(negativePrefix);
        initialLength += counterStyleSymbolLength(negativeSuffix);
    }

    size_t padRepetitions = 0;
    if(padLength > initialLength) {
        padRepetitions = padLength - initialLength;
    }

    std::string representation;
    if(needsNegativeSign(value))
        representation += negativePrefix;
    for(size_t i = 0; i < padRepetitions; ++i)
        representation += padSymbol;
    representation += initialRepresentation;
    if(needsNegativeSign(value))
        representation += negativeSuffix;
    return representation;
}

bool CSSCounterStyle::rangeContains(int value) const
{
    if(m_range == nullptr) {
        switch(system()) {
        case CSSValueID::Cyclic:
        case CSSValueID::Numeric:
        case CSSValueID::Fixed:
            return true;
        case CSSValueID::Symbolic:
        case CSSValueID::Alphabetic:
            return value >= 1;
        case CSSValueID::Additive:
            return value >= 0;
        default:
            assert(false);
        }
    }

    for(const auto& range : *m_range) {
        const auto& bounds = to<CSSPairValue>(*range);
        auto lowerBound = std::numeric_limits<int>::min();
        auto upperBound = std::numeric_limits<int>::max();
        if(auto bound = to<CSSIntegerValue>(bounds.first()))
            lowerBound = bound->value();
        if(auto bound = to<CSSIntegerValue>(bounds.second()))
            upperBound = bound->value();
        if(value >= lowerBound && value <= upperBound) {
            return true;
        }
    }

    return false;
}

bool CSSCounterStyle::needsNegativeSign(int value) const
{
    if(value < 0) {
        switch(system()) {
        case CSSValueID::Symbolic:
        case CSSValueID::Alphabetic:
        case CSSValueID::Numeric:
        case CSSValueID::Additive:
            return true;
        case CSSValueID::Cyclic:
        case CSSValueID::Fixed:
            return false;
        default:
            assert(false);
        }
    }

    return false;
}

const GlobalString& CSSCounterStyle::name() const
{
    return m_rule->name();
}

const GlobalString& CSSCounterStyle::extendsName() const
{
    if(m_extends)
        return m_extends->value();
    return emptyGlo;
}

const GlobalString& CSSCounterStyle::fallbackName() const
{
    static const GlobalString defaultFallback("decimal");
    if(m_fallback)
        return m_fallback->value();
    return defaultFallback;
}

const CSSValueID CSSCounterStyle::system() const
{
    if(m_system)
        return m_system->value();
    return CSSValueID::Symbolic;
}

const HeapString& CSSCounterStyle::prefix() const
{
    if(m_prefix)
        return counterStyleSymbol(*m_prefix);
    return emptyGlo;
}

const HeapString& CSSCounterStyle::suffix() const
{
    static const GlobalString defaultSuffix(". ");
    if(m_suffix)
        return counterStyleSymbol(*m_suffix);
    return defaultSuffix;
}

void CSSCounterStyle::extend(const CSSCounterStyle& extended)
{
    assert(m_system && m_system->value() == CSSValueID::Extends);
    m_system = extended.m_system;
    m_fixed = extended.m_fixed;
    m_symbols = extended.m_symbols;
    m_additiveSymbols = extended.m_additiveSymbols;

    if(!m_negative) { m_negative = extended.m_negative; }
    if(!m_prefix) { m_prefix = extended.m_prefix; }
    if(!m_suffix) { m_suffix = extended.m_suffix; }
    if(!m_range) { m_range = extended.m_range; }
    if(!m_pad) { m_pad = extended.m_pad; }
}

CSSCounterStyle& CSSCounterStyle::defaultStyle()
{
    static CSSCounterStyle* defaultStyle = []() {
        const GlobalString decimal("decimal");
        return userAgentCounterStyleMap()->findCounterStyle(decimal);
    }();

    return *defaultStyle;
}

CSSCounterStyle::CSSCounterStyle(RefPtr<CSSCounterStyleRule> rule)
    : m_rule(std::move(rule))
{
    for(const auto& property : m_rule->properties()) {
        switch(property.id()) {
        case CSSPropertyID::System: {
            m_system = to<CSSIdentValue>(property.value());
            if(m_system == nullptr) {
                const auto& pair = to<CSSPairValue>(*property.value());
                m_system = to<CSSIdentValue>(*pair.first());
                if(m_system->value() == CSSValueID::Fixed) {
                    m_fixed = to<CSSIntegerValue>(*pair.second());
                } else {
                    m_extends = to<CSSCustomIdentValue>(*pair.second());
                }
            }

            break;
        }

        case CSSPropertyID::Symbols:
            m_symbols = to<CSSListValue>(*property.value());
            break;
        case CSSPropertyID::AdditiveSymbols:
            m_additiveSymbols = to<CSSListValue>(*property.value());
            break;
        case CSSPropertyID::Fallback:
            m_fallback = to<CSSCustomIdentValue>(*property.value());
            break;
        case CSSPropertyID::Pad:
            m_pad = to<CSSPairValue>(*property.value());
            break;
        case CSSPropertyID::Range:
            m_range = to<CSSListValue>(property.value());
            break;
        case CSSPropertyID::Negative:
            m_negative = property.value();
            break;
        case CSSPropertyID::Prefix:
            m_prefix = property.value();
            break;
        case CSSPropertyID::Suffix:
            m_suffix = property.value();
            break;
        default:
            assert(false);
        }
    }
}

std::unique_ptr<CSSCounterStyleMap> CSSCounterStyleMap::create(Heap* heap, const CSSRuleList& rules, const CSSCounterStyleMap* parent)
{
    return std::unique_ptr<CSSCounterStyleMap>(new (heap) CSSCounterStyleMap(heap, rules, parent));
}

CSSCounterStyle* CSSCounterStyleMap::findCounterStyle(const GlobalString& name) const
{
    auto it = m_counterStyles.find(name);
    if(it != m_counterStyles.end())
        return it->second.get();
    if(m_parent == nullptr)
        return nullptr;
    return m_parent->findCounterStyle(name);
}

CSSCounterStyleMap::CSSCounterStyleMap(Heap* heap, const CSSRuleList& rules, const CSSCounterStyleMap* parent)
    : m_parent(parent), m_counterStyles(heap)
{
    for(const auto& rule : rules) {
        auto& counterStyleRule = to<CSSCounterStyleRule>(*rule);
        auto counterStyle = CSSCounterStyle::create(heap, counterStyleRule);
        m_counterStyles.emplace(counterStyle->name(), std::move(counterStyle));
    }

    for(const auto& [name, style] : m_counterStyles) {
        if(style->system() == CSSValueID::Extends) {
            std::set<CSSCounterStyle*> unresolvedStyles;
            std::vector<CSSCounterStyle*> extendsStyles;

            extendsStyles.push_back(style.get());
            auto currentStyle = extendsStyles.back();
            do {
                unresolvedStyles.insert(currentStyle);
                extendsStyles.push_back(findCounterStyle(currentStyle->extendsName()));
                currentStyle = extendsStyles.back();
            } while(currentStyle && currentStyle->system() == CSSValueID::Extends && !unresolvedStyles.contains(currentStyle));

            if(currentStyle && currentStyle->system() == CSSValueID::Extends) {
                assert(parent != nullptr);
                do {
                    extendsStyles.back()->extend(CSSCounterStyle::defaultStyle());
                    extendsStyles.pop_back();
                } while(currentStyle != extendsStyles.back());
            }

            while(extendsStyles.size() > 1) {
                extendsStyles.pop_back();
                if(currentStyle == nullptr) {
                    assert(parent != nullptr);
                    extendsStyles.back()->extend(CSSCounterStyle::defaultStyle());
                } else {
                    extendsStyles.back()->extend(*currentStyle);
                }

                currentStyle = extendsStyles.back();
            }
        }

        if(auto fallbackStyle = findCounterStyle(style->fallbackName())) {
            style->setFallbackStyle(*fallbackStyle);
        } else {
            assert(parent != nullptr);
            style->setFallbackStyle(CSSCounterStyle::defaultStyle());
        }
    }
}

const CSSCounterStyleMap* userAgentCounterStyleMap()
{
    static std::unique_ptr<CSSCounterStyleMap> counterStyleMap = []() {
        static Heap heap(1024 * 96);
        CSSParserContext context(nullptr, CSSStyleOrigin::UserAgent, ResourceLoader::baseUrl());
        CSSParser parser(context, &heap);
        CSSRuleList rules(parser.parseSheet(kUserAgentCounterStyle));
        return CSSCounterStyleMap::create(&heap, rules, nullptr);
    }();

    return counterStyleMap.get();
}

} // namespace plutobook
