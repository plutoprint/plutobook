#ifndef PLUTOBOOK_CSSRULE_H
#define PLUTOBOOK_CSSRULE_H

#include "pointer.h"
#include "globalstring.h"
#include "color.h"
#include "url.h"

#include <map>
#include <forward_list>
#include <memory>
#include <numbers>

namespace plutobook {

class Document;

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
    Attr,
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
    Counter,
    Counters,
    Cover,
    CurrentColor,
    Cyclic,
    Darken,
    Dashed,
    DiagonalFractions,
    Difference,
    DiscretionaryLigatures,
    Dotted,
    Double,
    Ellipsis,
    Embed,
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
    Format,
    FullWidth,
    Groove,
    Hanging,
    HardLight,
    Hidden,
    Hide,
    HistoricalLigatures,
    Hue,
    Ideographic,
    Infinite,
    Inherit,
    Initial,
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
    Leader,
    Ledger,
    Left,
    Legal,
    Length,
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
    Matrix,
    MaxContent,
    Medium,
    Middle,
    MinContent,
    Miter,
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
    Rotate,
    Round,
    Row,
    RowReverse,
    Rtl,
    Ruby,
    Saturation,
    Scale,
    ScaleDown,
    ScaleX,
    ScaleY,
    Screen,
    Scroll,
    SemiCondensed,
    SemiExpanded,
    Separate,
    Show,
    Simplified,
    Skew,
    SkewX,
    SkewY,
    SlashedZero,
    Small,
    SmallCaps,
    Smaller,
    SoftLight,
    Solid,
    Space,
    SpaceAround,
    SpaceBetween,
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
    TargetCounter,
    TargetCounters,
    TextAfterEdge,
    TextBeforeEdge,
    TextBottom,
    TextTop,
    Thick,
    Thin,
    TitlingCaps,
    Top,
    Traditional,
    Translate,
    TranslateX,
    TranslateY,
    UltraCondensed,
    UltraExpanded,
    Underline,
    Unicase,
    Uppercase,
    UseScript,
    Verso,
    Visible,
    Wavy,
    Wrap,
    WrapReverse,
    XLarge,
    XSmall,
    XxLarge,
    XxSmall,
    XxxLarge
};

enum class CSSValueType {
    Initial,
    Inherit,
    Ident,
    CustomIdent,
    Integer,
    Number,
    Percent,
    Angle,
    Length,
    String,
    LocalUrl,
    Url,
    Image,
    Color,
    Counter,
    FontFeature,
    FontVariation,
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
    static RefPtr<CSSInitialValue> create(Heap* heap);

    CSSValueType type() const final { return CSSValueType::Initial; }

private:
    CSSInitialValue() = default;
};

inline RefPtr<CSSInitialValue> CSSInitialValue::create(Heap* heap)
{
    return adoptPtr(new (heap) CSSInitialValue);
}

template<>
struct is_a<CSSInitialValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Initial; }
};

class CSSInheritValue final : public CSSValue {
public:
    static RefPtr<CSSInheritValue> create(Heap* heap);

    CSSValueType type() const final { return CSSValueType::Inherit; }

private:
    CSSInheritValue() = default;
};

inline RefPtr<CSSInheritValue> CSSInheritValue::create(Heap* heap)
{
    return adoptPtr(new (heap) CSSInheritValue);
}

template<>
struct is_a<CSSInheritValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Inherit; }
};

class CSSIdentValue final : public CSSValue {
public:
    static RefPtr<CSSIdentValue> create(Heap* heap, CSSValueID value);

    CSSValueID value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::Ident; }

private:
    CSSIdentValue(CSSValueID value) : m_value(value) {}
    CSSValueID m_value;
};

inline RefPtr<CSSIdentValue> CSSIdentValue::create(Heap* heap, CSSValueID value)
{
    return adoptPtr(new (heap) CSSIdentValue(value));
}

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
    CSSCustomIdentValue(const HeapString& value) : m_value(value) {}
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
    enum class Unit {
        Degrees,
        Radians,
        Gradians,
        Turns
    };

    static RefPtr<CSSAngleValue> create(Heap* heap, float value, Unit unit);

    float value() const { return m_value; }
    Unit unit() const { return m_unit; }
    CSSValueType type() const final { return CSSValueType::Angle; }

    float valueInDegrees() const;

private:
    CSSAngleValue(float value, Unit unit)
        : m_value(value), m_unit(unit)
    {}

    float m_value;
    Unit m_unit;
};

inline float CSSAngleValue::valueInDegrees() const
{
    switch(m_unit) {
    case CSSAngleValue::Unit::Degrees:
        return m_value;
    case CSSAngleValue::Unit::Radians:
        return m_value * 180.0 / std::numbers::pi;
    case CSSAngleValue::Unit::Gradians:
        return m_value * 360.0 / 400.0;
    case CSSAngleValue::Unit::Turns:
        return m_value * 360.0;
    default:
        assert(false);
    }

    return 0.0;
}

inline RefPtr<CSSAngleValue> CSSAngleValue::create(Heap* heap, float value, Unit unit)
{
    return adoptPtr(new (heap) CSSAngleValue(value, unit));
}

template<>
struct is_a<CSSAngleValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Angle; }
};

class CSSLengthValue final : public CSSValue {
public:
    enum class Units {
        None,
        Pixels,
        Points,
        Picas,
        Centimeters,
        Millimeters,
        Inches,
        ViewportWidth,
        ViewportHeight,
        ViewportMin,
        ViewportMax,
        Ems,
        Exs,
        Chs,
        Rems
    };

    static RefPtr<CSSLengthValue> create(Heap* heap, float value, Units units);

    float value() const { return m_value; }
    Units units() const { return m_units; }
    CSSValueType type() const final { return CSSValueType::Length; }

private:
    CSSLengthValue(float value, Units units)
        : m_value(value), m_units(units)
    {}

    float m_value;
    Units m_units;
};

inline RefPtr<CSSLengthValue> CSSLengthValue::create(Heap* heap, float value, Units units)
{
    return adoptPtr(new (heap) CSSLengthValue(value, units));
}

template<>
struct is_a<CSSLengthValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Length; }
};

class Font;

class CSSLengthResolver {
public:
    CSSLengthResolver(const Document* document, const Font* font);

    float resolveLength(const CSSValue& value) const;

private:
    float emFontSize() const;
    float exFontSize() const;
    float chFontSize() const;
    float remFontSize() const;

    float viewportWidth() const;
    float viewportHeight() const;
    float viewportMin() const;
    float viewportMax() const;

    const Document* m_document;
    const Font* m_font;
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
    const RefPtr<Image>& image() const { return m_image; }
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

class CSSFunctionValue final : public CSSListValue {
public:
    static RefPtr<CSSFunctionValue> create(Heap* heap, CSSValueID id, CSSValueList values);

    CSSValueID id() const { return m_id; }
    CSSValueType type() const final { return CSSValueType::Function; }

private:
    CSSFunctionValue(CSSValueID id, CSSValueList values)
        : CSSListValue(std::move(values))
        , m_id(id)
    {}

    CSSValueID m_id;
};

inline RefPtr<CSSFunctionValue> CSSFunctionValue::create(Heap* heap, CSSValueID id, CSSValueList values)
{
    return adoptPtr(new (heap) CSSFunctionValue(id, std::move(values)));
}

template<>
struct is_a<CSSFunctionValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::Function; }
};

class CSSUnaryFunctionValue final : public CSSValue {
public:
    static RefPtr<CSSUnaryFunctionValue> create(Heap* heap, CSSValueID id, RefPtr<CSSValue> value);

    CSSValueID id() const { return m_id; }
    const RefPtr<CSSValue>& value() const { return m_value; }
    CSSValueType type() const final { return CSSValueType::UnaryFunction; }

private:
    CSSUnaryFunctionValue(CSSValueID id, RefPtr<CSSValue> value)
        : m_id(id), m_value(std::move(value))
    {}

    CSSValueID m_id;
    RefPtr<CSSValue> m_value;
};

inline RefPtr<CSSUnaryFunctionValue> CSSUnaryFunctionValue::create(Heap* heap, CSSValueID id, RefPtr<CSSValue> value)
{
    return adoptPtr(new (heap) CSSUnaryFunctionValue(id, std::move(value)));
}

template<>
struct is_a<CSSUnaryFunctionValue> {
    static bool check(const CSSValue& value) { return value.type() == CSSValueType::UnaryFunction; }
};

enum class CSSPropertyID : uint16_t {
    Unknown,
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
    Border,
    BorderBottom,
    BorderBottomColor,
    BorderBottomLeftRadius,
    BorderBottomRightRadius,
    BorderBottomStyle,
    BorderBottomWidth,
    BorderCollapse,
    BorderColor,
    BorderHorizontalSpacing,
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
    FontVariantLigatures,
    FontVariantNumeric,
    FontVariantPosition,
    FontVariationSettings,
    FontWeight,
    Height,
    Hyphens,
    JustifyContent,
    Left,
    LetterSpacing,
    LineHeight,
    ListStyle,
    ListStyleImage,
    ListStylePosition,
    ListStyleType,
    Margin,
    MarginBottom,
    MarginLeft,
    MarginRight,
    MarginTop,
    Marker,
    MarkerEnd,
    MarkerMid,
    MarkerStart,
    Mask,
    MaskType,
    MaxHeight,
    MaxWidth,
    MinHeight,
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
    Pad,
    Padding,
    PaddingBottom,
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
    TextOverflow,
    TextTransform,
    Top,
    Transform,
    TransformOrigin,
    UnicodeBidi,
    VectorEffect,
    VerticalAlign,
    Visibility,
    WhiteSpace,
    Widows,
    Width,
    WordBreak,
    WordSpacing,
    X,
    Y,
    ZIndex
};

enum class CSSStyleOrigin : uint8_t {
    UserAgent,
    PresentationAttribute,
    Author,
    Inline,
    User
};

class CSSProperty {
public:
    CSSProperty(CSSPropertyID id, CSSStyleOrigin origin, bool important, RefPtr<CSSValue> value)
        : m_id(id), m_origin(origin), m_important(important), m_value(std::move(value))
    {}

    CSSPropertyID id() const { return m_id; }
    CSSStyleOrigin origin() const { return m_origin; }
    bool important() const { return m_important; }
    const RefPtr<CSSValue>& value() const { return m_value; }

private:
    CSSPropertyID m_id;
    CSSStyleOrigin m_origin;
    bool m_important;
    RefPtr<CSSValue> m_value;
};

using CSSPropertyList = std::pmr::vector<CSSProperty>;

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
        PseudoClassHover,
        PseudoClassIs,
        PseudoClassLang,
        PseudoClassLastChild,
        PseudoClassLastOfType,
        PseudoClassLink,
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
    CSSSimpleSelector(MatchType matchType, const MatchPattern& matchPattern) : m_matchType(matchType), m_matchPattern(matchPattern) {}
    CSSSimpleSelector(MatchType matchType, CSSSelectorList subSelectors) : m_matchType(matchType), m_subSelectors(std::move(subSelectors)) {}
    CSSSimpleSelector(MatchType matchType, AttributeCaseType attributeCaseType, const GlobalString& name, const HeapString& value)
        : m_matchType(matchType), m_attributeCaseType(attributeCaseType), m_name(name), m_value(value)
    {}

    MatchType matchType() const { return m_matchType; }
    AttributeCaseType attributeCaseType() const { return m_attributeCaseType; }
    const MatchPattern& matchPattern() const { return m_matchPattern; }
    const GlobalString& name() const { return m_name; }
    const HeapString& value() const { return m_value; }
    const CSSSelectorList& subSelectors() const { return m_subSelectors; }
    bool isCaseSensitive() const { return m_attributeCaseType == AttributeCaseType::Sensitive; }

    bool matchnth(int count) const;
    PseudoType pseudoType() const;
    uint32_t specificity() const;

private:
    MatchType m_matchType;
    AttributeCaseType m_attributeCaseType;
    MatchPattern m_matchPattern;
    GlobalString m_name;
    HeapString m_value;
    CSSSelectorList m_subSelectors;
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
        : m_combinator(combinator), m_compoundSelector(std::move(compoundSelector))
    {}

    Combinator combinator() const { return m_combinator; }
    const CSSCompoundSelector& compoundSelector() const { return m_compoundSelector; }

private:
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
    enum class Type {
        None,
        All,
        Print,
        Screen
    };

    enum class Restrictor {
        None,
        Only,
        Not
    };

    CSSMediaQuery(Type type, Restrictor restrictor, CSSMediaFeatureList features)
        : m_type(type), m_restrictor(restrictor), m_features(std::move(features))
    {}

    Type type() const { return m_type; }
    Restrictor restrictor() const { return m_restrictor; }
    const CSSMediaFeatureList& features() const { return m_features; }

private:
    Type m_type;
    Restrictor m_restrictor;
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

class CSSRuleData {
public:
    CSSRuleData(const RefPtr<CSSStyleRule>& rule, const CSSSelector& selector, uint32_t specificity, uint32_t position)
        : m_rule(rule), m_selector(&selector), m_specificity(specificity), m_position(position)
    {}

    const RefPtr<CSSStyleRule>& rule() const { return m_rule; }
    const CSSSelector* selector() const { return m_selector; }
    const CSSPropertyList& properties() const { return m_rule->properties(); }
    const uint32_t specificity() const { return m_specificity; }
    const uint32_t position() const { return m_position; }

    bool match(const Element* element, PseudoType pseudoType) const;

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

    static bool matchPseudoClassLinkSelector(const Element* element, const CSSSimpleSelector& selector);
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

    RefPtr<CSSStyleRule> m_rule;
    const CSSSelector* m_selector;
    uint32_t m_specificity;
    uint32_t m_position;
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
