/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_CSSPROPERTY_H
#define PLUTOBOOK_CSSPROPERTY_H

#include "pointer.h"
#include "globalstring.h"
#include "csstokenizer.h"
#include "color.h"
#include "url.h"

#include <numbers>
#include <span>
#include <vector>
#include <set>

namespace plutobook {

enum class CSSPropertyID : uint16_t {
    Unknown,
    Custom,
    AdditiveSymbols,
    AlignContent,
    AlignItems,
    AlignSelf,
    AlignmentBaseline,
    Background,
    BackgroundAttachment,
    BackgroundClip,
    BackgroundColor,
    BackgroundImage,
    BackgroundOrigin,
    BackgroundPosition,
    BackgroundRepeat,
    BackgroundSize,
    BaselineShift,
    BlockSize,
    Border,
    BorderBlock,
    BorderBlockColor,
    BorderBlockEnd,
    BorderBlockEndColor,
    BorderBlockEndStyle,
    BorderBlockEndWidth,
    BorderBlockStart,
    BorderBlockStartColor,
    BorderBlockStartStyle,
    BorderBlockStartWidth,
    BorderBlockStyle,
    BorderBlockWidth,
    BorderBottom,
    BorderBottomColor,
    BorderBottomLeftRadius,
    BorderBottomRightRadius,
    BorderBottomStyle,
    BorderBottomWidth,
    BorderCollapse,
    BorderColor,
    BorderEndEndRadius,
    BorderEndStartRadius,
    BorderHorizontalSpacing,
    BorderInline,
    BorderInlineColor,
    BorderInlineEnd,
    BorderInlineEndColor,
    BorderInlineEndStyle,
    BorderInlineEndWidth,
    BorderInlineStart,
    BorderInlineStartColor,
    BorderInlineStartStyle,
    BorderInlineStartWidth,
    BorderInlineStyle,
    BorderInlineWidth,
    BorderLeft,
    BorderLeftColor,
    BorderLeftStyle,
    BorderLeftWidth,
    BorderRadius,
    BorderRight,
    BorderRightColor,
    BorderRightStyle,
    BorderRightWidth,
    BorderSpacing,
    BorderStartEndRadius,
    BorderStartStartRadius,
    BorderStyle,
    BorderTop,
    BorderTopColor,
    BorderTopLeftRadius,
    BorderTopRightRadius,
    BorderTopStyle,
    BorderTopWidth,
    BorderVerticalSpacing,
    BorderWidth,
    Bottom,
    BoxSizing,
    BreakAfter,
    BreakBefore,
    BreakInside,
    CaptionSide,
    Clear,
    Clip,
    ClipPath,
    ClipRule,
    Color,
    ColumnBreakAfter,
    ColumnBreakBefore,
    ColumnBreakInside,
    ColumnCount,
    ColumnFill,
    ColumnGap,
    ColumnRule,
    ColumnRuleColor,
    ColumnRuleStyle,
    ColumnRuleWidth,
    ColumnSpan,
    ColumnWidth,
    Columns,
    Content,
    CounterIncrement,
    CounterReset,
    CounterSet,
    Cx,
    Cy,
    Direction,
    Display,
    DominantBaseline,
    EmptyCells,
    Fallback,
    Fill,
    FillOpacity,
    FillRule,
    Flex,
    FlexBasis,
    FlexDirection,
    FlexFlow,
    FlexGrow,
    FlexShrink,
    FlexWrap,
    Float,
    Font,
    FontFamily,
    FontFeatureSettings,
    FontKerning,
    FontSize,
    FontStretch,
    FontStyle,
    FontVariant,
    FontVariantCaps,
    FontVariantEastAsian,
    FontVariantEmoji,
    FontVariantLigatures,
    FontVariantNumeric,
    FontVariantPosition,
    FontVariationSettings,
    FontWeight,
    Gap,
    Height,
    Hyphens,
    InlineSize,
    Inset,
    InsetBlock,
    InsetBlockEnd,
    InsetBlockStart,
    InsetInline,
    InsetInlineEnd,
    InsetInlineStart,
    JustifyContent,
    Lang,
    Left,
    LetterSpacing,
    LineHeight,
    ListStyle,
    ListStyleImage,
    ListStylePosition,
    ListStyleType,
    Margin,
    MarginBlock,
    MarginBlockEnd,
    MarginBlockStart,
    MarginBottom,
    MarginInline,
    MarginInlineEnd,
    MarginInlineStart,
    MarginLeft,
    MarginRight,
    MarginTop,
    Marker,
    MarkerEnd,
    MarkerMid,
    MarkerStart,
    Mask,
    MaskType,
    MaxBlockSize,
    MaxHeight,
    MaxInlineSize,
    MaxWidth,
    MinBlockSize,
    MinHeight,
    MinInlineSize,
    MinWidth,
    MixBlendMode,
    Negative,
    ObjectFit,
    ObjectPosition,
    Opacity,
    Order,
    Orientation,
    Orphans,
    Outline,
    OutlineColor,
    OutlineOffset,
    OutlineStyle,
    OutlineWidth,
    Overflow,
    OverflowWrap,
    Pad,
    Padding,
    PaddingBlock,
    PaddingBlockEnd,
    PaddingBlockStart,
    PaddingBottom,
    PaddingInline,
    PaddingInlineEnd,
    PaddingInlineStart,
    PaddingLeft,
    PaddingRight,
    PaddingTop,
    Page,
    PageBreakAfter,
    PageBreakBefore,
    PageBreakInside,
    PageScale,
    PaintOrder,
    Position,
    Prefix,
    Quotes,
    R,
    Range,
    Right,
    RowGap,
    Rx,
    Ry,
    Size,
    Src,
    StopColor,
    StopOpacity,
    Stroke,
    StrokeDasharray,
    StrokeDashoffset,
    StrokeLinecap,
    StrokeLinejoin,
    StrokeMiterlimit,
    StrokeOpacity,
    StrokeWidth,
    Suffix,
    Symbols,
    System,
    TabSize,
    TableLayout,
    TextAlign,
    TextAnchor,
    TextDecoration,
    TextDecorationColor,
    TextDecorationLine,
    TextDecorationStyle,
    TextIndent,
    TextOrientation,
    TextOverflow,
    TextTransform,
    Top,
    Transform,
    TransformOrigin,
    UnicodeBidi,
    UnicodeRange,
    VectorEffect,
    VerticalAlign,
    Visibility,
    WhiteSpace,
    Widows,
    Width,
    WordBreak,
    WordSpacing,
    WritingMode,
    X,
    Y,
    ZIndex
};

using CSSShorthand = std::span<const CSSPropertyID>;

enum class CSSStyleOrigin : uint8_t {
    UserAgent,
    PresentationAttribute,
    Author,
    Inline,
    User
};

class CSSValue;

class CSSProperty {
public:
    CSSProperty(CSSPropertyID id, CSSStyleOrigin origin, bool important, RefPtr<CSSValue> value)
        : m_id(id), m_origin(origin), m_important(important), m_value(std::move(value))
    {}

    CSSPropertyID id() const { return m_id; }
    CSSStyleOrigin origin() const { return m_origin; }
    bool important() const { return m_important; }
    const RefPtr<CSSValue>& value() const { return m_value; }

    static CSSPropertyID id(std::string_view name);
    static CSSShorthand shorthand(CSSPropertyID id);

private:
    CSSPropertyID m_id;
    CSSStyleOrigin m_origin;
    bool m_important;
    RefPtr<CSSValue> m_value;
};

using CSSPropertyList = std::pmr::vector<CSSProperty>;

enum class CSSValueID : uint16_t {
    Unknown,
    A3,
    A4,
    A5,
    Absolute,
    Additive,
    AfterEdge,
    All,
    AllPetiteCaps,
    AllSmallCaps,
    Alpha,
    Alphabetic,
    Anywhere,
    Auto,
    Avoid,
    AvoidColumn,
    AvoidPage,
    B4,
    B5,
    Balance,
    Baseline,
    BeforeEdge,
    Bevel,
    BidiOverride,
    Block,
    Bold,
    Bolder,
    BorderBox,
    Both,
    Bottom,
    BreakAll,
    BreakWord,
    Butt,
    Capitalize,
    Center,
    Central,
    Circle,
    Clip,
    CloseQuote,
    Collapse,
    Color,
    ColorBurn,
    ColorDodge,
    Column,
    ColumnReverse,
    CommonLigatures,
    Condensed,
    Contain,
    ContentBox,
    Contextual,
    Cover,
    CurrentColor,
    Cyclic,
    Darken,
    Dashed,
    DiagonalFractions,
    Difference,
    Disc,
    DiscretionaryLigatures,
    Dotted,
    Double,
    Ellipsis,
    Embed,
    Emoji,
    End,
    Evenodd,
    Exclusion,
    Expanded,
    Extends,
    ExtraCondensed,
    ExtraExpanded,
    Fill,
    FitContent,
    Fixed,
    Flex,
    FlexEnd,
    FlexStart,
    FullWidth,
    Groove,
    Hanging,
    HardLight,
    Hidden,
    Hide,
    HistoricalLigatures,
    HorizontalTb,
    Hue,
    Ideographic,
    Infinite,
    Inline,
    InlineBlock,
    InlineFlex,
    InlineTable,
    Inset,
    Inside,
    Isolate,
    IsolateOverride,
    Italic,
    Jis04,
    Jis78,
    Jis83,
    Jis90,
    Justify,
    KeepAll,
    Landscape,
    Large,
    Larger,
    Ledger,
    Left,
    Legal,
    Letter,
    Lighten,
    Lighter,
    LineThrough,
    LiningNums,
    ListItem,
    Local,
    Lowercase,
    Ltr,
    Luminance,
    Luminosity,
    Manual,
    Markers,
    Mathematical,
    MaxContent,
    Medium,
    Middle,
    MinContent,
    Miter,
    Mixed,
    Multiply,
    NoChange,
    NoCloseQuote,
    NoCommonLigatures,
    NoContextual,
    NoDiscretionaryLigatures,
    NoHistoricalLigatures,
    NoOpenQuote,
    NoRepeat,
    NonScalingStroke,
    None,
    Nonzero,
    Normal,
    Nowrap,
    Numeric,
    Oblique,
    Off,
    OldstyleNums,
    On,
    OpenQuote,
    Ordinal,
    Outset,
    Outside,
    Overlay,
    Overline,
    PaddingBox,
    Page,
    PetiteCaps,
    Portrait,
    Pre,
    PreLine,
    PreWrap,
    ProportionalNums,
    ProportionalWidth,
    Recto,
    Relative,
    Repeat,
    RepeatX,
    RepeatY,
    ResetSize,
    Ridge,
    Right,
    Round,
    Row,
    RowReverse,
    Rtl,
    Ruby,
    Saturation,
    ScaleDown,
    Screen,
    Scroll,
    SemiCondensed,
    SemiExpanded,
    Separate,
    Show,
    Simplified,
    SlashedZero,
    Small,
    SmallCaps,
    Smaller,
    SoftLight,
    Solid,
    Space,
    SpaceAround,
    SpaceBetween,
    SpaceEvenly,
    Square,
    StackedFractions,
    Start,
    Static,
    Stretch,
    Stroke,
    Sub,
    Super,
    Symbolic,
    Table,
    TableCaption,
    TableCell,
    TableColumn,
    TableColumnGroup,
    TableFooterGroup,
    TableHeaderGroup,
    TableRow,
    TableRowGroup,
    TabularNums,
    Text,
    TextAfterEdge,
    TextBeforeEdge,
    TextBottom,
    TextTop,
    Thick,
    Thin,
    TitlingCaps,
    Top,
    Traditional,
    UltraCondensed,
    UltraExpanded,
    Underline,
    Unicase,
    Unicode,
    Uppercase,
    Upright,
    UseScript,
    Verso,
    VerticalLr,
    VerticalRl,
    Visible,
    Wavy,
    Wrap,
    WrapReverse,
    XLarge,
    XSmall,
    XxLarge,
    XxSmall,
    XxxLarge,
    MaxCSSValueID
};

constexpr auto kNumCSSValueIDs = static_cast<int>(CSSValueID::MaxCSSValueID);

enum class CSSValueType {
    Initial,
    Inherit,
    Unset,
    Ident,
    CustomIdent,
    CustomProperty,
    VariableReference,
    Integer,
    Number,
    Percent,
    Angle,
    Length,
    Calc,
    Attr,
    String,
    LocalUrl,
    Url,
    Image,
    Color,
    Counter,
    FontFeature,
    FontVariation,
    UnicodeRange,
    Pair,
    Rect,
    List,
    Function,
    UnaryFunction
};

class CSSValue : public HeapMember, public RefCounted<CSSValue> {
public:
    CSSValue() = default;
    virtual ~CSSValue() = default;
    virtual CSSValueType type() const = 0;
    CSSValueID id() const;
};

using CSSValueList = std::pmr::vector<RefPtr<CSSValue>>;

class CSSInitialValue final : public CSSValue {
public:
    static RefPtr<CSSInitialValue> create();

    CSSValueType type() const final { return CSSValueType::Initial; }

private:
    CSSInitialValue() = default;
    friend class CSSValuePool;
};

template<>
struct is_a<CSSInitialValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Initial; }
};

class CSSInheritValue final : public CSSValue {
public:
    static RefPtr<CSSInheritValue> create();

    CSSValueType type() const final { return CSSValueType::Inherit; }

private:
    CSSInheritValue() = default;
    friend class CSSValuePool;
};

template<>
struct is_a<CSSInheritValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Inherit; }
};

class CSSUnsetValue final : public CSSValue {
public:
    static RefPtr<CSSUnsetValue> create();

    CSSValueType type() const final { return CSSValueType::Unset; }

private:
    CSSUnsetValue() = default;
    friend class CSSValuePool;
};

template<>
struct is_a<CSSUnsetValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Unset; }
};

class CSSIdentValue final : public CSSValue {
public:
    static RefPtr<CSSIdentValue> create(CSSValueID value);

    CSSValueID value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::Ident; }

private:
    CSSIdentValue(CSSValueID value) : m_value(value) {}
    CSSValueID m_value;
    friend class CSSValuePool;
};

template<>
struct is_a<CSSIdentValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Ident; }
};

inline CSSValueID CSSValue::id() const
{
    if(auto ident = to<CSSIdentValue>(this))
        return ident->value();
    return CSSValueID::Unknown;
}

class CSSCustomIdentValue final : public CSSValue {
public:
    static RefPtr<CSSCustomIdentValue> create(Heap* heap, const GlobalString& value);

    const GlobalString& value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::CustomIdent; }

private:
    CSSCustomIdentValue(const GlobalString& value) : m_value(value) {}
    GlobalString m_value;
};

inline RefPtr<CSSCustomIdentValue> CSSCustomIdentValue::create(Heap* heap, const GlobalString& value)
{
    return adoptPtr(new (heap) CSSCustomIdentValue(value));
}

template<>
struct is_a<CSSCustomIdentValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::CustomIdent; }
};

class BoxStyle;

class CSSVariableData : public HeapMember, public RefCounted<CSSVariableData> {
public:
    static RefPtr<CSSVariableData> create(Heap* heap, const CSSTokenStream& value);

    bool resolve(const BoxStyle* style, CSSTokenList& tokens, std::set<CSSVariableData*>& references) const;

private:
    CSSVariableData(Heap* heap, const CSSTokenStream& value);
    bool resolve(CSSTokenStream input, const BoxStyle* style, CSSTokenList& tokens, std::set<CSSVariableData*>& references) const;
    bool resolveVar(CSSTokenStream input, const BoxStyle* style, CSSTokenList& tokens, std::set<CSSVariableData*>& references) const;
    std::pmr::vector<CSSToken> m_tokens;
};

class CSSCustomPropertyValue final : public CSSValue {
public:
    static RefPtr<CSSCustomPropertyValue> create(Heap* heap, const HeapString& name, RefPtr<CSSVariableData> value);

    const HeapString& name() const { return m_name; }
    const RefPtr<CSSVariableData>& value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::CustomProperty; }

private:
    CSSCustomPropertyValue(const HeapString& name, RefPtr<CSSVariableData> value);
    HeapString m_name;
    RefPtr<CSSVariableData> m_value;
};

template<>
struct is_a<CSSCustomPropertyValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::CustomProperty; }
};

class Node;

class CSSParserContext {
public:
    CSSParserContext(const Node* node, CSSStyleOrigin origin, Url baseUrl);

    bool inHTMLDocument() const { return m_inHTMLDocument; }
    bool inSVGElement() const { return m_inSVGElement; }

    CSSStyleOrigin origin() const { return m_origin; }
    const Url& baseUrl() const { return m_baseUrl; }

    Url completeUrl(std::string_view url) const { return m_baseUrl.complete(url); }

private:
    bool m_inHTMLDocument;
    bool m_inSVGElement;
    CSSStyleOrigin m_origin;
    Url m_baseUrl;
};

class CSSVariableReferenceValue final : public CSSValue {
public:
    static RefPtr<CSSVariableReferenceValue> create(Heap* heap, const CSSParserContext& context, CSSPropertyID id, bool important, RefPtr<CSSVariableData> value);

    const CSSParserContext& context() const { return m_context; }
    CSSPropertyID id() const { return m_id; }
    bool important() const { return m_important; }
    const RefPtr<CSSVariableData>& value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::VariableReference; }

    CSSPropertyList resolve(const BoxStyle* style) const;

private:
    CSSVariableReferenceValue(const CSSParserContext& context, CSSPropertyID id, bool important, RefPtr<CSSVariableData> value);
    CSSParserContext m_context;
    CSSPropertyID m_id;
    bool m_important;
    RefPtr<CSSVariableData> m_value;
};

template<>
struct is_a<CSSVariableReferenceValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::VariableReference; }
};

class CSSIntegerValue final : public CSSValue {
public:
    static RefPtr<CSSIntegerValue> create(Heap* heap, int value);

    int value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::Integer; }

private:
    CSSIntegerValue(int value) : m_value(value) {}
    int m_value;
};

inline RefPtr<CSSIntegerValue> CSSIntegerValue::create(Heap* heap, int value)
{
    return adoptPtr(new (heap) CSSIntegerValue(value));
}

template<>
struct is_a<CSSIntegerValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Integer; }
};

class CSSNumberValue final : public CSSValue {
public:
    static RefPtr<CSSNumberValue> create(Heap* heap, float value);

    float value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::Number; }

private:
    CSSNumberValue(float value) : m_value(value) {}
    float m_value;
};

inline RefPtr<CSSNumberValue> CSSNumberValue::create(Heap* heap, float value)
{
    return adoptPtr(new (heap) CSSNumberValue(value));
}

template<>
struct is_a<CSSNumberValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Number; }
};

class CSSPercentValue final : public CSSValue {
public:
    static RefPtr<CSSPercentValue> create(Heap* heap, float value);

    float value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::Percent; }

private:
    CSSPercentValue(float value) : m_value(value) {}
    float m_value;
};

inline RefPtr<CSSPercentValue> CSSPercentValue::create(Heap* heap, float value)
{
    return adoptPtr(new (heap) CSSPercentValue(value));
}

template<>
struct is_a<CSSPercentValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Percent; }
};

class CSSAngleValue final : public CSSValue {
public:
    enum class Units {
        Degrees,
        Radians,
        Gradians,
        Turns
    };

    static RefPtr<CSSAngleValue> create(Heap* heap, float value, Units units);

    float value() const { return m_value; }
    Units units() const { return m_units; }
    CSSValueType type() const final { return CSSValueType::Angle; }

    float valueInDegrees() const;

private:
    CSSAngleValue(float value, Units units)
        : m_value(value), m_units(units)
    {}

    float m_value;
    Units m_units;
};

inline float CSSAngleValue::valueInDegrees() const
{
    switch(m_units) {
    case CSSAngleValue::Units::Degrees:
        return m_value;
    case CSSAngleValue::Units::Radians:
        return m_value * 180.0 / std::numbers::pi;
    case CSSAngleValue::Units::Gradians:
        return m_value * 360.0 / 400.0;
    case CSSAngleValue::Units::Turns:
        return m_value * 360.0;
    default:
        assert(false);
    }

    return 0.0;
}

inline RefPtr<CSSAngleValue> CSSAngleValue::create(Heap* heap, float value, Units units)
{
    return adoptPtr(new (heap) CSSAngleValue(value, units));
}

template<>
struct is_a<CSSAngleValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Angle; }
};

enum class CSSLengthUnits : uint8_t {
    None,
    Pixels,
    Points,
    Picas,
    Inches,
    Centimeters,
    Millimeters,
    ViewportWidth,
    ViewportHeight,
    ViewportMin,
    ViewportMax,
    Ems,
    Rems,
    Exs,
    Rexs,
    Chs,
    Rchs,
    Ics,
    Rics,
    Caps,
    Rcaps,
    Lhs,
    Rlhs
};

class CSSLengthValue final : public CSSValue {
public:
    static RefPtr<CSSLengthValue> create(Heap* heap, float value, CSSLengthUnits units = CSSLengthUnits::Pixels);

    float value() const { return m_value; }
    CSSLengthUnits units() const { return m_units; }
    CSSValueType type() const final { return CSSValueType::Length; }

private:
    CSSLengthValue(float value, CSSLengthUnits units)
        : m_value(value), m_units(units)
    {}

    float m_value;
    CSSLengthUnits m_units;
};

inline RefPtr<CSSLengthValue> CSSLengthValue::create(Heap* heap, float value, CSSLengthUnits units)
{
    return adoptPtr(new (heap) CSSLengthValue(value, units));
}

template<>
struct is_a<CSSLengthValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Length; }
};

class Document;

class CSSLengthResolver {
public:
    explicit CSSLengthResolver(const BoxStyle* style);
    explicit CSSLengthResolver(const Document* document);

    float resolveLength(const CSSValue& value) const;
    float resolveLength(const CSSLengthValue& length) const;
    float resolveLength(float value, CSSLengthUnits units) const;

private:
    float emFontSize() const;
    float remFontSize() const;

    float exFontSize() const;
    float rexFontSize() const;

    float chFontSize() const;
    float rchFontSize() const;

    float icFontSize() const;
    float ricFontSize() const;

    float capFontSize() const;
    float rcapFontSize() const;

    float lineHeight() const;
    float rootLineHeight() const;

    float viewportWidth() const;
    float viewportHeight() const;

    float viewportMin() const;
    float viewportMax() const;

    const BoxStyle* m_style;
    const Document* m_document;
};

enum class CSSCalcOperator : uint8_t {
    None,
    Add,
    Sub,
    Mul,
    Div,
    Min,
    Max
};

struct CSSCalc {
    CSSCalc() = default;
    explicit CSSCalc(CSSCalcOperator op) : op(op) {}
    CSSCalc(float value, CSSLengthUnits units = CSSLengthUnits::None)
        : value(value), units(units)
    {}

    float value = 0;
    CSSLengthUnits units = CSSLengthUnits::None;
    CSSCalcOperator op = CSSCalcOperator::None;
};

using CSSCalcList = std::pmr::vector<CSSCalc>;

class CSSCalcValue final : public CSSValue {
public:
    static RefPtr<CSSCalcValue> create(Heap* heap, bool negative, bool unitless, CSSCalcList values);

    const bool negative() const { return m_negative; }
    const bool unitless() const { return m_unitless; }
    const CSSCalcList& values() const { return m_values; }
    CSSValueType type() const final { return CSSValueType::Calc; }

    std::optional<CSSCalc> resolve(const CSSLengthResolver& resolver) const;

private:
    CSSCalcValue(bool negative, bool unitless, CSSCalcList values)
        : m_negative(negative), m_unitless(unitless)
        , m_values(std::move(values))
    {}

    const bool m_negative;
    const bool m_unitless;
    CSSCalcList m_values;
};

inline RefPtr<CSSCalcValue> CSSCalcValue::create(Heap* heap, bool negative, bool unitless, CSSCalcList values)
{
    return adoptPtr(new (heap) CSSCalcValue(negative, unitless, std::move(values)));
}

template<>
struct is_a<CSSCalcValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Calc; }
};

class CSSAttrValue final : public CSSValue {
public:
    static RefPtr<CSSAttrValue> create(Heap* heap, const GlobalString& name, const HeapString& fallback);

    const GlobalString& name() const { return m_name; }
    const HeapString& fallback() const { return m_fallback; }
    CSSValueType type() const final { return CSSValueType::Attr; }

private:
    CSSAttrValue(const GlobalString& name, const HeapString& fallback)
        : m_name(name), m_fallback(fallback)
    {}

    GlobalString m_name;
    HeapString m_fallback;
};

inline RefPtr<CSSAttrValue> CSSAttrValue::create(Heap* heap, const GlobalString& name, const HeapString& fallback)
{
    return adoptPtr(new (heap) CSSAttrValue(name, fallback));
}

template<>
struct is_a<CSSAttrValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Attr; }
};

class CSSStringValue final : public CSSValue {
public:
    static RefPtr<CSSStringValue> create(Heap* heap, const HeapString& value);

    const HeapString& value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::String; }

private:
    CSSStringValue(const HeapString& value) : m_value(value) {}
    HeapString m_value;
};

inline RefPtr<CSSStringValue> CSSStringValue::create(Heap* heap, const HeapString& value)
{
    return adoptPtr(new (heap) CSSStringValue(value));
}

template<>
struct is_a<CSSStringValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::String; }
};

class CSSLocalUrlValue final : public CSSValue {
public:
    static RefPtr<CSSLocalUrlValue> create(Heap* heap, const HeapString& value);

    const HeapString& value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::LocalUrl; }

private:
    CSSLocalUrlValue(const HeapString& value) : m_value(value) {}
    HeapString m_value;
};

inline RefPtr<CSSLocalUrlValue> CSSLocalUrlValue::create(Heap* heap, const HeapString& value)
{
    return adoptPtr(new (heap) CSSLocalUrlValue(value));
}

template<>
struct is_a<CSSLocalUrlValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::LocalUrl; }
};

class CSSUrlValue final : public CSSValue {
public:
    static RefPtr<CSSUrlValue> create(Heap* heap, Url value);

    const Url& value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::Url; }

private:
    CSSUrlValue(Url value) : m_value(std::move(value)) {}
    Url m_value;
};

inline RefPtr<CSSUrlValue> CSSUrlValue::create(Heap* heap, Url value)
{
    return adoptPtr(new (heap) CSSUrlValue(std::move(value)));
}

template<>
struct is_a<CSSUrlValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Url; }
};

class Image;

class CSSImageValue final : public CSSValue {
public:
    static RefPtr<CSSImageValue> create(Heap* heap, Url value);

    const Url& value() const { return m_value; }
    const RefPtr<Image>& fetch(Document* document) const;
    CSSValueType type() const final { return CSSValueType::Image; }

private:
    CSSImageValue(Url value);
    Url m_value;
    mutable RefPtr<Image> m_image;
};

template<>
struct is_a<CSSImageValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Image; }
};

class CSSColorValue final : public CSSValue {
public:
    static RefPtr<CSSColorValue> create(Heap* heap, const Color& value);

    const Color& value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::Color; }

private:
    CSSColorValue(Color value) : m_value(value) {}
    Color m_value;
};

inline RefPtr<CSSColorValue> CSSColorValue::create(Heap* heap, const Color& value)
{
    return adoptPtr(new (heap) CSSColorValue(value));
}

template<>
struct is_a<CSSColorValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Color; }
};

class CSSCounterValue final : public CSSValue {
public:
    static RefPtr<CSSCounterValue> create(Heap* heap, const GlobalString& identifier, const GlobalString& listStyle, const HeapString& separator);

    const GlobalString& identifier() const { return m_identifier; }
    const GlobalString& listStyle() const { return m_listStyle; }
    const HeapString& separator() const { return m_separator; }
    CSSValueType type() const final { return CSSValueType::Counter; }

private:
    CSSCounterValue(const GlobalString& identifier, const GlobalString& listStyle, const HeapString& separator)
        : m_identifier(identifier), m_listStyle(listStyle), m_separator(separator)
    {}

    GlobalString m_identifier;
    GlobalString m_listStyle;
    HeapString m_separator;
};

inline RefPtr<CSSCounterValue> CSSCounterValue::create(Heap* heap, const GlobalString& identifier, const GlobalString& listStyle, const HeapString& separator)
{
    return adoptPtr(new (heap) CSSCounterValue(identifier, listStyle, separator));
}

template<>
struct is_a<CSSCounterValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Counter; }
};

class CSSFontFeatureValue : public CSSValue {
public:
    static RefPtr<CSSFontFeatureValue> create(Heap* heap, const GlobalString& tag, int value);

    const GlobalString& tag() const { return m_tag; }
    int value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::FontFeature; }

private:
    CSSFontFeatureValue(const GlobalString& tag, int value)
        : m_tag(tag), m_value(value)
    {}

    GlobalString m_tag;
    int m_value;
};

inline RefPtr<CSSFontFeatureValue> CSSFontFeatureValue::create(Heap* heap, const GlobalString& tag, int value)
{
    return adoptPtr(new (heap) CSSFontFeatureValue(tag, value));
}

template<>
struct is_a<CSSFontFeatureValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::FontFeature; }
};

class CSSFontVariationValue : public CSSValue {
public:
    static RefPtr<CSSFontVariationValue> create(Heap* heap, const GlobalString& tag, float value);

    const GlobalString& tag() const { return m_tag; }
    float value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::FontVariation; }

private:
    CSSFontVariationValue(const GlobalString& tag, float value)
        : m_tag(tag), m_value(value)
    {}

    GlobalString m_tag;
    float m_value;
};

inline RefPtr<CSSFontVariationValue> CSSFontVariationValue::create(Heap* heap, const GlobalString& tag, float value)
{
    return adoptPtr(new (heap) CSSFontVariationValue(tag, value));
}

template<>
struct is_a<CSSFontVariationValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::FontVariation; }
};

class CSSUnicodeRangeValue : public CSSValue {
public:
    static RefPtr<CSSUnicodeRangeValue> create(Heap* heap, uint32_t from, uint32_t to);

    uint32_t from() const { return m_from; }
    uint32_t to() const { return m_to; }
    CSSValueType type() const final { return CSSValueType::UnicodeRange; }

private:
    CSSUnicodeRangeValue(uint32_t from, uint32_t to)
        : m_from(from), m_to(to)
    {}

    uint32_t m_from;
    uint32_t m_to;
};

inline RefPtr<CSSUnicodeRangeValue> CSSUnicodeRangeValue::create(Heap* heap, uint32_t from, uint32_t to)
{
    return adoptPtr(new (heap) CSSUnicodeRangeValue(from, to));
}

template<>
struct is_a<CSSUnicodeRangeValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::UnicodeRange; }
};

class CSSPairValue final : public CSSValue {
public:
    static RefPtr<CSSPairValue> create(Heap* heap, RefPtr<CSSValue> first, RefPtr<CSSValue> second);

    const RefPtr<CSSValue>& first() const { return m_first; }
    const RefPtr<CSSValue>& second() const { return m_second; }
    CSSValueType type() const final { return CSSValueType::Pair; }

private:
    CSSPairValue(RefPtr<CSSValue> first, RefPtr<CSSValue> second)
        : m_first(std::move(first)), m_second(std::move(second))
    {}

    RefPtr<CSSValue> m_first;
    RefPtr<CSSValue> m_second;
};

inline RefPtr<CSSPairValue> CSSPairValue::create(Heap* heap, RefPtr<CSSValue> first, RefPtr<CSSValue> second)
{
    return adoptPtr(new (heap) CSSPairValue(std::move(first), std::move(second)));
}

template<>
struct is_a<CSSPairValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Pair; }
};

class CSSRectValue final : public CSSValue {
public:
    static RefPtr<CSSRectValue> create(Heap* heap, RefPtr<CSSValue> top, RefPtr<CSSValue> right, RefPtr<CSSValue> bottom, RefPtr<CSSValue> left);

    const RefPtr<CSSValue>& top() const { return m_top; }
    const RefPtr<CSSValue>& right() const { return m_right; }
    const RefPtr<CSSValue>& bottom() const { return m_bottom; }
    const RefPtr<CSSValue>& left() const { return m_left; }
    CSSValueType type() const final { return CSSValueType::Rect; }

private:
    CSSRectValue(RefPtr<CSSValue> top, RefPtr<CSSValue> right, RefPtr<CSSValue> bottom, RefPtr<CSSValue> left)
        : m_top(std::move(top)), m_right(std::move(right)), m_bottom(std::move(bottom)), m_left(std::move(left))
    {}

    RefPtr<CSSValue> m_top;
    RefPtr<CSSValue> m_right;
    RefPtr<CSSValue> m_bottom;
    RefPtr<CSSValue> m_left;
};

inline RefPtr<CSSRectValue> CSSRectValue::create(Heap* heap, RefPtr<CSSValue> top, RefPtr<CSSValue> right, RefPtr<CSSValue> bottom, RefPtr<CSSValue> left)
{
    return adoptPtr(new (heap) CSSRectValue(std::move(top), std::move(right), std::move(bottom), std::move(left)));
}

template<>
struct is_a<CSSRectValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Rect; }
};

class CSSListValue : public CSSValue {
public:
    static RefPtr<CSSListValue> create(Heap* heap, CSSValueList values);

    using Iterator = CSSValueList::const_iterator;
    Iterator begin() const { return m_values.begin(); }
    Iterator end() const { return m_values.end(); }

    const RefPtr<CSSValue>& front() const { return m_values.front(); }
    const RefPtr<CSSValue>& back() const { return m_values.back(); }
    const RefPtr<CSSValue>& at(size_t index) const { return m_values.at(index); }
    const CSSValueList& values() const { return m_values; }
    size_t size() const { return m_values.size(); }
    bool empty() const { return m_values.empty(); }
    CSSValueType type() const override { return CSSValueType::List; }

protected:
    CSSListValue(CSSValueList values) : m_values(std::move(values)) {}
    CSSValueList m_values;
};

inline RefPtr<CSSListValue> CSSListValue::create(Heap* heap, CSSValueList values)
{
    return adoptPtr(new (heap) CSSListValue(std::move(values)));
}

template<>
struct is_a<CSSListValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::List; }
};

enum class CSSFunctionID {
    Element,
    Format,
    Leader,
    Local,
    Matrix,
    Qrcode,
    Rotate,
    Running,
    Scale,
    ScaleX,
    ScaleY,
    Skew,
    SkewX,
    SkewY,
    TargetCounter,
    TargetCounters,
    Translate,
    TranslateX,
    TranslateY
};

class CSSFunctionValue final : public CSSListValue {
public:
    static RefPtr<CSSFunctionValue> create(Heap* heap, CSSFunctionID id, CSSValueList values);

    CSSFunctionID id() const { return m_id; }
    CSSValueType type() const final { return CSSValueType::Function; }

private:
    CSSFunctionValue(CSSFunctionID id, CSSValueList values)
        : CSSListValue(std::move(values))
        , m_id(id)
    {}

    CSSFunctionID m_id;
};

inline RefPtr<CSSFunctionValue> CSSFunctionValue::create(Heap* heap, CSSFunctionID id, CSSValueList values)
{
    return adoptPtr(new (heap) CSSFunctionValue(id, std::move(values)));
}

template<>
struct is_a<CSSFunctionValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Function; }
};

class CSSUnaryFunctionValue final : public CSSValue {
public:
    static RefPtr<CSSUnaryFunctionValue> create(Heap* heap, CSSFunctionID id, RefPtr<CSSValue> value);

    CSSFunctionID id() const { return m_id; }
    const RefPtr<CSSValue>& value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::UnaryFunction; }

private:
    CSSUnaryFunctionValue(CSSFunctionID id, RefPtr<CSSValue> value)
        : m_id(id), m_value(std::move(value))
    {}

    CSSFunctionID m_id;
    RefPtr<CSSValue> m_value;
};

inline RefPtr<CSSUnaryFunctionValue> CSSUnaryFunctionValue::create(Heap* heap, CSSFunctionID id, RefPtr<CSSValue> value)
{
    return adoptPtr(new (heap) CSSUnaryFunctionValue(id, std::move(value)));
}

template<>
struct is_a<CSSUnaryFunctionValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::UnaryFunction; }
};

} // namespace plutobook

#endif // PLUTOBOOK_CSSPROPERTY_H
