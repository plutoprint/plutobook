/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "cssparser.h"
#include "csstokenizer.h"
#include "stringutils.h"
#include "document.h"

#include <sstream>
#include <algorithm>
#include <cmath>

namespace plutobook {

CSSParser::CSSParser(const CSSParserContext& context, Heap* heap)
    : m_heap(heap), m_context(context)
{
}

CSSRuleList CSSParser::parseSheet(const std::string_view& content)
{
    CSSRuleList rules(m_heap);
    CSSTokenizer tokenizer(content);
    CSSTokenStream input(tokenizer.tokenize());
    consumeRuleList(input, rules);
    return rules;
}

CSSPropertyList CSSParser::parseStyle(const std::string_view& content)
{
    CSSPropertyList properties(m_heap);
    CSSTokenizer tokenizer(content);
    CSSTokenStream input(tokenizer.tokenize());
    consumeDeclaractionList(input, properties, CSSRuleType::Style);
    return properties;
}

CSSMediaQueryList CSSParser::parseMediaQueries(const std::string_view& content)
{
    CSSMediaQueryList queries(m_heap);
    CSSTokenizer tokenizer(content);
    CSSTokenStream input(tokenizer.tokenize());
    consumeMediaQueries(input, queries);
    return queries;
}

CSSPropertyList CSSParser::parsePropertyValue(CSSTokenStream input, CSSPropertyID id, bool important)
{
    CSSPropertyList properties(m_heap);
    consumeDescriptor(input, properties, id, important);
    return properties;
}

template<typename T>
struct CSSIdentEntry {
    constexpr CSSIdentEntry(const char* name, T value)
        : name(name), value(value), length(std::strlen(name))
    {}

    const char* name;
    const T value;
    const int length;
};

using CSSIdentValueEntry = CSSIdentEntry<CSSValueID>;

static bool identMatches(const char* name, int length, const std::string_view& ident)
{
    if(length != ident.length())
        return false;
    for(int i = 0; i < length; ++i) {
        auto cc = name[i];
        assert(!isUpper(cc));
        if(cc != toLower(ident[i])) {
            return false;
        }
    }

    return true;
}

template<typename T, unsigned int N>
static std::optional<T> matchIdent(const CSSIdentEntry<T>(&table)[N], const std::string_view& ident)
{
    for(const auto& entry : table) {
        if(identMatches(entry.name, entry.length, ident)) {
            return entry.value;
        }
    }

    return std::nullopt;
}

static bool consumeIdentIncludingWhitespace(CSSTokenStream& input, const char* name, int length)
{
    if(input->type() == CSSToken::Type::Ident && identMatches(name, length, input->data())) {
        input.consumeIncludingWhitespace();
        return true;
    }

    return false;
}

static CSSMediaQuery::Type consumeMediaType(CSSTokenStream& input)
{
    if(consumeIdentIncludingWhitespace(input, "all", 3))
        return CSSMediaQuery::Type::All;
    if(consumeIdentIncludingWhitespace(input, "print", 5))
        return CSSMediaQuery::Type::Print;
    if(consumeIdentIncludingWhitespace(input, "screen", 6))
        return CSSMediaQuery::Type::Screen;
    return CSSMediaQuery::Type::None;
}

static CSSMediaQuery::Restrictor consumeMediaRestrictor(CSSTokenStream& input)
{
    if(consumeIdentIncludingWhitespace(input, "only", 4))
        return CSSMediaQuery::Restrictor::Only;
    if(consumeIdentIncludingWhitespace(input, "not", 3))
        return CSSMediaQuery::Restrictor::Not;
    return CSSMediaQuery::Restrictor::None;
}

bool CSSParser::consumeMediaFeature(CSSTokenStream& input, CSSMediaFeatureList& features)
{
    if(input->type() != CSSToken::Type::LeftParenthesis)
        return false;
    static const CSSIdentEntry<CSSPropertyID> table[] = {
        {"width", CSSPropertyID::Width},
        {"min-width", CSSPropertyID::MinWidth},
        {"max-width", CSSPropertyID::MaxWidth},
        {"height", CSSPropertyID::Height},
        {"min-height", CSSPropertyID::MinHeight},
        {"max-height", CSSPropertyID::MaxHeight},
        {"orientation", CSSPropertyID::Orientation}
    };

    auto block = input.consumeBlock();
    block.consumeWhitespace();
    if(block->type() != CSSToken::Type::Ident)
        return false;
    auto id = matchIdent(table, block->data());
    if(id == std::nullopt)
        return false;
    block.consumeIncludingWhitespace();
    if(block->type() == CSSToken::Type::Colon) {
        block.consumeIncludingWhitespace();
        RefPtr<CSSValue> value;
        switch(id.value()) {
        case CSSPropertyID::Width:
        case CSSPropertyID::MinWidth:
        case CSSPropertyID::MaxWidth:
        case CSSPropertyID::Height:
        case CSSPropertyID::MinHeight:
        case CSSPropertyID::MaxHeight:
            value = consumeLength(block, false, false);
            break;
        case CSSPropertyID::Orientation:
            value = consumeOrientation(block);
            break;
        default:
            assert(false);
        }

        block.consumeWhitespace();
        if(value && block.empty()) {
            features.emplace_front(*id, std::move(value));
            input.consumeWhitespace();
            return true;
        }
    }

    return false;
}

bool CSSParser::consumeMediaFeatures(CSSTokenStream& input, CSSMediaFeatureList& features)
{
    do {
        if(!consumeMediaFeature(input, features))
            return false;
    } while(consumeIdentIncludingWhitespace(input, "and", 3));
    return true;
}

bool CSSParser::consumeMediaQuery(CSSTokenStream& input, CSSMediaQueryList& queries)
{
    auto restrictor = consumeMediaRestrictor(input);
    auto type = consumeMediaType(input);
    if(restrictor != CSSMediaQuery::Restrictor::None && type == CSSMediaQuery::Type::None)
        return false;
    CSSMediaFeatureList features(m_heap);
    if(type != CSSMediaQuery::Type::None && consumeIdentIncludingWhitespace(input, "and", 3) && !consumeMediaFeatures(input, features))
        return false;
    if(type == CSSMediaQuery::Type::None && !consumeMediaFeatures(input, features)) {
        return false;
    }

    queries.emplace_front(type, restrictor, std::move(features));
    return true;
}

bool CSSParser::consumeMediaQueries(CSSTokenStream& input, CSSMediaQueryList& queries)
{
    input.consumeWhitespace();
    if(!input.empty()) {
        do {
            if(!consumeMediaQuery(input, queries))
                return false;
        } while(input.consumeCommaIncludingWhitespace());
    }

    return true;
}

RefPtr<CSSRule> CSSParser::consumeRule(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::AtKeyword)
        return consumeAtRule(input);
    return consumeStyleRule(input);
}

RefPtr<CSSRule> CSSParser::consumeAtRule(CSSTokenStream& input)
{
    assert(input->type() == CSSToken::Type::AtKeyword);
    auto name = input->data();
    input.consume();
    auto preludeBegin = input.begin();
    while(input->type() != CSSToken::Type::EndOfFile
        && input->type() != CSSToken::Type::LeftCurlyBracket
        && input->type() != CSSToken::Type::Semicolon) {
        input.consumeComponent();
    }

    CSSTokenStream prelude(preludeBegin, input.begin());
    if(input->type() == CSSToken::Type::EndOfFile
        || input->type() == CSSToken::Type::Semicolon) {
        if(input->type() == CSSToken::Type::Semicolon)
            input.consume();
        if(identMatches("import", 6, name))
            return consumeImportRule(prelude);
        if(identMatches("namespace", 9, name))
            return consumeNamespaceRule(prelude);
        return nullptr;
    }

    auto block = input.consumeBlock();
    if(identMatches("font-face", 9, name))
        return consumeFontFaceRule(prelude, block);
    if(identMatches("media", 5, name))
        return consumeMediaRule(prelude, block);
    if(identMatches("counter-style", 13, name))
        return consumeCounterStyleRule(prelude, block);
    if(identMatches("page", 4, name))
        return consumePageRule(prelude, block);
    return nullptr;
}

RefPtr<CSSStyleRule> CSSParser::consumeStyleRule(CSSTokenStream& input)
{
    auto preludeBegin = input.begin();
    while(!input.empty() && input->type() != CSSToken::Type::LeftCurlyBracket) {
        input.consumeComponent();
    }

    if(input.empty())
        return nullptr;
    CSSTokenStream prelude(preludeBegin, input.begin());
    auto block = input.consumeBlock();
    CSSSelectorList selectors(m_heap);
    if(!consumeSelectorList(prelude, selectors, false))
        return nullptr;
    CSSPropertyList properties(m_heap);
    consumeDeclaractionList(block, properties, CSSRuleType::Style);
    return CSSStyleRule::create(m_heap, std::move(selectors), std::move(properties));
}

static const CSSToken* consumeUrlToken(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::Url) {
        auto token = input.begin();
        input.consumeIncludingWhitespace();
        return token;
    }

    if(input->type() == CSSToken::Type::Function && identMatches("url", 3, input->data())) {
        CSSTokenStreamGuard guard(input);
        auto block = input.consumeBlock();
        block.consumeWhitespace();
        auto token = block.begin();
        block.consumeIncludingWhitespace();
        if(token->type() == CSSToken::Type::BadString || !block.empty())
            return nullptr;
        assert(token->type() == CSSToken::Type::String);
        input.consumeWhitespace();
        guard.release();
        return token;
    }

    return nullptr;
}

static const CSSToken* consumeStringOrUrlToken(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::String) {
        auto token = input.begin();
        input.consumeIncludingWhitespace();
        return token;
    }

    return consumeUrlToken(input);
}

RefPtr<CSSImportRule> CSSParser::consumeImportRule(CSSTokenStream& input)
{
    input.consumeWhitespace();
    auto token = consumeStringOrUrlToken(input);
    if(token == nullptr)
        return nullptr;
    CSSMediaQueryList queries(m_heap);
    if(!consumeMediaQueries(input, queries))
        return nullptr;
    return CSSImportRule::create(m_heap, m_context.origin(), m_context.completeUrl(token->data()), std::move(queries));
}

RefPtr<CSSNamespaceRule> CSSParser::consumeNamespaceRule(CSSTokenStream& input)
{
    GlobalString prefix;
    input.consumeWhitespace();
    if(input->type() == CSSToken::Type::Ident) {
        prefix = GlobalString(input->data());
        input.consumeIncludingWhitespace();
    }

    auto token = consumeStringOrUrlToken(input);
    if(token == nullptr || !input.empty())
        return nullptr;
    GlobalString uri(token->data());
    if(prefix.isEmpty()) {
        m_defaultNamespace = uri;
    } else {
        m_namespaces.emplace(prefix, uri);
    }

    return CSSNamespaceRule::create(m_heap, prefix, uri);
}

RefPtr<CSSMediaRule> CSSParser::consumeMediaRule(CSSTokenStream& prelude, CSSTokenStream& block)
{
    CSSMediaQueryList queries(m_heap);
    if(!consumeMediaQueries(prelude, queries))
        return nullptr;
    CSSRuleList rules(m_heap);
    consumeRuleList(block, rules);
    return CSSMediaRule::create(m_heap, std::move(queries), std::move(rules));
}

RefPtr<CSSFontFaceRule> CSSParser::consumeFontFaceRule(CSSTokenStream& prelude, CSSTokenStream& block)
{
    prelude.consumeWhitespace();
    if(!prelude.empty())
        return nullptr;
    CSSPropertyList properties(m_heap);
    consumeDeclaractionList(block, properties, CSSRuleType::FontFace);
    return CSSFontFaceRule::create(m_heap, std::move(properties));
}

RefPtr<CSSCounterStyleRule> CSSParser::consumeCounterStyleRule(CSSTokenStream& prelude, CSSTokenStream& block)
{
    prelude.consumeWhitespace();
    if(prelude->type() != CSSToken::Type::Ident || identMatches("none", 4, prelude->data()))
        return nullptr;
    GlobalString name(prelude->data());
    prelude.consumeIncludingWhitespace();
    if(!prelude.empty())
        return nullptr;
    CSSPropertyList properties(m_heap);
    consumeDeclaractionList(block, properties, CSSRuleType::CounterStyle);
    return CSSCounterStyleRule::create(m_heap, name, std::move(properties));
}

RefPtr<CSSPageRule> CSSParser::consumePageRule(CSSTokenStream& prelude, CSSTokenStream& block)
{
    CSSPageSelectorList selectors(m_heap);
    if(!consumePageSelectorList(prelude, selectors))
        return nullptr;
    CSSPageMarginRuleList margins(m_heap);
    CSSPropertyList properties(m_heap);
    while(!block.empty()) {
        switch(block->type()) {
        case CSSToken::Type::Whitespace:
        case CSSToken::Type::Semicolon:
            block.consume();
            break;
        case CSSToken::Type::AtKeyword:
            if(auto margin = consumePageMarginRule(block))
                margins.push_back(std::move(margin));
            break;
        default:
            consumeDeclaraction(block, properties, CSSRuleType::Page);
            break;
        }
    }

    return CSSPageRule::create(m_heap, std::move(selectors), std::move(margins), std::move(properties));
}

RefPtr<CSSPageMarginRule> CSSParser::consumePageMarginRule(CSSTokenStream& input)
{
    assert(input->type() == CSSToken::Type::AtKeyword);
    auto name = input->data();
    input.consume();
    auto preludeBegin = input.begin();
    while(!input.empty() && input->type() != CSSToken::Type::LeftCurlyBracket) {
        input.consumeComponent();
    }

    if(input.empty())
        return nullptr;
    CSSTokenStream prelude(preludeBegin, input.begin());
    auto block = input.consumeBlock();
    prelude.consumeWhitespace();
    if(!prelude.empty())
        return nullptr;
    static const CSSIdentEntry<PageMarginType> table[] = {
        {"top-left-corner", PageMarginType::TopLeftCorner},
        {"top-left", PageMarginType::TopLeft},
        {"top-center", PageMarginType::TopCenter},
        {"top-right", PageMarginType::TopRight},
        {"top-right-corner", PageMarginType::TopRightCorner},
        {"bottom-left-corner", PageMarginType::BottomLeftCorner},
        {"bottom-left", PageMarginType::BottomLeft},
        {"bottom-center", PageMarginType::BottomCenter},
        {"bottom-right", PageMarginType::BottomRight},
        {"bottom-right-corner", PageMarginType::BottomRightCorner},
        {"left-top", PageMarginType::LeftTop},
        {"left-middle", PageMarginType::LeftMiddle},
        {"left-bottom", PageMarginType::LeftBottom},
        {"right-top", PageMarginType::RightTop},
        {"right-middle", PageMarginType::RightMiddle},
        {"right-bottom", PageMarginType::RightBottom}
    };

    auto marginType = matchIdent(table, name);
    if(marginType == std::nullopt)
        return nullptr;
    CSSPropertyList properties(m_heap);
    consumeDeclaractionList(block, properties, CSSRuleType::PageMargin);
    return CSSPageMarginRule::create(m_heap, marginType.value(), std::move(properties));
}

void CSSParser::consumeRuleList(CSSTokenStream& input, CSSRuleList& rules)
{
    while(!input.empty()) {
        input.consumeWhitespace();
        if(input->type() == CSSToken::Type::CDC
            || input->type() == CSSToken::Type::CDO) {
            input.consume();
            continue;
        }

        if(auto rule = consumeRule(input)) {
            rules.push_back(std::move(rule));
        }
    }
}

bool CSSParser::consumePageSelectorList(CSSTokenStream& input, CSSPageSelectorList& selectors)
{
    input.consumeWhitespace();
    if(!input.empty()) {
        do {
            CSSPageSelector selector(m_heap);
            if(!consumePageSelector(input, selector))
                return false;
            selectors.push_front(std::move(selector));
        } while(input.consumeCommaIncludingWhitespace());
    }

    return input.empty();
}

bool CSSParser::consumePageSelector(CSSTokenStream& input, CSSPageSelector& selector)
{
    if(input->type() != CSSToken::Type::Ident
        && input->type() != CSSToken::Type::Colon) {
        return false;
    }

    if(input->type() == CSSToken::Type::Ident) {
        selector.emplace_front(CSSSimpleSelector::MatchType::PseudoPageName, GlobalString(input->data()));
        input.consumeIncludingWhitespace();
    }

    while(input->type() == CSSToken::Type::Colon) {
        input.consumeIncludingWhitespace();
        if(input->type() == CSSToken::Type::Function) {
            if(!identMatches("nth", 3, input->data()))
                return false;
            auto block = input.consumeBlock();
            block.consumeWhitespace();
            CSSSimpleSelector::MatchPattern pattern;
            if(!consumeMatchPattern(block, pattern))
                return false;
            block.consumeWhitespace();
            if(!block.empty())
                return false;
            input.consumeWhitespace();
            selector.emplace_front(CSSSimpleSelector::MatchType::PseudoPageNth, pattern);
            continue;
        }

        if(input->type() != CSSToken::Type::Ident)
            return false;
        static const CSSIdentEntry<CSSSimpleSelector::MatchType> table[] = {
            {"first", CSSSimpleSelector::MatchType::PseudoPageFirst},
            {"left", CSSSimpleSelector::MatchType::PseudoPageLeft},
            {"right", CSSSimpleSelector::MatchType::PseudoPageRight},
            {"blank", CSSSimpleSelector::MatchType::PseudoPageBlank}
        };

        auto name = input->data();
        input.consumeIncludingWhitespace();
        auto matchType = matchIdent(table, name);
        if(matchType == std::nullopt)
            return false;
        selector.emplace_front(matchType.value());
    }

    return true;
}

bool CSSParser::consumeSelectorList(CSSTokenStream& input, CSSSelectorList& selectors, bool relative)
{
    do {
        CSSSelector selector(m_heap);
        if(!consumeSelector(input, selector, relative))
            return false;
        selectors.push_front(std::move(selector));
    } while(input.consumeCommaIncludingWhitespace());
    return input.empty();
}

bool CSSParser::consumeSelector(CSSTokenStream& input, CSSSelector& selector, bool relative)
{
    auto combinator = CSSComplexSelector::Combinator::None;
    if(relative) {
        consumeCombinator(input, combinator);
    }

    do {
        bool failed = false;
        CSSCompoundSelector sel(m_heap);
        if(!consumeCompoundSelector(input, sel, failed))
            return !failed ? combinator == CSSComplexSelector::Combinator::Descendant : false;
        selector.emplace_front(combinator, std::move(sel));
    } while(consumeCombinator(input, combinator));
    return true;
}

bool CSSParser::consumeCompoundSelector(CSSTokenStream& input, CSSCompoundSelector& selector, bool& failed)
{
    if(!consumeTagSelector(input, selector)) {
        if(m_defaultNamespace != starGlo)
            selector.emplace_front(CSSSimpleSelector::MatchType::Namespace, m_defaultNamespace);
        if(!consumeSimpleSelector(input, selector, failed)) {
            return false;
        }
    }

    while(consumeSimpleSelector(input, selector, failed));
    return !failed;
}

bool CSSParser::consumeSimpleSelector(CSSTokenStream& input, CSSCompoundSelector& selector, bool& failed)
{
    if(input->type() == CSSToken::Type::Hash)
        failed = !consumeIdSelector(input, selector);
    else if(input->type() == CSSToken::Type::Delim && input->delim() == '.')
        failed = !consumeClassSelector(input, selector);
    else if(input->type() == CSSToken::Type::LeftSquareBracket)
        failed = !consumeAttributeSelector(input, selector);
    else if(input->type() == CSSToken::Type::Colon)
        failed = !consumePseudoSelector(input, selector);
    else
        return false;
    return !failed;
}

bool CSSParser::consumeTagSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    GlobalString name;
    CSSTokenStreamGuard guard(input);
    if(input->type() == CSSToken::Type::Ident) {
        name = GlobalString(input->data());
        input.consume();
    } else if(input->type() == CSSToken::Type::Delim && input->delim() == '*') {
        name = starGlo;
        input.consume();
    } else {
        return false;
    }

    auto namespaceURI = m_defaultNamespace;
    if(input->type() == CSSToken::Type::Delim && input->delim() == '|') {
        input.consume();
        namespaceURI = determineNamespace(name);
        if(input->type() == CSSToken::Type::Ident) {
            name = GlobalString(input->data());
            input.consume();
        } else if(input->type() == CSSToken::Type::Delim && input->delim() == '*') {
            name = starGlo;
            input.consume();
        } else {
            return false;
        }
    }

    if(namespaceURI != starGlo)
        selector.emplace_front(CSSSimpleSelector::MatchType::Namespace, namespaceURI);
    if(name == starGlo) {
        selector.emplace_front(CSSSimpleSelector::MatchType::Universal);
    } else {
        if(m_context.inHTMLDocument())
            name = name.foldCase();
        selector.emplace_front(CSSSimpleSelector::MatchType::Tag, name);
    }

    guard.release();
    return true;
}

bool CSSParser::consumeIdSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    assert(input->type() == CSSToken::Type::Hash);
    if(input->hashType() == CSSToken::HashType::Identifier) {
        selector.emplace_front(CSSSimpleSelector::MatchType::Id, m_heap->createString(input->data()));
        input.consume();
        return true;
    }

    return false;
}

bool CSSParser::consumeClassSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    assert(input->type() == CSSToken::Type::Delim);
    input.consume();
    if(input->type() == CSSToken::Type::Ident) {
        selector.emplace_front(CSSSimpleSelector::MatchType::Class, m_heap->createString(input->data()));
        input.consume();
        return true;
    }

    return false;
}

bool CSSParser::consumeAttributeSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    assert(input->type() == CSSToken::Type::LeftSquareBracket);
    auto block = input.consumeBlock();
    block.consumeWhitespace();
    if(block->type() != CSSToken::Type::Ident)
        return false;
    GlobalString name(block->data());
    if(m_context.inHTMLDocument())
        name = name.foldCase();
    block.consumeIncludingWhitespace();
    if(block.empty()) {
        selector.emplace_front(CSSSimpleSelector::MatchType::AttributeHas, name);
        return true;
    }

    if(block->type() != CSSToken::Type::Delim)
        return false;
    CSSSimpleSelector::MatchType matchType;
    switch(block->delim()) {
    case '=':
        matchType = CSSSimpleSelector::MatchType::AttributeEquals;
        break;
    case '~':
        matchType = CSSSimpleSelector::MatchType::AttributeIncludes;
        break;
    case '*':
        matchType = CSSSimpleSelector::MatchType::AttributeContains;
        break;
    case '|':
        matchType = CSSSimpleSelector::MatchType::AttributeDashEquals;
        break;
    case '^':
        matchType = CSSSimpleSelector::MatchType::AttributeStartsWith;
        break;
    case '$':
        matchType = CSSSimpleSelector::MatchType::AttributeEndsWith;
        break;
    default:
        return false;
    }

    if(matchType != CSSSimpleSelector::MatchType::AttributeEquals) {
        block.consume();
        if(block->type() != CSSToken::Type::Delim && block->delim() != '=') {
            return false;
        }
    }

    block.consumeIncludingWhitespace();
    if(block->type() != CSSToken::Type::Ident && block->type() != CSSToken::Type::String)
        return false;
    auto value = m_heap->createString(block->data());
    block.consumeIncludingWhitespace();
    auto caseType = CSSSimpleSelector::AttributeCaseType::Sensitive;
    if(block->type() == CSSToken::Type::Ident && block->data() == "i") {
        caseType = CSSSimpleSelector::AttributeCaseType::InSensitive;
        block.consumeIncludingWhitespace();
    }

    if(!block.empty())
        return false;
    selector.emplace_front(matchType, caseType, name, value);
    return true;
}

bool CSSParser::consumePseudoSelector(CSSTokenStream& input, CSSCompoundSelector& selector)
{
    assert(input->type() == CSSToken::Type::Colon);
    input.consume();
    if(input->type() == CSSToken::Type::Colon) {
        input.consume();
        if(input->type() != CSSToken::Type::Ident)
            return false;
        auto name = input->data();
        input.consume();
        static const CSSIdentEntry<CSSSimpleSelector::MatchType> table[] = {
            {"after", CSSSimpleSelector::MatchType::PseudoElementAfter},
            {"before", CSSSimpleSelector::MatchType::PseudoElementBefore},
            {"first-letter", CSSSimpleSelector::MatchType::PseudoElementFirstLetter},
            {"first-line", CSSSimpleSelector::MatchType::PseudoElementFirstLine},
            {"marker", CSSSimpleSelector::MatchType::PseudoElementMarker}
        };

        auto matchType = matchIdent(table, name);
        if(matchType == std::nullopt)
            return false;
        selector.emplace_front(matchType.value());
        return true;
    }

    if(input->type() == CSSToken::Type::Ident) {
        auto name = input->data();
        input.consume();
        static const CSSIdentEntry<CSSSimpleSelector::MatchType> table[] = {
            {"active", CSSSimpleSelector::MatchType::PseudoClassActive},
            {"any-link", CSSSimpleSelector::MatchType::PseudoClassAnyLink},
            {"checked", CSSSimpleSelector::MatchType::PseudoClassChecked},
            {"disabled", CSSSimpleSelector::MatchType::PseudoClassDisabled},
            {"empty", CSSSimpleSelector::MatchType::PseudoClassEmpty},
            {"enabled", CSSSimpleSelector::MatchType::PseudoClassEnabled},
            {"first-child", CSSSimpleSelector::MatchType::PseudoClassFirstChild},
            {"first-of-type", CSSSimpleSelector::MatchType::PseudoClassFirstOfType},
            {"focus", CSSSimpleSelector::MatchType::PseudoClassFocus},
            {"focus-visible", CSSSimpleSelector::MatchType::PseudoClassFocusVisible},
            {"focus-within", CSSSimpleSelector::MatchType::PseudoClassFocusWithin},
            {"hover", CSSSimpleSelector::MatchType::PseudoClassHover},
            {"last-child", CSSSimpleSelector::MatchType::PseudoClassLastChild},
            {"last-of-type", CSSSimpleSelector::MatchType::PseudoClassLastOfType},
            {"link", CSSSimpleSelector::MatchType::PseudoClassLink},
            {"local-link", CSSSimpleSelector::MatchType::PseudoClassLocalLink},
            {"only-child", CSSSimpleSelector::MatchType::PseudoClassOnlyChild},
            {"only-of-type", CSSSimpleSelector::MatchType::PseudoClassOnlyOfType},
            {"root", CSSSimpleSelector::MatchType::PseudoClassRoot},
            {"scope", CSSSimpleSelector::MatchType::PseudoClassScope},
            {"target", CSSSimpleSelector::MatchType::PseudoClassTarget},
            {"target-within", CSSSimpleSelector::MatchType::PseudoClassTargetWithin},
            {"visited", CSSSimpleSelector::MatchType::PseudoClassVisited},
            {"after", CSSSimpleSelector::MatchType::PseudoElementAfter},
            {"before", CSSSimpleSelector::MatchType::PseudoElementBefore},
            {"first-letter", CSSSimpleSelector::MatchType::PseudoElementFirstLetter},
            {"first-line", CSSSimpleSelector::MatchType::PseudoElementFirstLine}
        };

        auto matchType = matchIdent(table, name);
        if(matchType == std::nullopt)
            return false;
        selector.emplace_front(matchType.value());
        return true;
    }

    if(input->type() == CSSToken::Type::Function) {
        auto name = input->data();
        auto block = input.consumeBlock();
        block.consumeWhitespace();
        static const CSSIdentEntry<CSSSimpleSelector::MatchType> table[] = {
            {"is", CSSSimpleSelector::MatchType::PseudoClassIs},
            {"not", CSSSimpleSelector::MatchType::PseudoClassNot},
            {"has", CSSSimpleSelector::MatchType::PseudoClassHas},
            {"where", CSSSimpleSelector::MatchType::PseudoClassWhere},
            {"lang", CSSSimpleSelector::MatchType::PseudoClassLang},
            {"nth-child", CSSSimpleSelector::MatchType::PseudoClassNthChild},
            {"nth-last-child", CSSSimpleSelector::MatchType::PseudoClassNthLastChild},
            {"nth-last-of-type", CSSSimpleSelector::MatchType::PseudoClassNthLastOfType},
            {"nth-of-type", CSSSimpleSelector::MatchType::PseudoClassNthOfType}
        };

        auto matchType = matchIdent(table, name);
        if(matchType == std::nullopt)
            return false;
        switch(matchType.value()) {
        case CSSSimpleSelector::MatchType::PseudoClassIs:
        case CSSSimpleSelector::MatchType::PseudoClassNot:
        case CSSSimpleSelector::MatchType::PseudoClassHas:
        case CSSSimpleSelector::MatchType::PseudoClassWhere: {
            CSSSelectorList subSelectors(m_heap);
            if(!consumeSelectorList(block, subSelectors, matchType == CSSSimpleSelector::MatchType::PseudoClassHas))
                return false;
            selector.emplace_front(*matchType, std::move(subSelectors));
            break;
        }

        case CSSSimpleSelector::MatchType::PseudoClassLang: {
            if(block->type() != CSSToken::Type::Ident)
                return false;
            selector.emplace_front(*matchType, m_heap->createString(block->data()));
            block.consume();
            break;
        }

        case CSSSimpleSelector::MatchType::PseudoClassNthChild:
        case CSSSimpleSelector::MatchType::PseudoClassNthLastChild:
        case CSSSimpleSelector::MatchType::PseudoClassNthOfType:
        case CSSSimpleSelector::MatchType::PseudoClassNthLastOfType: {
            CSSSimpleSelector::MatchPattern pattern;
            if(!consumeMatchPattern(block, pattern))
                return false;
            selector.emplace_front(*matchType, pattern);
            break;
        }

        default:
            assert(false);
        }

        block.consumeWhitespace();
        return block.empty();
    }

    return false;
}

bool CSSParser::consumeCombinator(CSSTokenStream& input, CSSComplexSelector::Combinator& combinator)
{
    combinator = CSSComplexSelector::Combinator::None;
    while(input->type() == CSSToken::Type::Whitespace) {
        combinator = CSSComplexSelector::Combinator::Descendant;
        input.consume();
    }

    if(input->type() == CSSToken::Type::Delim) {
        if(input->delim() == '+') {
            combinator = CSSComplexSelector::Combinator::DirectAdjacent;
            input.consumeIncludingWhitespace();
            return true;
        }

        if(input->delim() == '~') {
            combinator = CSSComplexSelector::Combinator::InDirectAdjacent;
            input.consumeIncludingWhitespace();
            return true;
        }

        if(input->delim() == '>') {
            combinator = CSSComplexSelector::Combinator::Child;
            input.consumeIncludingWhitespace();
            return true;
        }
    }

    return combinator == CSSComplexSelector::Combinator::Descendant;
}

bool CSSParser::consumeMatchPattern(CSSTokenStream& input, CSSSimpleSelector::MatchPattern& pattern)
{
    if(input->type() == CSSToken::Type::Number) {
        if(input->numberType() != CSSToken::NumberType::Integer)
            return false;
        pattern = std::make_pair(0, input->integer());
        input.consume();
        return true;
    }

    if(input->type() == CSSToken::Type::Ident) {
        if(identMatches("odd", 3, input->data())) {
            pattern = std::make_pair(2, 1);
            input.consume();
            return true;
        }

        if(identMatches("even", 4, input->data())) {
            pattern = std::make_pair(2, 0);
            input.consume();
            return true;
        }
    }

    std::stringstream ss;
    if(input->type() == CSSToken::Type::Delim) {
        if(input->delim() != '+')
            return false;
        input.consume();
        if(input->type() != CSSToken::Type::Ident)
            return false;
        pattern.first = 1;
        ss << input->data();
        input.consume();
    } else if(input->type() == CSSToken::Type::Ident) {
        auto ident = input->data();
        input.consume();
        if(ident.front() == '-') {
            pattern.first = -1;
            ss << ident.substr(1);
        } else {
            pattern.first = 1;
            ss << ident;
        }
    } else if(input->type() == CSSToken::Type::Dimension) {
        if(input->numberType() != CSSToken::NumberType::Integer)
            return false;
        pattern.first = input->integer();
        ss << input->data();
        input.consume();
    }

    constexpr auto eof = std::stringstream::traits_type::eof();
    if(ss.peek() == eof || !equals(ss.get(), 'n', false))
        return false;
    auto sign = CSSToken::NumberSign::None;
    if(ss.peek() != eof) {
        if(ss.get() != '-')
            return false;
        sign = CSSToken::NumberSign::Minus;
        if(ss.peek() != eof) {
            ss >> pattern.second;
            if(ss.fail())
                return false;
            pattern.second = -pattern.second;
            return true;
        }
    }

    input.consumeWhitespace();
    if(sign == CSSToken::NumberSign::None && input->type() == CSSToken::Type::Delim) {
        auto delim = input->delim();
        if(delim == '+')
            sign = CSSToken::NumberSign::Plus;
        else if(delim == '-')
            sign = CSSToken::NumberSign::Minus;
        else
            return false;
        input.consumeIncludingWhitespace();
    }

    if(sign == CSSToken::NumberSign::None && input->type() != CSSToken::Type::Number) {
        pattern.second = 0;
        return true;
    }

    if(input->type() != CSSToken::Type::Number || input->numberType() != CSSToken::NumberType::Integer)
        return false;
    if(sign == CSSToken::NumberSign::None && input->numberSign() == CSSToken::NumberSign::None)
        return false;
    if(sign != CSSToken::NumberSign::None && input->numberSign() != CSSToken::NumberSign::None)
        return false;
    pattern.second = input->integer();
    if(sign == CSSToken::NumberSign::Minus)
        pattern.second = -pattern.second;
    input.consume();
    return true;
}

bool CSSParser::consumeFontFaceDescriptor(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id)
{
    RefPtr<CSSValue> value;
    switch(id) {
    case CSSPropertyID::Src:
        value = consumeFontFaceSrc(input);
        break;
    case CSSPropertyID::FontFamily:
        value = consumeFontFamilyName(input);
        break;
    case CSSPropertyID::FontWeight:
        value = consumeFontFaceWeight(input);
        break;
    case CSSPropertyID::FontStretch:
        value = consumeFontFaceStretch(input);
        break;
    case CSSPropertyID::FontStyle:
        value = consumeFontFaceStyle(input);
        break;
    case CSSPropertyID::UnicodeRange:
        value = consumeFontFaceUnicodeRange(input);
        break;
    case CSSPropertyID::FontFeatureSettings:
        value = consumeFontFeatureSettings(input);
        break;
    case CSSPropertyID::FontVariationSettings:
        value = consumeFontVariationSettings(input);
        break;
    default:
        return false;
    }

    input.consumeWhitespace();
    if(value && input.empty()) {
        addProperty(properties, id, false, std::move(value));
        return true;
    }

    return false;
}

bool CSSParser::consumeCounterStyleDescriptor(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id)
{
    RefPtr<CSSValue> value;
    switch(id) {
    case CSSPropertyID::System:
        value = consumeCounterStyleSystem(input);
        break;
    case CSSPropertyID::Negative:
        value = consumeCounterStyleNegative(input);
        break;
    case CSSPropertyID::Prefix:
    case CSSPropertyID::Suffix:
        value = consumeCounterStyleSymbol(input);
        break;
    case CSSPropertyID::Range:
        value = consumeCounterStyleRange(input);
        break;
    case CSSPropertyID::Pad:
        value = consumeCounterStylePad(input);
        break;
    case CSSPropertyID::Fallback:
        value = consumeCounterStyleName(input);
        break;
    case CSSPropertyID::Symbols:
        value = consumeCounterStyleSymbols(input);
        break;
    case CSSPropertyID::AdditiveSymbols:
        value = consumeCounterStyleAdditiveSymbols(input);
        break;
    default:
        return false;
    }

    input.consumeWhitespace();
    if(value && input.empty()) {
        addProperty(properties, id, false, std::move(value));
        return true;
    }

    return false;
}

static RefPtr<CSSValue> consumeWideKeyword(CSSTokenStream& input)
{
    if(input->type() != CSSToken::Type::Ident) {
        return nullptr;
    }

    if(identMatches("initial", 7, input->data())) {
        input.consumeIncludingWhitespace();
        return CSSInitialValue::create();
    }

    if(identMatches("inherit", 7, input->data())) {
        input.consumeIncludingWhitespace();
        return CSSInheritValue::create();
    }

    if(identMatches("unset", 5, input->data())) {
        input.consumeIncludingWhitespace();
        return CSSUnsetValue::create();
    }

    return nullptr;
}

static bool containsVariableReferences(CSSTokenStream input)
{
    while(!input.empty()) {
        if(input->type() == CSSToken::Type::Function && identMatches("var", 3, input->data()))
            return true;
        input.consumeIncludingWhitespace();
    }

    return false;
}

bool CSSParser::consumeDescriptor(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important)
{
    if(containsVariableReferences(input)) {
        auto variable = CSSVariableReferenceValue::create(m_heap, m_context, id, important, CSSVariableData::create(m_heap, input));
        addProperty(properties, id, important, std::move(variable));
        return true;
    }

    if(auto value = consumeWideKeyword(input)) {
        if(!input.empty())
            return false;
        addExpandedProperty(properties, id, important, std::move(value));
        return true;
    }

    switch(id) {
    case CSSPropertyID::BorderTop:
    case CSSPropertyID::BorderRight:
    case CSSPropertyID::BorderBottom:
    case CSSPropertyID::BorderLeft:
    case CSSPropertyID::FlexFlow:
    case CSSPropertyID::ColumnRule:
    case CSSPropertyID::Outline:
    case CSSPropertyID::TextDecoration:
        return consumeShorthand(input, properties, id, important);
    case CSSPropertyID::Margin:
    case CSSPropertyID::Padding:
    case CSSPropertyID::BorderColor:
    case CSSPropertyID::BorderStyle:
    case CSSPropertyID::BorderWidth:
        return consume4Shorthand(input, properties, id, important);
    case CSSPropertyID::Gap:
    case CSSPropertyID::BorderSpacing:
        return consume2Shorthand(input, properties, id, important);
    case CSSPropertyID::Background:
        return consumeBackground(input, properties, important);
    case CSSPropertyID::Font:
        return consumeFont(input, properties, important);
    case CSSPropertyID::FontVariant:
        return consumeFontVariant(input, properties, important);
    case CSSPropertyID::Border:
        return consumeBorder(input, properties, important);
    case CSSPropertyID::BorderRadius:
        return consumeBorderRadius(input, properties, important);
    case CSSPropertyID::Columns:
        return consumeColumns(input, properties, important);
    case CSSPropertyID::Flex:
        return consumeFlex(input, properties, important);
    case CSSPropertyID::ListStyle:
        return consumeListStyle(input, properties, important);
    case CSSPropertyID::Marker:
        return consumeMarker(input, properties, important);
    default:
        break;
    }

    if(auto value = consumeLonghand(input, id)) {
        input.consumeWhitespace();
        if(input.empty()) {
            addProperty(properties, id, important, std::move(value));
            return true;
        }
    }

    return false;
}

constexpr bool isCustomPropertyName(std::string_view name)
{
    return name.length() > 2 && name[0] == '-' && name[1] == '-';
}

static CSSPropertyID csspropertyid(const std::string_view& name)
{
    if(isCustomPropertyName(name))
        return CSSPropertyID::Custom;
    static const struct {
        std::string_view name;
        CSSPropertyID value;
    } table[] = {
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
        {"border", CSSPropertyID::Border},
        {"border-bottom", CSSPropertyID::BorderBottom},
        {"border-bottom-color", CSSPropertyID::BorderBottomColor},
        {"border-bottom-left-radius", CSSPropertyID::BorderBottomLeftRadius},
        {"border-bottom-right-radius", CSSPropertyID::BorderBottomRightRadius},
        {"border-bottom-style", CSSPropertyID::BorderBottomStyle},
        {"border-bottom-width", CSSPropertyID::BorderBottomWidth},
        {"border-collapse", CSSPropertyID::BorderCollapse},
        {"border-color", CSSPropertyID::BorderColor},
        {"border-horizontal-spacing", CSSPropertyID::BorderHorizontalSpacing},
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
        {"justify-content", CSSPropertyID::JustifyContent},
        {"left", CSSPropertyID::Left},
        {"letter-spacing", CSSPropertyID::LetterSpacing},
        {"line-height", CSSPropertyID::LineHeight},
        {"list-style", CSSPropertyID::ListStyle},
        {"list-style-image", CSSPropertyID::ListStyleImage},
        {"list-style-position", CSSPropertyID::ListStylePosition},
        {"list-style-type", CSSPropertyID::ListStyleType},
        {"margin", CSSPropertyID::Margin},
        {"margin-bottom", CSSPropertyID::MarginBottom},
        {"margin-left", CSSPropertyID::MarginLeft},
        {"margin-right", CSSPropertyID::MarginRight},
        {"margin-top", CSSPropertyID::MarginTop},
        {"marker", CSSPropertyID::Marker},
        {"marker-end", CSSPropertyID::MarkerEnd},
        {"marker-mid", CSSPropertyID::MarkerMid},
        {"marker-start", CSSPropertyID::MarkerStart},
        {"mask", CSSPropertyID::Mask},
        {"mask-type", CSSPropertyID::MaskType},
        {"max-height", CSSPropertyID::MaxHeight},
        {"max-width", CSSPropertyID::MaxWidth},
        {"min-height", CSSPropertyID::MinHeight},
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
        {"padding-bottom", CSSPropertyID::PaddingBottom},
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

bool CSSParser::consumeDeclaraction(CSSTokenStream& input, CSSPropertyList& properties, CSSRuleType ruleType)
{
    auto begin = input.begin();
    while(!input.empty() && input->type() != CSSToken::Type::Semicolon) {
        input.consumeComponent();
    }

    CSSTokenStream newInput(begin, input.begin());
    if(newInput->type() != CSSToken::Type::Ident)
        return false;
    auto name = newInput->data();
    auto id = csspropertyid(name);
    if(id == CSSPropertyID::Unknown)
        return false;
    newInput.consumeIncludingWhitespace();
    if(newInput->type() != CSSToken::Type::Colon)
        return false;
    newInput.consumeIncludingWhitespace();
    auto valueBegin = newInput.begin();
    auto valueEnd = newInput.end();
    auto it = valueEnd - 1;
    while(it->type() == CSSToken::Type::Whitespace) {
        --it;
    }

    bool important = false;
    if(it->type() == CSSToken::Type::Ident && identMatches("important", 9, it->data())) {
        do {
            --it;
        } while(it->type() == CSSToken::Type::Whitespace);
        if(it->type() == CSSToken::Type::Delim && it->delim() == '!') {
            important = true;
            valueEnd = it;
        }
    }

    if(important && (ruleType == CSSRuleType::FontFace || ruleType == CSSRuleType::CounterStyle))
        return false;
    CSSTokenStream value(valueBegin, valueEnd);
    if(id == CSSPropertyID::Custom) {
        if(ruleType == CSSRuleType::FontFace || ruleType == CSSRuleType::CounterStyle)
            return false;
        auto custom = CSSCustomPropertyValue::create(m_heap, GlobalString(name), CSSVariableData::create(m_heap, value));
        addProperty(properties, id, important, std::move(custom));
        return true;
    }

    switch(ruleType) {
    case CSSRuleType::FontFace:
        return consumeFontFaceDescriptor(value, properties, id);
    case CSSRuleType::CounterStyle:
        return consumeCounterStyleDescriptor(value, properties, id);
    default:
        return consumeDescriptor(value, properties, id, important);
    }
}

void CSSParser::consumeDeclaractionList(CSSTokenStream& input, CSSPropertyList& properties, CSSRuleType ruleType)
{
    while(!input.empty()) {
        switch(input->type()) {
        case CSSToken::Type::Whitespace:
        case CSSToken::Type::Semicolon:
            input.consume();
            break;
        default:
            consumeDeclaraction(input, properties, ruleType);
            break;
        }
    }
}

void CSSParser::addProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, RefPtr<CSSValue> value)
{
    if(value == nullptr) {
        switch(id) {
        case CSSPropertyID::FontStyle:
        case CSSPropertyID::FontWeight:
        case CSSPropertyID::FontStretch:
        case CSSPropertyID::FontVariantCaps:
        case CSSPropertyID::FontVariantEmoji:
        case CSSPropertyID::FontVariantEastAsian:
        case CSSPropertyID::FontVariantLigatures:
        case CSSPropertyID::FontVariantNumeric:
        case CSSPropertyID::FontVariantPosition:
        case CSSPropertyID::LineHeight:
            value = CSSIdentValue::create(CSSValueID::Normal);
            break;
        case CSSPropertyID::ColumnWidth:
        case CSSPropertyID::ColumnCount:
            value = CSSIdentValue::create(CSSValueID::Auto);
            break;
        case CSSPropertyID::FlexGrow:
        case CSSPropertyID::FlexShrink:
            value = CSSNumberValue::create(m_heap, 1.0);
            break;
        case CSSPropertyID::FlexBasis:
            value = CSSPercentValue::create(m_heap, 0.0);
            break;
        default:
            value = CSSInitialValue::create();
            break;
        }
    }

    properties.emplace_back(id, m_context.origin(), important, std::move(value));
}

class CSSShorthand {
public:
    static CSSShorthand longhand(CSSPropertyID id);

    CSSPropertyID at(size_t index) const { return m_data[index]; }
    size_t length() const { return m_length; }
    bool empty() const { return m_length == 0; }

private:
    CSSShorthand(const CSSPropertyID* data, size_t length)
        : m_data(data), m_length(length)
    {}

    const CSSPropertyID* m_data;
    size_t m_length;
};

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

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderStyle: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopStyle,
            CSSPropertyID::BorderRightStyle,
            CSSPropertyID::BorderBottomStyle,
            CSSPropertyID::BorderLeftStyle
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderWidth: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopWidth,
            CSSPropertyID::BorderRightWidth,
            CSSPropertyID::BorderBottomWidth,
            CSSPropertyID::BorderLeftWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderTop: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopColor,
            CSSPropertyID::BorderTopStyle,
            CSSPropertyID::BorderTopWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderRight: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderRightColor,
            CSSPropertyID::BorderRightStyle,
            CSSPropertyID::BorderRightWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderBottom: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderBottomColor,
            CSSPropertyID::BorderBottomStyle,
            CSSPropertyID::BorderBottomWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderLeft: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderLeftColor,
            CSSPropertyID::BorderLeftStyle,
            CSSPropertyID::BorderLeftWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderRadius: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderTopRightRadius,
            CSSPropertyID::BorderTopLeftRadius,
            CSSPropertyID::BorderBottomLeftRadius,
            CSSPropertyID::BorderBottomRightRadius
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::BorderSpacing: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::BorderHorizontalSpacing,
            CSSPropertyID::BorderVerticalSpacing
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Padding: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::PaddingTop,
            CSSPropertyID::PaddingRight,
            CSSPropertyID::PaddingBottom,
            CSSPropertyID::PaddingLeft
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Margin: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::MarginTop,
            CSSPropertyID::MarginRight,
            CSSPropertyID::MarginBottom,
            CSSPropertyID::MarginLeft
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Outline: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::OutlineColor,
            CSSPropertyID::OutlineStyle,
            CSSPropertyID::OutlineWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::ListStyle: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::ListStyleType,
            CSSPropertyID::ListStylePosition,
            CSSPropertyID::ListStyleImage
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::ColumnRule: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::ColumnRuleColor,
            CSSPropertyID::ColumnRuleStyle,
            CSSPropertyID::ColumnRuleWidth
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::FlexFlow: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::FlexDirection,
            CSSPropertyID::FlexWrap
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Flex: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::FlexGrow,
            CSSPropertyID::FlexShrink,
            CSSPropertyID::FlexBasis
        };

        return CSSShorthand(data, std::size(data));
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

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Gap: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::RowGap,
            CSSPropertyID::ColumnGap
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Columns: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::ColumnWidth,
            CSSPropertyID::ColumnCount
        };

        return CSSShorthand(data, std::size(data));
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

        return CSSShorthand(data, std::size(data));
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

        return CSSShorthand(data, std::size(data));
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

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::TextDecoration: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::TextDecorationLine,
            CSSPropertyID::TextDecorationStyle,
            CSSPropertyID::TextDecorationColor
        };

        return CSSShorthand(data, std::size(data));
    }

    case CSSPropertyID::Marker: {
        static const CSSPropertyID data[] = {
            CSSPropertyID::MarkerStart,
            CSSPropertyID::MarkerMid,
            CSSPropertyID::MarkerEnd
        };

        return CSSShorthand(data, std::size(data));
    }

    default:
        return CSSShorthand(nullptr, 0);
    }
}

void CSSParser::addExpandedProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, RefPtr<CSSValue> value)
{
    auto longhand = CSSShorthand::longhand(id);
    if(longhand.empty()) {
        addProperty(properties, id, important, std::move(value));
        return;
    }

    size_t index = 0;
    do {
        addProperty(properties, longhand.at(index), important, value);
        index += 1;
    } while(index < longhand.length());
}

template<unsigned int N>
static CSSValueID matchIdent(const CSSTokenStream& input, const CSSIdentValueEntry(&table)[N])
{
    if(input->type() == CSSToken::Type::Ident) {
        if(auto id = matchIdent(table, input->data())) {
            return id.value();
        }
    }

    return CSSValueID::Unknown;
}

template<unsigned int N>
static RefPtr<CSSIdentValue> consumeIdent(CSSTokenStream& input, const CSSIdentValueEntry(&table)[N])
{
    auto id = matchIdent(input, table);
    if(id == CSSValueID::Unknown)
        return nullptr;
    input.consumeIncludingWhitespace();
    return CSSIdentValue::create(id);
}

RefPtr<CSSIdentValue> CSSParser::consumeFontStyleIdent(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"normal", CSSValueID::Normal},
        {"italic", CSSValueID::Italic},
        {"oblique", CSSValueID::Oblique}
    };

    return consumeIdent(input, table);
}

RefPtr<CSSIdentValue> CSSParser::consumeFontStretchIdent(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"normal", CSSValueID::Normal},
        {"ultra-condensed", CSSValueID::UltraCondensed},
        {"extra-condensed", CSSValueID::ExtraCondensed},
        {"condensed", CSSValueID::Condensed},
        {"semi-condensed", CSSValueID::SemiCondensed},
        {"semi-expanded", CSSValueID::SemiExpanded},
        {"expanded", CSSValueID::Expanded},
        {"extra-expanded", CSSValueID::ExtraExpanded},
        {"ultra-expanded", CSSValueID::UltraExpanded}
    };

    return consumeIdent(input, table);
}

RefPtr<CSSIdentValue> CSSParser::consumeFontVariantCapsIdent(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"small-caps", CSSValueID::SmallCaps},
        {"all-small-caps", CSSValueID::AllSmallCaps},
        {"petite-caps", CSSValueID::PetiteCaps},
        {"all-petite-caps", CSSValueID::AllPetiteCaps},
        {"unicase", CSSValueID::Unicase},
        {"titling-caps", CSSValueID::TitlingCaps}
    };

    return consumeIdent(input, table);
}

RefPtr<CSSIdentValue> CSSParser::consumeFontVariantEmojiIdent(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"text", CSSValueID::Text},
        {"emoji", CSSValueID::Emoji},
        {"unicode", CSSValueID::Unicode}
    };

    return consumeIdent(input, table);
}

RefPtr<CSSIdentValue> CSSParser::consumeFontVariantPositionIdent(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"sub", CSSValueID::Sub},
        {"super", CSSValueID::Super}
    };

    return consumeIdent(input, table);
}

RefPtr<CSSIdentValue> CSSParser::consumeFontVariantEastAsianIdent(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"jis78", CSSValueID::Jis78},
        {"jis83", CSSValueID::Jis83},
        {"jis90", CSSValueID::Jis90},
        {"jis04", CSSValueID::Jis04},
        {"simplified", CSSValueID::Simplified},
        {"traditional", CSSValueID::Traditional},
        {"full-width", CSSValueID::FullWidth},
        {"proportional-width", CSSValueID::ProportionalWidth},
        {"ruby", CSSValueID::Ruby}
    };

    return consumeIdent(input, table);
}

RefPtr<CSSIdentValue> CSSParser::consumeFontVariantLigaturesIdent(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"common-ligatures", CSSValueID::CommonLigatures},
        {"no-common-ligatures", CSSValueID::NoCommonLigatures},
        {"historical-ligatures", CSSValueID::HistoricalLigatures},
        {"no-historical-ligatures", CSSValueID::NoHistoricalLigatures},
        {"discretionary-ligatures", CSSValueID::DiscretionaryLigatures},
        {"no-discretionary-ligatures", CSSValueID::NoDiscretionaryLigatures},
        {"contextual", CSSValueID::Contextual},
        {"no-contextual", CSSValueID::NoContextual}
    };

    return consumeIdent(input, table);
}

RefPtr<CSSIdentValue> CSSParser::consumeFontVariantNumericIdent(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"lining-nums", CSSValueID::LiningNums},
        {"oldstyle-nums", CSSValueID::OldstyleNums},
        {"proportional-nums", CSSValueID::ProportionalNums},
        {"tabular-nums", CSSValueID::TabularNums},
        {"diagonal-fractions", CSSValueID::DiagonalFractions},
        {"stacked-fractions", CSSValueID::StackedFractions},
        {"ordinal", CSSValueID::Ordinal},
        {"slashed-zero", CSSValueID::SlashedZero}
    };

    return consumeIdent(input, table);
}

RefPtr<CSSValue> CSSParser::consumeNone(CSSTokenStream& input)
{
    if(consumeIdentIncludingWhitespace(input, "none", 4))
        return CSSIdentValue::create(CSSValueID::None);
    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeAuto(CSSTokenStream& input)
{
    if(consumeIdentIncludingWhitespace(input, "auto", 4))
        return CSSIdentValue::create(CSSValueID::Auto);
    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeNormal(CSSTokenStream& input)
{
    if(consumeIdentIncludingWhitespace(input, "normal", 6))
        return CSSIdentValue::create(CSSValueID::Normal);
    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeNoneOrAuto(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;
    return consumeAuto(input);
}

RefPtr<CSSValue> CSSParser::consumeNoneOrNormal(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;
    return consumeNormal(input);
}

RefPtr<CSSValue> CSSParser::consumeInteger(CSSTokenStream& input, bool negative)
{
    if(input->type() != CSSToken::Type::Number || input->numberType() != CSSToken::NumberType::Integer || (input->integer() < 0 && !negative))
        return nullptr;
    auto value = input->integer();
    input.consumeIncludingWhitespace();
    return CSSIntegerValue::create(m_heap, value);
}

RefPtr<CSSValue> CSSParser::consumeIntegerOrAuto(CSSTokenStream& input, bool negative)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumeInteger(input, negative);
}

RefPtr<CSSValue> CSSParser::consumePositiveInteger(CSSTokenStream& input)
{
    if(input->type() != CSSToken::Type::Number || input->numberType() != CSSToken::NumberType::Integer || input->integer() < 1)
        return nullptr;
    auto value = input->integer();
    input.consumeIncludingWhitespace();
    return CSSIntegerValue::create(m_heap, value);
}

RefPtr<CSSValue> CSSParser::consumePositiveIntegerOrAuto(CSSTokenStream& input)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumePositiveInteger(input);
}

RefPtr<CSSValue> CSSParser::consumeNumber(CSSTokenStream& input, bool negative)
{
    if(input->type() != CSSToken::Type::Number || (input->number() < 0 && !negative))
        return nullptr;
    auto value = input->number();
    input.consumeIncludingWhitespace();
    return CSSNumberValue::create(m_heap, value);
}

RefPtr<CSSValue> CSSParser::consumePercent(CSSTokenStream& input, bool negative)
{
    if(input->type() != CSSToken::Type::Percentage || (input->number() < 0 && !negative))
        return nullptr;
    auto value = input->number();
    input.consumeIncludingWhitespace();
    return CSSPercentValue::create(m_heap, value);
}

RefPtr<CSSValue> CSSParser::consumeNumberOrPercent(CSSTokenStream& input, bool negative)
{
    if(auto value = consumeNumber(input, negative))
        return value;
    return consumePercent(input, negative);
}

RefPtr<CSSValue> CSSParser::consumeNumberOrPercentOrAuto(CSSTokenStream& input, bool negative)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumeNumberOrPercent(input, negative);
}

static std::optional<CSSLengthUnits> matchUnitType(std::string_view name)
{
    static const CSSIdentEntry<CSSLengthUnits> table[] = {
        {"px", CSSLengthUnits::Pixels},
        {"pt", CSSLengthUnits::Points},
        {"pc", CSSLengthUnits::Picas},
        {"cm", CSSLengthUnits::Centimeters},
        {"mm", CSSLengthUnits::Millimeters},
        {"in", CSSLengthUnits::Inches},
        {"vw", CSSLengthUnits::ViewportWidth},
        {"vh", CSSLengthUnits::ViewportHeight},
        {"vmin", CSSLengthUnits::ViewportMin},
        {"vmax", CSSLengthUnits::ViewportMax},
        {"em", CSSLengthUnits::Ems},
        {"ex", CSSLengthUnits::Exs},
        {"ch", CSSLengthUnits::Chs},
        {"rem", CSSLengthUnits::Rems}
    };

    return matchIdent(table, name);
}

static bool isValidCalcFunction(std::string_view name)
{
    return identMatches("calc", 4, name) || identMatches("clamp", 5, name)
        || identMatches("min", 3, name) || identMatches("max", 3, name);
}

static CSSCalcOperator convertCalcDelim(const CSSToken& token)
{
    switch(token.delim()) {
    case '+':
        return CSSCalcOperator::Add;
    case '-':
        return CSSCalcOperator::Sub;
    case '*':
        return CSSCalcOperator::Mul;
    case '/':
        return CSSCalcOperator::Div;
    default:
        return CSSCalcOperator::None;
    }
}

static bool consumeCalcBlock(CSSTokenStream& input, CSSTokenList& stack, CSSCalcList& values)
{
    assert(input->type() == CSSToken::Type::Function || input->type() == CSSToken::Type::LeftParenthesis);
    stack.push_back(input.get());
    auto block = input.consumeBlock();
    block.consumeWhitespace();
    while(!block.empty()) {
        const auto& token = block.get();
        if(token.type() == CSSToken::Type::Number) {
            values.emplace_back(token.number());
            block.consumeIncludingWhitespace();
        } else if(token.type() == CSSToken::Type::Dimension) {
            auto unitType = matchUnitType(token.data());
            if(unitType == std::nullopt)
                return false;
            values.emplace_back(token.number(), unitType.value());
            block.consumeIncludingWhitespace();
        } else if(token.type() == CSSToken::Type::Delim) {
            auto token_op = convertCalcDelim(token);
            if(token_op == CSSCalcOperator::None)
                return false;
            while(!stack.empty()) {
                if(stack.back().type() != CSSToken::Type::Delim)
                    break;
                auto stack_op = convertCalcDelim(stack.back());
                if((token_op == CSSCalcOperator::Mul || token_op == CSSCalcOperator::Div)
                    && (stack_op == CSSCalcOperator::Add || stack_op == CSSCalcOperator::Sub)) {
                    break;
                }

                values.emplace_back(stack_op);
                stack.pop_back();
            }

            stack.push_back(token);
            block.consumeIncludingWhitespace();
        } else if(token.type() == CSSToken::Type::Function) {
            if(!isValidCalcFunction(token.data()))
                return false;
            if(!consumeCalcBlock(block, stack, values))
                return false;
            block.consumeWhitespace();
        } else if(token.type() == CSSToken::Type::LeftParenthesis) {
            if(!consumeCalcBlock(block, stack, values))
                return false;
            block.consumeWhitespace();
        } else if(token.type() == CSSToken::Type::Comma) {
            while(!stack.empty()) {
                if(stack.back().type() != CSSToken::Type::Delim)
                    break;
                values.emplace_back(convertCalcDelim(stack.back()));
                stack.pop_back();
            }

            if(stack.empty() || stack.back().type() == CSSToken::Type::LeftParenthesis)
                return false;
            stack.push_back(token);
            block.consumeIncludingWhitespace();
        } else {
            return false;
        }
    }

    size_t commaCount = 0;
    while(!stack.empty()) {
        if(stack.back().type() == CSSToken::Type::Delim) {
            values.emplace_back(convertCalcDelim(stack.back()));
        } else if(stack.back().type() == CSSToken::Type::Comma) {
            ++commaCount;
        } else {
            break;
        }

        stack.pop_back();
    }

    if(stack.empty())
        return false;
    auto left = stack.back();
    stack.pop_back();
    if(left.type() == CSSToken::Type::LeftParenthesis)
        return commaCount == 0;
    assert(left.type() == CSSToken::Type::Function);
    if(identMatches("calc", 4, left.data())) {
        return commaCount == 0;
    }

    if(identMatches("clamp", 5, left.data())) {
        if(commaCount != 2)
            return false;
        values.emplace_back(CSSCalcOperator::Min);
        values.emplace_back(CSSCalcOperator::Max);
        return true;
    }

    auto op = identMatches("min", 3, left.data()) ? CSSCalcOperator::Min : CSSCalcOperator::Max;
    for(size_t i = 0; i < commaCount; ++i)
        values.emplace_back(op);
    return true;
}

RefPtr<CSSValue> CSSParser::consumeCalc(CSSTokenStream& input, bool negative, bool unitless)
{
    if(input->type() != CSSToken::Type::Function || !isValidCalcFunction(input->data()))
        return nullptr;
    CSSTokenList stack;
    CSSCalcList values(m_heap);
    CSSTokenStreamGuard guard(input);
    if(!consumeCalcBlock(input, stack, values))
        return nullptr;
    input.consumeWhitespace();
    guard.release();

    unitless |= m_context.inSVGElement();
    while(!stack.empty()) {
        if(stack.back().type() == CSSToken::Type::Delim)
            values.emplace_back(convertCalcDelim(stack.back()));
        stack.pop_back();
    }

    return CSSCalcValue::create(m_heap, negative, unitless, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeLength(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumeCalc(input, negative, unitless))
        return value;
    if(input->type() != CSSToken::Type::Dimension && input->type() != CSSToken::Type::Number)
        return nullptr;
    auto value = input->number();
    if(value < 0.0 && !negative)
        return nullptr;
    if(input->type() == CSSToken::Type::Number) {
        if(value && !unitless && !m_context.inSVGElement())
            return nullptr;
        input.consumeIncludingWhitespace();
        return CSSLengthValue::create(m_heap, value, CSSLengthUnits::None);
    }

    auto unitType = matchUnitType(input->data());
    if(unitType == std::nullopt)
        return nullptr;
    input.consumeIncludingWhitespace();
    return CSSLengthValue::create(m_heap, value, unitType.value());
}

RefPtr<CSSValue> CSSParser::consumeLengthOrPercent(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumePercent(input, negative))
        return value;
    return consumeLength(input, negative, unitless);
}

RefPtr<CSSValue> CSSParser::consumeLengthOrAuto(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumeLength(input, negative, unitless);
}

RefPtr<CSSValue> CSSParser::consumeLengthOrNormal(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumeNormal(input))
        return value;
    return consumeLength(input, negative, unitless);
}

RefPtr<CSSValue> CSSParser::consumeLengthOrPercentOrAuto(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumeLengthOrPercent(input, negative, unitless);
}

RefPtr<CSSValue> CSSParser::consumeLengthOrPercentOrNone(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumeNone(input))
        return value;
    return consumeLengthOrPercent(input, negative, unitless);
}

RefPtr<CSSValue> CSSParser::consumeLengthOrPercentOrNormal(CSSTokenStream& input, bool negative, bool unitless)
{
    if(auto value = consumeNormal(input))
        return value;
    return consumeLengthOrPercent(input, negative, unitless);
}

RefPtr<CSSValue> CSSParser::consumeWidthOrHeight(CSSTokenStream& input, bool unitless)
{
    static const CSSIdentValueEntry table[] = {
        {"min-content", CSSValueID::MinContent},
        {"max-content", CSSValueID::MaxContent},
        {"fit-content", CSSValueID::FitContent}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    return consumeLengthOrPercent(input, false, unitless);
}

RefPtr<CSSValue> CSSParser::consumeWidthOrHeightOrAuto(CSSTokenStream& input, bool unitless)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumeWidthOrHeight(input, unitless);
}

RefPtr<CSSValue> CSSParser::consumeWidthOrHeightOrNone(CSSTokenStream& input, bool unitless)
{
    if(auto value = consumeNone(input))
        return value;
    return consumeWidthOrHeight(input, unitless);
}

RefPtr<CSSValue> CSSParser::consumeString(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::String) {
        auto value = m_heap->createString(input->data());
        input.consumeIncludingWhitespace();
        return CSSStringValue::create(m_heap, value);
    }

    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeCustomIdent(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::Ident) {
        auto value = GlobalString(input->data());
        input.consumeIncludingWhitespace();
        return CSSCustomIdentValue::create(m_heap, value);
    }

    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeStringOrCustomIdent(CSSTokenStream& input)
{
    if(auto value = consumeString(input))
        return value;
    return consumeCustomIdent(input);
}

RefPtr<CSSValue> CSSParser::consumeAttr(CSSTokenStream& input)
{
    if(input->type() != CSSToken::Type::Function || !identMatches("attr", 4, input->data()))
        return nullptr;
    CSSTokenStreamGuard guard(input);
    auto block = input.consumeBlock();
    block.consumeWhitespace();
    if(block->type() != CSSToken::Type::Ident)
        return nullptr;
    GlobalString name(block->data());
    if(m_context.inHTMLDocument()) {
        name = name.foldCase();
    }

    block.consumeIncludingWhitespace();
    if(block->type() == CSSToken::Type::Ident) {
        if(!identMatches("url", 3, block->data()) && !identMatches("string", 6, block->data()))
            return nullptr;
        block.consumeIncludingWhitespace();
    }

    HeapString fallback;
    if(block.consumeCommaIncludingWhitespace()) {
        if(block->type() != CSSToken::Type::String)
            return nullptr;
        fallback = m_heap->createString(block->data());
        block.consumeIncludingWhitespace();
    }

    if(!block.empty())
        return nullptr;
    input.consumeWhitespace();
    guard.release();
    return CSSAttrValue::create(m_heap, name, fallback);
}

RefPtr<CSSValue> CSSParser::consumeLocalUrl(CSSTokenStream& input)
{
    if(auto token = consumeUrlToken(input))
        return CSSLocalUrlValue::create(m_heap, m_heap->createString(token->data()));
    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeLocalUrlOrAttr(CSSTokenStream &input)
{
    if(auto value = consumeAttr(input))
        return value;
    return consumeLocalUrl(input);
}

RefPtr<CSSValue> CSSParser::consumeLocalUrlOrNone(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;
    return consumeLocalUrl(input);
}

RefPtr<CSSValue> CSSParser::consumeUrl(CSSTokenStream& input)
{
    if(auto token = consumeUrlToken(input))
        return CSSUrlValue::create(m_heap, m_context.completeUrl(token->data()));
    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeUrlOrNone(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;
    return consumeUrl(input);
}

RefPtr<CSSValue> CSSParser::consumeImage(CSSTokenStream& input)
{
    if(auto token = consumeUrlToken(input))
        return CSSImageValue::create(m_heap, m_context.completeUrl(token->data()));
    return nullptr;
}

RefPtr<CSSValue> CSSParser::consumeImageOrNone(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;
    return consumeImage(input);
}

RefPtr<CSSValue> CSSParser::consumeColor(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::Hash) {
        auto data = input->data();
        for(auto cc : data) {
            if(!isHexDigit(cc)) {
                return nullptr;
            }
        }

        int r, g, b, a = 255;
        if(data.size() == 3 || data.size() == 4) {
            r = toHexByte(data[0], data[0]);
            g = toHexByte(data[1], data[1]);
            b = toHexByte(data[2], data[2]);
            if(data.size() == 4) {
                a = toHexByte(data[3], data[3]);
            }
        } else if(data.size() == 6 || data.size() == 8) {
            r = toHexByte(data[0], data[1]);
            g = toHexByte(data[2], data[3]);
            b = toHexByte(data[4], data[5]);
            if(data.size() == 8) {
                a = toHexByte(data[6], data[7]);
            }
        } else {
            return nullptr;
        }

        input.consumeIncludingWhitespace();
        return CSSColorValue::create(m_heap, Color(r, g, b, a));
    }

    if(input->type() == CSSToken::Type::Function) {
        auto name = input->data();
        if(identMatches("rgb", 3, name) || identMatches("rgba", 4, name))
            return consumeRgb(input);
        if(identMatches("hsl", 3, name) || identMatches("hsla", 4, name))
            return consumeHsl(input);
        if(identMatches("hwb", 3, name))
            return consumeHwb(input);
        return nullptr;
    }

    if(input->type() == CSSToken::Type::Ident) {
        auto name = input->data();
        if(identMatches("currentcolor", 12, name)) {
            input.consumeIncludingWhitespace();
            return CSSIdentValue::create(CSSValueID::CurrentColor);
        }

        if(identMatches("transparent", 11, name)) {
            input.consumeIncludingWhitespace();
            return CSSColorValue::create(m_heap, Color::Transparent);
        }

        auto color = Color::named(name);
        if(color == std::nullopt)
            return nullptr;
        input.consumeIncludingWhitespace();
        return CSSColorValue::create(m_heap, color.value());
    }

    return nullptr;
}

static bool consumeRgbComponent(CSSTokenStream& input, int& component, bool requiresPercent)
{
    if(input->type() != CSSToken::Type::Number
        && input->type() != CSSToken::Type::Percentage) {
        return false;
    }

    if(requiresPercent && input->type() != CSSToken::Type::Percentage)
        return false;
    auto value = input->number();
    if(input->type() == CSSToken::Type::Percentage)
        value *= 2.55f;
    component = std::lroundf(std::clamp(value, 0.f, 255.f));
    input.consumeIncludingWhitespace();
    return true;
}

static bool consumeAlphaComponent(CSSTokenStream& input, int& component)
{
    if(input->type() != CSSToken::Type::Number
        && input->type() != CSSToken::Type::Percentage) {
        return false;
    }

    auto value = input->number();
    if(input->type() == CSSToken::Type::Percentage)
        value /= 100.f;
    component = std::lroundf(255.f * std::clamp(value, 0.f, 1.f));
    input.consumeIncludingWhitespace();
    return true;
}

static bool consumeAlphaDelimiter(CSSTokenStream& input, bool requiresComma)
{
    if(requiresComma)
        return input.consumeCommaIncludingWhitespace();
    if(input->type() == CSSToken::Type::Delim && input->delim() == '/') {
        input.consumeIncludingWhitespace();
        return true;
    }

    return false;
}

RefPtr<CSSValue> CSSParser::consumeRgb(CSSTokenStream& input)
{
    assert(input->type() == CSSToken::Type::Function);
    CSSTokenStreamGuard guard(input);
    auto block = input.consumeBlock();
    block.consumeWhitespace();

    auto requiresPercent = block->type() == CSSToken::Type::Percentage;

    int red = 0;
    if(!consumeRgbComponent(block, red, requiresPercent)) {
        return nullptr;
    }

    auto requiresComma = block.consumeCommaIncludingWhitespace();

    int green = 0;
    if(!consumeRgbComponent(block, green, requiresPercent)) {
        return nullptr;
    }

    if(requiresComma && !block.consumeCommaIncludingWhitespace()) {
        return nullptr;
    }

    int blue = 0;
    if(!consumeRgbComponent(block, blue, requiresPercent)) {
        return nullptr;
    }

    int alpha = 255;
    if(consumeAlphaDelimiter(block, requiresComma)) {
        if(!consumeAlphaComponent(block, alpha)) {
            return nullptr;
        }
    }

    if(!block.empty())
        return nullptr;
    input.consumeWhitespace();
    guard.release();
    return CSSColorValue::create(m_heap, Color(red, green, blue, alpha));
}

static bool consumeAngleComponent(CSSTokenStream& input, float& component)
{
    if(input->type() != CSSToken::Type::Number
        && input->type() != CSSToken::Type::Dimension) {
        return false;
    }

    component = input->number();
    if(input->type() == CSSToken::Type::Dimension) {
        static const CSSIdentEntry<CSSAngleValue::Unit> table[] = {
            {"deg", CSSAngleValue::Unit::Degrees},
            {"rad", CSSAngleValue::Unit::Radians},
            {"grad", CSSAngleValue::Unit::Gradians},
            {"turn", CSSAngleValue::Unit::Turns}
        };

        auto unitType = matchIdent(table, input->data());
        if(unitType == std::nullopt)
            return false;
        switch(unitType.value()) {
        case CSSAngleValue::Unit::Degrees:
            break;
        case CSSAngleValue::Unit::Radians:
            component = component * 180.0 / std::numbers::pi;
            break;
        case CSSAngleValue::Unit::Gradians:
            component = component * 360.0 / 400.0;
            break;
        case CSSAngleValue::Unit::Turns:
            component = component * 360.0;
            break;
        }
    }

    component = std::fmod(component, 360.f);
    if(component < 0.f) { component += 360.f; }

    input.consumeIncludingWhitespace();
    return true;
}

static bool consumePercentComponent(CSSTokenStream& input, float& component)
{
    if(input->type() != CSSToken::Type::Percentage)
        return false;
    auto value = input->number() / 100.f;
    component = std::clamp(value, 0.f, 1.f);
    input.consumeIncludingWhitespace();
    return true;
}

static int computeHslComponent(float h, float s, float l, float n)
{
    auto k = fmodf(n + h / 30.f, 12.f);
    auto a = s * std::min(l, 1.f - l);
    auto v = l - a * std::max(-1.f, std::min(1.f, std::min(k - 3.f, 9.f - k)));
    return std::lroundf(v * 255.f);
}

RefPtr<CSSValue> CSSParser::consumeHsl(CSSTokenStream& input)
{
    assert(input->type() == CSSToken::Type::Function);
    CSSTokenStreamGuard guard(input);
    auto block = input.consumeBlock();
    block.consumeWhitespace();

    float h, s, l, a = 1.f;
    if(!consumeAngleComponent(block, h)) {
        return nullptr;
    }

    auto requiresComma = block.consumeCommaIncludingWhitespace();

    if(!consumePercentComponent(block, s)) {
        return nullptr;
    }

    if(requiresComma && !block.consumeCommaIncludingWhitespace()) {
        return nullptr;
    }

    if(!consumePercentComponent(block, l)) {
        return nullptr;
    }

    int alpha = 255;
    if(consumeAlphaDelimiter(block, requiresComma)) {
        if(!consumeAlphaComponent(block, alpha)) {
            return nullptr;
        }
    }

    if(!block.empty())
        return nullptr;
    input.consumeWhitespace();
    guard.release();

    auto r = computeHslComponent(h, s, l, 0);
    auto g = computeHslComponent(h, s, l, 8);
    auto b = computeHslComponent(h, s, l, 4);
    return CSSColorValue::create(m_heap, Color(r, g, b, alpha));
}

RefPtr<CSSValue> CSSParser::consumeHwb(CSSTokenStream& input)
{
    assert(input->type() == CSSToken::Type::Function);
    CSSTokenStreamGuard guard(input);
    auto block = input.consumeBlock();
    block.consumeWhitespace();

    float hue, white, black;
    if(!consumeAngleComponent(block, hue)) {
        return nullptr;
    }

    auto requiresComma = block.consumeCommaIncludingWhitespace();

    if(!consumePercentComponent(block, white)) {
        return nullptr;
    }

    if(requiresComma && !block.consumeCommaIncludingWhitespace()) {
        return nullptr;
    }

    if(!consumePercentComponent(block, black)) {
        return nullptr;
    }

    int alpha = 255;
    if(consumeAlphaDelimiter(block, requiresComma)) {
        if(!consumeAlphaComponent(block, alpha)) {
            return nullptr;
        }
    }

    if(!block.empty())
        return nullptr;
    input.consumeWhitespace();
    guard.release();

    if(white + black > 1.0f) {
        auto sum = white + black;
        white /= sum;
        black /= sum;
    }

    int components[3] = { 0, 8, 4 };
    for(auto& component : components) {
        auto channel = computeHslComponent(hue, 1.0f, 0.5f, component);
        component = std::lroundf(channel * (1 - white - black) + (white * 255));
    }

    const auto r = components[0];
    const auto g = components[1];
    const auto b = components[2];
    return CSSColorValue::create(m_heap, Color(r, g, b, alpha));
}

RefPtr<CSSValue> CSSParser::consumePaint(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;
    auto first = consumeLocalUrl(input);
    if(first == nullptr)
        return consumeColor(input);
    auto second = consumeNone(input);
    if(second == nullptr)
        second = consumeColor(input);
    if(second == nullptr)
        return first;
    return CSSPairValue::create(m_heap, first, second);
}

RefPtr<CSSValue> CSSParser::consumeListStyleType(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"none", CSSValueID::None},
        {"disc", CSSValueID::Disc},
        {"circle", CSSValueID::Circle},
        {"square", CSSValueID::Square}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    return consumeStringOrCustomIdent(input);
}

RefPtr<CSSValue> CSSParser::consumeQuotes(CSSTokenStream& input)
{
    if(auto value = consumeNoneOrAuto(input))
        return value;
    CSSValueList values(m_heap);
    do {
        auto first = consumeString(input);
        if(first == nullptr)
            return nullptr;
        auto second = consumeString(input);
        if(second == nullptr)
            return nullptr;
        values.push_back(CSSPairValue::create(m_heap, first, second));
    } while(!input.empty());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeContent(CSSTokenStream& input)
{
    if(auto value = consumeNoneOrNormal(input))
        return value;
    CSSValueList values(m_heap);
    do {
        auto value = consumeString(input);
        if(value == nullptr)
            value = consumeImage(input);
        if(value == nullptr)
            value = consumeAttr(input);
        if(value == nullptr && input->type() == CSSToken::Type::Ident) {
            static const CSSIdentValueEntry table[] = {
                {"open-quote", CSSValueID::OpenQuote},
                {"close-quote", CSSValueID::CloseQuote},
                {"no-open-quote", CSSValueID::NoOpenQuote},
                {"no-close-quote", CSSValueID::NoCloseQuote}
            };

            value = consumeIdent(input, table);
        }

        if(value == nullptr && input->type() == CSSToken::Type::Function) {
            auto name = input->data();
            auto block = input.consumeBlock();
            block.consumeWhitespace();
            if(identMatches("leader", 6, name))
                value = consumeContentLeader(block);
            else if(identMatches("element", 7, name))
                value = consumeContentElement(block);
            else if(identMatches("counter", 7, name))
                value = consumeContentCounter(block, false);
            else if(identMatches("counters", 8, name))
                value = consumeContentCounter(block, true);
            else if(identMatches("target-counter", 14, name))
                value = consumeContentTargetCounter(block, false);
            else if(identMatches("target-counters", 15, name))
                value = consumeContentTargetCounter(block, false);
            else if(identMatches("-pluto-qrcode", 13, name))
                value = consumeContentQrCode(block);
            input.consumeWhitespace();
        }

        if(value == nullptr)
            return nullptr;
        values.push_back(std::move(value));
    } while(!input.empty());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeContentLeader(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"dotted", CSSValueID::Dotted},
        {"solid", CSSValueID::Solid},
        {"space", CSSValueID::Space}
    };

    auto value = consumeString(input);
    if(value == nullptr)
        value = consumeIdent(input, table);
    if(value == nullptr || !input.empty())
        return nullptr;
    return CSSUnaryFunctionValue::create(m_heap, CSSFunctionID::Leader, std::move(value));
}

RefPtr<CSSValue> CSSParser::consumeContentElement(CSSTokenStream& input)
{
    auto value = consumeCustomIdent(input);
    if(value == nullptr || !input.empty())
        return nullptr;
    return CSSUnaryFunctionValue::create(m_heap, CSSFunctionID::Element, std::move(value));
}

RefPtr<CSSValue> CSSParser::consumeContentCounter(CSSTokenStream& input, bool counters)
{
    if(input->type() != CSSToken::Type::Ident)
        return nullptr;
    auto identifier = GlobalString(input->data());
    input.consumeIncludingWhitespace();
    HeapString separator;
    if(counters) {
        if(!input.consumeCommaIncludingWhitespace())
            return nullptr;
        if(input->type() != CSSToken::Type::String)
            return nullptr;
        separator = m_heap->createString(input->data());
        input.consumeIncludingWhitespace();
    }

    GlobalString listStyle("decimal");
    if(input.consumeCommaIncludingWhitespace()) {
        if(input->type() != CSSToken::Type::Ident || identMatches("none", 4, input->data()))
            return nullptr;
        listStyle = GlobalString(input->data());
        input.consumeIncludingWhitespace();
    }

    if(!input.empty())
        return nullptr;
    return CSSCounterValue::create(m_heap, identifier, listStyle, separator);
}

RefPtr<CSSValue> CSSParser::consumeContentTargetCounter(CSSTokenStream& input, bool counters)
{
    auto fragment = consumeLocalUrlOrAttr(input);
    if(fragment == nullptr || !input.consumeCommaIncludingWhitespace())
        return nullptr;
    auto identifier = consumeCustomIdent(input);
    if(identifier == nullptr) {
        return nullptr;
    }

    CSSValueList values(m_heap);
    values.push_back(std::move(fragment));
    values.push_back(std::move(identifier));
    if(counters) {
        if(!input.consumeCommaIncludingWhitespace())
            return nullptr;
        auto separator = consumeString(input);
        if(separator == nullptr)
            return nullptr;
        values.push_back(std::move(separator));
        input.consumeWhitespace();
    }

    auto id = counters ? CSSFunctionID::TargetCounters : CSSFunctionID::TargetCounter;
    if(input.consumeCommaIncludingWhitespace()) {
        auto listStyle = consumeCustomIdent(input);
        if(listStyle == nullptr)
            return nullptr;
        values.push_back(std::move(listStyle));
        input.consumeWhitespace();
    }

    if(!input.empty())
        return nullptr;
    return CSSFunctionValue::create(m_heap, id, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeContentQrCode(CSSTokenStream& input)
{
    auto text = consumeString(input);
    if(text == nullptr)
        return nullptr;
    CSSValueList values(m_heap);
    values.push_back(std::move(text));
    if(input.consumeCommaIncludingWhitespace()) {
        auto fill = consumeColor(input);
        if(fill == nullptr)
            return nullptr;
        values.push_back(std::move(fill));
        input.consumeWhitespace();
    }

    if(!input.empty())
        return nullptr;
    return CSSFunctionValue::create(m_heap, CSSFunctionID::Qrcode, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeCounter(CSSTokenStream& input, bool increment)
{
    if(auto value = consumeNone(input))
        return value;
    CSSValueList values(m_heap);
    do {
        auto name = consumeCustomIdent(input);
        if(name == nullptr)
            return nullptr;
        auto value = consumeInteger(input, true);
        if(value == nullptr)
            value = CSSIntegerValue::create(m_heap, increment ? 1 : 0);
        values.push_back(CSSPairValue::create(m_heap, name, value));
    } while(!input.empty());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumePage(CSSTokenStream& input)
{
    if(auto value = consumeAuto(input))
        return value;
    return consumeCustomIdent(input);
}

RefPtr<CSSValue> CSSParser::consumeSize(CSSTokenStream& input)
{
    if(auto value = consumeAuto(input))
        return value;
    if(auto width = consumeLength(input, false, false)) {
        auto height = consumeLength(input, false, false);
        if(height == nullptr)
            height = width;
        return CSSPairValue::create(m_heap, width, height);
    }

    RefPtr<CSSValue> size;
    RefPtr<CSSValue> orientation;
    for(int index = 0; index < 2; ++index) {
        static const CSSIdentValueEntry table[] = {
            {"a3", CSSValueID::A3},
            {"a4", CSSValueID::A4},
            {"a5", CSSValueID::A5},
            {"b4", CSSValueID::B4},
            {"b5", CSSValueID::B5},
            {"ledger", CSSValueID::Ledger},
            {"legal", CSSValueID::Legal},
            {"letter", CSSValueID::Letter}
        };

        if(size == nullptr && (size = consumeIdent(input, table)))
            continue;
        if(orientation == nullptr && (orientation = consumeOrientation(input))) {
            continue;
        }

        break;
    }

    if(size == nullptr && orientation == nullptr)
        return nullptr;
    if(size == nullptr)
        return orientation;
    if(orientation == nullptr)
        return size;
    return CSSPairValue::create(m_heap, size, orientation);
}

RefPtr<CSSValue> CSSParser::consumeOrientation(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"portrait", CSSValueID::Portrait},
        {"landscape", CSSValueID::Landscape}
    };

    return consumeIdent(input, table);
}

RefPtr<CSSValue> CSSParser::consumeFontSize(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"xx-small", CSSValueID::XxSmall},
        {"x-small", CSSValueID::XSmall},
        {"small", CSSValueID::Small},
        {"medium", CSSValueID::Medium},
        {"large", CSSValueID::Large},
        {"x-large", CSSValueID::XLarge},
        {"xx-large", CSSValueID::XxLarge},
        {"xxx-large", CSSValueID::XxxLarge},
        {"smaller", CSSValueID::Smaller},
        {"larger", CSSValueID::Larger}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    return consumeLengthOrPercent(input, false, false);
}

RefPtr<CSSValue> CSSParser::consumeFontWeight(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"normal", CSSValueID::Normal},
        {"bold", CSSValueID::Bold},
        {"bolder", CSSValueID::Bolder},
        {"lighter", CSSValueID::Lighter}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    if(input->type() == CSSToken::Type::Number && (input->number() < 1 || input->number() > 1000))
        return nullptr;
    return consumeNumber(input, false);
}

RefPtr<CSSValue> CSSParser::consumeFontStyle(CSSTokenStream& input)
{
    auto ident = consumeFontStyleIdent(input);
    if(ident == nullptr)
        return nullptr;
    if(ident->value() == CSSValueID::Oblique) {
        if(auto angle = consumeAngle(input)) {
            return CSSPairValue::create(m_heap, ident, angle);
        }
    }

    return ident;
}

RefPtr<CSSValue> CSSParser::consumeFontStretch(CSSTokenStream& input)
{
    if(auto value = consumeFontStretchIdent(input))
        return value;
    return consumePercent(input, false);
}

RefPtr<CSSValue> CSSParser::consumeFontFamilyName(CSSTokenStream& input)
{
    if(input->type() == CSSToken::Type::String) {
        auto value = GlobalString(input->data());
        input.consumeIncludingWhitespace();
        return CSSCustomIdentValue::create(m_heap, value);
    }

    std::string value;
    while(input->type() == CSSToken::Type::Ident) {
        if(!value.empty())
            value += ' ';
        value += input->data();
        input.consumeIncludingWhitespace();
    }

    if(value.empty())
        return nullptr;
    return CSSCustomIdentValue::create(m_heap, GlobalString(value));
}

RefPtr<CSSValue> CSSParser::consumeFontFamily(CSSTokenStream& input)
{
    CSSValueList values(m_heap);
    do {
        auto value = consumeFontFamilyName(input);
        if(value == nullptr)
            return nullptr;
        values.push_back(std::move(value));
    } while(input.consumeCommaIncludingWhitespace());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeFontFeature(CSSTokenStream& input)
{
    constexpr auto kTagLength = 4;
    if(input->type() != CSSToken::Type::String)
        return nullptr;
    if(input->data().length() != kTagLength)
        return nullptr;
    for(auto cc : input->data()) {
        if(cc < 0x20 || cc > 0x7E) {
            return nullptr;
        }
    }

    GlobalString tag(input->data());
    input.consumeIncludingWhitespace();

    int value = 1;
    if(input->type() == CSSToken::Type::Number
        && input->numberType() == CSSToken::NumberType::Integer) {
        value = input->integer();
        input.consumeIncludingWhitespace();
    } else if(input->type() == CSSToken::Type::Ident) {
        static const CSSIdentValueEntry table[] = {
            {"on", CSSValueID::On},
            {"off", CSSValueID::Off}
        };

        switch(matchIdent(input, table)) {
        case CSSValueID::On:
            value = 1;
            break;
        case CSSValueID::Off:
            value = 0;
            break;
        default:
            return nullptr;
        };

        input.consumeIncludingWhitespace();
    }

    return CSSFontFeatureValue::create(m_heap, tag, value);
}

RefPtr<CSSValue> CSSParser::consumeFontFeatureSettings(CSSTokenStream& input)
{
    if(auto value = consumeNormal(input))
        return value;
    CSSValueList values(m_heap);
    do {
        auto value = consumeFontFeature(input);
        if(value == nullptr)
            return nullptr;
        values.push_back(std::move(value));
    } while(input.consumeCommaIncludingWhitespace());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeFontVariation(CSSTokenStream& input)
{
    constexpr auto kTagLength = 4;
    if(input->type() != CSSToken::Type::String)
        return nullptr;
    if(input->data().length() != kTagLength)
        return nullptr;
    for(auto cc : input->data()) {
        if(cc < 0x20 || cc > 0x7E) {
            return nullptr;
        }
    }

    GlobalString tag(input->data());
    input.consumeIncludingWhitespace();
    if(input->type() != CSSToken::Type::Number)
        return nullptr;
    auto value = input->number();
    input.consumeIncludingWhitespace();
    return CSSFontVariationValue::create(m_heap, tag, value);
}

RefPtr<CSSValue> CSSParser::consumeFontVariationSettings(CSSTokenStream& input)
{
    if(auto value = consumeNormal(input))
        return value;
    CSSValueList values(m_heap);
    do {
        auto value = consumeFontVariation(input);
        if(value == nullptr)
            return nullptr;
        values.push_back(std::move(value));
    } while(input.consumeCommaIncludingWhitespace());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeFontVariantCaps(CSSTokenStream& input)
{
    if(auto value = consumeNormal(input))
        return value;
    return consumeFontVariantCapsIdent(input);
}

RefPtr<CSSValue> CSSParser::consumeFontVariantEmoji(CSSTokenStream& input)
{
    if(auto value = consumeNormal(input))
        return value;
    return consumeFontVariantEmojiIdent(input);
}

RefPtr<CSSValue> CSSParser::consumeFontVariantPosition(CSSTokenStream& input)
{
    if(auto value = consumeNormal(input))
        return value;
    return consumeFontVariantPositionIdent(input);
}

RefPtr<CSSValue> CSSParser::consumeFontVariantEastAsian(CSSTokenStream& input)
{
    if(auto value = consumeNormal(input)) {
        return value;
    }

    bool consumedEastAsianVariant = false;
    bool consumedEastAsianWidth = false;
    bool consumedEastAsianRuby = false;

    CSSValueList values(m_heap);
    do {
        auto ident = consumeFontVariantEastAsianIdent(input);
        if(ident == nullptr)
            return nullptr;
        switch(ident->value()) {
        case CSSValueID::Jis78:
        case CSSValueID::Jis83:
        case CSSValueID::Jis90:
        case CSSValueID::Jis04:
        case CSSValueID::Simplified:
        case CSSValueID::Traditional:
            if(consumedEastAsianVariant)
                return nullptr;
            consumedEastAsianVariant = true;
            break;
        case CSSValueID::FullWidth:
        case CSSValueID::ProportionalWidth:
            if(consumedEastAsianWidth)
                return nullptr;
            consumedEastAsianWidth = true;
            break;
        case CSSValueID::Ruby:
            if(consumedEastAsianRuby)
                return nullptr;
            consumedEastAsianRuby = true;
            break;
        default:
            assert(false);
        }

        values.push_back(std::move(ident));
    } while(!input.empty());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeFontVariantLigatures(CSSTokenStream& input)
{
    if(auto value = consumeNoneOrNormal(input)) {
        return value;
    }

    bool consumedCommonLigatures = false;
    bool consumedHistoricalLigatures = false;
    bool consumedDiscretionaryLigatures = false;
    bool consumedContextualLigatures = false;

    CSSValueList values(m_heap);
    do {
        auto ident = consumeFontVariantLigaturesIdent(input);
        if(ident == nullptr)
            return nullptr;
        switch(ident->value()) {
        case CSSValueID::CommonLigatures:
        case CSSValueID::NoCommonLigatures:
            if(consumedCommonLigatures)
                return nullptr;
            consumedCommonLigatures = true;
            break;
        case CSSValueID::HistoricalLigatures:
        case CSSValueID::NoHistoricalLigatures:
            if(consumedHistoricalLigatures)
                return nullptr;
            consumedHistoricalLigatures = true;
            break;
        case CSSValueID::DiscretionaryLigatures:
        case CSSValueID::NoDiscretionaryLigatures:
            if(consumedDiscretionaryLigatures)
                return nullptr;
            consumedDiscretionaryLigatures = true;
            break;
        case CSSValueID::Contextual:
        case CSSValueID::NoContextual:
            if(consumedContextualLigatures)
                return nullptr;
            consumedContextualLigatures = true;
            break;
        default:
            assert(false);
        }

        values.push_back(std::move(ident));
    } while(!input.empty());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeFontVariantNumeric(CSSTokenStream& input)
{
    if(auto value = consumeNormal(input)) {
        return value;
    }

    bool consumedNumericFigure = false;
    bool consumedNumericSpacing = false;
    bool consumedNumericFraction = false;
    bool consumedOrdinal = false;
    bool consumedSlashedZero = false;

    CSSValueList values(m_heap);
    do {
        auto ident = consumeFontVariantNumericIdent(input);
        if(ident == nullptr)
            return nullptr;
        switch(ident->value()) {
        case CSSValueID::LiningNums:
        case CSSValueID::OldstyleNums:
            if(consumedNumericFigure)
                return nullptr;
            consumedNumericFigure = true;
            break;
        case CSSValueID::ProportionalNums:
        case CSSValueID::TabularNums:
            if(consumedNumericSpacing)
                return nullptr;
            consumedNumericSpacing = true;
            break;
        case CSSValueID::DiagonalFractions:
        case CSSValueID::StackedFractions:
            if(consumedNumericFraction)
                return nullptr;
            consumedNumericFraction = true;
            break;
        case CSSValueID::Ordinal:
            if(consumedOrdinal)
                return nullptr;
            consumedOrdinal = true;
            break;
        case CSSValueID::SlashedZero:
            if(consumedSlashedZero)
                return nullptr;
            consumedSlashedZero = true;
            break;
        default:
            assert(false);
        }

        values.push_back(std::move(ident));
    } while(!input.empty());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeLineWidth(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"thin", CSSValueID::Thin},
        {"medium", CSSValueID::Medium},
        {"thick", CSSValueID::Thick}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    return consumeLength(input, false, false);
}

RefPtr<CSSValue> CSSParser::consumeBorderRadiusValue(CSSTokenStream& input)
{
    auto first = consumeLengthOrPercent(input, false, false);
    if(first == nullptr)
        return nullptr;
    auto second = consumeLengthOrPercent(input, false, false);
    if(second == nullptr)
        second = first;
    return CSSPairValue::create(m_heap, first, second);
}

RefPtr<CSSValue> CSSParser::consumeClip(CSSTokenStream& input)
{
    if(auto value = consumeAuto(input))
        return value;
    if(input->type() != CSSToken::Type::Function || !identMatches("rect", 4, input->data())) {
        return nullptr;
    }

    auto block = input.consumeBlock();
    block.consumeWhitespace();
    auto top = consumeLengthOrPercentOrAuto(block, true, false);
    if(top == nullptr)
        return nullptr;
    if(block->type() == CSSToken::Type::Comma)
        block.consumeIncludingWhitespace();
    auto right = consumeLengthOrPercentOrAuto(block, true, false);
    if(right == nullptr)
        return nullptr;
    if(block->type() == CSSToken::Type::Comma)
        block.consumeIncludingWhitespace();
    auto bottom = consumeLengthOrPercentOrAuto(block, true, false);
    if(bottom == nullptr)
        return nullptr;
    if(block->type() == CSSToken::Type::Comma)
        block.consumeIncludingWhitespace();
    auto left = consumeLengthOrPercentOrAuto(block, true, false);
    if(left == nullptr || !block.empty())
        return nullptr;
    return CSSRectValue::create(m_heap, top, right, bottom, left);
}

RefPtr<CSSValue> CSSParser::consumeDashList(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;
    CSSValueList values(m_heap);
    do {
        auto value = consumeLengthOrPercent(input, false, true);
        if(value == nullptr || (input.consumeCommaIncludingWhitespace() && input.empty()))
            return nullptr;
        values.push_back(std::move(value));
    } while(!input.empty());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumePosition(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"static", CSSValueID::Static},
        {"relative", CSSValueID::Relative},
        {"absolute", CSSValueID::Absolute},
        {"fixed", CSSValueID::Fixed}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    if(input->type() != CSSToken::Type::Function || !identMatches("running", 7, input->data()))
        return nullptr;
    CSSTokenStreamGuard guard(input);
    auto block = input.consumeBlock();
    block.consumeWhitespace();
    auto value = consumeCustomIdent(block);
    if(value == nullptr || !block.empty())
        return nullptr;
    input.consumeWhitespace();
    guard.release();
    return CSSUnaryFunctionValue::create(m_heap, CSSFunctionID::Running, std::move(value));
}

RefPtr<CSSValue> CSSParser::consumeVerticalAlign(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"baseline", CSSValueID::Baseline},
        {"sub", CSSValueID::Sub},
        {"super", CSSValueID::Super},
        {"text-top", CSSValueID::TextTop},
        {"text-bottom", CSSValueID::TextBottom},
        {"middle", CSSValueID::Middle},
        {"top", CSSValueID::Top},
        {"bottom", CSSValueID::Bottom}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    return consumeLengthOrPercent(input, true, false);
}

RefPtr<CSSValue> CSSParser::consumeBaselineShift(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"baseline", CSSValueID::Baseline},
        {"sub", CSSValueID::Sub},
        {"super", CSSValueID::Super}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    return consumeLengthOrPercent(input, true, false);
}

RefPtr<CSSValue> CSSParser::consumeTextDecorationLine(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;
    static const CSSIdentValueEntry table[] = {
        {"underline", CSSValueID::Underline},
        {"overline", CSSValueID::Overline},
        {"line-through", CSSValueID::LineThrough}
    };

    bool consumedUnderline = false;
    bool consumedOverline = false;
    bool consumedLineThrough = false;

    CSSValueList values(m_heap);
    do {
        auto ident = consumeIdent(input, table);
        if(ident == nullptr)
            break;
        switch(ident->value()) {
        case CSSValueID::Underline:
            if(consumedUnderline)
                return nullptr;
            consumedUnderline = true;
            break;
        case CSSValueID::Overline:
            if(consumedOverline)
                return nullptr;
            consumedOverline = true;
            break;
        case CSSValueID::LineThrough:
            if(consumedLineThrough)
                return nullptr;
            consumedLineThrough = true;
            break;
        default:
            assert(false);
        }

        values.push_back(std::move(ident));
    } while(!input.empty());
    if(values.empty())
        return nullptr;
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumePositionCoordinate(CSSTokenStream& input)
{
    RefPtr<CSSValue> first;
    RefPtr<CSSValue> second;
    for(int index = 0; index < 2; ++index) {
        if(first == nullptr && (first = consumeLengthOrPercent(input, true, false)))
            continue;
        if(second == nullptr && (second = consumeLengthOrPercent(input, true, false)))
            continue;
        static const CSSIdentValueEntry table[] = {
            {"left", CSSValueID::Left},
            {"right", CSSValueID::Right},
            {"center", CSSValueID::Center}
        };

        if(first == nullptr && (first = consumeIdent(input, table))) {
            continue;
        }
        {
            static const CSSIdentValueEntry table[] = {
                {"top", CSSValueID::Top},
                {"bottom", CSSValueID::Bottom},
                {"center", CSSValueID::Center}
            };

            if(second == nullptr && (second = consumeIdent(input, table))) {
                continue;
            }
        }

        break;
    }

    if(first == nullptr && second == nullptr)
        return nullptr;
    if(first == nullptr)
        first = CSSIdentValue::create(CSSValueID::Center);
    if(second == nullptr)
        second = CSSIdentValue::create(CSSValueID::Center);
    return CSSPairValue::create(m_heap, first, second);
}

RefPtr<CSSValue> CSSParser::consumeBackgroundSize(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"contain", CSSValueID::Contain},
        {"cover", CSSValueID::Cover}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    auto first = consumeLengthOrPercentOrAuto(input, false, false);
    if(first == nullptr)
        return nullptr;
    auto second = consumeLengthOrPercentOrAuto(input, false, false);
    if(second == nullptr)
        second = CSSIdentValue::create(CSSValueID::Auto);
    return CSSPairValue::create(m_heap, first, second);
}

RefPtr<CSSValue> CSSParser::consumeAngle(CSSTokenStream& input)
{
    if(input->type() != CSSToken::Type::Dimension)
        return nullptr;
    static const CSSIdentEntry<CSSAngleValue::Unit> table[] = {
        {"deg", CSSAngleValue::Unit::Degrees},
        {"rad", CSSAngleValue::Unit::Radians},
        {"grad", CSSAngleValue::Unit::Gradians},
        {"turn", CSSAngleValue::Unit::Turns}
    };

    auto unitType = matchIdent(table, input->data());
    if(unitType == std::nullopt)
        return nullptr;
    auto value = input->number();
    input.consumeIncludingWhitespace();
    return CSSAngleValue::create(m_heap, value, unitType.value());
}

RefPtr<CSSValue> CSSParser::consumeTransformValue(CSSTokenStream& input)
{
    if(input->type() != CSSToken::Type::Function)
        return nullptr;
    static const CSSIdentEntry<CSSFunctionID> table[] = {
        {"skew", CSSFunctionID::Skew},
        {"skewx", CSSFunctionID::SkewX},
        {"skewy", CSSFunctionID::SkewY},
        {"scale", CSSFunctionID::Scale},
        {"scalex", CSSFunctionID::ScaleX},
        {"scaley", CSSFunctionID::ScaleY},
        {"translate", CSSFunctionID::Translate},
        {"translatex", CSSFunctionID::TranslateX},
        {"translatey", CSSFunctionID::TranslateY},
        {"rotate", CSSFunctionID::Rotate},
        {"matrix", CSSFunctionID::Matrix}
    };

    auto id = matchIdent(table, input->data());
    if(id == std::nullopt)
        return nullptr;
    CSSValueList values(m_heap);
    auto block = input.consumeBlock();
    block.consumeWhitespace();
    switch(id.value()) {
    case CSSFunctionID::Skew:
    case CSSFunctionID::SkewX:
    case CSSFunctionID::SkewY:
    case CSSFunctionID::Rotate: {
        auto value = consumeAngle(block);
        if(value == nullptr)
            return nullptr;
        values.push_back(std::move(value));
        if(id.value() == CSSFunctionID::Skew && block->type() == CSSToken::Type::Comma) {
            block.consumeIncludingWhitespace();
            auto value = consumeAngle(block);
            if(value == nullptr)
                return nullptr;
            values.push_back(std::move(value));
        }

        break;
    }

    case CSSFunctionID::Scale:
    case CSSFunctionID::ScaleX:
    case CSSFunctionID::ScaleY: {
        auto value = consumeNumberOrPercent(block, true);
        if(value == nullptr)
            return nullptr;
        values.push_back(std::move(value));
        if(id.value() == CSSFunctionID::Scale && block->type() == CSSToken::Type::Comma) {
            block.consumeIncludingWhitespace();
            auto value = consumeNumberOrPercent(block, true);
            if(value == nullptr)
                return nullptr;
            values.push_back(std::move(value));
        }

        break;
    }

    case CSSFunctionID::Translate:
    case CSSFunctionID::TranslateX:
    case CSSFunctionID::TranslateY: {
        auto value = consumeLengthOrPercent(block, true, false);
        if(value == nullptr)
            return nullptr;
        values.push_back(std::move(value));
        if(id.value() == CSSFunctionID::Translate && block->type() == CSSToken::Type::Comma) {
            block.consumeIncludingWhitespace();
            auto value = consumeLengthOrPercent(block, true, false);
            if(value == nullptr)
                return nullptr;
            values.push_back(std::move(value));
        }

        break;
    }

    case CSSFunctionID::Matrix: {
        int count = 6;
        while(count > 0) {
            auto value = consumeNumber(block, true);
            if(value == nullptr)
                return nullptr;
            count -= 1;
            if(count > 0 && block->type() == CSSToken::Type::Comma)
                block.consumeIncludingWhitespace();
            values.push_back(std::move(value));
        }

        break;
    }

    default:
        return nullptr;
    }

    if(!block.empty())
        return nullptr;
    input.consumeWhitespace();
    return CSSFunctionValue::create(m_heap, *id, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeTransform(CSSTokenStream& input)
{
    if(auto value = consumeNone(input))
        return value;
    CSSValueList values(m_heap);
    do {
        auto value = consumeTransformValue(input);
        if(value == nullptr)
            return nullptr;
        values.push_back(std::move(value));
    } while(!input.empty());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumePaintOrder(CSSTokenStream& input)
{
    if(auto value = consumeNormal(input))
        return value;
    static const CSSIdentValueEntry table[] = {
        {"fill", CSSValueID::Fill},
        {"stroke", CSSValueID::Stroke},
        {"markers", CSSValueID::Markers}
    };

    CSSValueList values(m_heap);
    do {
        auto value = consumeIdent(input, table);
        if(value == nullptr)
            return nullptr;
        values.push_back(std::move(value));
    } while(!input.empty());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeLonghand(CSSTokenStream& input, CSSPropertyID id)
{
    switch(id) {
    case CSSPropertyID::FlexGrow:
    case CSSPropertyID::FlexShrink:
    case CSSPropertyID::StrokeMiterlimit:
        return consumeNumber(input, false);
    case CSSPropertyID::TabSize:
        return consumeLength(input, false, true);
    case CSSPropertyID::OutlineOffset:
        return consumeLength(input, true, false);
    case CSSPropertyID::BorderHorizontalSpacing:
    case CSSPropertyID::BorderVerticalSpacing:
        return consumeLength(input, false, false);
    case CSSPropertyID::Order:
        return consumeInteger(input, true);
    case CSSPropertyID::Widows:
    case CSSPropertyID::Orphans:
        return consumePositiveInteger(input);
    case CSSPropertyID::ColumnCount:
        return consumePositiveIntegerOrAuto(input);
    case CSSPropertyID::ZIndex:
        return consumeIntegerOrAuto(input, true);
    case CSSPropertyID::X:
    case CSSPropertyID::Y:
    case CSSPropertyID::Cx:
    case CSSPropertyID::Cy:
    case CSSPropertyID::TextIndent:
        return consumeLengthOrPercent(input, true, false);
    case CSSPropertyID::R:
    case CSSPropertyID::Rx:
    case CSSPropertyID::Ry:
    case CSSPropertyID::PaddingTop:
    case CSSPropertyID::PaddingRight:
    case CSSPropertyID::PaddingBottom:
    case CSSPropertyID::PaddingLeft:
        return consumeLengthOrPercent(input, false, false);
    case CSSPropertyID::StrokeWidth:
        return consumeLengthOrPercent(input, false, true);
    case CSSPropertyID::StrokeDashoffset:
        return consumeLengthOrPercent(input, true, true);
    case CSSPropertyID::Opacity:
    case CSSPropertyID::FillOpacity:
    case CSSPropertyID::StrokeOpacity:
    case CSSPropertyID::StopOpacity:
        return consumeNumberOrPercent(input, false);
    case CSSPropertyID::PageScale:
        return consumeNumberOrPercentOrAuto(input, false);
    case CSSPropertyID::Bottom:
    case CSSPropertyID::Left:
    case CSSPropertyID::Right:
    case CSSPropertyID::Top:
    case CSSPropertyID::MarginTop:
    case CSSPropertyID::MarginRight:
    case CSSPropertyID::MarginBottom:
    case CSSPropertyID::MarginLeft:
        return consumeLengthOrPercentOrAuto(input, true, false);
    case CSSPropertyID::Width:
    case CSSPropertyID::Height:
    case CSSPropertyID::MinWidth:
    case CSSPropertyID::MinHeight:
        return consumeWidthOrHeightOrAuto(input, false);
    case CSSPropertyID::MaxWidth:
    case CSSPropertyID::MaxHeight:
        return consumeWidthOrHeightOrNone(input, false);
    case CSSPropertyID::FlexBasis:
        return consumeWidthOrHeightOrAuto(input, false);
    case CSSPropertyID::Fill:
    case CSSPropertyID::Stroke:
        return consumePaint(input);
    case CSSPropertyID::BorderBottomWidth:
    case CSSPropertyID::BorderLeftWidth:
    case CSSPropertyID::BorderRightWidth:
    case CSSPropertyID::BorderTopWidth:
        return consumeLineWidth(input);
    case CSSPropertyID::LineHeight:
        return consumeLengthOrPercentOrNormal(input, false, true);
    case CSSPropertyID::LetterSpacing:
    case CSSPropertyID::WordSpacing:
        return consumeLengthOrNormal(input, true, false);
    case CSSPropertyID::OutlineWidth:
    case CSSPropertyID::ColumnRuleWidth:
        return consumeLineWidth(input);
    case CSSPropertyID::RowGap:
    case CSSPropertyID::ColumnGap:
        return consumeLengthOrNormal(input, false, false);
    case CSSPropertyID::ColumnWidth:
        return consumeLengthOrAuto(input, false, false);
    case CSSPropertyID::Quotes:
        return consumeQuotes(input);
    case CSSPropertyID::Clip:
        return consumeClip(input);
    case CSSPropertyID::Size:
        return consumeSize(input);
    case CSSPropertyID::Page:
        return consumePage(input);
    case CSSPropertyID::FontWeight:
        return consumeFontWeight(input);
    case CSSPropertyID::FontStretch:
        return consumeFontStretch(input);
    case CSSPropertyID::FontStyle:
        return consumeFontStyle(input);
    case CSSPropertyID::FontSize:
        return consumeFontSize(input);
    case CSSPropertyID::FontFamily:
        return consumeFontFamily(input);
    case CSSPropertyID::FontFeatureSettings:
        return consumeFontFeatureSettings(input);
    case CSSPropertyID::FontVariationSettings:
        return consumeFontVariationSettings(input);
    case CSSPropertyID::FontVariantCaps:
        return consumeFontVariantCaps(input);
    case CSSPropertyID::FontVariantEmoji:
        return consumeFontVariantEmoji(input);
    case CSSPropertyID::FontVariantPosition:
        return consumeFontVariantPosition(input);
    case CSSPropertyID::FontVariantEastAsian:
        return consumeFontVariantEastAsian(input);
    case CSSPropertyID::FontVariantLigatures:
        return consumeFontVariantLigatures(input);
    case CSSPropertyID::FontVariantNumeric:
        return consumeFontVariantNumeric(input);
    case CSSPropertyID::BorderBottomLeftRadius:
    case CSSPropertyID::BorderBottomRightRadius:
    case CSSPropertyID::BorderTopLeftRadius:
    case CSSPropertyID::BorderTopRightRadius:
        return consumeBorderRadiusValue(input);
    case CSSPropertyID::Color:
    case CSSPropertyID::BackgroundColor:
    case CSSPropertyID::TextDecorationColor:
    case CSSPropertyID::StopColor:
    case CSSPropertyID::OutlineColor:
    case CSSPropertyID::ColumnRuleColor:
    case CSSPropertyID::BorderBottomColor:
    case CSSPropertyID::BorderLeftColor:
    case CSSPropertyID::BorderRightColor:
    case CSSPropertyID::BorderTopColor:
        return consumeColor(input);
    case CSSPropertyID::ClipPath:
    case CSSPropertyID::MarkerEnd:
    case CSSPropertyID::MarkerMid:
    case CSSPropertyID::MarkerStart:
    case CSSPropertyID::Mask:
        return consumeLocalUrlOrNone(input);
    case CSSPropertyID::ListStyleImage:
    case CSSPropertyID::BackgroundImage:
        return consumeImageOrNone(input);
    case CSSPropertyID::Content:
        return consumeContent(input);
    case CSSPropertyID::CounterReset:
    case CSSPropertyID::CounterSet:
        return consumeCounter(input, false);
    case CSSPropertyID::CounterIncrement:
        return consumeCounter(input, true);
    case CSSPropertyID::ListStyleType:
        return consumeListStyleType(input);
    case CSSPropertyID::StrokeDasharray:
        return consumeDashList(input);
    case CSSPropertyID::BaselineShift:
        return consumeBaselineShift(input);
    case CSSPropertyID::Position:
        return consumePosition(input);
    case CSSPropertyID::VerticalAlign:
        return consumeVerticalAlign(input);
    case CSSPropertyID::TextDecorationLine:
        return consumeTextDecorationLine(input);
    case CSSPropertyID::BackgroundSize:
        return consumeBackgroundSize(input);
    case CSSPropertyID::BackgroundPosition:
    case CSSPropertyID::ObjectPosition:
    case CSSPropertyID::TransformOrigin:
        return consumePositionCoordinate(input);
    case CSSPropertyID::Transform:
        return consumeTransform(input);
    case CSSPropertyID::PaintOrder:
        return consumePaintOrder(input);
    case CSSPropertyID::FontKerning: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"normal", CSSValueID::Normal},
            {"none", CSSValueID::None}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BackgroundAttachment: {
        static const CSSIdentValueEntry table[] = {
            {"scroll", CSSValueID::Scroll},
            {"fixed", CSSValueID::Fixed},
            {"local", CSSValueID::Local}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BackgroundClip:
    case CSSPropertyID::BackgroundOrigin: {
        static const CSSIdentValueEntry table[] = {
            {"border-box", CSSValueID::BorderBox},
            {"padding-box", CSSValueID::PaddingBox},
            {"content-box", CSSValueID::ContentBox}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BackgroundRepeat: {
        static const CSSIdentValueEntry table[] = {
            {"repeat", CSSValueID::Repeat},
            {"repeat-x", CSSValueID::RepeatX},
            {"repeat-y", CSSValueID::RepeatY},
            {"no-repeat", CSSValueID::NoRepeat}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BorderCollapse: {
        static const CSSIdentValueEntry table[] = {
            {"collapse", CSSValueID::Collapse},
            {"separate", CSSValueID::Separate}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BorderTopStyle:
    case CSSPropertyID::BorderRightStyle:
    case CSSPropertyID::BorderBottomStyle:
    case CSSPropertyID::BorderLeftStyle:
    case CSSPropertyID::ColumnRuleStyle:
    case CSSPropertyID::OutlineStyle: {
        static const CSSIdentValueEntry table[] = {
            {"none", CSSValueID::None},
            {"hidden", CSSValueID::Hidden},
            {"inset", CSSValueID::Inset},
            {"groove", CSSValueID::Groove},
            {"ridge", CSSValueID::Ridge},
            {"outset", CSSValueID::Outset},
            {"dotted", CSSValueID::Dotted},
            {"dashed", CSSValueID::Dashed},
            {"solid", CSSValueID::Solid},
            {"double", CSSValueID::Double}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BoxSizing: {
        static const CSSIdentValueEntry table[] = {
            {"border-box", CSSValueID::BorderBox},
            {"content-box", CSSValueID::ContentBox}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::CaptionSide: {
        static const CSSIdentValueEntry table[] = {
            {"top", CSSValueID::Top},
            {"bottom", CSSValueID::Bottom}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Clear: {
        static const CSSIdentValueEntry table[] = {
            {"none", CSSValueID::None},
            {"left", CSSValueID::Left},
            {"right", CSSValueID::Right},
            {"both", CSSValueID::Both}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::EmptyCells: {
        static const CSSIdentValueEntry table[] = {
            {"show", CSSValueID::Show},
            {"hide", CSSValueID::Hide}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::FillRule:
    case CSSPropertyID::ClipRule: {
        static const CSSIdentValueEntry table[] = {
            {"nonzero", CSSValueID::Nonzero},
            {"evenodd", CSSValueID::Evenodd}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Float: {
        static const CSSIdentValueEntry table[] = {
            {"none", CSSValueID::None},
            {"left", CSSValueID::Left},
            {"right", CSSValueID::Right}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Hyphens: {
        static const CSSIdentValueEntry table[] = {
            {"none", CSSValueID::None},
            {"auto", CSSValueID::Auto},
            {"manual", CSSValueID::Manual}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ListStylePosition: {
        static const CSSIdentValueEntry table[] = {
            {"inside", CSSValueID::Inside},
            {"outside", CSSValueID::Outside}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::WordBreak: {
        static const CSSIdentValueEntry table[] = {
            {"normal", CSSValueID::Normal},
            {"keep-all", CSSValueID::KeepAll},
            {"break-all", CSSValueID::BreakAll},
            {"break-word", CSSValueID::BreakWord}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::OverflowWrap: {
        static const CSSIdentValueEntry table[] = {
            {"normal", CSSValueID::Normal},
            {"anywhere", CSSValueID::Anywhere},
            {"break-word", CSSValueID::BreakWord}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Overflow: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"visible", CSSValueID::Visible},
            {"hidden", CSSValueID::Hidden},
            {"scroll", CSSValueID::Scroll}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BreakBefore:
    case CSSPropertyID::BreakAfter: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"avoid", CSSValueID::Avoid},
            {"avoid-column", CSSValueID::AvoidColumn},
            {"avoid-page", CSSValueID::AvoidPage},
            {"column", CSSValueID::Column},
            {"page", CSSValueID::Page},
            {"left", CSSValueID::Left},
            {"right", CSSValueID::Right},
            {"recto", CSSValueID::Recto},
            {"verso", CSSValueID::Verso}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::BreakInside: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"avoid", CSSValueID::Avoid},
            {"avoid-column", CSSValueID::AvoidColumn},
            {"avoid-page", CSSValueID::AvoidPage}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ColumnBreakBefore:
    case CSSPropertyID::ColumnBreakAfter: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"always", CSSValueID::Column},
            {"avoid", CSSValueID::Avoid}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::PageBreakBefore:
    case CSSPropertyID::PageBreakAfter: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"always", CSSValueID::Page},
            {"avoid", CSSValueID::Avoid},
            {"left", CSSValueID::Left},
            {"right", CSSValueID::Right}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ColumnBreakInside:
    case CSSPropertyID::PageBreakInside: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"avoid", CSSValueID::Avoid}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::StrokeLinecap: {
        static const CSSIdentValueEntry table[] = {
            {"butt", CSSValueID::Butt},
            {"round", CSSValueID::Round},
            {"square", CSSValueID::Square}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::StrokeLinejoin: {
        static const CSSIdentValueEntry table[] = {
            {"miter", CSSValueID::Miter},
            {"round", CSSValueID::Round},
            {"bevel", CSSValueID::Bevel}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TableLayout: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"fixed", CSSValueID::Fixed}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::AlignmentBaseline: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"baseline", CSSValueID::Baseline},
            {"before-edge", CSSValueID::BeforeEdge},
            {"text-before-edge", CSSValueID::TextBeforeEdge},
            {"middle", CSSValueID::Middle},
            {"central", CSSValueID::Central},
            {"after-edge", CSSValueID::AfterEdge},
            {"text-after-edge", CSSValueID::TextAfterEdge},
            {"ideographic", CSSValueID::Ideographic},
            {"alphabetic", CSSValueID::Alphabetic},
            {"hanging", CSSValueID::Hanging},
            {"mathematical", CSSValueID::Mathematical}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::DominantBaseline: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"use-script", CSSValueID::UseScript},
            {"no-change", CSSValueID::NoChange},
            {"reset-size", CSSValueID::ResetSize},
            {"ideographic", CSSValueID::Ideographic},
            {"alphabetic", CSSValueID::Alphabetic},
            {"hanging", CSSValueID::Hanging},
            {"mathematical", CSSValueID::Mathematical},
            {"central", CSSValueID::Central},
            {"middle", CSSValueID::Middle},
            {"text-after-edge", CSSValueID::TextAfterEdge},
            {"text-before-edge", CSSValueID::TextBeforeEdge}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TextAlign: {
        static const CSSIdentValueEntry table[] = {
            {"left", CSSValueID::Left},
            {"right", CSSValueID::Right},
            {"center", CSSValueID::Center},
            {"justify", CSSValueID::Justify},
            {"start", CSSValueID::Start},
            {"end", CSSValueID::End}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::WritingMode: {
        static const CSSIdentValueEntry table[] = {
            {"horizontal-tb", CSSValueID::HorizontalTb},
            {"vertical-rl", CSSValueID::VerticalRl},
            {"vertical-lr", CSSValueID::VerticalLr},
            {"lr-tb", CSSValueID::HorizontalTb},
            {"rl-tb", CSSValueID::HorizontalTb},
            {"lr", CSSValueID::HorizontalTb},
            {"rl", CSSValueID::HorizontalTb},
            {"tb-rl", CSSValueID::VerticalRl},
            {"tb", CSSValueID::VerticalLr}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TextOrientation: {
        static const CSSIdentValueEntry table[] = {
            {"mixed", CSSValueID::Mixed},
            {"upright", CSSValueID::Upright}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TextAnchor: {
        static const CSSIdentValueEntry table[] = {
            {"start", CSSValueID::Start},
            {"middle", CSSValueID::Middle},
            {"end", CSSValueID::End}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TextDecorationStyle: {
        static const CSSIdentValueEntry table[] = {
            {"solid", CSSValueID::Solid},
            {"double", CSSValueID::Double},
            {"dotted", CSSValueID::Dotted},
            {"dashed", CSSValueID::Dashed},
            {"wavy", CSSValueID::Wavy}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TextOverflow: {
        static const CSSIdentValueEntry table[] = {
            {"clip", CSSValueID::Clip},
            {"ellipsis", CSSValueID::Ellipsis}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::TextTransform: {
        static const CSSIdentValueEntry table[] = {
            {"none", CSSValueID::None},
            {"capitalize", CSSValueID::Capitalize},
            {"uppercase", CSSValueID::Uppercase},
            {"lowercase", CSSValueID::Lowercase}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::MixBlendMode: {
        static const CSSIdentValueEntry table[] = {
            {"normal", CSSValueID::Normal},
            {"multiply", CSSValueID::Multiply},
            {"screen", CSSValueID::Screen},
            {"overlay", CSSValueID::Overlay},
            {"darken", CSSValueID::Darken},
            {"lighten", CSSValueID::Lighten},
            {"color-dodge", CSSValueID::ColorDodge},
            {"color-burn", CSSValueID::ColorBurn},
            {"hard-light", CSSValueID::HardLight},
            {"soft-light", CSSValueID::SoftLight},
            {"difference", CSSValueID::Difference},
            {"exclusion", CSSValueID::Exclusion},
            {"hue", CSSValueID::Hue},
            {"saturation", CSSValueID::Saturation},
            {"color", CSSValueID::Color},
            {"luminosity", CSSValueID::Luminosity}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::MaskType: {
        static const CSSIdentValueEntry table[] = {
            {"luminance", CSSValueID::Luminance},
            {"alpha", CSSValueID::Alpha}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::VectorEffect: {
        static const CSSIdentValueEntry table[] = {
            {"none", CSSValueID::None},
            {"non-scaling-stroke", CSSValueID::NonScalingStroke}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Visibility: {
        static const CSSIdentValueEntry table[] = {
            {"visible", CSSValueID::Visible},
            {"hidden", CSSValueID::Hidden},
            {"collapse", CSSValueID::Collapse}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Display: {
        static const CSSIdentValueEntry table[] = {
            {"none", CSSValueID::None},
            {"block", CSSValueID::Block},
            {"flex", CSSValueID::Flex},
            {"inline", CSSValueID::Inline},
            {"inline-block", CSSValueID::InlineBlock},
            {"inline-flex", CSSValueID::InlineFlex},
            {"inline-table", CSSValueID::InlineTable},
            {"list-item", CSSValueID::ListItem},
            {"table", CSSValueID::Table},
            {"table-caption", CSSValueID::TableCaption},
            {"table-cell", CSSValueID::TableCell},
            {"table-column", CSSValueID::TableColumn},
            {"table-column-group", CSSValueID::TableColumnGroup},
            {"table-footer-group", CSSValueID::TableFooterGroup},
            {"table-header-group", CSSValueID::TableHeaderGroup},
            {"table-row", CSSValueID::TableRow},
            {"table-row-group", CSSValueID::TableRowGroup}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::FlexDirection: {
        static const CSSIdentValueEntry table[] = {
            {"row", CSSValueID::Row},
            {"row-reverse", CSSValueID::RowReverse},
            {"column", CSSValueID::Column},
            {"column-reverse", CSSValueID::ColumnReverse}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::FlexWrap: {
        static const CSSIdentValueEntry table[] = {
            {"nowrap", CSSValueID::Nowrap},
            {"wrap", CSSValueID::Wrap},
            {"wrap-reverse", CSSValueID::WrapReverse}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::WhiteSpace: {
        static const CSSIdentValueEntry table[] = {
            {"normal", CSSValueID::Normal},
            {"pre", CSSValueID::Pre},
            {"pre-wrap", CSSValueID::PreWrap},
            {"pre-line", CSSValueID::PreLine},
            {"nowrap", CSSValueID::Nowrap}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::Direction: {
        static const CSSIdentValueEntry table[] = {
            {"ltr", CSSValueID::Ltr},
            {"rtl", CSSValueID::Rtl}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::UnicodeBidi: {
        static const CSSIdentValueEntry table[] = {
            {"normal", CSSValueID::Normal},
            {"embed", CSSValueID::Embed},
            {"bidi-override", CSSValueID::BidiOverride},
            {"isolate", CSSValueID::Isolate},
            {"isolate-override", CSSValueID::IsolateOverride}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ColumnSpan: {
        static const CSSIdentValueEntry table[] = {
            {"none", CSSValueID::None},
            {"all", CSSValueID::All}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ColumnFill: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"balance", CSSValueID::Balance}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::JustifyContent: {
        static const CSSIdentValueEntry table[] = {
            {"flex-start", CSSValueID::FlexStart},
            {"flex-end", CSSValueID::FlexEnd},
            {"center", CSSValueID::Center},
            {"space-between", CSSValueID::SpaceBetween},
            {"space-around", CSSValueID::SpaceAround},
            {"space-evenly", CSSValueID::SpaceEvenly}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::AlignContent: {
        static const CSSIdentValueEntry table[] = {
            {"flex-start", CSSValueID::FlexStart},
            {"flex-end", CSSValueID::FlexEnd},
            {"center", CSSValueID::Center},
            {"space-between", CSSValueID::SpaceBetween},
            {"space-around", CSSValueID::SpaceAround},
            {"space-evenly", CSSValueID::SpaceEvenly},
            {"stretch", CSSValueID::Stretch}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::AlignItems: {
        static const CSSIdentValueEntry table[] = {
            {"flex-start", CSSValueID::FlexStart},
            {"flex-end", CSSValueID::FlexEnd},
            {"center", CSSValueID::Center},
            {"baseline", CSSValueID::Baseline},
            {"stretch", CSSValueID::Stretch}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::AlignSelf: {
        static const CSSIdentValueEntry table[] = {
            {"auto", CSSValueID::Auto},
            {"flex-start", CSSValueID::FlexStart},
            {"flex-end", CSSValueID::FlexEnd},
            {"center", CSSValueID::Center},
            {"baseline", CSSValueID::Baseline},
            {"stretch", CSSValueID::Stretch}
        };

        return consumeIdent(input, table);
    }

    case CSSPropertyID::ObjectFit: {
        static const CSSIdentValueEntry table[] = {
            {"fill", CSSValueID::Fill},
            {"contain", CSSValueID::Contain},
            {"cover", CSSValueID::Cover},
            {"none", CSSValueID::None},
            {"scale-down", CSSValueID::ScaleDown}
        };

        return consumeIdent(input, table);
    }

    default:
        return nullptr;
    }
}

bool CSSParser::consumeFlex(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    if(consumeIdentIncludingWhitespace(input, "none", 4)) {
        if(!input.empty())
            return false;
        addProperty(properties, CSSPropertyID::FlexGrow, important, CSSNumberValue::create(m_heap, 0.0));
        addProperty(properties, CSSPropertyID::FlexShrink, important, CSSNumberValue::create(m_heap, 0.0));
        addProperty(properties, CSSPropertyID::FlexBasis, important, CSSIdentValue::create(CSSValueID::Auto));
        return true;
    }

    RefPtr<CSSValue> grow;
    RefPtr<CSSValue> shrink;
    RefPtr<CSSValue> basis;
    for(int index = 0; index < 3; ++index) {
        if(input->type() == CSSToken::Type::Number) {
            if(input->number() < 0.0)
                return false;
            if(grow == nullptr)
                grow = CSSNumberValue::create(m_heap, input->number());
            else if(shrink == nullptr)
                shrink = CSSNumberValue::create(m_heap, input->number());
            else if(input->number() == 0.0)
                basis = CSSLengthValue::create(m_heap, 0.0, CSSLengthUnits::None);
            else
                return false;
            input.consumeIncludingWhitespace();
            continue;
        }

        if(basis == nullptr && (basis = consumeWidthOrHeightOrAuto(input, false))) {
            if(index == 1 && !input.empty())
                return false;
            continue;
        }

        break;
    }

    if(!input.empty())
        return false;
    addProperty(properties, CSSPropertyID::FlexGrow, important, std::move(grow));
    addProperty(properties, CSSPropertyID::FlexShrink, important, std::move(shrink));
    addProperty(properties, CSSPropertyID::FlexBasis, important, std::move(basis));
    return true;
}

bool CSSParser::consumeBackground(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    RefPtr<CSSValue> color;
    RefPtr<CSSValue> image;
    RefPtr<CSSValue> repeat;
    RefPtr<CSSValue> attachment;
    RefPtr<CSSValue> origin;
    RefPtr<CSSValue> clip;
    RefPtr<CSSValue> position;
    RefPtr<CSSValue> size;
    while(!input.empty()) {
        if(position == nullptr && (position = consumePositionCoordinate(input))) {
            if(input->type() == CSSToken::Type::Delim && input->delim() == '/') {
                input.consumeIncludingWhitespace();
                if(size == nullptr && (size = consumeBackgroundSize(input)))
                    continue;
                return false;
            }

            continue;
        }

        if(image == nullptr && (image = consumeImage(input)))
            continue;
        if(repeat == nullptr && (repeat = consumeLonghand(input, CSSPropertyID::BackgroundRepeat)))
            continue;
        if(attachment == nullptr && (attachment = consumeLonghand(input, CSSPropertyID::BackgroundAttachment)))
            continue;
        if(origin == nullptr && (origin = consumeLonghand(input, CSSPropertyID::BackgroundOrigin)))
            continue;
        if(clip == nullptr && (clip = consumeLonghand(input, CSSPropertyID::BackgroundClip)))
            continue;
        if(color == nullptr && (color = consumeColor(input)))
            continue;
        return false;
    }

    if(clip == nullptr)
        clip = origin;
    addProperty(properties, CSSPropertyID::BackgroundColor, important, std::move(color));
    addProperty(properties, CSSPropertyID::BackgroundImage, important, std::move(image));
    addProperty(properties, CSSPropertyID::BackgroundRepeat, important, std::move(repeat));
    addProperty(properties, CSSPropertyID::BackgroundAttachment, important, std::move(attachment));
    addProperty(properties, CSSPropertyID::BackgroundOrigin, important, std::move(origin));
    addProperty(properties, CSSPropertyID::BackgroundClip, important, std::move(clip));
    addProperty(properties, CSSPropertyID::BackgroundPosition, important, std::move(position));
    addProperty(properties, CSSPropertyID::BackgroundSize, important, std::move(size));
    return true;
}

bool CSSParser::consumeColumns(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    RefPtr<CSSValue> width;
    RefPtr<CSSValue> count;
    for(int index = 0; index < 2; ++index) {
        if(consumeIdentIncludingWhitespace(input, "auto", 4))
            continue;
        if(width == nullptr && (width = consumeLength(input, false, false)))
            continue;
        if(count == nullptr && (count = consumePositiveInteger(input)))
            continue;
        break;
    }

    if(!input.empty())
        return false;
    addProperty(properties, CSSPropertyID::ColumnWidth, important, std::move(width));
    addProperty(properties, CSSPropertyID::ColumnCount, important, std::move(count));
    return true;
}

bool CSSParser::consumeListStyle(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    RefPtr<CSSValue> none;
    RefPtr<CSSValue> position;
    RefPtr<CSSValue> image;
    RefPtr<CSSValue> type;
    while(!input.empty()) {
        if(none == nullptr && (none = consumeNone(input)))
            continue;
        if(position == nullptr && (position = consumeLonghand(input, CSSPropertyID::ListStylePosition)))
            continue;
        if(image == nullptr && (image = consumeLonghand(input, CSSPropertyID::ListStyleImage)))
            continue;
        if(type == nullptr && (type = consumeLonghand(input, CSSPropertyID::ListStyleType)))
            continue;
        return false;
    }

    if(none) {
        if(!type) {
            type = none;
        } else if(!image) {
            image = none;
        } else {
            return false;
        }
    }

    addProperty(properties, CSSPropertyID::ListStyleType, important, std::move(type));
    addProperty(properties, CSSPropertyID::ListStylePosition, important, std::move(position));
    addProperty(properties, CSSPropertyID::ListStyleImage, important, std::move(image));
    return true;
}

bool CSSParser::consumeFont(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    RefPtr<CSSValue> style;
    RefPtr<CSSValue> weight;
    RefPtr<CSSValue> variant;
    RefPtr<CSSValue> stretch;
    for(int index = 0; index < 4; ++index) {
        if(consumeIdentIncludingWhitespace(input, "normal", 6))
            continue;
        if(style == nullptr && (style = consumeFontStyle(input)))
            continue;
        if(weight == nullptr && (weight = consumeFontWeight(input)))
            continue;
        if(variant == nullptr && (variant = consumeFontVariantCapsIdent(input)))
            continue;
        if(stretch == nullptr && (stretch = consumeFontStretchIdent(input)))
            continue;
        break;
    }

    if(input.empty())
        return false;
    addProperty(properties, CSSPropertyID::FontStyle, important, std::move(style));
    addProperty(properties, CSSPropertyID::FontWeight, important, std::move(weight));
    addProperty(properties, CSSPropertyID::FontVariantCaps, important, std::move(variant));
    addProperty(properties, CSSPropertyID::FontStretch, important, std::move(stretch));

    auto size = consumeFontSize(input);
    if(size == nullptr || input.empty())
        return false;
    addProperty(properties, CSSPropertyID::FontSize, important, std::move(size));
    if(input->type() == CSSToken::Type::Delim && input->delim() == '/') {
        input.consumeIncludingWhitespace();
        auto value = consumeLengthOrPercentOrNormal(input, false, true);
        if(value == nullptr)
            return false;
        addProperty(properties, CSSPropertyID::LineHeight, important, std::move(value));
    } else {
        addProperty(properties, CSSPropertyID::LineHeight, important, nullptr);
    }

    auto family = consumeFontFamily(input);
    if(family == nullptr || !input.empty())
        return false;
    addProperty(properties, CSSPropertyID::FontFamily, important, std::move(family));
    return true;
}

bool CSSParser::consumeFontVariant(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    if(auto value = consumeNoneOrNormal(input)) {
        if(!input.empty())
            return false;
        addProperty(properties, CSSPropertyID::FontVariantCaps, important, nullptr);
        addProperty(properties, CSSPropertyID::FontVariantEmoji, important, nullptr);
        addProperty(properties, CSSPropertyID::FontVariantPosition, important, nullptr);
        addProperty(properties, CSSPropertyID::FontVariantEastAsian, important, nullptr);
        addProperty(properties, CSSPropertyID::FontVariantNumeric, important, nullptr);
        addProperty(properties, CSSPropertyID::FontVariantLigatures, important, std::move(value));
        return true;
    }

    RefPtr<CSSValue> caps;
    RefPtr<CSSValue> emoji;
    RefPtr<CSSValue> position;

    CSSValueList eastAsian;
    CSSValueList ligatures;
    CSSValueList numeric;
    while(!input.empty()) {
        if(caps == nullptr && (caps = consumeFontVariantCapsIdent(input)))
            continue;
        if(emoji == nullptr && (emoji = consumeFontVariantEmojiIdent(input)))
            continue;
        if(position == nullptr && (position = consumeFontVariantPositionIdent(input)))
            continue;
        if(auto value = consumeFontVariantEastAsianIdent(input)) {
            eastAsian.push_back(std::move(value));
            continue;
        }

        if(auto value = consumeFontVariantLigaturesIdent(input)) {
            ligatures.push_back(std::move(value));
            continue;
        }

        if(auto value = consumeFontVariantNumericIdent(input)) {
            numeric.push_back(std::move(value));
            continue;
        }

        return false;
    }

    addProperty(properties, CSSPropertyID::FontVariantCaps, important, std::move(caps));
    addProperty(properties, CSSPropertyID::FontVariantEmoji, important, std::move(emoji));
    addProperty(properties, CSSPropertyID::FontVariantPosition, important, std::move(position));
    auto addListProperty = [&](CSSPropertyID id, CSSValueList&& values) {
        if(values.empty())
            addProperty(properties, id, important, CSSIdentValue::create(CSSValueID::Normal));
        else {
            addProperty(properties, id, important, CSSListValue::create(m_heap, std::move(values)));
        }
    };

    addListProperty(CSSPropertyID::FontVariantEastAsian, std::move(eastAsian));
    addListProperty(CSSPropertyID::FontVariantLigatures, std::move(ligatures));
    addListProperty(CSSPropertyID::FontVariantNumeric, std::move(numeric));
    return true;
}

bool CSSParser::consumeBorder(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    RefPtr<CSSValue> width;
    RefPtr<CSSValue> style;
    RefPtr<CSSValue> color;
    while(!input.empty()) {
        if(width == nullptr && (width = consumeLineWidth(input)))
            continue;
        if(style == nullptr && (style = consumeLonghand(input, CSSPropertyID::BorderTopStyle)))
            continue;
        if(color == nullptr && (color = consumeColor(input)))
            continue;
        return false;
    }

    addExpandedProperty(properties, CSSPropertyID::BorderWidth, important, std::move(width));
    addExpandedProperty(properties, CSSPropertyID::BorderStyle, important, std::move(style));
    addExpandedProperty(properties, CSSPropertyID::BorderColor, important, std::move(color));
    return true;
}

bool CSSParser::consumeBorderRadius(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    auto completesides = [](auto sides[4]) {
        if(sides[1] == nullptr) sides[1] = sides[0];
        if(sides[2] == nullptr) sides[2] = sides[0];
        if(sides[3] == nullptr) sides[3] = sides[1];
    };

    RefPtr<CSSValue> horizontal[4];
    for(auto& side : horizontal) {
        if(input.empty() || input->type() == CSSToken::Type::Delim)
            break;
        auto value = consumeLengthOrPercent(input, false, false);
        if(value == nullptr)
            return false;
        side = std::move(value);
    }

    if(horizontal[0] == nullptr)
        return false;
    completesides(horizontal);

    RefPtr<CSSValue> vertical[4];
    if(input->type() == CSSToken::Type::Delim && input->delim() == '/') {
        input.consumeIncludingWhitespace();
        for(auto& side : vertical) {
            if(input->type() == CSSToken::Type::EndOfFile)
                break;
            auto value = consumeLengthOrPercent(input, false, false);
            if(value == nullptr)
                return false;
            side = std::move(value);
        }

        if(vertical[0] == nullptr)
            return false;
        completesides(vertical);
    } else if(input->type() == CSSToken::Type::EndOfFile) {
        vertical[0] = horizontal[0];
        vertical[1] = horizontal[1];
        vertical[2] = horizontal[2];
        vertical[3] = horizontal[3];
    } else {
        return false;
    }

    auto tl = CSSPairValue::create(m_heap, horizontal[0], vertical[0]);
    auto tr = CSSPairValue::create(m_heap, horizontal[1], vertical[1]);
    auto br = CSSPairValue::create(m_heap, horizontal[2], vertical[2]);
    auto bl = CSSPairValue::create(m_heap, horizontal[3], vertical[3]);

    addProperty(properties, CSSPropertyID::BorderTopLeftRadius, important, std::move(tl));
    addProperty(properties, CSSPropertyID::BorderTopRightRadius, important, std::move(tr));
    addProperty(properties, CSSPropertyID::BorderBottomRightRadius, important, std::move(br));
    addProperty(properties, CSSPropertyID::BorderBottomLeftRadius, important, std::move(bl));
    return true;
}

bool CSSParser::consumeMarker(CSSTokenStream& input, CSSPropertyList& properties, bool important)
{
    auto marker = consumeLocalUrlOrNone(input);
    if(!marker || !input.empty())
        return false;
    addProperty(properties, CSSPropertyID::MarkerStart, important, marker);
    addProperty(properties, CSSPropertyID::MarkerMid, important, marker);
    addProperty(properties, CSSPropertyID::MarkerEnd, important, marker);
    return true;
}

bool CSSParser::consume2Shorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important)
{
    auto longhand = CSSShorthand::longhand(id);
    assert(longhand.length() == 2);
    auto first = consumeLonghand(input, longhand.at(0));
    if(first == nullptr)
        return false;
    addProperty(properties, longhand.at(0), important, first);
    auto second = consumeLonghand(input, longhand.at(1));
    if(second == nullptr) {
        addProperty(properties, longhand.at(1), important, first);
        return true;
    }

    addProperty(properties, longhand.at(1), important, second);
    return true;
}

bool CSSParser::consume4Shorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important)
{
    auto longhand = CSSShorthand::longhand(id);
    assert(longhand.length() == 4);
    auto top = consumeLonghand(input, longhand.at(0));
    if(top == nullptr)
        return false;
    addProperty(properties, longhand.at(0), important, top);
    auto right = consumeLonghand(input, longhand.at(1));
    if(right == nullptr) {
        addProperty(properties, longhand.at(1), important, top);
        addProperty(properties, longhand.at(2), important, top);
        addProperty(properties, longhand.at(3), important, top);
        return true;
    }

    addProperty(properties, longhand.at(1), important, right);
    auto bottom = consumeLonghand(input, longhand.at(1));
    if(bottom == nullptr) {
        addProperty(properties, longhand.at(2), important, top);
        addProperty(properties, longhand.at(3), important, right);
        return true;
    }

    addProperty(properties, longhand.at(2), important, bottom);
    auto left = consumeLonghand(input, longhand.at(3));
    if(left == nullptr) {
        addProperty(properties, longhand.at(3), important, right);
        return true;
    }

    addProperty(properties, longhand.at(3), important, left);
    return true;
}

bool CSSParser::consumeShorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important)
{
    RefPtr<CSSValue> values[6];
    auto longhand = CSSShorthand::longhand(id);
    assert(longhand.length() <= sizeof(values));
    while(!input.empty()) {
        bool consumed = false;
        for(size_t i = 0; i < longhand.length(); ++i) {
            if(values[i] == nullptr && (values[i] = consumeLonghand(input, longhand.at(i)))) {
                consumed = true;
            }
        }

        if(!consumed) {
            return false;
        }
    }

    for(size_t i = 0; i < longhand.length(); ++i)
        addProperty(properties, longhand.at(i), important, std::move(values[i]));
    return true;
}

RefPtr<CSSValue> CSSParser::consumeFontFaceSource(CSSTokenStream& input)
{
    CSSValueList values(m_heap);
    if(input->type() == CSSToken::Type::Function && identMatches("local", 5, input->data())) {
        auto block = input.consumeBlock();
        block.consumeWhitespace();
        auto value = consumeFontFamilyName(block);
        if(value == nullptr || !block.empty())
            return nullptr;
        auto function = CSSUnaryFunctionValue::create(m_heap, CSSFunctionID::Local, std::move(value));
        input.consumeWhitespace();
        values.push_back(std::move(function));
    } else {
        auto url = consumeUrl(input);
        if(url == nullptr)
            return nullptr;
        values.push_back(std::move(url));
        if(input->type() == CSSToken::Type::Function && identMatches("format", 6, input->data())) {
            auto block = input.consumeBlock();
            block.consumeWhitespace();
            auto value = consumeStringOrCustomIdent(block);
            if(value == nullptr || !block.empty())
                return nullptr;
            auto format = CSSUnaryFunctionValue::create(m_heap, CSSFunctionID::Format, std::move(value));
            input.consumeWhitespace();
            values.push_back(std::move(format));
        }
    }

    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeFontFaceSrc(CSSTokenStream& input)
{
    CSSValueList values(m_heap);
    do {
        auto value = consumeFontFaceSource(input);
        if(value == nullptr)
            return nullptr;
        values.push_back(std::move(value));
    } while(input.consumeCommaIncludingWhitespace());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeFontFaceWeight(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"normal", CSSValueID::Normal},
        {"bold", CSSValueID::Bold}
    };

    if(auto value = consumeIdent(input, table))
        return value;
    auto startWeight = consumeNumber(input, false);
    if(startWeight == nullptr)
        return nullptr;
    auto endWeight = consumeNumber(input, false);
    if(endWeight == nullptr)
        endWeight = startWeight;
    return CSSPairValue::create(m_heap, startWeight, endWeight);
}

RefPtr<CSSValue> CSSParser::consumeFontFaceStyle(CSSTokenStream& input)
{
    auto ident = consumeFontStyleIdent(input);
    if(ident == nullptr)
        return nullptr;
    if(ident->value() != CSSValueID::Oblique)
        return ident;
    auto startAngle = consumeAngle(input);
    if(startAngle == nullptr)
        return ident;
    auto endAngle = consumeAngle(input);
    if(endAngle == nullptr)
        endAngle = startAngle;
    CSSValueList values(m_heap);
    values.push_back(std::move(ident));
    values.push_back(std::move(startAngle));
    values.push_back(std::move(endAngle));
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeFontFaceStretch(CSSTokenStream& input)
{
    if(auto value = consumeFontStretchIdent(input))
        return value;
    auto startPercent = consumePercent(input, false);
    if(startPercent == nullptr)
        return nullptr;
    auto endPercent = consumePercent(input, false);
    if(endPercent == nullptr)
        endPercent = startPercent;
    return CSSPairValue::create(m_heap, startPercent, endPercent);
}

RefPtr<CSSValue> CSSParser::consumeFontFaceUnicodeRange(CSSTokenStream& input)
{
    CSSValueList values(m_heap);
    do {
        if(input->type() != CSSToken::Type::UnicodeRange)
            return nullptr;
        if(input->to() > 0x10FFFF || input->from() > input->to())
            return nullptr;
        values.push_back(CSSUnicodeRangeValue::create(m_heap, input->from(), input->to()));
        input.consumeIncludingWhitespace();
    } while(input.consumeCommaIncludingWhitespace());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeCounterStyleName(CSSTokenStream& input)
{
    if(input->type() != CSSToken::Type::Ident || identMatches("none", 4, input->data()))
        return nullptr;
    GlobalString name(input->data());
    input.consumeIncludingWhitespace();
    return CSSCustomIdentValue::create(m_heap, name);
}

RefPtr<CSSValue> CSSParser::consumeCounterStyleSystem(CSSTokenStream& input)
{
    static const CSSIdentValueEntry table[] = {
        {"cyclic", CSSValueID::Cyclic},
        {"symbolic", CSSValueID::Symbolic},
        {"alphabetic", CSSValueID::Alphabetic},
        {"numeric", CSSValueID::Numeric},
        {"additive", CSSValueID::Additive},
        {"fixed", CSSValueID::Fixed},
        {"extends", CSSValueID::Extends}
    };

    auto ident = consumeIdent(input, table);
    if(ident == nullptr)
        return nullptr;
    if(ident->value() == CSSValueID::Fixed) {
        auto fixed = consumeInteger(input, true);
        if(fixed == nullptr)
            fixed = CSSIntegerValue::create(m_heap, 1);
        return CSSPairValue::create(m_heap, ident, fixed);
    }

    if(ident->value() == CSSValueID::Extends) {
        auto extends = consumeCounterStyleName(input);
        if(extends == nullptr)
            return nullptr;
        return CSSPairValue::create(m_heap, ident, extends);
    }

    return ident;
}

RefPtr<CSSValue> CSSParser::consumeCounterStyleNegative(CSSTokenStream& input)
{
    auto prepend = consumeCounterStyleSymbol(input);
    if(prepend == nullptr)
        return nullptr;
    if(auto append = consumeCounterStyleSymbol(input))
        return CSSPairValue::create(m_heap, prepend, append);
    return prepend;
}

RefPtr<CSSValue> CSSParser::consumeCounterStyleSymbol(CSSTokenStream& input)
{
    if(auto value = consumeStringOrCustomIdent(input))
        return value;
    return consumeImage(input);
}

RefPtr<CSSValue> CSSParser::consumeCounterStyleRangeBound(CSSTokenStream& input)
{
    if(consumeIdentIncludingWhitespace(input, "infinite", 8))
        return CSSIdentValue::create(CSSValueID::Infinite);
    return consumeInteger(input, true);
}

RefPtr<CSSValue> CSSParser::consumeCounterStyleRange(CSSTokenStream& input)
{
    if(auto value = consumeAuto(input))
        return value;
    CSSValueList values(m_heap);
    do {
        auto lowerBound = consumeCounterStyleRangeBound(input);
        if(lowerBound == nullptr)
            return nullptr;
        auto upperBound = consumeCounterStyleRangeBound(input);
        if(upperBound == nullptr)
            return nullptr;
        values.push_back(CSSPairValue::create(m_heap, lowerBound, upperBound));
    } while(input.consumeCommaIncludingWhitespace());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeCounterStylePad(CSSTokenStream& input)
{
    RefPtr<CSSValue> integer;
    RefPtr<CSSValue> symbol;
    while(!integer || !symbol) {
        if(integer == nullptr && (integer = consumeInteger(input, false)))
            continue;
        if(symbol == nullptr && (symbol = consumeCounterStyleSymbol(input)))
            continue;
        return nullptr;
    }

    return CSSPairValue::create(m_heap, integer, symbol);
}

RefPtr<CSSValue> CSSParser::consumeCounterStyleSymbols(CSSTokenStream& input)
{
    CSSValueList values(m_heap);
    do {
        auto symbol = consumeCounterStyleSymbol(input);
        if(symbol == nullptr)
            return nullptr;
        values.push_back(std::move(symbol));
    } while(!input.empty());
    return CSSListValue::create(m_heap, std::move(values));
}

RefPtr<CSSValue> CSSParser::consumeCounterStyleAdditiveSymbols(CSSTokenStream& input)
{
    CSSValueList values(m_heap);
    do {
        auto value = consumeCounterStylePad(input);
        if(value == nullptr)
            return nullptr;
        values.push_back(std::move(value));
    } while(input.consumeCommaIncludingWhitespace());
    return CSSListValue::create(m_heap, std::move(values));
}

const GlobalString& CSSParser::determineNamespace(const GlobalString& prefix) const
{
    if(prefix.isEmpty())
        return m_defaultNamespace;
    if(prefix == starGlo)
        return starGlo;
    if(m_namespaces.contains(prefix))
        return m_namespaces.at(prefix);
    return emptyGlo;
}

} // namespace plutobook
