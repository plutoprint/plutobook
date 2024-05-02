#ifndef PLUTOBOOK_SVGPROPERTY_H
#define PLUTOBOOK_SVGPROPERTY_H

#include "geometry.h"

#include <string>

namespace plutobook {

class SVGProperty {
public:
    SVGProperty() = default;
    virtual ~SVGProperty() = default;
    virtual bool parse(std::string_view input) = 0;
};

class SVGString final : public SVGProperty {
public:
    SVGString() = default;
    explicit SVGString(const std::string& value) : m_value(value) {}

    const std::string& value() const { return m_value; }
    bool parse(std::string_view input) final;

private:
    std::string m_value;
};

using SVGEnumerationEntry = std::pair<int, std::string_view>;
using SVGEnumerationEntries = std::vector<SVGEnumerationEntry>;

class SVGEnumerationBase : public SVGProperty {
public:
    SVGEnumerationBase(int value, const SVGEnumerationEntries& entries)
        : m_value(value), m_entries(entries)
    {}

    int value() const { return m_value; }
    bool parse(std::string_view input) override;

protected:
    int m_value;
    const SVGEnumerationEntries& m_entries;
};

template<typename Enum>
const SVGEnumerationEntries& getEnumerationEntries();

enum SVGUnitsType {
    SVGUnitsTypeUserSpaceOnUse,
    SVGUnitsTypeObjectBoundingBox
};

template<>
const SVGEnumerationEntries& getEnumerationEntries<SVGUnitsType>();

enum SVGMarkerUnitsType {
    SVGMarkerUnitsTypeUserSpaceOnUse,
    SVGMarkerUnitsTypeStrokeWidth
};

template<>
const SVGEnumerationEntries& getEnumerationEntries<SVGMarkerUnitsType>();

enum SVGSpreadMethodType {
    SVGSpreadMethodTypePad,
    SVGSpreadMethodTypeReflect,
    SVGSpreadMethodTypeRepeat
};

template<>
const SVGEnumerationEntries& getEnumerationEntries<SVGSpreadMethodType>();

template<typename Enum>
class SVGEnumeration final : public SVGEnumerationBase {
public:
    explicit SVGEnumeration(Enum value)
        : SVGEnumerationBase(value, getEnumerationEntries<Enum>())
    {}

    Enum value() const { return static_cast<Enum>(m_value); }
};

class SVGAngle final : public SVGProperty {
public:
    enum class OrientType {
        Auto,
        AutoStartReverse,
        Angle
    };

    SVGAngle() = default;
    SVGAngle(float value, OrientType orientType)
        : m_value(value), m_orientType(orientType)
    {}

    float value() const { return m_value; }
    OrientType orientType() const { return m_orientType; }
    bool parse(std::string_view input) final;

private:
    float m_value = 0.f;
    OrientType m_orientType = OrientType::Angle;
};

class SVGLength final : public SVGProperty {
public:
    enum class UnitType : uint8_t {
        Number,
        Percent,
        Em,
        Ex,
        Px,
        Cm,
        Mm,
        In,
        Pt,
        Pc,
        Rem,
        Ch
    };

    enum class Direction : uint8_t {
        Horizontal,
        Vertical,
        Diagonal
    };

    enum class NegativeMode : uint8_t {
        Allow,
        Forbid
    };

    SVGLength(Direction direction, NegativeMode negativeMode)
        : SVGLength(0.f, UnitType::Number, direction, negativeMode)
    {}

    SVGLength(float value, UnitType unitType, Direction direction, NegativeMode negativeMode)
        : m_value(value), m_unitType(unitType), m_direction(direction), m_negativeMode(negativeMode)
    {}

    float value() const { return m_value; }
    UnitType unitType() const { return m_unitType; }
    Direction direction() const { return m_direction; }
    NegativeMode negativeMode() const { return m_negativeMode; }
    bool parse(std::string_view input) final;

private:
    float m_value;
    UnitType m_unitType;
    const Direction m_direction;
    const NegativeMode m_negativeMode;
};

class Length;
class SVGElement;

class SVGLengthContext {
public:
    SVGLengthContext(const SVGElement* element, SVGUnitsType unitType = SVGUnitsTypeUserSpaceOnUse)
        : m_element(element), m_unitType(unitType)
    {}

    float valueForLength(const SVGLength& length) const;
    float valueForLength(const Length& length, SVGLength::Direction direction = SVGLength::Direction::Diagonal) const;

private:
    float viewportDimension(SVGLength::Direction direction) const;
    const SVGElement* m_element;
    const SVGUnitsType m_unitType;
};

class SVGLengthList final : public SVGProperty {
public:
    SVGLengthList(SVGLength::Direction direction, SVGLength::NegativeMode negativeMode)
        : m_direction(direction), m_negativeMode(negativeMode)
    {}

    const std::vector<SVGLength>& values() const { return m_values; }
    SVGLength::Direction direction() const { return m_direction; }
    SVGLength::NegativeMode negativeMode() const { return m_negativeMode; }
    bool parse(std::string_view input) final;

private:
    std::vector<SVGLength> m_values;
    const SVGLength::Direction m_direction;
    const SVGLength::NegativeMode m_negativeMode;
};

class SVGNumber : public SVGProperty {
public:
    SVGNumber() = default;
    explicit SVGNumber(float value) : m_value(value) {}

    float value() const { return m_value; }
    bool parse(std::string_view input) override;

protected:
    float m_value = 0.f;
};

class SVGNumberPercentage final : public SVGNumber {
public:
    SVGNumberPercentage() = default;
    explicit SVGNumberPercentage(float value) : SVGNumber(value) {}

    bool parse(std::string_view input) final;
};

class SVGNumberList final : public SVGProperty {
public:
    SVGNumberList() = default;

    const std::vector<float>& values() const { return m_values; }
    bool parse(std::string_view input) final;

private:
    std::vector<float> m_values;
};

class SVGPath final : public SVGProperty {
public:
    SVGPath() = default;
    explicit SVGPath(const Path& value) : m_value(value) {}

    const Path& value() const { return m_value; }
    bool parse(std::string_view input) final;

private:
    Path m_value;
};

class SVGPoint final : public SVGProperty {
public:
    SVGPoint() = default;
    explicit SVGPoint(const Point& value) : m_value(value) {}

    const Point& value() const { return m_value; }
    bool parse(std::string_view input) final;

private:
    Point m_value;
};

class SVGPointList final : public SVGProperty {
public:
    SVGPointList() = default;

    const std::vector<Point>& values() const { return m_values; }
    bool parse(std::string_view input) final;

private:
    std::vector<Point> m_values;
};

class SVGRect final : public SVGProperty {
public:
    SVGRect() = default;
    explicit SVGRect(const Rect& value) : m_value(value) {}

    const Rect& value() const { return m_value; }
    bool parse(std::string_view input) final;

private:
    Rect m_value = Rect::Invalid;
};

class SVGTransform final : public SVGProperty {
public:
    SVGTransform() = default;
    explicit SVGTransform(const Transform& value) : m_value(value) {}

    const Transform& value() const { return m_value; }
    bool parse(std::string_view input) final;

private:
    Transform m_value;
};

class SVGPreserveAspectRatio final : public SVGProperty {
public:
    enum class AlignType {
        None,
        xMinYMin,
        xMidYMin,
        xMaxYMin,
        xMinYMid,
        xMidYMid,
        xMaxYMid,
        xMinYMax,
        xMidYMax,
        xMaxYMax
    };

    enum class MeetOrSlice {
        Meet,
        Slice
    };

    SVGPreserveAspectRatio() = default;
    SVGPreserveAspectRatio(AlignType alignType, MeetOrSlice meetOrSlice)
        : m_alignType(alignType), m_meetOrSlice(meetOrSlice)
    {}

    AlignType alignType() const { return m_alignType; }
    MeetOrSlice meetOrSlice() const { return m_meetOrSlice; }
    bool parse(std::string_view input) final;

    Rect getClipRect(const Rect& viewBoxRect, const Size& viewportSize) const;
    Transform getTransform(const Rect& viewBoxRect, const Size& viewportSize) const;
    void transformRect(Rect& dstRect, Rect& srcRect) const;

private:
    AlignType m_alignType = AlignType::xMidYMid;
    MeetOrSlice m_meetOrSlice = MeetOrSlice::Meet;
};

} // namespace plutobook

#endif // PLUTOBOOK_SVGPROPERTY_H
