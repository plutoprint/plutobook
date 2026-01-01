/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "boxstyle.h"
#include "cssrule.h"
#include "document.h"
#include "imageresource.h"
#include "fontresource.h"
#include "geometry.h"

#include "plutobook.hpp"

namespace plutobook {

const Length Length::None(Length::Type::None);
const Length Length::Auto(Length::Type::Auto);
const Length Length::MinContent(Length::Type::MinContent);
const Length Length::MaxContent(Length::Type::MaxContent);
const Length Length::FitContent(Length::Type::FitContent);
const Length Length::ZeroFixed(Length::Type::Fixed);
const Length Length::ZeroPercent(Length::Type::Percent);

RefPtr<BoxStyle> BoxStyle::create(Node* node, PseudoType pseudoType, Display display)
{
    return adoptPtr(new (node->heap()) BoxStyle(node, pseudoType, display));
}

RefPtr<BoxStyle> BoxStyle::create(Node* node, const BoxStyle* parentStyle, PseudoType pseudoType, Display display)
{
    auto newStyle = create(node, pseudoType, display);
    newStyle->inheritFrom(parentStyle);
    return newStyle;
}

RefPtr<BoxStyle> BoxStyle::create(const BoxStyle* parentStyle, PseudoType pseudoType, Display display)
{
    return create(parentStyle->node(), parentStyle, pseudoType, display);
}

RefPtr<BoxStyle> BoxStyle::create(const BoxStyle* parentStyle, Display display)
{
    return create(parentStyle, PseudoType::None, display);
}

Document* BoxStyle::document() const
{
    return m_node->document();
}

Heap* BoxStyle::heap() const
{
    return m_node->heap();
}

Book* BoxStyle::book() const
{
    return document()->book();
}

void BoxStyle::setFont(RefPtr<Font> font)
{
    m_font = std::move(font);
}

float BoxStyle::fontAscent() const
{
    if(auto fontData = m_font->primaryFont())
        return fontData->ascent();
    return 0.f;
}

float BoxStyle::fontDescent() const
{
    if(auto fontData = m_font->primaryFont())
        return fontData->descent();
    return 0.f;
}

float BoxStyle::fontHeight() const
{
    if(auto fontData = m_font->primaryFont())
        return fontData->height();
    return 0.f;
}

float BoxStyle::fontLineGap() const
{
    if(auto fontData = m_font->primaryFont())
        return fontData->lineGap();
    return 0.f;
}

float BoxStyle::fontLineSpacing() const
{
    if(auto fontData = m_font->primaryFont())
        return fontData->lineSpacing();
    return 0.f;
}

const FontDescription& BoxStyle::fontDescription() const
{
    return m_font->description();
}

void BoxStyle::setFontDescription(const FontDescription& description)
{
    if(m_font && description == m_font->description())
        return;
    m_font = document()->createFont(description);
}

float BoxStyle::fontSize() const
{
    return m_font->size();
}

float BoxStyle::fontWeight() const
{
    return m_font->weight();
}

float BoxStyle::fontStretch() const
{
    return m_font->stretch();
}

float BoxStyle::fontStyle() const
{
    return m_font->style();
}

const FontFamilyList& BoxStyle::fontFamily() const
{
    return m_font->family();
}

const FontVariationList& BoxStyle::fontVariationSettings() const
{
    return m_font->variationSettings();
}

Length BoxStyle::left() const
{
    auto value = get(CSSPropertyID::Left);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::right() const
{
    auto value = get(CSSPropertyID::Right);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::top() const
{
    auto value = get(CSSPropertyID::Top);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::bottom() const
{
    auto value = get(CSSPropertyID::Bottom);
    if(value == nullptr)
        return Length::Auto;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::width() const
{
    auto value = get(CSSPropertyID::Width);
    if(value == nullptr)
        return Length::Auto;
    return convertWidthOrHeightLength(*value);
}

Length BoxStyle::height() const
{
    auto value = get(CSSPropertyID::Height);
    if(value == nullptr)
        return Length::Auto;
    return convertWidthOrHeightLength(*value);
}

Length BoxStyle::minWidth() const
{
    auto value = get(CSSPropertyID::MinWidth);
    if(value == nullptr)
        return Length::Auto;
    return convertWidthOrHeightLength(*value);
}

Length BoxStyle::minHeight() const
{
    auto value = get(CSSPropertyID::MinHeight);
    if(value == nullptr)
        return Length::Auto;
    return convertWidthOrHeightLength(*value);
}

Length BoxStyle::maxWidth() const
{
    auto value = get(CSSPropertyID::MaxWidth);
    if(value == nullptr)
        return Length::None;
    return convertWidthOrHeightLength(*value);
}

Length BoxStyle::maxHeight() const
{
    auto value = get(CSSPropertyID::MaxHeight);
    if(value == nullptr)
        return Length::None;
    return convertWidthOrHeightLength(*value);
}

Length BoxStyle::marginLeft() const
{
    auto value = get(CSSPropertyID::MarginLeft);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::marginRight() const
{
    auto value = get(CSSPropertyID::MarginRight);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::marginTop() const
{
    auto value = get(CSSPropertyID::MarginTop);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::marginBottom() const
{
    auto value = get(CSSPropertyID::MarginBottom);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercentOrAuto(*value);
}

Length BoxStyle::paddingLeft() const
{
    auto value = get(CSSPropertyID::PaddingLeft);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercent(*value);
}

Length BoxStyle::paddingRight() const
{
    auto value = get(CSSPropertyID::PaddingRight);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercent(*value);
}

Length BoxStyle::paddingTop() const
{
    auto value = get(CSSPropertyID::PaddingTop);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercent(*value);
}

Length BoxStyle::paddingBottom() const
{
    auto value = get(CSSPropertyID::PaddingBottom);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercent(*value);
}

LineStyle BoxStyle::borderLeftStyle() const
{
    auto value = get(CSSPropertyID::BorderLeftStyle);
    if(value == nullptr)
        return LineStyle::None;
    return convertLineStyle(*value);
}

LineStyle BoxStyle::borderRightStyle() const
{
    auto value = get(CSSPropertyID::BorderRightStyle);
    if(value == nullptr)
        return LineStyle::None;
    return convertLineStyle(*value);
}

LineStyle BoxStyle::borderTopStyle() const
{
    auto value = get(CSSPropertyID::BorderTopStyle);
    if(value == nullptr)
        return LineStyle::None;
    return convertLineStyle(*value);
}

LineStyle BoxStyle::borderBottomStyle() const
{
    auto value = get(CSSPropertyID::BorderBottomStyle);
    if(value == nullptr)
        return LineStyle::None;
    return convertLineStyle(*value);
}

Color BoxStyle::borderLeftColor() const
{
    auto value = get(CSSPropertyID::BorderLeftColor);
    if(value == nullptr)
        return m_color;
    return convertColor(*value);
}

Color BoxStyle::borderRightColor() const
{
    auto value = get(CSSPropertyID::BorderRightColor);
    if(value == nullptr)
        return m_color;
    return convertColor(*value);
}

Color BoxStyle::borderTopColor() const
{
    auto value = get(CSSPropertyID::BorderTopColor);
    if(value == nullptr)
        return m_color;
    return convertColor(*value);
}

Color BoxStyle::borderBottomColor() const
{
    auto value = get(CSSPropertyID::BorderBottomColor);
    if(value == nullptr)
        return m_color;
    return convertColor(*value);
}

float BoxStyle::borderLeftWidth() const
{
    auto value = get(CSSPropertyID::BorderLeftWidth);
    if(value == nullptr)
        return 3.0;
    return convertLineWidth(*value);
}

float BoxStyle::borderRightWidth() const
{
    auto value = get(CSSPropertyID::BorderRightWidth);
    if(value == nullptr)
        return 3.0;
    return convertLineWidth(*value);
}

float BoxStyle::borderTopWidth() const
{
    auto value = get(CSSPropertyID::BorderTopWidth);
    if(value == nullptr)
        return 3.0;
    return convertLineWidth(*value);
}

float BoxStyle::borderBottomWidth() const
{
    auto value = get(CSSPropertyID::BorderBottomWidth);
    if(value == nullptr)
        return 3.0;
    return convertLineWidth(*value);
}

void BoxStyle::getBorderEdgeInfo(BorderEdge edges[], bool includeLeftEdge, bool includeRightEdge) const
{
    edges[BoxSideTop] = BorderEdge(borderTopWidth(), borderTopColor(), borderTopStyle());
    if(includeRightEdge) {
        edges[BoxSideRight] = BorderEdge(borderRightWidth(), borderRightColor(), borderRightStyle());
    }

    edges[BoxSideBottom] = BorderEdge(borderBottomWidth(), borderBottomColor(), borderBottomStyle());
    if(includeLeftEdge) {
        edges[BoxSideLeft] = BorderEdge(borderLeftWidth(), borderLeftColor(), borderLeftStyle());
    }
}

LengthSize BoxStyle::borderTopLeftRadius() const
{
    auto value = get(CSSPropertyID::BorderTopLeftRadius);
    if(value == nullptr)
        return LengthSize(Length::ZeroFixed);
    return convertBorderRadius(*value);
}

LengthSize BoxStyle::borderTopRightRadius() const
{
    auto value = get(CSSPropertyID::BorderTopRightRadius);
    if(value == nullptr)
        return LengthSize(Length::ZeroFixed);
    return convertBorderRadius(*value);
}

LengthSize BoxStyle::borderBottomLeftRadius() const
{
    auto value = get(CSSPropertyID::BorderBottomLeftRadius);
    if(value == nullptr)
        return LengthSize(Length::ZeroFixed);
    return convertBorderRadius(*value);
}

LengthSize BoxStyle::borderBottomRightRadius() const
{
    auto value = get(CSSPropertyID::BorderBottomRightRadius);
    if(value == nullptr)
        return LengthSize(Length::ZeroFixed);
    return convertBorderRadius(*value);
}

RoundedRect BoxStyle::getBorderRoundedRect(const Rect& borderRect, bool includeLeftEdge, bool includeRightEdge) const
{
    auto calc = [&borderRect](const LengthSize& size) {
        return Size(size.width().calc(borderRect.w), size.height().calc(borderRect.h));
    };

    RectRadii borderRadii;
    if(includeLeftEdge) {
        borderRadii.tl = calc(borderTopLeftRadius());
        borderRadii.bl = calc(borderBottomLeftRadius());
    }

    if(includeRightEdge) {
        borderRadii.tr = calc(borderTopRightRadius());
        borderRadii.br = calc(borderBottomRightRadius());
    }

    borderRadii.constrain(borderRect.w, borderRect.h);
    return RoundedRect(borderRect, borderRadii);
}

ListStylePosition BoxStyle::listStylePosition() const
{
    auto value = get(CSSPropertyID::ListStylePosition);
    if(value == nullptr)
        return ListStylePosition::Outside;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Inside:
        return ListStylePosition::Inside;
    case CSSValueID::Outside:
        return ListStylePosition::Outside;
    default:
        assert(false);
    }

    return ListStylePosition::Outside;
}

RefPtr<Image> BoxStyle::listStyleImage() const
{
    auto value = get(CSSPropertyID::ListStyleImage);
    if(value == nullptr)
        return nullptr;
    return convertImageOrNone(*value);
}

RefPtr<Image> BoxStyle::backgroundImage() const
{
    auto value = get(CSSPropertyID::BackgroundImage);
    if(value == nullptr)
        return nullptr;
    return convertImageOrNone(*value);
}

Color BoxStyle::backgroundColor() const
{
    auto value = get(CSSPropertyID::BackgroundColor);
    if(value == nullptr)
        return Color::Transparent;
    return convertColor(*value);
}

BackgroundRepeat BoxStyle::backgroundRepeat() const
{
    auto value = get(CSSPropertyID::BackgroundRepeat);
    if(value == nullptr)
        return BackgroundRepeat::Repeat;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Repeat:
        return BackgroundRepeat::Repeat;
    case CSSValueID::RepeatX:
        return BackgroundRepeat::RepeatX;
    case CSSValueID::RepeatY:
        return BackgroundRepeat::RepeatY;
    case CSSValueID::NoRepeat:
        return BackgroundRepeat::NoRepeat;
    default:
        assert(false);
    }

    return BackgroundRepeat::Repeat;
}

BackgroundBox BoxStyle::backgroundOrigin() const
{
    auto value = get(CSSPropertyID::BackgroundOrigin);
    if(value == nullptr)
        return BackgroundBox::PaddingBox;
    return convertBackgroundBox(*value);
}

BackgroundBox BoxStyle::backgroundClip() const
{
    auto value = get(CSSPropertyID::BackgroundClip);
    if(value == nullptr)
        return BackgroundBox::BorderBox;
    return convertBackgroundBox(*value);
}

BackgroundAttachment BoxStyle::backgroundAttachment() const
{
    auto value = get(CSSPropertyID::BackgroundAttachment);
    if(value == nullptr)
        return BackgroundAttachment::Scroll;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Scroll:
        return BackgroundAttachment::Scroll;
    case CSSValueID::Fixed:
        return BackgroundAttachment::Fixed;
    case CSSValueID::Local:
        return BackgroundAttachment::Local;
    default:
        assert(false);
    }

    return BackgroundAttachment::Scroll;
}

BackgroundSize BoxStyle::backgroundSize() const
{
    auto value = get(CSSPropertyID::BackgroundSize);
    if(value == nullptr)
        return BackgroundSize();
    if(auto ident = to<CSSIdentValue>(value)) {
        switch(ident->value()) {
        case CSSValueID::Contain:
            return BackgroundSize(BackgroundSize::Type::Contain);
        case CSSValueID::Cover:
            return BackgroundSize(BackgroundSize::Type::Cover);
        default:
            assert(false);
        }
    }

    const auto& pair = to<CSSPairValue>(*value);
    auto width = convertLengthOrPercentOrAuto(*pair.first());
    auto height = convertLengthOrPercentOrAuto(*pair.second());
    return BackgroundSize(width, height);
}

LengthPoint BoxStyle::backgroundPosition() const
{
    auto value = get(CSSPropertyID::BackgroundPosition);
    if(value == nullptr)
        return LengthPoint(Length::ZeroFixed);
    return convertPositionCoordinate(*value);
}

ObjectFit BoxStyle::objectFit() const
{
    auto value = get(CSSPropertyID::ObjectFit);
    if(value == nullptr)
        return ObjectFit::Fill;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Fill:
        return ObjectFit::Fill;
    case CSSValueID::Contain:
        return ObjectFit::Contain;
    case CSSValueID::Cover:
        return ObjectFit::Cover;
    case CSSValueID::None:
        return ObjectFit::None;
    case CSSValueID::ScaleDown:
        return ObjectFit::ScaleDown;
    default:
        assert(false);
    }

    return ObjectFit::Fill;
}

LengthPoint BoxStyle::objectPosition() const
{
    auto value = get(CSSPropertyID::ObjectPosition);
    if(value == nullptr)
        return LengthPoint(Length(Length::Type::Percent, 50.f));
    return convertPositionCoordinate(*value);
}

TableLayout BoxStyle::tableLayout() const
{
    auto value = get(CSSPropertyID::TableLayout);
    if(value == nullptr)
        return TableLayout::Auto;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Auto:
        return TableLayout::Auto;
    case CSSValueID::Fixed:
        return TableLayout::Fixed;
    default:
        assert(false);
    }

    return TableLayout::Auto;
}

float BoxStyle::borderHorizontalSpacing() const
{
    auto value = get(CSSPropertyID::BorderHorizontalSpacing);
    if(value == nullptr)
        return 0.0;
    return convertLengthValue(*value);
}

float BoxStyle::borderVerticalSpacing() const
{
    auto value = get(CSSPropertyID::BorderVerticalSpacing);
    if(value == nullptr)
        return 0.0;
    return convertLengthValue(*value);
}

TextAnchor BoxStyle::textAnchor() const
{
    auto value = get(CSSPropertyID::TextAnchor);
    if(value == nullptr)
        return TextAnchor::Start;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Start:
        return TextAnchor::Start;
    case CSSValueID::Middle:
        return TextAnchor::Middle;
    case CSSValueID::End:
        return TextAnchor::End;
    default:
        assert(false);
    }

    return TextAnchor::Start;
}

TextTransform BoxStyle::textTransform() const
{
    auto value = get(CSSPropertyID::TextTransform);
    if(value == nullptr)
        return TextTransform::None;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::None:
        return TextTransform::None;
    case CSSValueID::Capitalize:
        return TextTransform::Capitalize;
    case CSSValueID::Uppercase:
        return TextTransform::Uppercase;
    case CSSValueID::Lowercase:
        return TextTransform::Lowercase;
    default:
        assert(false);
    }

    return TextTransform::None;
}

TextOverflow BoxStyle::textOverflow() const
{
    auto value = get(CSSPropertyID::TextOverflow);
    if(value == nullptr)
        return TextOverflow::Clip;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Clip:
        return TextOverflow::Clip;
    case CSSValueID::Ellipsis:
        return TextOverflow::Ellipsis;
    default:
        assert(false);
    }

    return TextOverflow::Clip;
}

constexpr TextDecorationLine operator|(TextDecorationLine a, TextDecorationLine b)
{
    return static_cast<TextDecorationLine>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr TextDecorationLine& operator|=(TextDecorationLine& a, TextDecorationLine b)
{
    return a = a | b;
}

TextDecorationLine BoxStyle::textDecorationLine() const
{
    auto value = get(CSSPropertyID::TextDecorationLine);
    if(value == nullptr || value->id() == CSSValueID::None)
        return TextDecorationLine::None;
    TextDecorationLine decorations = TextDecorationLine::None;
    for(const auto& decoration : to<CSSListValue>(*value)) {
        const auto& ident = to<CSSIdentValue>(*decoration);
        switch(ident.value()) {
        case CSSValueID::Underline:
            decorations |= TextDecorationLine::Underline;
            break;
        case CSSValueID::Overline:
            decorations |= TextDecorationLine::Overline;
            break;
        case CSSValueID::LineThrough:
            decorations |= TextDecorationLine::LineThrough;
            break;
        default:
            assert(false);
        }
    }

    return decorations;
}

TextDecorationStyle BoxStyle::textDecorationStyle() const
{
    auto value = get(CSSPropertyID::TextDecorationStyle);
    if(value == nullptr)
        return TextDecorationStyle::Solid;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Solid:
        return TextDecorationStyle::Solid;
    case CSSValueID::Double:
        return TextDecorationStyle::Double;
    case CSSValueID::Dotted:
        return TextDecorationStyle::Dotted;
    case CSSValueID::Dashed:
        return TextDecorationStyle::Dashed;
    case CSSValueID::Wavy:
        return TextDecorationStyle::Wavy;
    default:
        assert(false);
    }

    return TextDecorationStyle::Solid;
}

Color BoxStyle::textDecorationColor() const
{
    auto value = get(CSSPropertyID::TextDecorationColor);
    if(value == nullptr)
        return m_color;
    return convertColor(*value);
}

FontVariantEmoji BoxStyle::fontVariantEmoji() const
{
    auto value = get(CSSPropertyID::FontVariantEmoji);
    if(value == nullptr)
        return FontVariantEmoji::Normal;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Normal:
        return FontVariantEmoji::Normal;
    case CSSValueID::Unicode:
        return FontVariantEmoji::Unicode;
    case CSSValueID::Emoji:
        return FontVariantEmoji::Emoji;
    case CSSValueID::Text:
        return FontVariantEmoji::Text;
    default:
        assert(false);
    }

    return FontVariantEmoji::Normal;
}

Hyphens BoxStyle::hyphens() const
{
    auto value = get(CSSPropertyID::Hyphens);
    if(value == nullptr)
        return Hyphens::Manual;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::None:
        return Hyphens::None;
    case CSSValueID::Auto:
        return Hyphens::Auto;
    case CSSValueID::Manual:
        return Hyphens::Manual;
    default:
        assert(false);
    }

    return Hyphens::Manual;
}

Length BoxStyle::textIndent() const
{
    auto value = get(CSSPropertyID::TextIndent);
    if(value == nullptr)
        return Length::ZeroFixed;
    return convertLengthOrPercent(*value);
}

float BoxStyle::letterSpacing() const
{
    auto value = get(CSSPropertyID::LetterSpacing);
    if(value == nullptr)
        return 0.f;
    return convertSpacing(*value);
}

float BoxStyle::wordSpacing() const
{
    auto value = get(CSSPropertyID::WordSpacing);
    if(value == nullptr)
        return 0.f;
    return convertSpacing(*value);
}

float BoxStyle::lineHeight() const
{
    auto value = get(CSSPropertyID::LineHeight);
    if(value == nullptr || value->id() == CSSValueID::Normal)
        return fontLineSpacing();
    if(auto percent = to<CSSPercentValue>(value))
        return percent->value() * fontSize() / 100.f;
    const auto& length = to<CSSLengthValue>(*value);
    if(length.units() == CSSLengthUnits::None)
        return length.value() * fontSize();
    return convertLengthValue(length);
}

float BoxStyle::tabWidth(float spaceWidth) const
{
    auto value = get(CSSPropertyID::TabSize);
    if(value == nullptr)
        return 8 * spaceWidth;
    const auto& length = to<CSSLengthValue>(*value);
    if(length.units() == CSSLengthUnits::None)
        return spaceWidth * length.value();
    return convertLengthValue(length);
}

Overflow BoxStyle::overflow() const
{
    auto value = get(CSSPropertyID::Overflow);
    if(value == nullptr) {
        if(m_node->isSVGElement())
            return Overflow::Hidden;
        return Overflow::Visible;
    }

    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Auto:
        return Overflow::Auto;
    case CSSValueID::Visible:
        return Overflow::Visible;
    case CSSValueID::Hidden:
        return Overflow::Hidden;
    case CSSValueID::Scroll:
        return Overflow::Scroll;
    default:
        assert(false);
    }

    return Overflow::Visible;
}

std::optional<int> BoxStyle::zIndex() const
{
    auto value = get(CSSPropertyID::ZIndex);
    if(value == nullptr)
        return std::nullopt;
    return convertIntegerOrAuto(*value);
}

VerticalAlign BoxStyle::verticalAlign() const
{
    if(m_verticalAlignType != VerticalAlignType::Length)
        return VerticalAlign(m_verticalAlignType);
    auto value = get(CSSPropertyID::VerticalAlign);
    return VerticalAlign(m_verticalAlignType, convertLengthOrPercent(*value));
}

LengthBox BoxStyle::clip() const
{
    auto value = get(CSSPropertyID::Clip);
    if(value == nullptr || value->id() == CSSValueID::Auto)
        return LengthBox(Length::Auto);
    const auto& rect = to<CSSRectValue>(*value);
    auto left = convertLengthOrPercentOrAuto(*rect.left());
    auto right = convertLengthOrPercentOrAuto(*rect.right());
    auto top = convertLengthOrPercentOrAuto(*rect.top());
    auto bottom = convertLengthOrPercentOrAuto(*rect.bottom());
    return LengthBox(left, right, top, bottom);
}

Length BoxStyle::flexBasis() const
{
    auto value = get(CSSPropertyID::FlexBasis);
    if(value == nullptr)
        return Length::Auto;
    return convertWidthOrHeightLength(*value);
}

float BoxStyle::flexGrow() const
{
    auto value = get(CSSPropertyID::FlexGrow);
    if(value == nullptr)
        return 0.0;
    return convertNumber(*value);
}

float BoxStyle::flexShrink() const
{
    auto value = get(CSSPropertyID::FlexShrink);
    if(value == nullptr)
        return 1.0;
    return convertNumber(*value);
}

int BoxStyle::order() const
{
    auto value = get(CSSPropertyID::Order);
    if(value == nullptr)
        return 0;
    return convertInteger(*value);
}

FlexDirection BoxStyle::flexDirection() const
{
    auto value = get(CSSPropertyID::FlexDirection);
    if(value == nullptr)
        return FlexDirection::Row;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Row:
        return FlexDirection::Row;
    case CSSValueID::RowReverse:
        return FlexDirection::RowReverse;
    case CSSValueID::Column:
        return FlexDirection::Column;
    case CSSValueID::ColumnReverse:
        return FlexDirection::ColumnReverse;
    default:
        assert(false);
    }

    return FlexDirection::Row;
}

FlexWrap BoxStyle::flexWrap() const
{
    auto value = get(CSSPropertyID::FlexWrap);
    if(value == nullptr)
        return FlexWrap::Nowrap;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Nowrap:
        return FlexWrap::Nowrap;
    case CSSValueID::Wrap:
        return FlexWrap::Wrap;
    case CSSValueID::WrapReverse:
        return FlexWrap::WrapReverse;
    default:
        assert(false);
    }

    return FlexWrap::Nowrap;
}

AlignContent BoxStyle::justifyContent() const
{
    auto value = get(CSSPropertyID::JustifyContent);
    if(value == nullptr)
        return AlignContent::FlexStart;
    return convertAlignContent(*value);
}

AlignContent BoxStyle::alignContent() const
{
    auto value = get(CSSPropertyID::AlignContent);
    if(value == nullptr)
        return AlignContent::Stretch;
    return convertAlignContent(*value);
}

AlignItem BoxStyle::alignItems() const
{
    auto value = get(CSSPropertyID::AlignItems);
    if(value == nullptr)
        return AlignItem::Stretch;
    return convertAlignItem(*value);
}

AlignItem BoxStyle::alignSelf() const
{
    auto value = get(CSSPropertyID::AlignSelf);
    if(value == nullptr)
        return AlignItem::Auto;
    return convertAlignItem(*value);
}

float BoxStyle::outlineOffset() const
{
    auto value = get(CSSPropertyID::OutlineOffset);
    if(value == nullptr)
        return 0.0;
    return convertLengthValue(*value);
}

Color BoxStyle::outlineColor() const
{
    auto value = get(CSSPropertyID::OutlineColor);
    if(value == nullptr)
        return m_color;
    return convertColor(*value);
}

float BoxStyle::outlineWidth() const
{
    auto value = get(CSSPropertyID::OutlineWidth);
    if(value == nullptr)
        return 3.0;
    return convertLineWidth(*value);
}

LineStyle BoxStyle::outlineStyle() const
{
    auto value = get(CSSPropertyID::OutlineStyle);
    if(value == nullptr)
        return LineStyle::None;
    return convertLineStyle(*value);
}

BorderEdge BoxStyle::getOutlineEdge() const
{
    return BorderEdge(outlineWidth(), outlineColor(), outlineStyle());
}

int BoxStyle::widows() const
{
    auto value = get(CSSPropertyID::Widows);
    if(value == nullptr)
        return 2;
    return convertInteger(*value);
}

int BoxStyle::orphans() const
{
    auto value = get(CSSPropertyID::Orphans);
    if(value == nullptr)
        return 2;
    return convertInteger(*value);
}

Color BoxStyle::columnRuleColor() const
{
    auto value = get(CSSPropertyID::ColumnRuleColor);
    if(value == nullptr)
        return m_color;
    return convertColor(*value);
}

LineStyle BoxStyle::columnRuleStyle() const
{
    auto value = get(CSSPropertyID::ColumnRuleStyle);
    if(value == nullptr)
        return LineStyle::None;
    return convertLineStyle(*value);
}

float BoxStyle::columnRuleWidth() const
{
    auto value = get(CSSPropertyID::ColumnRuleWidth);
    if(value == nullptr)
        return 3.0;
    return convertLineWidth(*value);
}

ColumnSpan BoxStyle::columnSpan() const
{
    auto value = get(CSSPropertyID::ColumnSpan);
    if(value == nullptr)
        return ColumnSpan::None;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::None:
        return ColumnSpan::None;
    case CSSValueID::All:
        return ColumnSpan::All;
    default:
        assert(false);
    }

    return ColumnSpan::None;
}

ColumnFill BoxStyle::columnFill() const
{
    auto value = get(CSSPropertyID::ColumnFill);
    if(value == nullptr)
        return ColumnFill::Balance;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Auto:
        return ColumnFill::Auto;
    case CSSValueID::Balance:
        return ColumnFill::Balance;
    default:
        assert(false);
    }

    return ColumnFill::Balance;
}

std::optional<float> BoxStyle::rowGap() const
{
    auto value = get(CSSPropertyID::RowGap);
    if(value == nullptr)
        return std::nullopt;
    return convertLengthOrNormal(*value);
}

std::optional<float> BoxStyle::columnGap() const
{
    auto value = get(CSSPropertyID::ColumnGap);
    if(value == nullptr)
        return std::nullopt;
    return convertLengthOrNormal(*value);
}

std::optional<float> BoxStyle::columnWidth() const
{
    auto value = get(CSSPropertyID::ColumnWidth);
    if(value == nullptr)
        return std::nullopt;
    return convertLengthOrAuto(*value);
}

std::optional<int> BoxStyle::columnCount() const
{
    auto value = get(CSSPropertyID::ColumnCount);
    if(value == nullptr)
        return std::nullopt;
    return convertIntegerOrAuto(*value);
}

std::optional<float> BoxStyle::pageScale() const
{
    auto value = get(CSSPropertyID::PageScale);
    if(value == nullptr || value->id() == CSSValueID::Auto)
        return std::nullopt;
    return convertNumberOrPercent(*value);
}

GlobalString BoxStyle::page() const
{
    auto value = get(CSSPropertyID::Page);
    if(value == nullptr || value->id() == CSSValueID::Auto)
        return emptyGlo;
    return convertCustomIdent(*value);
}

PageSize BoxStyle::getPageSize(const PageSize& deviceSize) const
{
    auto value = get(CSSPropertyID::Size);
    if(value == nullptr)
        return deviceSize;
    if(auto ident = to<CSSIdentValue>(value)) {
        switch(ident->value()) {
        case CSSValueID::Auto:
            return deviceSize;
        case CSSValueID::Portrait:
            return deviceSize.portrait();
        case CSSValueID::Landscape:
            return deviceSize.landscape();
        default:
            return convertPageSize(*value);
        }
    }

    const auto& pair = to<CSSPairValue>(*value);
    if(auto size = to<CSSIdentValue>(pair.first())) {
        const auto& orientation = to<CSSIdentValue>(*pair.second());
        auto pageSize = convertPageSize(*size);
        switch(orientation.value()) {
        case CSSValueID::Portrait:
            return pageSize.portrait();
        case CSSValueID::Landscape:
            return pageSize.landscape();
        default:
            assert(false);
        }
    }

    auto width = convertLengthValue(*pair.first());
    auto height = convertLengthValue(*pair.second());
    return PageSize(width * units::px, height * units::px);
}

Paint BoxStyle::fill() const
{
    auto value = get(CSSPropertyID::Fill);
    if(value == nullptr)
        return Paint(Color::Black);
    return convertPaint(*value);
}

Paint BoxStyle::stroke() const
{
    auto value = get(CSSPropertyID::Stroke);
    if(value == nullptr)
        return Paint();
    return convertPaint(*value);
}

Color BoxStyle::stopColor() const
{
    auto value = get(CSSPropertyID::StopColor);
    if(value == nullptr)
        return Color::Black;
    return convertColor(*value);
}

float BoxStyle::opacity() const
{
    auto value = get(CSSPropertyID::Opacity);
    if(value == nullptr)
        return 1.f;
    return convertNumberOrPercent(*value);
}

float BoxStyle::stopOpacity() const
{
    auto value = get(CSSPropertyID::StopOpacity);
    if(value == nullptr)
        return 1.f;
    return convertNumberOrPercent(*value);
}

float BoxStyle::fillOpacity() const
{
    auto value = get(CSSPropertyID::FillOpacity);
    if(value == nullptr)
        return 1.f;
    return convertNumberOrPercent(*value);
}

float BoxStyle::strokeOpacity() const
{
    auto value = get(CSSPropertyID::StrokeOpacity);
    if(value == nullptr)
        return 1.f;
    return convertNumberOrPercent(*value);
}

float BoxStyle::strokeMiterlimit() const
{
    auto value = get(CSSPropertyID::StrokeMiterlimit);
    if(value == nullptr)
        return 4.f;
    return convertNumber(*value);
}

Length BoxStyle::strokeWidth() const
{
    auto value = get(CSSPropertyID::StrokeWidth);
    if(value == nullptr)
        return Length(1.f);
    return convertLengthOrPercent(*value);
}

Length BoxStyle::strokeDashoffset() const
{
    auto value = get(CSSPropertyID::StrokeDashoffset);
    if(value == nullptr)
        return Length(0.f);
    return convertLengthOrPercent(*value);
}

LengthList BoxStyle::strokeDasharray() const
{
    auto value = get(CSSPropertyID::StrokeDasharray);
    if(value == nullptr || value->id() == CSSValueID::None)
        return LengthList();

    LengthList dashes;
    for(const auto& dash : to<CSSListValue>(*value)) {
        dashes.push_back(convertLengthOrPercent(*dash));
    }

    return dashes;
}

LineCap BoxStyle::strokeLinecap() const
{
    auto value = get(CSSPropertyID::StrokeLinecap);
    if(value == nullptr)
        return LineCap::Butt;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Butt:
        return LineCap::Butt;
    case CSSValueID::Round:
        return LineCap::Round;
    case CSSValueID::Square:
        return LineCap::Square;
    default:
        assert(false);
    }

    return LineCap::Butt;
}

LineJoin BoxStyle::strokeLinejoin() const
{
    auto value = get(CSSPropertyID::StrokeLinejoin);
    if(value == nullptr)
        return LineJoin::Miter;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Miter:
        return LineJoin::Miter;
    case CSSValueID::Round:
        return LineJoin::Round;
    case CSSValueID::Bevel:
        return LineJoin::Bevel;
    default:
        assert(false);
    }

    return LineJoin::Miter;
}

HeapString BoxStyle::mask() const
{
    auto value = get(CSSPropertyID::Mask);
    if(value == nullptr)
        return emptyGlo;
    return convertLocalUrlOrNone(*value);
}

HeapString BoxStyle::clipPath() const
{
    auto value = get(CSSPropertyID::ClipPath);
    if(value == nullptr)
        return emptyGlo;
    return convertLocalUrlOrNone(*value);
}

HeapString BoxStyle::markerStart() const
{
    auto value = get(CSSPropertyID::MarkerStart);
    if(value == nullptr)
        return emptyGlo;
    return convertLocalUrlOrNone(*value);
}

HeapString BoxStyle::markerMid() const
{
    auto value = get(CSSPropertyID::MarkerMid);
    if(value == nullptr)
        return emptyGlo;
    return convertLocalUrlOrNone(*value);
}

HeapString BoxStyle::markerEnd() const
{
    auto value = get(CSSPropertyID::MarkerEnd);
    if(value == nullptr)
        return emptyGlo;
    return convertLocalUrlOrNone(*value);
}

AlignmentBaseline BoxStyle::alignmentBaseline() const
{
    auto value = get(CSSPropertyID::AlignmentBaseline);
    if(value == nullptr)
        return AlignmentBaseline::Baseline;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Auto:
        return AlignmentBaseline::Auto;
    case CSSValueID::Baseline:
        return AlignmentBaseline::Baseline;
    case CSSValueID::BeforeEdge:
        return AlignmentBaseline::BeforeEdge;
    case CSSValueID::TextBeforeEdge:
        return AlignmentBaseline::TextBeforeEdge;
    case CSSValueID::Middle:
        return AlignmentBaseline::Middle;
    case CSSValueID::Central:
        return AlignmentBaseline::Central;
    case CSSValueID::AfterEdge:
        return AlignmentBaseline::AfterEdge;
    case CSSValueID::TextAfterEdge:
        return AlignmentBaseline::TextAfterEdge;
    case CSSValueID::Ideographic:
        return AlignmentBaseline::Ideographic;
    case CSSValueID::Alphabetic:
        return AlignmentBaseline::Alphabetic;
    case CSSValueID::Hanging:
        return AlignmentBaseline::Hanging;
    case CSSValueID::Mathematical:
        return AlignmentBaseline::Mathematical;
    default:
        assert(false);
    }

    return AlignmentBaseline::Auto;
}

DominantBaseline BoxStyle::dominantBaseline() const
{
    auto value = get(CSSPropertyID::DominantBaseline);
    if(value == nullptr)
        return DominantBaseline::Auto;
    const auto& ident = to<CSSIdentValue>(*value);
    switch(ident.value()) {
    case CSSValueID::Auto:
        return DominantBaseline::Auto;
    case CSSValueID::UseScript:
        return DominantBaseline::UseScript;
    case CSSValueID::NoChange:
        return DominantBaseline::NoChange;
    case CSSValueID::ResetSize:
        return DominantBaseline::ResetSize;
    case CSSValueID::Ideographic:
        return DominantBaseline::Ideographic;
    case CSSValueID::Alphabetic:
        return DominantBaseline::Alphabetic;
    case CSSValueID::Hanging:
        return DominantBaseline::Hanging;
    case CSSValueID::Mathematical:
        return DominantBaseline::Mathematical;
    case CSSValueID::Central:
        return DominantBaseline::Central;
    case CSSValueID::Middle:
        return DominantBaseline::Middle;
    case CSSValueID::TextAfterEdge:
        return DominantBaseline::TextAfterEdge;
    case CSSValueID::TextBeforeEdge:
        return DominantBaseline::TextBeforeEdge;
    default:
        assert(false);
    }

    return DominantBaseline::Auto;
}

BaselineShift BoxStyle::baselineShift() const
{
    auto value = get(CSSPropertyID::BaselineShift);
    if(value == nullptr)
        return BaselineShiftType::Baseline;
    if(auto ident = to<CSSIdentValue>(value)) {
        switch(ident->value()) {
        case CSSValueID::Baseline:
            return BaselineShiftType::Baseline;
        case CSSValueID::Sub:
            return BaselineShiftType::Sub;
        case CSSValueID::Super:
            return BaselineShiftType::Super;
        default:
            assert(false);
        }
    }

    return BaselineShift(BaselineShiftType::Length, convertLengthOrPercent(*value));
}

bool BoxStyle::isOriginalDisplayBlockType() const
{
    auto value = get(CSSPropertyID::Display);
    if(value == nullptr)
        return false;
    return isDisplayBlockType(convertDisplay(*value));
}

bool BoxStyle::isOriginalDisplayInlineType() const
{
    auto value = get(CSSPropertyID::Display);
    if(value == nullptr)
        return true;
    return isDisplayInlineType(convertDisplay(*value));
}

Point BoxStyle::getTransformOrigin(float width, float height) const
{
    auto value = get(CSSPropertyID::TransformOrigin);
    if(value == nullptr)
        return Point(width * 50.f / 100.f, height * 50.f / 100.f);
    const auto& coordinate = convertPositionCoordinate(*value);
    return Point(coordinate.x().calc(width), coordinate.y().calc(height));
}

Transform BoxStyle::getTransform(float width, float height) const
{
    auto value = get(CSSPropertyID::Transform);
    if(value == nullptr || value->id() == CSSValueID::None)
        return Transform();
    auto origin = getTransformOrigin(width, height);
    auto transform = Transform::makeTranslate(origin.x, origin.y);
    for(const auto& operation : to<CSSListValue>(*value)) {
        const auto& function = to<CSSFunctionValue>(*operation);
        switch(function.id()) {
        case CSSFunctionID::Translate: {
            float firstValue = convertLengthOrPercent(width, *function.at(0));
            float secondValue = 0.f;
            if(function.size() == 2)
                secondValue = convertLengthOrPercent(height, *function.at(1));
            transform.translate(firstValue, secondValue);
            break;
        }

        case CSSFunctionID::TranslateX:
            transform.translate(convertLengthOrPercent(width, *function.at(0)), 0.f);
            break;
        case CSSFunctionID::TranslateY:
            transform.translate(0.f, convertLengthOrPercent(height, *function.at(0)));
            break;
        case CSSFunctionID::Scale: {
            float firstValue = convertNumberOrPercent(*function.at(0));
            float secondValue = firstValue;
            if(function.size() == 2)
                secondValue = convertNumberOrPercent(*function.at(1));
            transform.scale(firstValue, secondValue);
            break;
        }

        case CSSFunctionID::ScaleX:
            transform.scale(convertNumberOrPercent(*function.at(0)), 1.f);
            break;
        case CSSFunctionID::ScaleY:
            transform.scale(1.f, convertNumberOrPercent(*function.at(0)));
            break;
        case CSSFunctionID::Skew: {
            float firstValue = convertAngle(*function.at(0));
            float secondValue = 0.f;
            if(function.size() == 2)
                secondValue = convertAngle(*function.at(1));
            transform.shear(firstValue, secondValue);
            break;
        }

        case CSSFunctionID::SkewX:
            transform.shear(convertAngle(*function.at(0)), 0.f);
            break;
        case CSSFunctionID::SkewY:
            transform.shear(0.f, convertAngle(*function.at(0)));
            break;
        case CSSFunctionID::Rotate:
            transform.rotate(convertAngle(*function.at(0)));
            break;
        default:
            assert(function.id() == CSSFunctionID::Matrix && function.size() == 6);
            auto a = convertNumber(*function.at(0));
            auto b = convertNumber(*function.at(1));
            auto c = convertNumber(*function.at(2));
            auto d = convertNumber(*function.at(3));
            auto e = convertNumber(*function.at(4));
            auto f = convertNumber(*function.at(5));
            transform.multiply(Transform(a, b, c, d, e, f));
            break;
        }
    }

    transform.translate(-origin.x, -origin.y);
    return transform;
}

bool BoxStyle::hasTransform() const
{
    auto value = get(CSSPropertyID::Transform);
    return value && value->id() != CSSValueID::None;
}

bool BoxStyle::hasContent() const
{
    auto value = get(CSSPropertyID::Content);
    return value && value->id() != CSSValueID::None;
}

bool BoxStyle::hasLineHeight() const
{
    auto value = get(CSSPropertyID::LineHeight);
    return value && value->id() != CSSValueID::Normal;
}

bool BoxStyle::hasStroke() const
{
    auto value = get(CSSPropertyID::Stroke);
    return value && value->id() != CSSValueID::None;
}

bool BoxStyle::hasBackground() const
{
    return backgroundColor().isVisible() || backgroundImage();
}

bool BoxStyle::hasColumns() const
{
    return columnCount() || columnWidth();
}

const HeapString& BoxStyle::getQuote(bool open, size_t depth) const
{
    static const GlobalString defaultQuote("\"");
    auto value = get(CSSPropertyID::Quotes);
    if(value == nullptr)
        return defaultQuote;
    if(auto ident = to<CSSIdentValue>(value)) {
        switch(ident->value()) {
        case CSSValueID::Auto:
            return defaultQuote;
        case CSSValueID::None:
            return emptyGlo;
        default:
            assert(false);
        }
    }

    const auto& list = to<CSSListValue>(*value);
    const auto& pair = to<CSSPairValue>(*list.at(std::min(depth, list.size() - 1)));
    const auto& quote = open ? pair.first() : pair.second();
    return to<CSSStringValue>(*quote).value();
}

CSSVariableData* BoxStyle::getCustom(const std::string_view& name) const
{
    auto it = m_customProperties.find(name);
    if(it == m_customProperties.end())
        return nullptr;
    return it->second.get();
}

void BoxStyle::setCustom(const GlobalString& name, RefPtr<CSSVariableData> value)
{
    m_customProperties.insert_or_assign(name, std::move(value));
}

void BoxStyle::set(CSSPropertyID id, RefPtr<CSSValue> value)
{
    switch(id) {
    case CSSPropertyID::Display:
        m_display = convertDisplay(*value);
        break;
    case CSSPropertyID::Position:
        m_position = convertPosition(*value);
        break;
    case CSSPropertyID::Float:
        m_floating = convertFloat(*value);
        break;
    case CSSPropertyID::Clear:
        m_clear = convertClear(*value);
        break;
    case CSSPropertyID::VerticalAlign:
        m_verticalAlignType = convertVerticalAlignType(*value);
        break;
    case CSSPropertyID::Direction:
        m_direction = convertDirection(*value);
        break;
    case CSSPropertyID::UnicodeBidi:
        m_unicodeBidi = convertUnicodeBidi(*value);
        break;
    case CSSPropertyID::Visibility:
        m_visibility = convertVisibility(*value);
        break;
    case CSSPropertyID::BoxSizing:
        m_boxSizing = convertBoxSizing(*value);
        break;
    case CSSPropertyID::MixBlendMode:
        m_blendMode = convertBlendMode(*value);
        break;
    case CSSPropertyID::MaskType:
        m_maskType = convertMaskType(*value);
        break;
    case CSSPropertyID::WritingMode:
        m_writingMode = convertWritingMode(*value);
        break;
    case CSSPropertyID::TextOrientation:
        m_textOrientation = convertTextOrientation(*value);
        break;
    case CSSPropertyID::TextAlign:
        m_textAlign = convertTextAlign(*value);
        break;
    case CSSPropertyID::WhiteSpace:
        m_whiteSpace = convertWhiteSpace(*value);
        break;
    case CSSPropertyID::WordBreak:
        m_wordBreak = convertWordBreak(*value);
        break;
    case CSSPropertyID::OverflowWrap:
        m_overflowWrap = convertOverflowWrap(*value);
        break;
    case CSSPropertyID::FillRule:
        m_fillRule = convertFillRule(*value);
        break;
    case CSSPropertyID::ClipRule:
        m_clipRule = convertFillRule(*value);
        break;
    case CSSPropertyID::CaptionSide:
        m_captionSide = convertCaptionSide(*value);
        break;
    case CSSPropertyID::EmptyCells:
        m_emptyCells = convertEmptyCells(*value);
        break;
    case CSSPropertyID::BorderCollapse:
        m_borderCollapse = convertBorderCollapse(*value);
        break;
    case CSSPropertyID::BreakAfter:
    case CSSPropertyID::ColumnBreakAfter:
    case CSSPropertyID::PageBreakAfter:
        m_breakAfter = convertBreakBetween(*value);
        break;
    case CSSPropertyID::BreakBefore:
    case CSSPropertyID::ColumnBreakBefore:
    case CSSPropertyID::PageBreakBefore:
        m_breakBefore = convertBreakBetween(*value);
        break;
    case CSSPropertyID::BreakInside:
    case CSSPropertyID::ColumnBreakInside:
    case CSSPropertyID::PageBreakInside:
        m_breakInside = convertBreakInside(*value);
        break;
    case CSSPropertyID::Color:
        m_color = convertColor(*value);
        break;
    default:
        break;
    }

    m_properties.insert_or_assign(id, std::move(value));
}

void BoxStyle::reset(CSSPropertyID id)
{
    switch(id) {
    case CSSPropertyID::Display:
        m_display = Display::Inline;
        break;
    case CSSPropertyID::Position:
        m_position = Position::Static;
        break;
    case CSSPropertyID::Float:
        m_floating = Float::None;
        break;
    case CSSPropertyID::Clear:
        m_clear = Clear::None;
        break;
    case CSSPropertyID::VerticalAlign:
        m_verticalAlignType = VerticalAlignType::Baseline;
        break;
    case CSSPropertyID::Direction:
        m_direction = Direction::Ltr;
        break;
    case CSSPropertyID::UnicodeBidi:
        m_unicodeBidi = UnicodeBidi::Normal;
        break;
    case CSSPropertyID::Visibility:
        m_visibility = Visibility::Visible;
        break;
    case CSSPropertyID::BoxSizing:
        m_boxSizing = BoxSizing::ContentBox;
        break;
    case CSSPropertyID::MixBlendMode:
        m_blendMode = BlendMode::Normal;
        break;
    case CSSPropertyID::MaskType:
        m_maskType = MaskType::Luminance;
        break;
    case CSSPropertyID::WritingMode:
        m_writingMode = WritingMode::HorizontalTb;
        break;
    case CSSPropertyID::TextOrientation:
        m_textOrientation = TextOrientation::Mixed;
        break;
    case CSSPropertyID::TextAlign:
        m_textAlign = TextAlign::Left;
        break;
    case CSSPropertyID::WhiteSpace:
        m_whiteSpace = WhiteSpace::Normal;
        break;
    case CSSPropertyID::WordBreak:
        m_wordBreak = WordBreak::Normal;
        break;
    case CSSPropertyID::OverflowWrap:
        m_overflowWrap = OverflowWrap::Normal;
        break;
    case CSSPropertyID::FillRule:
        m_fillRule = FillRule::NonZero;
        break;
    case CSSPropertyID::ClipRule:
        m_clipRule = FillRule::NonZero;
        break;
    case CSSPropertyID::CaptionSide:
        m_captionSide = CaptionSide::Top;
        break;
    case CSSPropertyID::EmptyCells:
        m_emptyCells = EmptyCells::Show;
        break;
    case CSSPropertyID::BorderCollapse:
        m_borderCollapse = BorderCollapse::Separate;
        break;
    case CSSPropertyID::BreakAfter:
    case CSSPropertyID::ColumnBreakAfter:
    case CSSPropertyID::PageBreakAfter:
        m_breakBefore = BreakBetween::Auto;
        break;
    case CSSPropertyID::BreakBefore:
    case CSSPropertyID::ColumnBreakBefore:
    case CSSPropertyID::PageBreakBefore:
        m_breakBefore = BreakBetween::Auto;
        break;
    case CSSPropertyID::BreakInside:
    case CSSPropertyID::ColumnBreakInside:
    case CSSPropertyID::PageBreakInside:
        m_breakInside = BreakInside::Auto;
        break;
    case CSSPropertyID::Color:
        m_color = Color::Black;
        break;
    default:
        break;
    }

    m_properties.erase(id);
}

void BoxStyle::inheritFrom(const BoxStyle* parentStyle)
{
    m_font = parentStyle->font();
    m_direction = parentStyle->direction();
    m_visibility = parentStyle->visibility();
    m_writingMode = parentStyle->writingMode();
    m_textOrientation = parentStyle->textOrientation();
    m_textAlign = parentStyle->textAlign();
    m_whiteSpace = parentStyle->whiteSpace();
    m_wordBreak = parentStyle->wordBreak();
    m_overflowWrap = parentStyle->overflowWrap();
    m_fillRule = parentStyle->fillRule();
    m_clipRule = parentStyle->clipRule();
    m_captionSide = parentStyle->captionSide();
    m_emptyCells = parentStyle->emptyCells();
    m_borderCollapse = parentStyle->borderCollapse();
    m_color = parentStyle->color();
    m_customProperties = parentStyle->customProperties();
    for(const auto& [id, value] : parentStyle->properties()) {
        switch(id) {
        case CSSPropertyID::BorderCollapse:
        case CSSPropertyID::CaptionSide:
        case CSSPropertyID::ClipRule:
        case CSSPropertyID::Color:
        case CSSPropertyID::Direction:
        case CSSPropertyID::DominantBaseline:
        case CSSPropertyID::EmptyCells:
        case CSSPropertyID::Fill:
        case CSSPropertyID::FillOpacity:
        case CSSPropertyID::FillRule:
        case CSSPropertyID::FontFamily:
        case CSSPropertyID::FontFeatureSettings:
        case CSSPropertyID::FontKerning:
        case CSSPropertyID::FontSize:
        case CSSPropertyID::FontStretch:
        case CSSPropertyID::FontStyle:
        case CSSPropertyID::FontVariantCaps:
        case CSSPropertyID::FontVariantEmoji:
        case CSSPropertyID::FontVariantEastAsian:
        case CSSPropertyID::FontVariantLigatures:
        case CSSPropertyID::FontVariantNumeric:
        case CSSPropertyID::FontVariantPosition:
        case CSSPropertyID::FontVariationSettings:
        case CSSPropertyID::FontWeight:
        case CSSPropertyID::Hyphens:
        case CSSPropertyID::LetterSpacing:
        case CSSPropertyID::LineHeight:
        case CSSPropertyID::ListStyleImage:
        case CSSPropertyID::ListStylePosition:
        case CSSPropertyID::ListStyleType:
        case CSSPropertyID::MarkerEnd:
        case CSSPropertyID::MarkerMid:
        case CSSPropertyID::MarkerStart:
        case CSSPropertyID::Orphans:
        case CSSPropertyID::OverflowWrap:
        case CSSPropertyID::PaintOrder:
        case CSSPropertyID::Quotes:
        case CSSPropertyID::Stroke:
        case CSSPropertyID::StrokeDasharray:
        case CSSPropertyID::StrokeDashoffset:
        case CSSPropertyID::StrokeLinecap:
        case CSSPropertyID::StrokeLinejoin:
        case CSSPropertyID::StrokeMiterlimit:
        case CSSPropertyID::StrokeOpacity:
        case CSSPropertyID::StrokeWidth:
        case CSSPropertyID::TabSize:
        case CSSPropertyID::TextAlign:
        case CSSPropertyID::TextAnchor:
        case CSSPropertyID::TextDecorationColor:
        case CSSPropertyID::TextDecorationLine:
        case CSSPropertyID::TextDecorationStyle:
        case CSSPropertyID::TextIndent:
        case CSSPropertyID::TextOrientation:
        case CSSPropertyID::TextTransform:
        case CSSPropertyID::Visibility:
        case CSSPropertyID::WhiteSpace:
        case CSSPropertyID::Widows:
        case CSSPropertyID::WordBreak:
        case CSSPropertyID::WordSpacing:
        case CSSPropertyID::WritingMode:
            m_properties.insert_or_assign(id, value);
            break;
        default:
            break;
        }
    }
}

float BoxStyle::exFontSize() const
{
    if(auto fontData = m_font->primaryFont())
        return fontData->xHeight();
    return fontSize() / 2.f;
}

float BoxStyle::chFontSize() const
{
    if(auto fontData = m_font->primaryFont())
        return fontData->zeroWidth();
    return fontSize() / 2.f;
}

float BoxStyle::remFontSize() const
{
    if(auto style = document()->rootStyle())
        return style->fontSize();
    return kMediumFontSize;
}

class FontFeaturesBuilder {
public:
    explicit FontFeaturesBuilder(const CSSPropertyMap& properties);

    FontFeatureList build() const;

private:
    void buildKerning(FontFeatureList& features) const;
    void buildVariantLigatures(FontFeatureList& features) const;
    void buildVariantPosition(FontFeatureList& features) const;
    void buildVariantCaps(FontFeatureList& features) const;
    void buildVariantNumeric(FontFeatureList& features) const;
    void buildVariantEastAsian(FontFeatureList& features) const;
    void buildFeatureSettings(FontFeatureList& features) const;

    RefPtr<CSSValue> m_kerning;
    RefPtr<CSSValue> m_variantLigatures;
    RefPtr<CSSValue> m_variantPosition;
    RefPtr<CSSValue> m_variantCaps;
    RefPtr<CSSValue> m_variantNumeric;
    RefPtr<CSSValue> m_variantEastAsian;
    RefPtr<CSSValue> m_featureSettings;
};

FontFeaturesBuilder::FontFeaturesBuilder(const CSSPropertyMap& properties)
{
    for(const auto& [id, value] : properties) {
        switch(id) {
        case CSSPropertyID::FontKerning:
            m_kerning = value;
            break;
        case CSSPropertyID::FontVariantLigatures:
            m_variantLigatures = value;
            break;
        case CSSPropertyID::FontVariantPosition:
            m_variantPosition = value;
            break;
        case CSSPropertyID::FontVariantCaps:
            m_variantCaps = value;
            break;
        case CSSPropertyID::FontVariantNumeric:
            m_variantNumeric = value;
            break;
        case CSSPropertyID::FontVariantEastAsian:
            m_variantEastAsian = value;
            break;
        case CSSPropertyID::FontFeatureSettings:
            m_featureSettings = value;
            break;
        default:
            break;
        }
    }
}

FontFeatureList FontFeaturesBuilder::build() const
{
    FontFeatureList features;
    buildKerning(features);
    buildVariantLigatures(features);
    buildVariantPosition(features);
    buildVariantCaps(features);
    buildVariantNumeric(features);
    buildVariantEastAsian(features);
    buildFeatureSettings(features);
    return features;
}

void FontFeaturesBuilder::buildKerning(FontFeatureList& features) const
{
    if(m_kerning == nullptr)
        return;
    constexpr FontTag kernTag("kern");
    const auto& ident = to<CSSIdentValue>(*m_kerning);
    switch(ident.id()) {
    case CSSValueID::Auto:
        break;
    case CSSValueID::Normal:
        features.emplace_front(kernTag, 1);
        break;
    case CSSValueID::None:
        features.emplace_front(kernTag, 0);
        break;
    default:
        assert(false);
    }
}

void FontFeaturesBuilder::buildVariantLigatures(FontFeatureList& features) const
{
    if(m_variantLigatures == nullptr)
        return;
    constexpr FontTag ligaTag("liga");
    constexpr FontTag cligTag("clig");
    constexpr FontTag hligTag("hlig");
    constexpr FontTag dligTag("dlig");
    constexpr FontTag caltTag("calt");
    if(auto ident = to<CSSIdentValue>(m_variantLigatures)) {
        if(ident->value() == CSSValueID::Normal)
            return;
        assert(ident->value() == CSSValueID::None);
        features.emplace_front(ligaTag, 0);
        features.emplace_front(cligTag, 0);
        features.emplace_front(hligTag, 0);
        features.emplace_front(dligTag, 0);
        features.emplace_front(caltTag, 0);
        return;
    }

    for(const auto& value : to<CSSListValue>(*m_variantLigatures)) {
        const auto& ident = to<CSSIdentValue>(*value);
        switch(ident.id()) {
        case CSSValueID::CommonLigatures:
            features.emplace_front(ligaTag, 1);
            features.emplace_front(cligTag, 1);
            break;
        case CSSValueID::NoCommonLigatures:
            features.emplace_front(ligaTag, 0);
            features.emplace_front(cligTag, 0);
            break;
        case CSSValueID::HistoricalLigatures:
            features.emplace_front(hligTag, 1);
            break;
        case CSSValueID::NoHistoricalLigatures:
            features.emplace_front(hligTag, 0);
            break;
        case CSSValueID::DiscretionaryLigatures:
            features.emplace_front(dligTag, 1);
            break;
        case CSSValueID::NoDiscretionaryLigatures:
            features.emplace_front(dligTag, 0);
            break;
        case CSSValueID::Contextual:
            features.emplace_front(caltTag, 1);
            break;
        case CSSValueID::NoContextual:
            features.emplace_front(caltTag, 0);
            break;
        default:
            assert(false);
        }
    }
}

void FontFeaturesBuilder::buildVariantPosition(FontFeatureList& features) const
{
    if(m_variantPosition == nullptr)
        return;
    constexpr FontTag subsTag("subs");
    constexpr FontTag supsTag("sups");
    const auto& ident = to<CSSIdentValue>(*m_variantPosition);
    switch(ident.id()) {
    case CSSValueID::Normal:
        break;
    case CSSValueID::Sub:
        features.emplace_front(subsTag, 1);
        break;
    case CSSValueID::Super:
        features.emplace_front(supsTag, 1);
        break;
    default:
        assert(false);
    }
}

void FontFeaturesBuilder::buildVariantCaps(FontFeatureList& features) const
{
    if(m_variantCaps == nullptr)
        return;
    constexpr FontTag smcpTag("smcp");
    constexpr FontTag c2scTag("c2sc");
    constexpr FontTag pcapTag("pcap");
    constexpr FontTag c2pcTag("c2pc");
    constexpr FontTag unicTag("unic");
    constexpr FontTag titlTag("titl");
    const auto& ident = to<CSSIdentValue>(*m_variantCaps);
    switch(ident.id()) {
    case CSSValueID::Normal:
        break;
    case CSSValueID::SmallCaps:
        features.emplace_front(smcpTag, 1);
        break;
    case CSSValueID::AllSmallCaps:
        features.emplace_front(c2scTag, 1);
        features.emplace_front(smcpTag, 1);
        break;
    case CSSValueID::PetiteCaps:
        features.emplace_front(pcapTag, 1);
        break;
    case CSSValueID::AllPetiteCaps:
        features.emplace_front(c2pcTag, 1);
        features.emplace_front(pcapTag, 1);
        break;
    case CSSValueID::Unicase:
        features.emplace_front(unicTag, 1);
        break;
    case CSSValueID::TitlingCaps:
        features.emplace_front(titlTag, 1);
        break;
    default:
        assert(false);
    }
}

void FontFeaturesBuilder::buildVariantNumeric(FontFeatureList& features) const
{
    if(m_variantNumeric == nullptr)
        return;
    if(auto ident = to<CSSIdentValue>(m_variantNumeric)) {
        assert(ident->value() == CSSValueID::Normal);
        return;
    }

    constexpr FontTag lnumTag("lnum");
    constexpr FontTag onumTag("onum");
    constexpr FontTag pnumTag("pnum");
    constexpr FontTag tnumTag("tnum");
    constexpr FontTag fracTag("frac");
    constexpr FontTag afrcTag("afrc");
    constexpr FontTag ordnTag("ordn");
    constexpr FontTag zeroTag("zero");
    for(const auto& value : to<CSSListValue>(*m_variantNumeric)) {
        const auto& ident = to<CSSIdentValue>(*value);
        switch(ident.id()) {
        case CSSValueID::LiningNums:
            features.emplace_front(lnumTag, 1);
            break;
        case CSSValueID::OldstyleNums:
            features.emplace_front(onumTag, 1);
            break;
        case CSSValueID::ProportionalNums:
            features.emplace_front(pnumTag, 1);
            break;
        case CSSValueID::TabularNums:
            features.emplace_front(tnumTag, 1);
            break;
        case CSSValueID::DiagonalFractions:
            features.emplace_front(fracTag, 1);
            break;
        case CSSValueID::StackedFractions:
            features.emplace_front(afrcTag, 1);
            break;
        case CSSValueID::Ordinal:
            features.emplace_front(ordnTag, 1);
            break;
        case CSSValueID::SlashedZero:
            features.emplace_front(zeroTag, 1);
            break;
        default:
            assert(false);
        }
    }
}

void FontFeaturesBuilder::buildVariantEastAsian(FontFeatureList& features) const
{
    if(m_variantEastAsian == nullptr)
        return;
    if(auto ident = to<CSSIdentValue>(m_variantEastAsian)) {
        assert(ident->value() == CSSValueID::Normal);
        return;
    }

    constexpr FontTag jp78Tag("jp78");
    constexpr FontTag jp83Tag("jp83");
    constexpr FontTag jp90Tag("jp90");
    constexpr FontTag jp04Tag("jp04");
    constexpr FontTag smplTag("smpl");
    constexpr FontTag tradTag("trad");
    constexpr FontTag fwidTag("fwid");
    constexpr FontTag pwidTag("pwid");
    constexpr FontTag rubyTag("ruby");
    for(const auto& value : to<CSSListValue>(*m_variantEastAsian)) {
        const auto& ident = to<CSSIdentValue>(*value);
        switch(ident.id()) {
        case CSSValueID::Jis78:
            features.emplace_front(jp78Tag, 1);
            break;
        case CSSValueID::Jis83:
            features.emplace_front(jp83Tag, 1);
            break;
        case CSSValueID::Jis90:
            features.emplace_front(jp90Tag, 1);
            break;
        case CSSValueID::Jis04:
            features.emplace_front(jp04Tag, 1);
            break;
        case CSSValueID::Simplified:
            features.emplace_front(smplTag, 1);
            break;
        case CSSValueID::Traditional:
            features.emplace_front(tradTag, 1);
            break;
        case CSSValueID::FullWidth:
            features.emplace_front(fwidTag, 1);
            break;
        case CSSValueID::ProportionalWidth:
            features.emplace_front(pwidTag, 1);
            break;
        case CSSValueID::Ruby:
            features.emplace_front(rubyTag, 1);
            break;
        default:
            assert(false);
        }
    }
}

void FontFeaturesBuilder::buildFeatureSettings(FontFeatureList& features) const
{
    if(m_featureSettings == nullptr)
        return;
    if(auto ident = to<CSSIdentValue>(m_featureSettings)) {
        assert(ident->value() == CSSValueID::Normal);
        return;
    }

    for(const auto& value : to<CSSListValue>(*m_featureSettings)) {
        const auto& feature = to<CSSFontFeatureValue>(*value);
        features.emplace_front(feature.tag(), feature.value());
    }
}

FontFeatureList BoxStyle::fontFeatures() const
{
    return FontFeaturesBuilder(properties()).build();
}

float BoxStyle::viewportWidth() const
{
    return document()->viewportWidth();
}

float BoxStyle::viewportHeight() const
{
    return document()->viewportHeight();
}

float BoxStyle::viewportMin() const
{
    return std::min(document()->viewportWidth(), document()->viewportHeight());
}

float BoxStyle::viewportMax() const
{
    return std::max(document()->viewportWidth(), document()->viewportHeight());
}

RefPtr<CSSValue> BoxStyle::resolveLength(const RefPtr<CSSValue>& value) const
{
    if(is<CSSLengthValue>(value)) {
        const auto& length = to<CSSLengthValue>(*value);
        switch(length.units()) {
        case CSSLengthUnits::None:
        case CSSLengthUnits::Pixels:
        case CSSLengthUnits::Points:
        case CSSLengthUnits::Picas:
        case CSSLengthUnits::Centimeters:
        case CSSLengthUnits::Millimeters:
        case CSSLengthUnits::Inches:
            return value;
        case CSSLengthUnits::ViewportWidth:
        case CSSLengthUnits::ViewportHeight:
        case CSSLengthUnits::ViewportMin:
        case CSSLengthUnits::ViewportMax:
        case CSSLengthUnits::Ems:
        case CSSLengthUnits::Exs:
        case CSSLengthUnits::Chs:
        case CSSLengthUnits::Rems:
            break;
        }
    }

    return CSSLengthValue::create(heap(), convertLengthValue(*value));
}

float BoxStyle::convertLengthValue(const CSSValue& value) const
{
    return CSSLengthResolver(document(), font()).resolveLength(value);
}

float BoxStyle::convertLineWidth(const CSSValue& value) const
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        switch(ident.value()) {
        case CSSValueID::Thin:
            return 1.0;
        case CSSValueID::Medium:
            return 3.0;
        case CSSValueID::Thick:
            return 5.0;
        default:
            assert(false);
        }
    }

    return convertLengthValue(value);
}

float BoxStyle::convertSpacing(const CSSValue& value) const
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        assert(ident.value() == CSSValueID::Normal);
        return 0.f;
    }

    return convertLengthValue(value);
}

float BoxStyle::convertLengthOrPercent(float maximum, const CSSValue& value) const
{
    if(is<CSSPercentValue>(value)) {
        const auto& percent = to<CSSPercentValue>(value);
        return percent.value() * maximum / 100.f;
    }

    return convertLengthValue(value);
}

std::optional<float> BoxStyle::convertLengthOrAuto(const CSSValue& value) const
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        assert(ident.value() == CSSValueID::Auto);
        return std::nullopt;
    }

    return convertLengthValue(value);
}

std::optional<float> BoxStyle::convertLengthOrNormal(const CSSValue& value) const
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        assert(ident.value() == CSSValueID::Normal);
        return std::nullopt;
    }

    return convertLengthValue(value);
}

Length BoxStyle::convertLength(const CSSValue& value) const
{
    return Length(Length::Type::Fixed, convertLengthValue(value));
}

Length BoxStyle::convertLengthOrPercent(const CSSValue& value) const
{
    if(is<CSSPercentValue>(value)) {
        const auto& percent = to<CSSPercentValue>(value);
        return Length(Length::Type::Percent, percent.value());
    }

    return convertLength(value);
}

Length BoxStyle::convertLengthOrPercentOrAuto(const CSSValue& value) const
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        assert(ident.value() == CSSValueID::Auto);
        return Length::Auto;
    }

    return convertLengthOrPercent(value);
}

Length BoxStyle::convertLengthOrPercentOrNone(const CSSValue& value) const
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        assert(ident.value() == CSSValueID::None);
        return Length::None;
    }

    return convertLengthOrPercent(value);
}

Length BoxStyle::convertWidthOrHeightLength(const CSSValue& value) const
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        switch(ident.value()) {
        case CSSValueID::None:
            return Length::None;
        case CSSValueID::Auto:
            return Length::Auto;
        case CSSValueID::MinContent:
            return Length::MinContent;
        case CSSValueID::MaxContent:
            return Length::MaxContent;
        case CSSValueID::FitContent:
            return Length::FitContent;
        default:
            assert(false);
        };
    }

    return convertLengthOrPercent(value);
}

Length BoxStyle::convertPositionComponent(CSSValueID min, CSSValueID max, const CSSValue& value) const
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        constexpr auto mid = CSSValueID::Center;
        if(min == ident.value())
            return Length(Length::Type::Percent, 0);
        if(mid == ident.value())
            return Length(Length::Type::Percent, 50);
        if(max == ident.value())
            return Length(Length::Type::Percent, 100);
        assert(false);
    }

    return convertLengthOrPercent(value);
}

LengthPoint BoxStyle::convertPositionCoordinate(const CSSValue& value) const
{
    const auto& pair = to<CSSPairValue>(value);
    auto horizontal = convertPositionComponent(CSSValueID::Left, CSSValueID::Right, *pair.first());
    auto vertical = convertPositionComponent(CSSValueID::Top, CSSValueID::Bottom, *pair.second());
    return LengthPoint(horizontal, vertical);
}

LengthSize BoxStyle::convertBorderRadius(const CSSValue& value) const
{
    const auto& pair = to<CSSPairValue>(value);
    auto horizontal = convertLengthOrPercent(*pair.first());
    auto vertical = convertLengthOrPercent(*pair.second());
    return LengthSize(horizontal, vertical);
}

Color BoxStyle::convertColor(const CSSValue& value) const
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        assert(ident.value() == CSSValueID::CurrentColor);
        return m_color;
    }

    return to<CSSColorValue>(value).value();
}

Paint BoxStyle::convertPaint(const CSSValue& value) const
{
    if(value.id() == CSSValueID::None)
        return Paint();
    if(is<CSSLocalUrlValue>(value)) {
        const auto& url = to<CSSLocalUrlValue>(value);
        return Paint(url.value());
    }

    if(is<CSSPairValue>(value)) {
        const auto& pair = to<CSSPairValue>(value);
        const auto& url = to<CSSLocalUrlValue>(*pair.first());
        if(auto ident = to<CSSIdentValue>(pair.second())) {
            if(ident->value() == CSSValueID::None) {
                return Paint(url.value());
            }
        }

        return Paint(url.value(), convertColor(*pair.second()));
    }

    return Paint(convertColor(value));
}

RefPtr<Image> BoxStyle::convertImage(const CSSValue& value) const
{
    return to<CSSImageValue>(value).fetch(document());
}

RefPtr<Image> BoxStyle::convertImageOrNone(const CSSValue& value) const
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        assert(ident.value() == CSSValueID::None);
        return nullptr;
    }

    return convertImage(value);
}

Display BoxStyle::convertDisplay(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::None:
        return Display::None;
    case CSSValueID::Block:
        return Display::Block;
    case CSSValueID::Flex:
        return Display::Flex;
    case CSSValueID::Inline:
        return Display::Inline;
    case CSSValueID::InlineBlock:
        return Display::InlineBlock;
    case CSSValueID::InlineFlex:
        return Display::InlineFlex;
    case CSSValueID::InlineTable:
        return Display::InlineTable;
    case CSSValueID::ListItem:
        return Display::ListItem;
    case CSSValueID::Table:
        return Display::Table;
    case CSSValueID::TableCaption:
        return Display::TableCaption;
    case CSSValueID::TableCell:
        return Display::TableCell;
    case CSSValueID::TableColumn:
        return Display::TableColumn;
    case CSSValueID::TableColumnGroup:
        return Display::TableColumnGroup;
    case CSSValueID::TableFooterGroup:
        return Display::TableFooterGroup;
    case CSSValueID::TableHeaderGroup:
        return Display::TableHeaderGroup;
    case CSSValueID::TableRow:
        return Display::TableRow;
    case CSSValueID::TableRowGroup:
        return Display::TableRowGroup;
    default:
        assert(false);
    }

    return Display::None;
}

Position BoxStyle::convertPosition(const CSSValue& value)
{
    if(is<CSSUnaryFunctionValue>(value)) {
        const auto& function = to<CSSUnaryFunctionValue>(value);
        assert(function.id() == CSSFunctionID::Running);
        return Position::Running;
    }

    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Static:
        return Position::Static;
    case CSSValueID::Relative:
        return Position::Relative;
    case CSSValueID::Absolute:
        return Position::Absolute;
    case CSSValueID::Fixed:
        return Position::Fixed;
    default:
        assert(false);
    }

    return Position::Static;
}

Float BoxStyle::convertFloat(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::None:
        return Float::None;
    case CSSValueID::Left:
        return Float::Left;
    case CSSValueID::Right:
        return Float::Right;
    default:
        assert(false);
    }

    return Float::None;
}

Clear BoxStyle::convertClear(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::None:
        return Clear::None;
    case CSSValueID::Left:
        return Clear::Left;
    case CSSValueID::Right:
        return Clear::Right;
    case CSSValueID::Both:
        return Clear::Both;
    default:
        assert(false);
    }

    return Clear::None;
}

VerticalAlignType BoxStyle::convertVerticalAlignType(const CSSValue& value)
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        switch(ident.value()) {
        case CSSValueID::Baseline:
            return VerticalAlignType::Baseline;
        case CSSValueID::Sub:
            return VerticalAlignType::Sub;
        case CSSValueID::Super:
            return VerticalAlignType::Super;
        case CSSValueID::TextTop:
            return VerticalAlignType::TextTop;
        case CSSValueID::TextBottom:
            return VerticalAlignType::TextBottom;
        case CSSValueID::Middle:
            return VerticalAlignType::Middle;
        case CSSValueID::Top:
            return VerticalAlignType::Top;
        case CSSValueID::Bottom:
            return VerticalAlignType::Bottom;
        default:
            assert(false);
        }
    }

    return VerticalAlignType::Length;
}

Direction BoxStyle::convertDirection(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Ltr:
        return Direction::Ltr;
    case CSSValueID::Rtl:
        return Direction::Rtl;
    default:
        assert(false);
    }

    return Direction::Ltr;
}

UnicodeBidi BoxStyle::convertUnicodeBidi(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Normal:
        return UnicodeBidi::Normal;
    case CSSValueID::Embed:
        return UnicodeBidi::Embed;
    case CSSValueID::BidiOverride:
        return UnicodeBidi::BidiOverride;
    case CSSValueID::Isolate:
        return UnicodeBidi::Isolate;
    case CSSValueID::IsolateOverride:
        return UnicodeBidi::IsolateOverride;
    default:
        assert(false);
    }

    return UnicodeBidi::Normal;
}

Visibility BoxStyle::convertVisibility(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Visible:
        return Visibility::Visible;
    case CSSValueID::Hidden:
        return Visibility::Hidden;
    case CSSValueID::Collapse:
        return Visibility::Collapse;
    default:
        assert(false);
    }

    return Visibility::Visible;
}

LineStyle BoxStyle::convertLineStyle(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::None:
        return LineStyle::None;
    case CSSValueID::Hidden:
        return LineStyle::Hidden;
    case CSSValueID::Inset:
        return LineStyle::Inset;
    case CSSValueID::Groove:
        return LineStyle::Groove;
    case CSSValueID::Outset:
        return LineStyle::Outset;
    case CSSValueID::Ridge:
        return LineStyle::Ridge;
    case CSSValueID::Dotted:
        return LineStyle::Dotted;
    case CSSValueID::Dashed:
        return LineStyle::Dashed;
    case CSSValueID::Solid:
        return LineStyle::Solid;
    case CSSValueID::Double:
        return LineStyle::Double;
    default:
        assert(false);
    }

    return LineStyle::None;
}

BackgroundBox BoxStyle::convertBackgroundBox(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::BorderBox:
        return BackgroundBox::BorderBox;
    case CSSValueID::PaddingBox:
        return BackgroundBox::PaddingBox;
    case CSSValueID::ContentBox:
        return BackgroundBox::ContentBox;
    default:
        assert(false);
    }

    return BackgroundBox::BorderBox;
}

WritingMode BoxStyle::convertWritingMode(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::HorizontalTb:
        return WritingMode::HorizontalTb;
    case CSSValueID::VerticalRl:
        return WritingMode::VerticalRl;
    case CSSValueID::VerticalLr:
        return WritingMode::VerticalLr;
    default:
        assert(false);
    }

    return WritingMode::HorizontalTb;
}

TextOrientation BoxStyle::convertTextOrientation(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Mixed:
        return TextOrientation::Mixed;
    case CSSValueID::Upright:
        return TextOrientation::Upright;
    default:
        assert(false);
    }

    return TextOrientation::Mixed;
}

TextAlign BoxStyle::convertTextAlign(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Left:
        return TextAlign::Left;
    case CSSValueID::Right:
        return TextAlign::Right;
    case CSSValueID::Center:
        return TextAlign::Center;
    case CSSValueID::Justify:
        return TextAlign::Justify;
    case CSSValueID::Start:
        return TextAlign::Start;
    case CSSValueID::End:
        return TextAlign::End;
    default:
        assert(false);
    }

    return TextAlign::Left;
}

WhiteSpace BoxStyle::convertWhiteSpace(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Normal:
        return WhiteSpace::Normal;
    case CSSValueID::Pre:
        return WhiteSpace::Pre;
    case CSSValueID::PreWrap:
        return WhiteSpace::PreWrap;
    case CSSValueID::PreLine:
        return WhiteSpace::PreLine;
    case CSSValueID::Nowrap:
        return WhiteSpace::Nowrap;
    default:
        assert(false);
    }

    return WhiteSpace::Normal;
}

WordBreak BoxStyle::convertWordBreak(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Normal:
        return WordBreak::Normal;
    case CSSValueID::KeepAll:
        return WordBreak::KeepAll;
    case CSSValueID::BreakAll:
        return WordBreak::BreakAll;
    case CSSValueID::BreakWord:
        return WordBreak::BreakWord;
    default:
        assert(false);
    }

    return WordBreak::Normal;
}

OverflowWrap BoxStyle::convertOverflowWrap(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Normal:
        return OverflowWrap::Normal;
    case CSSValueID::Anywhere:
        return OverflowWrap::Anywhere;
    case CSSValueID::BreakWord:
        return OverflowWrap::BreakWord;
    default:
        assert(false);
    }

    return OverflowWrap::Normal;
}

BoxSizing BoxStyle::convertBoxSizing(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::BorderBox:
        return BoxSizing::BorderBox;
    case CSSValueID::ContentBox:
        return BoxSizing::ContentBox;
    default:
        assert(false);
    }

    return BoxSizing::BorderBox;
}

BlendMode BoxStyle::convertBlendMode(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Normal:
        return BlendMode::Normal;
    case CSSValueID::Multiply:
        return BlendMode::Multiply;
    case CSSValueID::Screen:
        return BlendMode::Screen;
    case CSSValueID::Overlay:
        return BlendMode::Overlay;
    case CSSValueID::Darken:
        return BlendMode::Darken;
    case CSSValueID::Lighten:
        return BlendMode::Lighten;
    case CSSValueID::ColorDodge:
        return BlendMode::ColorDodge;
    case CSSValueID::ColorBurn:
        return BlendMode::ColorBurn;
    case CSSValueID::HardLight:
        return BlendMode::HardLight;
    case CSSValueID::SoftLight:
        return BlendMode::SoftLight;
    case CSSValueID::Difference:
        return BlendMode::Difference;
    case CSSValueID::Exclusion:
        return BlendMode::Exclusion;
    case CSSValueID::Hue:
        return BlendMode::Hue;
    case CSSValueID::Saturation:
        return BlendMode::Saturation;
    case CSSValueID::Color:
        return BlendMode::Color;
    case CSSValueID::Luminosity:
        return BlendMode::Luminosity;
    default:
        assert(false);
    }

    return BlendMode::Normal;
}

MaskType BoxStyle::convertMaskType(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Luminance:
        return MaskType::Luminance;
    case CSSValueID::Alpha:
        return MaskType::Alpha;
    default:
        assert(false);
    }

    return MaskType::Luminance;
}

AlignContent BoxStyle::convertAlignContent(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::FlexStart:
        return AlignContent::FlexStart;
    case CSSValueID::FlexEnd:
        return AlignContent::FlexEnd;
    case CSSValueID::Center:
        return AlignContent::Center;
    case CSSValueID::SpaceBetween:
        return AlignContent::SpaceBetween;
    case CSSValueID::SpaceAround:
        return AlignContent::SpaceAround;
    case CSSValueID::SpaceEvenly:
        return AlignContent::SpaceEvenly;
    case CSSValueID::Stretch:
        return AlignContent::Stretch;
    default:
        assert(false);
    }

    return AlignContent::FlexStart;
}

AlignItem BoxStyle::convertAlignItem(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Auto:
        return AlignItem::Auto;
    case CSSValueID::FlexStart:
        return AlignItem::FlexStart;
    case CSSValueID::FlexEnd:
        return AlignItem::FlexEnd;
    case CSSValueID::Center:
        return AlignItem::Center;
    case CSSValueID::Stretch:
        return AlignItem::Stretch;
    case CSSValueID::Baseline:
        return AlignItem::Baseline;
    default:
        assert(false);
    }

    return AlignItem::Auto;
}

FillRule BoxStyle::convertFillRule(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Nonzero:
        return FillRule::NonZero;
    case CSSValueID::Evenodd:
        return FillRule::EvenOdd;
    default:
        assert(false);
    }

    return FillRule::NonZero;
}

CaptionSide BoxStyle::convertCaptionSide(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Top:
        return CaptionSide::Top;
    case CSSValueID::Bottom:
        return CaptionSide::Bottom;
    default:
        assert(false);
    }

    return CaptionSide::Top;
}

EmptyCells BoxStyle::convertEmptyCells(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Show:
        return EmptyCells::Show;
    case CSSValueID::Hide:
        return EmptyCells::Hide;
    default:
        assert(false);
    }

    return EmptyCells::Show;
}

BorderCollapse BoxStyle::convertBorderCollapse(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Separate:
        return BorderCollapse::Separate;
    case CSSValueID::Collapse:
        return BorderCollapse::Collapse;
    default:
        assert(false);
    }

    return BorderCollapse::Separate;
}

BreakBetween BoxStyle::convertBreakBetween(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Auto:
        return BreakBetween::Auto;
    case CSSValueID::Avoid:
        return BreakBetween::Avoid;
    case CSSValueID::AvoidColumn:
        return BreakBetween::AvoidColumn;
    case CSSValueID::AvoidPage:
        return BreakBetween::AvoidPage;
    case CSSValueID::Column:
        return BreakBetween::Column;
    case CSSValueID::Page:
        return BreakBetween::Page;
    case CSSValueID::Left:
        return BreakBetween::Left;
    case CSSValueID::Right:
        return BreakBetween::Right;
    case CSSValueID::Recto:
        return BreakBetween::Recto;
    case CSSValueID::Verso:
        return BreakBetween::Verso;
    default:
        assert(false);
    }

    return BreakBetween::Auto;
}

BreakInside BoxStyle::convertBreakInside(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::Auto:
        return BreakInside::Auto;
    case CSSValueID::Avoid:
        return BreakInside::Avoid;
    case CSSValueID::AvoidColumn:
        return BreakInside::AvoidColumn;
    case CSSValueID::AvoidPage:
        return BreakInside::AvoidPage;
    default:
        assert(false);
    }

    return BreakInside::Auto;
}

PageSize BoxStyle::convertPageSize(const CSSValue& value)
{
    const auto& ident = to<CSSIdentValue>(value);
    switch(ident.value()) {
    case CSSValueID::A3:
        return PageSize::A3;
    case CSSValueID::A4:
        return PageSize::A4;
    case CSSValueID::A5:
        return PageSize::A5;
    case CSSValueID::B4:
        return PageSize::B4;
    case CSSValueID::B5:
        return PageSize::B5;
    case CSSValueID::Ledger:
        return PageSize::Ledger;
    case CSSValueID::Legal:
        return PageSize::Legal;
    case CSSValueID::Letter:
        return PageSize::Letter;
    default:
        assert(false);
    }

    return PageSize::A4;
}

int BoxStyle::convertInteger(const CSSValue& value)
{
    return to<CSSIntegerValue>(value).value();
}

std::optional<int> BoxStyle::convertIntegerOrAuto(const CSSValue& value)
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        assert(ident.value() == CSSValueID::Auto);
        return std::nullopt;
    }

    return convertInteger(value);
}

float BoxStyle::convertNumber(const CSSValue& value)
{
    return to<CSSNumberValue>(value).value();
}

float BoxStyle::convertNumberOrPercent(const CSSValue& value)
{
    if(is<CSSPercentValue>(value)) {
        const auto& percent = to<CSSPercentValue>(value);
        return percent.value() / 100.f;
    }

    return convertNumber(value);
}

float BoxStyle::convertAngle(const CSSValue& value)
{
    return to<CSSAngleValue>(value).valueInDegrees();
}

GlobalString BoxStyle::convertCustomIdent(const CSSValue& value)
{
    return to<CSSCustomIdentValue>(value).value();
}

HeapString BoxStyle::convertLocalUrl(const CSSValue& value)
{
    return to<CSSLocalUrlValue>(value).value();
}

HeapString BoxStyle::convertLocalUrlOrNone(const CSSValue& value)
{
    if(is<CSSIdentValue>(value)) {
        const auto& ident = to<CSSIdentValue>(value);
        assert(ident.value() == CSSValueID::None);
        return emptyGlo;
    }

    return convertLocalUrl(value);
}

BoxStyle::~BoxStyle() = default;

BoxStyle::BoxStyle(Node* node, PseudoType pseudoType, Display display)
    : m_node(node), m_properties(node->heap())
    , m_customProperties(node->heap())
    , m_pseudoType(pseudoType), m_display(display)
{
}

} // namespace plutobook
