/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_USTRING_H
#define PLUTOBOOK_USTRING_H

#include <unicode/unistr.h>

namespace plutobook {

using UString = icu::UnicodeString;

constexpr UChar kActivateArabicFormShapingCharacter = 0x206D;
constexpr UChar kActivateSymmetricSwappingCharacter = 0x206B;
constexpr UChar32 kAegeanWordSeparatorLineCharacter = 0x10100;
constexpr UChar32 kAegeanWordSeparatorDotCharacter = 0x10101;
constexpr UChar kArabicLetterMarkCharacter = 0x061C;
constexpr UChar32 kArabicMathematicalOperatorMeemWithHahWithTatweel = 0x1EEF0;
constexpr UChar32 kArabicMathematicalOperatorHahWithDal = 0x1EEF1;
constexpr UChar kBlackCircleCharacter = 0x25CF;
constexpr UChar kBlackDownPointingSmallTriangle = 0x25BE;
constexpr UChar kBlackRightPointingSmallTriangle = 0x25B8;
constexpr UChar kBlackSquareCharacter = 0x25A0;
constexpr UChar kBlackUpPointingTriangleCharacter = 0x25B2;
constexpr UChar kBulletCharacter = 0x2022;
constexpr UChar kBullseyeCharacter = 0x25CE;
constexpr UChar32 kCancelTag = 0xE007F;
constexpr UChar kCarriageReturnCharacter = 0x000D;
constexpr UChar kCjkWaterCharacter = 0x6C34;
constexpr UChar kColon = 0x3A;
constexpr UChar kCombiningAcuteAccentCharacter = 0x0301;
constexpr UChar kCombiningEnclosingCircleBackslashCharacter = 0x20E0;
constexpr UChar kCombiningEnclosingKeycapCharacter = 0x20E3;
constexpr UChar kCombiningLongSolidusOverlay = 0x0338;
constexpr UChar kCombiningLongVerticalLineOverlay = 0x20D2;
constexpr UChar kCombiningMinusSignBelow = 0x0320;
constexpr UChar kComma = 0x2C;
constexpr UChar kDeleteCharacter = 0x007F;
constexpr UChar kDoubleStruckItalicCapitalDCharacter = 0x2145;
constexpr UChar kDoubleStruckItalicSmallDCharacter = 0x2146;
constexpr UChar kEnQuadCharacter = 0x2000;
constexpr UChar kEthiopicNumberHundredCharacter = 0x137B;
constexpr UChar kEthiopicNumberTenThousandCharacter = 0x137C;
constexpr UChar kEthiopicPrefaceColonCharacter = 0x1366;
constexpr UChar kEthiopicWordspaceCharacter = 0x1361;
constexpr UChar kHeavyBlackHeartCharacter = 0x2764;
constexpr UChar32 kEyeCharacter = 0x1F441;
constexpr UChar32 kBoyCharacter = 0x1F466;
constexpr UChar32 kGirlCharacter = 0x1F467;
constexpr UChar32 kManCharacter = 0x1F468;
constexpr UChar32 kWomanCharacter = 0x1F469;
constexpr UChar32 kKissMarkCharacter = 0x1F48B;
constexpr UChar32 kFamilyCharacter = 0x1F46A;
constexpr UChar kFemaleSignCharacter = 0x2640;
constexpr UChar kFirstStrongIsolateCharacter = 0x2068;
constexpr UChar kFisheyeCharacter = 0x25C9;
constexpr UChar kFourthRootCharacter = 0x221C;
constexpr UChar kFullstopCharacter = 0x002E;
constexpr UChar kGreekCapitalReversedDottedLunateSigmaSymbol = 0x03FF;
constexpr UChar kGreekKappaSymbol = 0x03F0;
constexpr UChar kGreekLetterDigamma = 0x03DC;
constexpr UChar kGreekLowerAlpha = 0x03B1;
constexpr UChar kGreekLowerOmega = 0x03C9;
constexpr UChar kGreekLunateEpsilonSymbol = 0x03F5;
constexpr UChar kGreekPhiSymbol = 0x03D5;
constexpr UChar kGreekPiSymbol = 0x03D6;
constexpr UChar kGreekRhoSymbol = 0x03F1;
constexpr UChar kGreekSmallLetterDigamma = 0x03DD;
constexpr UChar kGreekThetaSymbol = 0x03D1;
constexpr UChar kGreekUpperAlpha = 0x0391;
constexpr UChar kGreekUpperOmega = 0x03A9;
constexpr UChar kGreekUpperTheta = 0x03F4;
constexpr UChar kHebrewPunctuationGereshCharacter = 0x05F3;
constexpr UChar kHebrewPunctuationGershayimCharacter = 0x05F4;
constexpr UChar kHellschreiberPauseSymbol = 0x2BFF;
constexpr UChar kHiraganaLetterSmallACharacter = 0x3041;
constexpr UChar kHoleGreekUpperTheta = 0x03A2;
constexpr UChar kHorizontalEllipsisCharacter = 0x2026;
constexpr UChar kHyphenCharacter = 0x2010;
constexpr UChar kHyphenMinusCharacter = 0x002D;
constexpr UChar kIdeographicCommaCharacter = 0x3001;
constexpr UChar kIdeographicFullStopCharacter = 0x3002;
constexpr UChar kIdeographicSpaceCharacter = 0x3000;
constexpr UChar kInhibitArabicFormShapingCharacter = 0x206C;
constexpr UChar kInhibitSymmetricSwappingCharacter = 0x206A;
constexpr UChar kLatinCapitalLetterIWithDotAbove = 0x0130;
constexpr UChar kLatinSmallLetterDotlessI = 0x0131;
constexpr UChar kLatinSmallLetterDotlessJ = 0x0237;
constexpr UChar kLeftCornerBracket = 0x300C;
constexpr UChar kLeftDoubleQuotationMarkCharacter = 0x201C;
constexpr UChar kLeftSingleQuotationMarkCharacter = 0x2018;
constexpr UChar32 kLeftSpeechBubbleCharacter = 0x1F5E8;
constexpr UChar kLeftToRightEmbedCharacter = 0x202A;
constexpr UChar kLeftToRightIsolateCharacter = 0x2066;
constexpr UChar kLeftToRightMarkCharacter = 0x200E;
constexpr UChar kLeftToRightOverrideCharacter = 0x202D;
constexpr UChar kLineSeparator = 0x2028;
constexpr UChar kLineTabulationCharacter = 0x000B;
constexpr UChar kLowLineCharacter = 0x005F;
constexpr UChar kMaleSignCharacter = 0x2642;
constexpr UChar32 kMathItalicSmallDotlessI = 0x1D6A4;
constexpr UChar32 kMathItalicSmallDotlessJ = 0x1D6A5;
constexpr UChar32 kMathBoldEpsilonSymbol = 0x1D6DC;
constexpr UChar32 kMathBoldKappaSymbol = 0x1D6DE;
constexpr UChar32 kMathBoldNabla = 0x1D6C1;
constexpr UChar32 kMathBoldPartialDifferential = 0x1D6DB;
constexpr UChar32 kMathBoldPhiSymbol = 0x1D6DF;
constexpr UChar32 kMathBoldPiSymbol = 0x1D6E1;
constexpr UChar32 kMathBoldRhoSymbol = 0x1D6E0;
constexpr UChar32 kMathBoldSmallA = 0x1D41A;
constexpr UChar32 kMathBoldSmallAlpha = 0x1D6C2;
constexpr UChar32 kMathBoldSmallDigamma = 0x1D7CB;
constexpr UChar32 kMathBoldUpperA = 0x1D400;
constexpr UChar32 kMathBoldUpperAlpha = 0x1D6A8;
constexpr UChar32 kMathBoldUpperTheta = 0x1D6B9;
constexpr UChar32 kMathBoldThetaSymbol = 0x1D6DD;
constexpr UChar32 kMathItalicUpperA = 0x1D434;
constexpr UChar32 kMathItalicUpperAlpha = 0x1D6E2;
constexpr UChar kMinusSignCharacter = 0x2212;
constexpr UChar kNewlineCharacter = 0x000A;
constexpr UChar32 kNonCharacter = 0xFFFF;
constexpr UChar kFormFeedCharacter = 0x000C;
constexpr UChar32 kNabla = 0x2207;
constexpr UChar kNationalDigitShapesCharacter = 0x206E;
constexpr UChar kNominalDigitShapesCharacter = 0x206F;
constexpr UChar kNoBreakSpaceCharacter = 0x00A0;
constexpr UChar kObjectReplacementCharacter = 0xFFFC;
constexpr UChar kOverlineCharacter = 0x203E;
constexpr UChar kParagraphSeparator = 0x2029;
constexpr UChar32 kPartialDifferential = 0x2202;
constexpr UChar kPopDirectionalFormattingCharacter = 0x202C;
constexpr UChar kPopDirectionalIsolateCharacter = 0x2069;
constexpr UChar32 kRainbowCharacter = 0x1F308;
constexpr UChar kReplacementCharacter = 0xFFFD;
constexpr UChar kReverseSolidusCharacter = 0x005C;
constexpr UChar kRightDoubleQuotationMarkCharacter = 0x201D;
constexpr UChar kRightSingleQuotationMarkCharacter = 0x2019;
constexpr UChar kRightToLeftEmbedCharacter = 0x202B;
constexpr UChar kRightToLeftIsolateCharacter = 0x2067;
constexpr UChar kRightToLeftMarkCharacter = 0x200F;
constexpr UChar kRightToLeftOverrideCharacter = 0x202E;
constexpr UChar kSemiColon = 0x3B;
constexpr UChar kSesameDotCharacter = 0xFE45;
constexpr UChar kSmallLetterSharpSCharacter = 0x00DF;
constexpr UChar kSolidusCharacter = 0x002F;
constexpr UChar kSoftHyphenCharacter = 0x00AD;
constexpr UChar kSpaceCharacter = 0x0020;
constexpr UChar32 kSquareRootCharacter = 0x221A;
constexpr UChar kStaffOfAesculapiusCharacter = 0x2695;
constexpr UChar kTabulationCharacter = 0x0009;
constexpr UChar32 kTagDigitZero = 0xE0030;
constexpr UChar32 kTagDigitNine = 0xE0039;
constexpr UChar32 kTagLatinSmallLetterA = 0xE0061;
constexpr UChar32 kTagLatinSmallLetterZ = 0xE007A;
constexpr UChar kTibetanMarkIntersyllabicTshegCharacter = 0x0F0B;
constexpr UChar kTibetanMarkDelimiterTshegBstarCharacter = 0x0F0C;
constexpr UChar kTildeOperatorCharacter = 0x223C;
constexpr UChar32 kUgariticWordDividerCharacter = 0x1039F;
constexpr UChar kVariationSelector15Character = 0xFE0E;
constexpr UChar kVariationSelector16Character = 0xFE0F;
constexpr UChar kVerticalLineCharacter = 0x7C;
constexpr UChar32 kWavingWhiteFlagCharacter = 0x1F3F3;
constexpr UChar kWhiteBulletCharacter = 0x25E6;
constexpr UChar kWhiteCircleCharacter = 0x25CB;
constexpr UChar kWhiteSesameDotCharacter = 0xFE46;
constexpr UChar kWhiteUpPointingTriangleCharacter = 0x25B3;
constexpr UChar kYenSignCharacter = 0x00A5;
constexpr UChar kZeroWidthJoinerCharacter = 0x200D;
constexpr UChar kZeroWidthNonJoinerCharacter = 0x200C;
constexpr UChar kZeroWidthSpaceCharacter = 0x200B;
constexpr UChar kZeroWidthNoBreakSpaceCharacter = 0xFEFF;
constexpr UChar kPrivateUseFirstCharacter = 0xE000;
constexpr UChar kPrivateUseLastCharacter = 0xF8FF;
constexpr UChar32 kMaxCodepoint = 0x10ffff;

constexpr bool treatAsZeroWidthSpace(UChar cc)
{
    return cc == kFormFeedCharacter || cc == kCarriageReturnCharacter
        || cc == kSoftHyphenCharacter || cc == kZeroWidthSpaceCharacter
        || cc == kZeroWidthNoBreakSpaceCharacter || cc == kObjectReplacementCharacter
        || cc == kZeroWidthNonJoinerCharacter || cc == kZeroWidthJoinerCharacter
        || (cc >= kLeftToRightMarkCharacter && cc <= kRightToLeftMarkCharacter)
        || (cc >= kLeftToRightEmbedCharacter && cc <= kRightToLeftOverrideCharacter);
}

constexpr bool treatAsSpace(UChar cc)
{
    return cc == kSpaceCharacter || cc == kTabulationCharacter
        || cc == kNewlineCharacter || cc == kNoBreakSpaceCharacter;
}

} // namespace plutobook

#endif // PLUTOBOOK_USTRING_H
