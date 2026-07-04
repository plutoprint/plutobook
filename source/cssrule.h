/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_CSSRULE_H
#define PLUTOBOOK_CSSRULE_H

#include "cssproperty.h"

#include <map>
#include <forward_list>
#include <memory>
#include <variant>

namespace plutobook {

class CSSSimpleSelector;
class CSSComplexSelector;

using CSSCompoundSelector = std::pmr::forward_list<CSSSimpleSelector>;
using CSSSelector = std::pmr::forward_list<CSSComplexSelector>;

using CSSCompoundSelectorList = std::pmr::forward_list<CSSCompoundSelector>;
using CSSSelectorList = std::pmr::forward_list<CSSSelector>;

using CSSPageSelector = CSSCompoundSelector;
using CSSPageSelectorList = CSSCompoundSelectorList;

enum class PseudoType : uint8_t;

class CSSSimpleSelector {
public:
    enum class MatchType {
        Universal,
        Namespace,
        Tag,
        Id,
        Class,
        AttributeContains,
        AttributeDashEquals,
        AttributeEndsWith,
        AttributeEquals,
        AttributeHas,
        AttributeIncludes,
        AttributeStartsWith,
        PseudoClassActive,
        PseudoClassAnyLink,
        PseudoClassChecked,
        PseudoClassDisabled,
        PseudoClassEmpty,
        PseudoClassEnabled,
        PseudoClassFirstChild,
        PseudoClassFirstOfType,
        PseudoClassFocus,
        PseudoClassFocusVisible,
        PseudoClassFocusWithin,
        PseudoClassHas,
        PseudoClassHover,
        PseudoClassIs,
        PseudoClassLang,
        PseudoClassLastChild,
        PseudoClassLastOfType,
        PseudoClassLink,
        PseudoClassLocalLink,
        PseudoClassNot,
        PseudoClassNthChild,
        PseudoClassNthLastChild,
        PseudoClassNthLastOfType,
        PseudoClassNthOfType,
        PseudoClassOnlyChild,
        PseudoClassOnlyOfType,
        PseudoClassRoot,
        PseudoClassScope,
        PseudoClassTarget,
        PseudoClassTargetWithin,
        PseudoClassVisited,
        PseudoClassWhere,
        PseudoElementAfter,
        PseudoElementBefore,
        PseudoElementFirstLetter,
        PseudoElementFirstLine,
        PseudoElementMarker,
        PseudoPageBlank,
        PseudoPageFirst,
        PseudoPageLeft,
        PseudoPageName,
        PseudoPageNth,
        PseudoPageRight
    };

    enum class AttributeCaseType {
        Sensitive,
        InSensitive
    };

    using MatchPattern = std::pair<int, int>;

    explicit CSSSimpleSelector(MatchType matchType) : m_matchType(matchType) {}
    CSSSimpleSelector(MatchType matchType, const GlobalString& name) : m_matchType(matchType), m_name(name) {}
    CSSSimpleSelector(MatchType matchType, const HeapString& value) : m_matchType(matchType), m_value(value) {}
    CSSSimpleSelector(MatchType matchType, const MatchPattern& matchPattern) : m_matchType(matchType), m_value(matchPattern) {}
    CSSSimpleSelector(MatchType matchType, CSSSelectorList subSelectors) : m_matchType(matchType), m_value(std::move(subSelectors)) {}
    CSSSimpleSelector(MatchType matchType, AttributeCaseType attributeCaseType, const GlobalString& name, const HeapString& value)
        : m_matchType(matchType), m_attributeCaseType(attributeCaseType), m_name(name), m_value(value)
    {}

    MatchType matchType() const { return m_matchType; }
    AttributeCaseType attributeCaseType() const { return m_attributeCaseType; }
    const GlobalString& name() const { return m_name; }

    const HeapString& value() const { return std::get<HeapString>(m_value); }
    const MatchPattern& matchPattern() const { return std::get<MatchPattern>(m_value); }
    const CSSSelectorList& subSelectors() const { return std::get<CSSSelectorList>(m_value); }

    bool isCaseSensitive() const { return m_attributeCaseType == AttributeCaseType::Sensitive; }

    bool matchNth(int count) const;
    PseudoType pseudoType() const;
    uint32_t specificity() const;

private:
    using Value = std::variant<HeapString, MatchPattern, CSSSelectorList>;
    MatchType m_matchType;
    AttributeCaseType m_attributeCaseType;
    GlobalString m_name;
    Value m_value;
};

class CSSComplexSelector {
public:
    enum class Combinator {
        None,
        Descendant,
        Child,
        DirectAdjacent,
        InDirectAdjacent
    };

    CSSComplexSelector(Combinator combinator, CSSCompoundSelector compoundSelector)
        : m_specificity(0), m_combinator(combinator)
        , m_compoundSelector(std::move(compoundSelector))
    {
        assert(!m_compoundSelector.empty());
        for(const auto& simpleSelector : m_compoundSelector) {
            m_specificity += simpleSelector.specificity();
        }
    }

    uint32_t specificity() const { return m_specificity; }
    Combinator combinator() const { return m_combinator; }
    const CSSCompoundSelector& compoundSelector() const { return m_compoundSelector; }

private:
    uint32_t m_specificity;
    Combinator m_combinator;
    CSSCompoundSelector m_compoundSelector;
};

enum class CSSRuleType : uint8_t {
    Style,
    Media,
    Import,
    Namespace,
    FontFace,
    CounterStyle,
    Page,
    PageMargin
};

class CSSRule : public HeapMember, public RefCounted<CSSRule> {
public:
    CSSRule() = default;
    virtual ~CSSRule() = default;
    virtual CSSRuleType type() const = 0;
};

using CSSRuleList = std::pmr::vector<RefPtr<CSSRule>>;

class CSSStyleRule final : public CSSRule {
public:
    static RefPtr<CSSStyleRule> create(Heap* heap, CSSSelectorList selectors, CSSPropertyList properties);

    const CSSSelectorList& selectors() const { return m_selectors; }
    const CSSPropertyList& properties() const { return m_properties; }
    CSSRuleType type() const final { return CSSRuleType::Style; }

private:
    CSSStyleRule(CSSSelectorList selectors, CSSPropertyList properties)
        : m_selectors(std::move(selectors))
        , m_properties(std::move(properties))
    {}

    CSSSelectorList m_selectors;
    CSSPropertyList m_properties;
};

inline RefPtr<CSSStyleRule> CSSStyleRule::create(Heap* heap, CSSSelectorList selectors, CSSPropertyList properties)
{
    return adoptPtr(new (heap) CSSStyleRule(std::move(selectors), std::move(properties)));
}

template<>
struct is_a<CSSStyleRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRuleType::Style; }
};

class CSSMediaFeature {
public:
    CSSMediaFeature(CSSPropertyID id, RefPtr<CSSValue> value)
        : m_id(id), m_value(std::move(value))
    {}

    CSSPropertyID id() const { return m_id; }
    const RefPtr<CSSValue>& value() const { return m_value; }

private:
    CSSPropertyID m_id;
    RefPtr<CSSValue> m_value;
};

using CSSMediaFeatureList = std::pmr::forward_list<CSSMediaFeature>;

class CSSMediaQuery {
public:
    enum class Restrictor {
        None,
        Only,
        Not
    };

    enum class Type {
        None,
        All,
        Print,
        Screen
    };

    CSSMediaQuery(Restrictor restrictor, Type type, CSSMediaFeatureList features = CSSMediaFeatureList())
        : m_restrictor(restrictor), m_type(type), m_features(std::move(features))
    {}

    Restrictor restrictor() const { return m_restrictor; }
    Type type() const { return m_type; }
    const CSSMediaFeatureList& features() const { return m_features; }

private:
    Restrictor m_restrictor;
    Type m_type;
    CSSMediaFeatureList m_features;
};

using CSSMediaQueryList = std::pmr::forward_list<CSSMediaQuery>;

class CSSMediaRule final : public CSSRule {
public:
    static RefPtr<CSSMediaRule> create(Heap* heap, CSSMediaQueryList queries, CSSRuleList rules);

    const CSSMediaQueryList& queries() const { return m_queries; }
    const CSSRuleList& rules() const { return m_rules; }
    CSSRuleType type() const final { return CSSRuleType::Media; }

private:
    CSSMediaRule(CSSMediaQueryList queries, CSSRuleList rules)
        : m_queries(std::move(queries)), m_rules(std::move(rules))
    {}

    CSSMediaQueryList m_queries;
    CSSRuleList m_rules;
};

inline RefPtr<CSSMediaRule> CSSMediaRule::create(Heap* heap, CSSMediaQueryList queries, CSSRuleList rules)
{
    return adoptPtr(new (heap) CSSMediaRule(std::move(queries), std::move(rules)));
}

template<>
struct is_a<CSSMediaRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRuleType::Media; }
};

class CSSImportRule final : public CSSRule {
public:
    static RefPtr<CSSImportRule> create(Heap* heap, CSSStyleOrigin origin, Url href, CSSMediaQueryList queries);

    CSSStyleOrigin origin() const { return m_origin; }
    const Url& href() const { return m_href; }
    const CSSMediaQueryList& queries() const { return m_queries; }
    CSSRuleType type() const final { return CSSRuleType::Import; }

private:
    CSSImportRule(CSSStyleOrigin origin, Url href, CSSMediaQueryList queries)
        : m_origin(origin), m_href(std::move(href)), m_queries(std::move(queries))
    {}

    CSSStyleOrigin m_origin;
    Url m_href;
    CSSMediaQueryList m_queries;
};

inline RefPtr<CSSImportRule> CSSImportRule::create(Heap* heap, CSSStyleOrigin origin, Url href, CSSMediaQueryList queries)
{
    return adoptPtr(new (heap) CSSImportRule(origin, std::move(href), std::move(queries)));
}

template<>
struct is_a<CSSImportRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRuleType::Import; }
};

class CSSNamespaceRule final : public CSSRule {
public:
    static RefPtr<CSSNamespaceRule> create(Heap* heap, const GlobalString& prefix, const GlobalString& uri);

    const GlobalString& prefix() const { return m_prefix; }
    const GlobalString& uri() const { return m_uri; }
    CSSRuleType type() const final { return CSSRuleType::Namespace; }

private:
    CSSNamespaceRule(const GlobalString& prefix, const GlobalString& uri)
        : m_prefix(prefix), m_uri(uri)
    {}

    GlobalString m_prefix;
    GlobalString m_uri;
};

inline RefPtr<CSSNamespaceRule> CSSNamespaceRule::create(Heap* heap, const GlobalString& prefix, const GlobalString& uri)
{
    return adoptPtr(new (heap) CSSNamespaceRule(prefix, uri));
}

template<>
struct is_a<CSSNamespaceRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRuleType::Namespace; }
};

class CSSFontFaceRule final : public CSSRule {
public:
    static RefPtr<CSSFontFaceRule> create(Heap* heap, CSSPropertyList properties);

    const CSSPropertyList& properties() const { return m_properties; }
    CSSRuleType type() const final { return CSSRuleType::FontFace; }

private:
    CSSFontFaceRule(CSSPropertyList properties)
        : m_properties(std::move(properties))
    {}

    CSSPropertyList m_properties;
};

inline RefPtr<CSSFontFaceRule> CSSFontFaceRule::create(Heap* heap, CSSPropertyList properties)
{
    return adoptPtr(new (heap) CSSFontFaceRule(std::move(properties)));
}

template<>
struct is_a<CSSFontFaceRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRuleType::FontFace; }
};

class CSSCounterStyleRule final : public CSSRule {
public:
    static RefPtr<CSSCounterStyleRule> create(Heap* heap, const GlobalString& name, CSSPropertyList properties);

    const GlobalString& name() const { return m_name; }
    const CSSPropertyList& properties() const { return m_properties; }
    CSSRuleType type() const final { return CSSRuleType::CounterStyle; }

private:
    CSSCounterStyleRule(const GlobalString& name, CSSPropertyList properties)
        : m_name(name), m_properties(std::move(properties))
    {}

    GlobalString m_name;
    CSSPropertyList m_properties;
};

inline RefPtr<CSSCounterStyleRule> CSSCounterStyleRule::create(Heap* heap, const GlobalString& name, CSSPropertyList properties)
{
    return adoptPtr(new (heap) CSSCounterStyleRule(name, std::move(properties)));
}

template<>
struct is_a<CSSCounterStyleRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRuleType::CounterStyle; }
};

enum class PageMarginType : uint8_t {
    TopLeftCorner,
    TopLeft,
    TopCenter,
    TopRight,
    TopRightCorner,
    RightTop,
    RightMiddle,
    RightBottom,
    BottomRightCorner,
    BottomRight,
    BottomCenter,
    BottomLeft,
    BottomLeftCorner,
    LeftBottom,
    LeftMiddle,
    LeftTop,
    None
};

class CSSPageMarginRule final : public CSSRule {
public:
    static RefPtr<CSSPageMarginRule> create(Heap* heap, PageMarginType marginType, CSSPropertyList properties);

    PageMarginType marginType() const { return m_marginType; }
    const CSSPropertyList& properties() const { return m_properties; }
    CSSRuleType type() const final { return CSSRuleType::PageMargin; }

private:
    CSSPageMarginRule(PageMarginType marginType, CSSPropertyList properties)
        : m_marginType(marginType), m_properties(std::move(properties))
    {}

    PageMarginType m_marginType;
    CSSPropertyList m_properties;
};

inline RefPtr<CSSPageMarginRule> CSSPageMarginRule::create(Heap* heap, PageMarginType marginType, CSSPropertyList properties)
{
    return adoptPtr(new (heap) CSSPageMarginRule(marginType, std::move(properties)));
}

template<>
struct is_a<CSSPageMarginRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRuleType::PageMargin; }
};

using CSSPageMarginRuleList = std::pmr::vector<RefPtr<CSSPageMarginRule>>;

class CSSPageRule : public CSSRule {
public:
    static RefPtr<CSSPageRule> create(Heap* heap, CSSPageSelectorList selectors, CSSPageMarginRuleList margins, CSSPropertyList properties);

    const CSSPageSelectorList& selectors() const { return m_selectors; }
    const CSSPageMarginRuleList& margins() const { return m_margins; }
    const CSSPropertyList& properties() const { return m_properties; }
    CSSRuleType type() const final { return CSSRuleType::Page; }

private:
    CSSPageRule(CSSPageSelectorList selectors, CSSPageMarginRuleList margins, CSSPropertyList properties)
        : m_selectors(std::move(selectors))
        , m_margins(std::move(margins))
        , m_properties(std::move(properties))
    {}

    CSSPageSelectorList m_selectors;
    CSSPageMarginRuleList m_margins;
    CSSPropertyList m_properties;
};

inline RefPtr<CSSPageRule> CSSPageRule::create(Heap* heap, CSSPageSelectorList selectors, CSSPageMarginRuleList margins, CSSPropertyList properties)
{
    return adoptPtr(new (heap) CSSPageRule(std::move(selectors), std::move(margins), std::move(properties)));
}

template<>
struct is_a<CSSPageRule> {
    static bool check(const CSSRule& value) { return value.type() == CSSRuleType::Page; }
};

class Element;

class SelectorFilter {
public:
    SelectorFilter() = default;

    void push(const Element* element);
    void pop();

private:
    bool contains(unsigned hash) const { return isSet(hash) && isSet(hash >> 16); }

    void add(unsigned hash);
    void remove(unsigned hash);

    bool isSet(unsigned key) const { return m_table && m_table[key & keyMask]; }

    void set(unsigned key);
    void unset(unsigned key);

    static const unsigned keyBits = 12;
    static const unsigned keyMask = (1 << keyBits) - 1;
    static const unsigned maxCount = (1 << 8) - 1;

    using HashVector = std::vector<unsigned>;

    std::unique_ptr<uint8_t[]> m_table;
    std::vector<HashVector> m_stack;

    friend class CSSRuleData;
};

inline void SelectorFilter::add(unsigned hash)
{
    set(hash);
    set(hash >> 16);
}

inline void SelectorFilter::remove(unsigned hash)
{
    unset(hash);
    unset(hash >> 16);
}

inline void SelectorFilter::set(unsigned key)
{
    auto& value = m_table[key & keyMask];
    if(value < maxCount) {
        value++;
    }
}

inline void SelectorFilter::unset(unsigned key)
{
    auto& value = m_table[key & keyMask];
    assert(value > 0);
    if(value < maxCount) {
        value--;
    }
}

class CSSRuleData {
public:
    CSSRuleData(const RefPtr<CSSStyleRule>& rule, const CSSSelector& selector, uint32_t specificity, uint32_t position);

    const RefPtr<CSSStyleRule>& rule() const { return m_rule; }
    const CSSSelector* selector() const { return m_selector; }
    const CSSPropertyList& properties() const { return m_rule->properties(); }
    const uint32_t specificity() const { return m_specificity; }
    const uint32_t position() const { return m_position; }

    bool match(const Element* element, PseudoType pseudoType, const SelectorFilter& selectorFilter) const;

private:
    static bool matchSelector(const Element* element, PseudoType pseudoType, const CSSSelector& selector);
    static bool matchCompoundSelector(const Element* element, PseudoType pseudoType, const CSSCompoundSelector& selector);
    static bool matchSimpleSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchNamespaceSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchTagSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchIdSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchClassSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchAttributeHasSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeEqualsSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeIncludesSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeContainsSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeDashEqualsSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeStartsWithSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchAttributeEndsWithSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassIsSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassNotSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassHasSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassLinkSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassLocalLinkSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassEnabledSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassDisabledSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassCheckedSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassLangSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassRootSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassEmptySelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassFirstChildSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassLastChildSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassOnlyChildSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassFirstOfTypeSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassLastOfTypeSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassOnlyOfTypeSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassNthChildSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassNthLastChildSelector(const Element* element, const CSSSimpleSelector& selector);

    static bool matchPseudoClassNthOfTypeSelector(const Element* element, const CSSSimpleSelector& selector);
    static bool matchPseudoClassNthLastOfTypeSelector(const Element* element, const CSSSimpleSelector& selector);

    static const unsigned maxHashCount = 4;

    RefPtr<CSSStyleRule> m_rule;
    const CSSSelector* m_selector;
    uint32_t m_specificity;
    uint32_t m_position;
    unsigned m_hashes[maxHashCount];
};

class CSSPageRuleData {
public:
    CSSPageRuleData(const RefPtr<CSSPageRule>& rule, const CSSPageSelector* selector, uint32_t specificity, uint32_t position)
        : m_rule(rule), m_selector(selector), m_specificity(specificity), m_position(position)
    {}

    const RefPtr<CSSPageRule>& rule() const { return m_rule; }
    const CSSPageSelector* selector() const { return m_selector; }
    const CSSPropertyList& properties() const { return m_rule->properties(); }
    const CSSPageMarginRuleList& margins() const { return m_rule->margins(); }
    const uint32_t specificity() const { return m_specificity; }
    const uint32_t position() const { return m_position; }

    bool match(const GlobalString& pageName, uint32_t pageIndex, PseudoType pseudoType) const;

private:
    static bool matchSelector(const GlobalString& pageName, uint32_t pageIndex, PseudoType pseudoType, const CSSSimpleSelector& selector);
    RefPtr<CSSPageRule> m_rule;
    const CSSPageSelector* m_selector;
    uint32_t m_specificity;
    uint32_t m_position;
};

class CSSCounterStyle : public HeapMember, public RefCounted<CSSCounterStyle> {
public:
    static RefPtr<CSSCounterStyle> create(Heap* heap, RefPtr<CSSCounterStyleRule> rule);

    std::string generateInitialRepresentation(int value) const;
    std::string generateFallbackRepresentation(int value) const;
    std::string generateRepresentation(int value) const;

    bool rangeContains(int value) const;
    bool needsNegativeSign(int value) const;

    const GlobalString& name() const;
    const GlobalString& extendsName() const;
    const GlobalString& fallbackName() const;
    const CSSValueID system() const;

    const HeapString& prefix() const;
    const HeapString& suffix() const;

    void setFallbackStyle(CSSCounterStyle& fallbackStyle) { m_fallbackStyle = fallbackStyle; }
    const RefPtr<CSSCounterStyle>& fallbackStyle() const { return m_fallbackStyle; }
    void extend(const CSSCounterStyle& extended);

    static CSSCounterStyle& defaultStyle();

private:
    explicit CSSCounterStyle(RefPtr<CSSCounterStyleRule> rule);
    RefPtr<CSSCounterStyleRule> m_rule;
    RefPtr<CSSIdentValue> m_system;
    RefPtr<CSSCustomIdentValue> m_extends;
    RefPtr<CSSIntegerValue> m_fixed;
    RefPtr<CSSValue> m_negative;
    RefPtr<CSSValue> m_prefix;
    RefPtr<CSSValue> m_suffix;
    RefPtr<CSSListValue> m_range;
    RefPtr<CSSPairValue> m_pad;
    RefPtr<CSSCustomIdentValue> m_fallback;
    RefPtr<CSSListValue> m_symbols;
    RefPtr<CSSListValue> m_additiveSymbols;
    mutable RefPtr<CSSCounterStyle> m_fallbackStyle;
};

class CSSCounterStyleMap : public HeapMember {
public:
    static std::unique_ptr<CSSCounterStyleMap> create(Heap* heap, const CSSRuleList& rules, const CSSCounterStyleMap* parent);

    CSSCounterStyle* findCounterStyle(const GlobalString& name) const;

private:
    CSSCounterStyleMap(Heap* heap, const CSSRuleList& rules, const CSSCounterStyleMap* parent);
    const CSSCounterStyleMap* m_parent;
    std::pmr::map<GlobalString, RefPtr<CSSCounterStyle>> m_counterStyles;
};

const CSSCounterStyleMap* userAgentCounterStyleMap();

} // namespace plutobook

#endif // PLUTOBOOK_CSSRULE_H
