/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_BOXSTYLE_H
#define PLUTOBOOK_BOXSTYLE_H

#include "pointer.h"
#include "heapstring.h"
#include "color.h"

#include <optional>
#include <forward_list>
#include <unordered_map>
#include <vector>
#include <map>

namespace plutobook {

enum class Display : uint8_t {
    None,
    Block,
    Flex,
    Inline,
    InlineBlock,
    InlineFlex,
    InlineTable,
    ListItem,
    Table,
    TableCaption,
    TableCell,
    TableColumn,
    TableColumnGroup,
    TableFooterGroup,
    TableHeaderGroup,
    TableRow,
    TableRowGroup
};

enum class Visibility : uint8_t {
    Visible,
    Hidden,
    Collapse
};

enum class Float : uint8_t {
    None,
    Left,
    Right
};

enum class Clear : uint8_t {
    None,
    Left,
    Right,
    Both
};

enum class Position : uint8_t {
    Static,
    Relative,
    Absolute,
    Fixed,
    Running
};

enum class Overflow : uint8_t {
    Auto,
    Visible,
    Hidden,
    Scroll
};

enum class Direction : uint8_t {
    Ltr,
    Rtl
};

enum class UnicodeBidi : uint8_t {
    Normal,
    Embed,
    BidiOverride,
    Isolate,
    IsolateOverride
};

enum class LineStyle : uint8_t {
    None,
    Hidden,
    Inset,
    Groove,
    Outset,
    Ridge,
    Dotted,
    Dashed,
    Solid,
    Double
};

enum class ListStylePosition : uint8_t {
    Outside,
    Inside
};

enum class ObjectFit : uint8_t {
    Fill,
    Contain,
    Cover,
    None,
    ScaleDown
};

enum class BackgroundRepeat : uint8_t {
    Repeat,
    RepeatX,
    RepeatY,
    NoRepeat
};

enum class BackgroundBox : uint8_t {
    BorderBox,
    PaddingBox,
    ContentBox
};

enum class BackgroundAttachment : uint8_t {
    Scroll,
    Fixed,
    Local
};

enum class WritingMode : uint8_t {
    HorizontalTb,
    VerticalRl,
    VerticalLr
};

enum class TextOrientation : uint8_t {
    Mixed,
    Upright
};

enum class TextAnchor : uint8_t {
    Start,
    Middle,
    End
};

enum class TextAlign : uint8_t {
    Left,
    Center,
    Right,
    Justify,
    Start,
    End
};

enum class TextTransform : uint8_t {
    None,
    Capitalize,
    Uppercase,
    Lowercase
};

enum class TextOverflow : uint8_t {
    Clip,
    Ellipsis
};

enum class TextDecorationLine : uint8_t {
    None = 0x0,
    Underline = 0x1,
    Overline = 0x2,
    LineThrough = 0x4
};

constexpr bool operator&(TextDecorationLine a, TextDecorationLine b) { return static_cast<int>(a) & static_cast<int>(b); }

enum class TextDecorationStyle : uint8_t {
    Solid,
    Double,
    Dotted,
    Dashed,
    Wavy
};

enum class FontVariantEmoji : uint8_t {
    Normal,
    Text,
    Emoji,
    Unicode
};

enum class WhiteSpace : uint8_t {
    Normal,
    Pre,
    Nowrap,
    PreLine,
    PreWrap
};

enum class WordBreak : uint8_t {
    Normal,
    KeepAll,
    BreakAll,
    BreakWord
};

enum class OverflowWrap : uint8_t {
    Normal,
    BreakWord,
    Anywhere
};

enum class Hyphens : uint8_t {
    Auto,
    None,
    Manual
};

enum class TableLayout : uint8_t {
    Auto,
    Fixed
};

enum class CaptionSide : uint8_t {
    Top,
    Bottom
};

enum class EmptyCells : uint8_t {
    Show,
    Hide
};

enum class BorderCollapse : uint8_t {
    Separate,
    Collapse
};

enum class BoxSizing : uint8_t {
    ContentBox,
    BorderBox
};

enum class FlexDirection : uint8_t {
    Row,
    RowReverse,
    Column,
    ColumnReverse
};

enum class FlexWrap : uint8_t {
    Nowrap,
    Wrap,
    WrapReverse
};

enum class AlignContent : uint8_t {
    FlexStart,
    FlexEnd,
    Center,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly,
    Stretch
};

enum class AlignItem : uint8_t {
    Auto,
    FlexStart,
    FlexEnd,
    Center,
    Baseline,
    Stretch
};

enum class BreakBetween : uint8_t {
    Auto,
    Avoid,
    AvoidColumn,
    AvoidPage,
    Column,
    Page,
    Left,
    Right,
    Recto,
    Verso
};

enum class BreakInside : uint8_t {
    Auto,
    Avoid,
    AvoidColumn,
    AvoidPage
};

enum class ColumnSpan : uint8_t {
    None,
    All
};

enum class ColumnFill : uint8_t {
    Auto,
    Balance
};

enum class QuoteType : uint8_t {
    Open,
    Close,
    NoOpen,
    NoClose
};

enum class BlendMode : uint8_t {
    Normal,
    Multiply,
    Screen,
    Overlay,
    Darken,
    Lighten,
    ColorDodge,
    ColorBurn,
    HardLight,
    SoftLight,
    Difference,
    Exclusion,
    Hue,
    Saturation,
    Color,
    Luminosity
};

enum class MaskType : uint8_t {
    Luminance,
    Alpha
};

enum class FillRule : uint8_t {
    NonZero,
    EvenOdd
};

enum class LineCap : uint8_t {
    Butt,
    Round,
    Square
};

enum class LineJoin : uint8_t {
    Miter,
    Round,
    Bevel
};

class Paint {
public:
    enum class Type {
        None,
        Color,
        UriNone,
        UriColor
    };

    Paint() = default;
    explicit Paint(const Color& color)
        : m_type(Type::Color), m_color(color)
    {}

    explicit Paint(const HeapString& uri)
        : m_type(Type::UriNone), m_uri(uri)
    {}

    Paint(const HeapString& uri, const Color& color)
        : m_type(Type::UriColor), m_color(color), m_uri(uri)
    {}

    Type type() const { return m_type; }
    const Color& color() const { return m_color; }
    const HeapString& uri() const { return m_uri; }

    bool isNone() const { return m_type == Type::None; }

private:
    Type m_type{Type::None};
    Color m_color{Color::Transparent};
    HeapString m_uri;
};

class Length {
public:
    enum class Type {
        None,
        Auto,
        MinContent,
        MaxContent,
        FitContent,
        Percent,
        Fixed
    };

    Length() = default;
    explicit Length(float value) : Length(Type::Fixed, value) {}
    Length(Type type, float value = 0.f) : m_type(type), m_value(value) {}

    bool isNone() const { return m_type == Type::None; }
    bool isAuto() const { return m_type == Type::Auto; }
    bool isMinContent() const { return m_type == Type::MinContent; }
    bool isMaxContent() const { return m_type == Type::MaxContent; }
    bool isFitContent() const { return m_type == Type::FitContent; }
    bool isIntrinsic() const { return isMinContent() || isMaxContent() || isFitContent(); }
    bool isFixed() const { return m_type == Type::Fixed; }
    bool isPercent() const { return m_type == Type::Percent; }
    bool isZero() const { return m_value == 0; }

    Type type() const { return m_type; }
    float value() const { return m_value; }

    static const Length None;
    static const Length Auto;
    static const Length MinContent;
    static const Length MaxContent;
    static const Length FitContent;
    static const Length ZeroFixed;
    static const Length ZeroPercent;

    float calc(float maximum) const;
    float calcMin(float maximum) const;

private:
    Type m_type{Type::Auto};
    float m_value{0.f};
};

inline float Length::calc(float maximum) const
{
    switch(m_type) {
    case Type::Fixed:
        return m_value;
    case Type::Percent:
        return m_value * maximum / 100.f;
    default:
        return maximum;
    }
}

inline float Length::calcMin(float maximum) const
{
    switch(m_type) {
    case Type::Fixed:
        return m_value;
    case Type::Percent:
        return m_value * maximum / 100.f;
    default:
        return 0;
    }
}

using LengthList = std::vector<Length>;

class LengthPoint {
public:
    explicit LengthPoint(const Length& value) : LengthPoint(value, value) {}
    LengthPoint(const Length& x, const Length& y)
        : m_x(x), m_y(y)
    {}

    const Length& x() const { return m_x; }
    const Length& y() const { return m_y; }

private:
    Length m_x;
    Length m_y;
};

class LengthSize {
public:
    explicit LengthSize(const Length& value) : LengthSize(value, value) {}
    LengthSize(const Length& width, const Length& height)
        : m_width(width), m_height(height)
    {}

    const Length& width() const { return m_width; }
    const Length& height() const { return m_height; }

private:
    Length m_width;
    Length m_height;
};

class LengthBox {
public:
    explicit LengthBox(const Length& value) : LengthBox(value, value, value, value) {}
    LengthBox(const Length& left, const Length& right, const Length& top, const Length& bottom)
        : m_left(left), m_right(right), m_top(top), m_bottom(bottom)
    {}

    const Length& left() const { return m_left; }
    const Length& right() const { return m_right; }
    const Length& top() const { return m_top; }
    const Length& bottom() const { return m_bottom; }

private:
    Length m_left;
    Length m_right;
    Length m_top;
    Length m_bottom;
};

class BackgroundSize {
public:
    enum class Type {
        Contain,
        Cover,
        Length
    };

    BackgroundSize() = default;
    explicit BackgroundSize(Type type) : m_type(type) {}
    BackgroundSize(const Length& width, const Length& height)
        : m_type(Type::Length), m_width(width), m_height(height)
    {}

    Type type() const { return m_type; }
    const Length& width() const { return m_width; }
    const Length& height() const { return m_height; }

private:
    Type m_type = Type::Length;
    Length m_width = Length::Auto;
    Length m_height = Length::Auto;
};

enum BoxSide {
    BoxSideTop = 0,
    BoxSideRight,
    BoxSideBottom,
    BoxSideLeft
};

class BorderEdge {
public:
    BorderEdge() = default;
    BorderEdge(float width, const Color& color, LineStyle style)
        : m_width(style > LineStyle::Hidden ? width : 0.f)
        , m_color(color), m_style(style)
    {
    }

    bool isRenderable() const { return m_width > 0 && m_style > LineStyle::Hidden && m_color.alpha() > 0; }

    float width() const { return m_width; }
    const Color& color() const { return m_color; }
    LineStyle style() const { return m_style; }

private:
    float m_width{0};
    Color m_color;
    LineStyle m_style{LineStyle::Hidden};
};

enum class AlignmentBaseline : uint8_t {
    Auto,
    Baseline,
    BeforeEdge,
    TextBeforeEdge,
    Middle,
    Central,
    AfterEdge,
    TextAfterEdge,
    Ideographic,
    Alphabetic,
    Hanging,
    Mathematical
};

enum class DominantBaseline : uint8_t {
    Auto,
    UseScript,
    NoChange,
    ResetSize,
    Ideographic,
    Alphabetic,
    Hanging,
    Mathematical,
    Central,
    Middle,
    TextAfterEdge,
    TextBeforeEdge
};

enum class BaselineShiftType : uint8_t {
    Baseline,
    Sub,
    Super,
    Length
};

class BaselineShift {
public:
    BaselineShift(BaselineShiftType type, const Length& length = Length::Auto)
        : m_type(type), m_length(length)
    {}

    BaselineShiftType type() const { return m_type; }
    const Length& length() const { return m_length; }

private:
    BaselineShiftType m_type;
    Length m_length;
};

enum class VerticalAlignType : uint8_t {
    Baseline,
    Sub,
    Super,
    TextTop,
    TextBottom,
    Middle,
    Top,
    Bottom,
    Length
};

class VerticalAlign {
public:
    VerticalAlign(VerticalAlignType type, const Length& length = Length::Auto)
        : m_type(type), m_length(length)
    {}

    VerticalAlignType type() const { return m_type; }
    const Length& length() const { return m_length; }

private:
    VerticalAlignType m_type;
    Length m_length;
};

class PageSize;
class Point;
class RoundedRect;
class Rect;
class Transform;
class GlobalString;
class FontTag;
class Font;
class Image;
class Node;
class Document;
class Book;

enum class CSSValueID : uint16_t;
enum class CSSPropertyID : uint16_t;

using FontFeature = std::pair<FontTag, int>;
using FontVariation = std::pair<FontTag, float>;
using FontFeatureList = std::forward_list<FontFeature>;
using FontVariationList = std::forward_list<FontVariation>;
using FontFamilyList = std::forward_list<GlobalString>;

class CSSValue;
class CSSVariableData;

using CSSPropertyMap = std::pmr::unordered_map<CSSPropertyID, RefPtr<CSSValue>>;
using CSSCustomPropertyMap = std::pmr::map<GlobalString, RefPtr<CSSVariableData>, std::less<>>;

enum class PseudoType : uint8_t {
    None,
    Before,
    After,
    Marker,
    FirstLetter,
    FirstLine,
    FirstPage,
    LeftPage,
    RightPage,
    BlankPage
};

struct FontDescription;

class BoxStyle : public HeapMember, public RefCounted<BoxStyle> {
public:
    static RefPtr<BoxStyle> create(Node* node, PseudoType pseudoType, Display display);
    static RefPtr<BoxStyle> create(Node* node, const BoxStyle* parentStyle, PseudoType pseudoType, Display display);
    static RefPtr<BoxStyle> create(const BoxStyle* parentStyle, PseudoType pseudoType, Display display);
    static RefPtr<BoxStyle> create(const BoxStyle* parentStyle, Display display);

    Document* document() const;
    Heap* heap() const;
    Book* book() const;

    Node* node() const { return m_node; }
    PseudoType pseudoType() const { return m_pseudoType; }
    const CSSPropertyMap& properties() const { return m_properties; }
    const CSSCustomPropertyMap& customProperties() const { return m_customProperties; }

    const RefPtr<Font>& font() const { return m_font; }
    void setFont(RefPtr<Font> font);

    float fontAscent() const;
    float fontDescent() const;
    float fontHeight() const;
    float fontLineGap() const;
    float fontLineSpacing() const;

    const FontDescription& fontDescription() const;
    void setFontDescription(const FontDescription& description);

    float fontSize() const;
    float fontWeight() const;
    float fontStretch() const;
    float fontStyle() const;

    const FontFamilyList& fontFamily() const;
    const FontVariationList& fontVariationSettings() const;

    void setDisplay(Display display) { m_display = display; }
    void setPosition(Position position) { m_position = position; }
    void setFloating(Float floating) { m_floating = floating; }
    void setDirection(Direction direction) { m_direction = direction; }

    void setTextAlign(TextAlign textAlign) { m_textAlign = textAlign; }
    void setVerticalAlignType(VerticalAlignType verticalAlignType) { m_verticalAlignType = verticalAlignType; }

    Display display() const { return m_display; }
    Position position() const { return m_position; }
    Float floating() const { return m_floating; }
    Clear clear() const { return m_clear; }
    VerticalAlignType verticalAlignType() const { return m_verticalAlignType; }
    Direction direction() const { return m_direction; }
    UnicodeBidi unicodeBidi() const { return m_unicodeBidi; }
    Visibility visibility() const { return m_visibility; }
    const Color& color() const { return m_color; }

    Length left() const;
    Length right() const;
    Length top() const;
    Length bottom() const;
    Length width() const;
    Length height() const;
    Length minWidth() const;
    Length minHeight() const;
    Length maxWidth() const;
    Length maxHeight() const;

    Length marginLeft() const;
    Length marginRight() const;
    Length marginTop() const;
    Length marginBottom() const;

    Length paddingLeft() const;
    Length paddingRight() const;
    Length paddingTop() const;
    Length paddingBottom() const;

    LineStyle borderLeftStyle() const;
    LineStyle borderRightStyle() const;
    LineStyle borderTopStyle() const;
    LineStyle borderBottomStyle() const;

    Color borderLeftColor() const;
    Color borderRightColor() const;
    Color borderTopColor() const;
    Color borderBottomColor() const;

    float borderLeftWidth() const;
    float borderRightWidth() const;
    float borderTopWidth() const;
    float borderBottomWidth() const;

    void getBorderEdgeInfo(BorderEdge edges[], bool includeLeftEdge, bool includeRightEdge) const;

    LengthSize borderTopLeftRadius() const;
    LengthSize borderTopRightRadius() const;
    LengthSize borderBottomLeftRadius() const;
    LengthSize borderBottomRightRadius() const;

    RoundedRect getBorderRoundedRect(const Rect& borderRect, bool includeLeftEdge, bool includeRightEdge) const;

    ListStylePosition listStylePosition() const;
    RefPtr<Image> listStyleImage() const;

    RefPtr<Image> backgroundImage() const;
    Color backgroundColor() const;
    BackgroundRepeat backgroundRepeat() const;
    BackgroundBox backgroundOrigin() const;
    BackgroundBox backgroundClip() const;
    BackgroundAttachment backgroundAttachment() const;
    BackgroundSize backgroundSize() const;
    LengthPoint backgroundPosition() const;

    ObjectFit objectFit() const;
    LengthPoint objectPosition() const;

    TableLayout tableLayout() const;
    CaptionSide captionSide() const { return m_captionSide; }
    EmptyCells emptyCells() const { return m_emptyCells; }
    BorderCollapse borderCollapse() const { return m_borderCollapse; }
    float borderHorizontalSpacing() const;
    float borderVerticalSpacing() const;

    WritingMode writingMode() const { return m_writingMode; }
    TextOrientation textOrientation() const { return m_textOrientation; }

    TextAlign textAlign() const { return m_textAlign; }
    TextAnchor textAnchor() const;
    TextTransform textTransform() const;
    TextOverflow textOverflow() const;
    TextDecorationLine textDecorationLine() const;
    TextDecorationStyle textDecorationStyle() const;
    Color textDecorationColor() const;
    WhiteSpace whiteSpace() const { return m_whiteSpace; }
    WordBreak wordBreak() const { return m_wordBreak; }
    OverflowWrap overflowWrap() const { return m_overflowWrap; }
    FontVariantEmoji fontVariantEmoji() const;
    Hyphens hyphens() const;
    Length textIndent() const;
    float letterSpacing() const;
    float wordSpacing() const;
    float lineHeight() const;

    float tabWidth(float spaceWidth) const;

    BoxSizing boxSizing() const { return m_boxSizing; }
    BlendMode blendMode() const { return m_blendMode; }
    MaskType maskType() const { return m_maskType; }
    Overflow overflow() const;
    std::optional<int> zIndex() const;
    VerticalAlign verticalAlign() const;
    LengthBox clip() const;

    Length flexBasis() const;
    float flexGrow() const;
    float flexShrink() const;
    int order() const;
    FlexDirection flexDirection() const;
    FlexWrap flexWrap() const;
    AlignContent justifyContent() const;
    AlignContent alignContent() const;
    AlignItem alignItems() const;
    AlignItem alignSelf() const;

    float outlineOffset() const;
    Color outlineColor() const;
    float outlineWidth() const;
    LineStyle outlineStyle() const;

    BorderEdge getOutlineEdge() const;

    int widows() const;
    int orphans() const;

    Color columnRuleColor() const;
    LineStyle columnRuleStyle() const;
    float columnRuleWidth() const;

    ColumnSpan columnSpan() const;
    ColumnFill columnFill() const;

    std::optional<float> rowGap() const;
    std::optional<float> columnGap() const;
    std::optional<float> columnWidth() const;
    std::optional<int> columnCount() const;

    BreakBetween breakAfter() const { return m_breakAfter; }
    BreakBetween breakBefore() const { return m_breakBefore; }
    BreakInside breakInside() const { return m_breakInside; }

    std::optional<float> pageScale() const;
    GlobalString page() const;

    PageSize getPageSize(const PageSize& deviceSize) const;

    Paint fill() const;
    Paint stroke() const;
    Color stopColor() const;

    float opacity() const;
    float stopOpacity() const;
    float fillOpacity() const;
    float strokeOpacity() const;
    float strokeMiterlimit() const;

    Length strokeWidth() const;
    Length strokeDashoffset() const;
    LengthList strokeDasharray() const;

    LineCap strokeLinecap() const;
    LineJoin strokeLinejoin() const;

    FillRule fillRule() const { return m_fillRule; }
    FillRule clipRule() const { return m_clipRule; }

    HeapString mask() const;
    HeapString clipPath() const;
    HeapString markerStart() const;
    HeapString markerMid() const;
    HeapString markerEnd() const;

    AlignmentBaseline alignmentBaseline() const;
    DominantBaseline dominantBaseline() const;
    BaselineShift baselineShift() const;

    static bool isDisplayBlockType(Display display);
    static bool isDisplayInlineType(Display display);

    bool isOriginalDisplayBlockType() const;
    bool isOriginalDisplayInlineType() const;

    bool isDisplayBlockType() const { return isDisplayBlockType(m_display); }
    bool isDisplayInlineType() const { return isDisplayInlineType(m_display); }
    bool isDisplayFlex() const { return m_display == Display::Flex || m_display == Display::InlineFlex; }

    bool isFloating() const { return m_floating == Float::Left || m_floating == Float::Right; }
    bool isPositioned() const { return m_position == Position::Absolute || m_position == Position::Fixed; }
    bool isLeftToRightDirection() const { return m_direction == Direction::Ltr; }
    bool isRightToLeftDirection() const { return m_direction == Direction::Rtl; }
    bool isClearLeft() const { return m_clear == Clear::Left || m_clear == Clear::Both; }
    bool isClearRight() const { return m_clear == Clear::Right || m_clear == Clear::Both; }

    bool isVerticalWritingMode() const { return m_writingMode != WritingMode::HorizontalTb; }
    bool isUprightTextOrientation() const { return m_textOrientation == TextOrientation::Upright; }

    bool isOverflowHidden() const { return overflow() != Overflow::Visible; }
    bool isVisibilityHidden() const { return visibility() != Visibility::Visible; }

    static bool autoWrap(WhiteSpace ws) { return ws != WhiteSpace::Nowrap && ws != WhiteSpace::Pre; }
    static bool preserveNewline(WhiteSpace ws) { return ws != WhiteSpace::Normal && ws != WhiteSpace::Nowrap; }
    static bool collapseWhiteSpace(WhiteSpace ws) { return ws != WhiteSpace::Pre && ws != WhiteSpace::PreWrap; }

    bool autoWrap() const { return autoWrap(whiteSpace()); }
    bool preserveNewline() const { return preserveNewline(whiteSpace()); }
    bool collapseWhiteSpace() const { return collapseWhiteSpace(whiteSpace()); }

    bool breakAnywhere() const { return m_overflowWrap == OverflowWrap::Anywhere || m_wordBreak == WordBreak::BreakAll; }
    bool breakWord() const { return m_wordBreak == WordBreak::BreakWord || m_overflowWrap == OverflowWrap::BreakWord; }

    Point getTransformOrigin(float width, float height) const;
    Transform getTransform(float width, float height) const;

    bool hasTransform() const;
    bool hasContent() const;
    bool hasLineHeight() const;
    bool hasStroke() const;
    bool hasBackground() const;
    bool hasColumns() const;

    bool hasOpacity() const { return opacity() < 1.0f; }
    bool hasBlendMode() const { return m_blendMode > BlendMode::Normal; }

    const HeapString& getQuote(bool open, size_t depth) const;

    CSSVariableData* getCustom(const std::string_view& name) const;
    void setCustom(const GlobalString& name, RefPtr<CSSVariableData> value);

    CSSValue* get(CSSPropertyID id) const;
    void set(CSSPropertyID id, RefPtr<CSSValue> value);
    void reset(CSSPropertyID id);

    void inheritFrom(const BoxStyle* parentStyle);

    float exFontSize() const;
    float chFontSize() const;
    float remFontSize() const;

    FontFeatureList fontFeatures() const;

    float viewportWidth() const;
    float viewportHeight() const;
    float viewportMin() const;
    float viewportMax() const;

    RefPtr<CSSValue> resolveLength(const RefPtr<CSSValue>& value) const;

    float convertLengthValue(const CSSValue& value) const;
    float convertLineWidth(const CSSValue& value) const;
    float convertSpacing(const CSSValue& value) const;
    float convertLengthOrPercent(float maximum, const CSSValue& value) const;
    std::optional<float> convertLengthOrAuto(const CSSValue& value) const;
    std::optional<float> convertLengthOrNormal(const CSSValue& value) const;
    Length convertLength(const CSSValue& value) const;
    Length convertLengthOrPercent(const CSSValue& value) const;
    Length convertLengthOrPercentOrAuto(const CSSValue& value) const;
    Length convertLengthOrPercentOrNone(const CSSValue& value) const;
    Length convertWidthOrHeightLength(const CSSValue& value) const;
    Length convertPositionComponent(CSSValueID min, CSSValueID max, const CSSValue& value) const;
    LengthPoint convertPositionCoordinate(const CSSValue& value) const;
    LengthSize convertBorderRadius(const CSSValue& value) const;
    Color convertColor(const CSSValue& value) const;
    Paint convertPaint(const CSSValue& value) const;
    RefPtr<Image> convertImage(const CSSValue& value) const;
    RefPtr<Image> convertImageOrNone(const CSSValue& value) const;

    static Display convertDisplay(const CSSValue& value);
    static Position convertPosition(const CSSValue& value);
    static Float convertFloat(const CSSValue& value);
    static Clear convertClear(const CSSValue& value);
    static VerticalAlignType convertVerticalAlignType(const CSSValue& value);
    static Direction convertDirection(const CSSValue& value);
    static UnicodeBidi convertUnicodeBidi(const CSSValue& value);
    static Visibility convertVisibility(const CSSValue& value);
    static LineStyle convertLineStyle(const CSSValue& value);
    static BackgroundBox convertBackgroundBox(const CSSValue& value);
    static WritingMode convertWritingMode(const CSSValue& value);
    static TextOrientation convertTextOrientation(const CSSValue& value);
    static TextAlign convertTextAlign(const CSSValue& value);
    static WhiteSpace convertWhiteSpace(const CSSValue& value);
    static WordBreak convertWordBreak(const CSSValue& value);
    static OverflowWrap convertOverflowWrap(const CSSValue& value);
    static BoxSizing convertBoxSizing(const CSSValue& value);
    static BlendMode convertBlendMode(const CSSValue& value);
    static MaskType convertMaskType(const CSSValue& value);
    static AlignContent convertAlignContent(const CSSValue& value);
    static AlignItem convertAlignItem(const CSSValue& value);
    static FillRule convertFillRule(const CSSValue& value);
    static CaptionSide convertCaptionSide(const CSSValue& value);
    static EmptyCells convertEmptyCells(const CSSValue& value);
    static BorderCollapse convertBorderCollapse(const CSSValue& value);
    static BreakBetween convertBreakBetween(const CSSValue& value);
    static BreakInside convertBreakInside(const CSSValue& value);
    static PageSize convertPageSize(const CSSValue& value);
    static int convertInteger(const CSSValue& value);
    static std::optional<int> convertIntegerOrAuto(const CSSValue& value);
    static float convertNumber(const CSSValue& value);
    static float convertNumberOrPercent(const CSSValue& value);
    static float convertAngle(const CSSValue& value);
    static GlobalString convertCustomIdent(const CSSValue& value);
    static HeapString convertLocalUrl(const CSSValue& value);
    static HeapString convertLocalUrlOrNone(const CSSValue& value);

    ~BoxStyle();

private:
    BoxStyle(Node* node, PseudoType pseudoType, Display display);
    Node* m_node;
    CSSPropertyMap m_properties;
    CSSCustomPropertyMap m_customProperties;
    RefPtr<Font> m_font;
    PseudoType m_pseudoType;
    Display m_display;
    Position m_position{Position::Static};
    Float m_floating{Float::None};
    Clear m_clear{Clear::None};
    VerticalAlignType m_verticalAlignType{VerticalAlignType::Baseline};
    Direction m_direction{Direction::Ltr};
    UnicodeBidi m_unicodeBidi{UnicodeBidi::Normal};
    Visibility m_visibility{Visibility::Visible};
    BoxSizing m_boxSizing{BoxSizing::ContentBox};
    BlendMode m_blendMode{BlendMode::Normal};
    MaskType m_maskType{MaskType::Luminance};
    WritingMode m_writingMode{WritingMode::HorizontalTb};
    TextOrientation m_textOrientation{TextOrientation::Mixed};
    TextAlign m_textAlign{TextAlign::Start};
    WhiteSpace m_whiteSpace{WhiteSpace::Normal};
    WordBreak m_wordBreak{WordBreak::Normal};
    OverflowWrap m_overflowWrap{OverflowWrap::Normal};
    FillRule m_fillRule{FillRule::NonZero};
    FillRule m_clipRule{FillRule::NonZero};
    CaptionSide m_captionSide{CaptionSide::Top};
    EmptyCells m_emptyCells{EmptyCells::Show};
    BorderCollapse m_borderCollapse{BorderCollapse::Separate};
    BreakBetween m_breakAfter{BreakBetween::Auto};
    BreakBetween m_breakBefore{BreakBetween::Auto};
    BreakInside m_breakInside{BreakInside::Auto};
    Color m_color{Color::Black};
};

inline bool BoxStyle::isDisplayBlockType(Display display)
{
    return display == Display::Block
        || display == Display::Flex
        || display == Display::ListItem
        || display == Display::Table;
}

inline bool BoxStyle::isDisplayInlineType(Display display)
{
    return display == Display::Inline
        || display == Display::InlineBlock
        || display == Display::InlineFlex
        || display == Display::InlineTable;
}

inline CSSValue* BoxStyle::get(CSSPropertyID id) const
{
    auto it = m_properties.find(id);
    if(it == m_properties.end())
        return nullptr;
    return it->second.get();
}

} // namespace plutobook

#endif // PLUTOBOOK_BOXSTYLE_H
