/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_CSSPARSER_H
#define PLUTOBOOK_CSSPARSER_H

#include "cssrule.h"

namespace plutobook {

class CSSParser {
public:
    CSSParser(const CSSParserContext& context, Heap* heap);

    CSSRuleList parseSheet(const std::string_view& content);
    CSSPropertyList parseStyle(const std::string_view& content);
    CSSMediaQueryList parseMediaQueries(const std::string_view& content);

    CSSPropertyList parsePropertyValue(CSSTokenStream input, CSSPropertyID id, bool important);

private:
    bool consumeMediaFeature(CSSTokenStream& input, CSSMediaFeatureList& features);
    bool consumeMediaFeatures(CSSTokenStream& input, CSSMediaFeatureList& features);
    bool consumeMediaQuery(CSSTokenStream& input, CSSMediaQueryList& queries);
    bool consumeMediaQueries(CSSTokenStream& input, CSSMediaQueryList& queries);

    RefPtr<CSSRule> consumeRule(CSSTokenStream& input);
    RefPtr<CSSRule> consumeAtRule(CSSTokenStream& input);
    RefPtr<CSSStyleRule> consumeStyleRule(CSSTokenStream& input);
    RefPtr<CSSImportRule> consumeImportRule(CSSTokenStream& input);
    RefPtr<CSSNamespaceRule> consumeNamespaceRule(CSSTokenStream& input);
    RefPtr<CSSMediaRule> consumeMediaRule(CSSTokenStream& prelude, CSSTokenStream& block);
    RefPtr<CSSFontFaceRule> consumeFontFaceRule(CSSTokenStream& prelude, CSSTokenStream& block);
    RefPtr<CSSCounterStyleRule> consumeCounterStyleRule(CSSTokenStream& prelude, CSSTokenStream& block);
    RefPtr<CSSPageRule> consumePageRule(CSSTokenStream& prelude, CSSTokenStream& block);
    RefPtr<CSSPageMarginRule> consumePageMarginRule(CSSTokenStream& input);

    void consumeRuleList(CSSTokenStream& input, CSSRuleList& rules);
    bool consumePageSelectorList(CSSTokenStream& input, CSSPageSelectorList& selectors);
    bool consumePageSelector(CSSTokenStream& input, CSSPageSelector& selector);

    bool consumeSelectorList(CSSTokenStream& input, CSSSelectorList& selectors, bool relative);
    bool consumeSelector(CSSTokenStream& input, CSSSelector& selector, bool relative);
    bool consumeCompoundSelector(CSSTokenStream& input, CSSCompoundSelector& selector, bool& failed);
    bool consumeSimpleSelector(CSSTokenStream& input, CSSCompoundSelector& selector, bool& failed);

    bool consumeTagSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    bool consumeIdSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    bool consumeClassSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    bool consumeAttributeSelector(CSSTokenStream& input, CSSCompoundSelector& selector);
    bool consumePseudoSelector(CSSTokenStream& input, CSSCompoundSelector& selector);

    bool consumeCombinator(CSSTokenStream& input, CSSComplexSelector::Combinator& combinator);
    bool consumeMatchPattern(CSSTokenStream& input, CSSSimpleSelector::MatchPattern& pattern);

    bool consumeFontFaceDescriptor(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id);
    bool consumeCounterStyleDescriptor(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id);
    bool consumeDescriptor(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);

    bool consumeDeclaraction(CSSTokenStream& input, CSSPropertyList& properties, CSSRuleType ruleType);
    void consumeDeclaractionList(CSSTokenStream& input, CSSPropertyList& properties, CSSRuleType ruleType);

    void addProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, RefPtr<CSSValue> value);
    void addExpandedProperty(CSSPropertyList& properties, CSSPropertyID id, bool important, RefPtr<CSSValue> value);

    RefPtr<CSSIdentValue> consumeFontStyleIdent(CSSTokenStream& input);
    RefPtr<CSSIdentValue> consumeFontStretchIdent(CSSTokenStream& input);
    RefPtr<CSSIdentValue> consumeFontVariantCapsIdent(CSSTokenStream& input);
    RefPtr<CSSIdentValue> consumeFontVariantEmojiIdent(CSSTokenStream& input);
    RefPtr<CSSIdentValue> consumeFontVariantPositionIdent(CSSTokenStream& input);
    RefPtr<CSSIdentValue> consumeFontVariantEastAsianIdent(CSSTokenStream& input);
    RefPtr<CSSIdentValue> consumeFontVariantLigaturesIdent(CSSTokenStream& input);
    RefPtr<CSSIdentValue> consumeFontVariantNumericIdent(CSSTokenStream& input);

    RefPtr<CSSValue> consumeNone(CSSTokenStream& input);
    RefPtr<CSSValue> consumeAuto(CSSTokenStream& input);
    RefPtr<CSSValue> consumeNormal(CSSTokenStream& input);
    RefPtr<CSSValue> consumeNoneOrAuto(CSSTokenStream& input);
    RefPtr<CSSValue> consumeNoneOrNormal(CSSTokenStream& input);

    RefPtr<CSSValue> consumeInteger(CSSTokenStream& input, bool negative);
    RefPtr<CSSValue> consumeIntegerOrAuto(CSSTokenStream& input, bool negative);
    RefPtr<CSSValue> consumePositiveInteger(CSSTokenStream& input);
    RefPtr<CSSValue> consumePositiveIntegerOrAuto(CSSTokenStream& input);
    RefPtr<CSSValue> consumePercent(CSSTokenStream& input, bool negative);
    RefPtr<CSSValue> consumeNumber(CSSTokenStream& input, bool negative);
    RefPtr<CSSValue> consumeNumberOrPercent(CSSTokenStream& input, bool negative);
    RefPtr<CSSValue> consumeNumberOrPercentOrAuto(CSSTokenStream& input, bool negative);
    RefPtr<CSSValue> consumeCalc(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLength(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLengthOrPercent(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLengthOrAuto(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLengthOrNormal(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLengthOrPercentOrAuto(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLengthOrPercentOrNone(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeLengthOrPercentOrNormal(CSSTokenStream& input, bool negative, bool unitless);
    RefPtr<CSSValue> consumeWidthOrHeight(CSSTokenStream& input, bool unitless);
    RefPtr<CSSValue> consumeWidthOrHeightOrAuto(CSSTokenStream& input, bool unitless);
    RefPtr<CSSValue> consumeWidthOrHeightOrNone(CSSTokenStream& input, bool unitless);

    RefPtr<CSSValue> consumeString(CSSTokenStream& input);
    RefPtr<CSSValue> consumeCustomIdent(CSSTokenStream& input);
    RefPtr<CSSValue> consumeStringOrCustomIdent(CSSTokenStream& input);
    RefPtr<CSSValue> consumeAttr(CSSTokenStream& input);
    RefPtr<CSSValue> consumeLocalUrl(CSSTokenStream& input);
    RefPtr<CSSValue> consumeLocalUrlOrAttr(CSSTokenStream& input);
    RefPtr<CSSValue> consumeLocalUrlOrNone(CSSTokenStream& input);
    RefPtr<CSSValue> consumeUrl(CSSTokenStream& input);
    RefPtr<CSSValue> consumeUrlOrNone(CSSTokenStream& input);
    RefPtr<CSSValue> consumeImage(CSSTokenStream& input);
    RefPtr<CSSValue> consumeImageOrNone(CSSTokenStream& input);
    RefPtr<CSSValue> consumeColor(CSSTokenStream& input);
    RefPtr<CSSValue> consumeRgb(CSSTokenStream& input);
    RefPtr<CSSValue> consumeHsl(CSSTokenStream& input);
    RefPtr<CSSValue> consumeHwb(CSSTokenStream& input);

    RefPtr<CSSValue> consumePaint(CSSTokenStream& input);
    RefPtr<CSSValue> consumeListStyleType(CSSTokenStream& input);
    RefPtr<CSSValue> consumeQuotes(CSSTokenStream& input);
    RefPtr<CSSValue> consumeContent(CSSTokenStream& input);
    RefPtr<CSSValue> consumeContentLeader(CSSTokenStream& input);
    RefPtr<CSSValue> consumeContentElement(CSSTokenStream& input);
    RefPtr<CSSValue> consumeContentCounter(CSSTokenStream& input, bool counters);
    RefPtr<CSSValue> consumeContentTargetCounter(CSSTokenStream& input, bool counters);
    RefPtr<CSSValue> consumeContentQrCode(CSSTokenStream& input);
    RefPtr<CSSValue> consumeCounter(CSSTokenStream& input, bool increment);
    RefPtr<CSSValue> consumePage(CSSTokenStream& input);
    RefPtr<CSSValue> consumeOrientation(CSSTokenStream& input);
    RefPtr<CSSValue> consumeSize(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontSize(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontWeight(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontStyle(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontStretch(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFamilyName(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFamily(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFeature(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFeatureSettings(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontVariation(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontVariationSettings(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontVariantCaps(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontVariantEmoji(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontVariantPosition(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontVariantEastAsian(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontVariantLigatures(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontVariantNumeric(CSSTokenStream& input);
    RefPtr<CSSValue> consumeLineWidth(CSSTokenStream& input);
    RefPtr<CSSValue> consumeBorderRadiusValue(CSSTokenStream& input);
    RefPtr<CSSValue> consumeClip(CSSTokenStream& input);
    RefPtr<CSSValue> consumeDashList(CSSTokenStream& input);
    RefPtr<CSSValue> consumePosition(CSSTokenStream& input);
    RefPtr<CSSValue> consumeVerticalAlign(CSSTokenStream& input);
    RefPtr<CSSValue> consumeBaselineShift(CSSTokenStream& input);
    RefPtr<CSSValue> consumeTextDecorationLine(CSSTokenStream& input);
    RefPtr<CSSValue> consumePositionCoordinate(CSSTokenStream& input);
    RefPtr<CSSValue> consumeBackgroundSize(CSSTokenStream& input);
    RefPtr<CSSValue> consumeAngle(CSSTokenStream& input);
    RefPtr<CSSValue> consumeTransformValue(CSSTokenStream& input);
    RefPtr<CSSValue> consumeTransform(CSSTokenStream& input);
    RefPtr<CSSValue> consumePaintOrder(CSSTokenStream& input);
    RefPtr<CSSValue> consumeLonghand(CSSTokenStream& input, CSSPropertyID id);

    bool consumeFlex(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeBackground(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeColumns(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeListStyle(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeFont(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeFontVariant(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeBorder(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeBorderRadius(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consumeMarker(CSSTokenStream& input, CSSPropertyList& properties, bool important);
    bool consume2Shorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);
    bool consume4Shorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);
    bool consumeShorthand(CSSTokenStream& input, CSSPropertyList& properties, CSSPropertyID id, bool important);

    RefPtr<CSSValue> consumeFontFaceSource(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFaceSrc(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFaceWeight(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFaceStyle(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFaceStretch(CSSTokenStream& input);
    RefPtr<CSSValue> consumeFontFaceUnicodeRange(CSSTokenStream& input);

    RefPtr<CSSValue> consumeCounterStyleName(CSSTokenStream& input);
    RefPtr<CSSValue> consumeCounterStyleSystem(CSSTokenStream& input);
    RefPtr<CSSValue> consumeCounterStyleNegative(CSSTokenStream& input);
    RefPtr<CSSValue> consumeCounterStyleSymbol(CSSTokenStream& input);
    RefPtr<CSSValue> consumeCounterStyleRangeBound(CSSTokenStream& input);
    RefPtr<CSSValue> consumeCounterStyleRange(CSSTokenStream& input);
    RefPtr<CSSValue> consumeCounterStylePad(CSSTokenStream& input);
    RefPtr<CSSValue> consumeCounterStyleSymbols(CSSTokenStream& input);
    RefPtr<CSSValue> consumeCounterStyleAdditiveSymbols(CSSTokenStream& input);

    const GlobalString& defaultNamespace() const { return m_defaultNamespace; }
    const GlobalString& determineNamespace(const GlobalString& prefix) const;

    Heap* m_heap;
    const CSSParserContext& m_context;
    std::map<GlobalString, GlobalString> m_namespaces;
    GlobalString m_defaultNamespace = starGlo;
};

} // namespace plutobook

#endif // PLUTOBOOK_CSSPARSER_H
