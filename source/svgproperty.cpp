/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "svgproperty.h"
#include "svgdocument.h"
#include "boxstyle.h"

#include <cmath>

namespace plutobook {

constexpr bool IS_NUM(int cc) { return cc >= '0' && cc <= '9'; }
constexpr bool IS_ALPHA(int cc) { return (cc >= 'a' && cc <= 'z') || (cc >= 'A' && cc <= 'Z'); }
constexpr bool IS_WS(int cc) { return cc == ' ' || cc == '\t' || cc == '\n' || cc == '\r'; }

constexpr void skipLeadingSpaces(std::string_view& input)
{
    while(!input.empty() && IS_WS(input.front())) {
        input.remove_prefix(1);
    }
}

constexpr void skipTrailingSpaces(std::string_view& input)
{
    while(!input.empty() && IS_WS(input.back())) {
        input.remove_suffix(1);
    }
}

constexpr void skipLeadingAndTrailingSpaces(std::string_view& input)
{
    skipLeadingSpaces(input);
    skipTrailingSpaces(input);
}

constexpr bool skipOptionalSpaces(std::string_view& input)
{
    while(!input.empty() && IS_WS(input.front()))
        input.remove_prefix(1);
    return !input.empty();
}

constexpr bool skipOptionalSpacesOrDelimiter(std::string_view& input, char delimiter)
{
    if(!input.empty() && !IS_WS(input.front()) && delimiter != input.front())
        return false;
    if(skipOptionalSpaces(input)) {
        if(delimiter == input.front()) {
            input.remove_prefix(1);
            skipOptionalSpaces(input);
        }
    }

    return !input.empty();
}

constexpr bool skipOptionalSpacesOrComma(std::string_view& input)
{
    return skipOptionalSpacesOrDelimiter(input, ',');
}

constexpr bool skipString(std::string_view& input, const std::string_view& value)
{
    if(input.size() >= value.size() && value == input.substr(0, value.size())) {
        input.remove_prefix(value.size());
        return true;
    }

    return false;
}

template<typename T>
static bool parseNumber(std::string_view& input, T& output)
{
    constexpr T maxValue = std::numeric_limits<T>::max();
    T integer = 0;
    T fraction = 0;
    int exponent = 0;
    int sign = 1;
    int expsign = 1;

    if(!input.empty() && input.front() == '+') {
        input.remove_prefix(1);
    } else if(!input.empty() && input.front() == '-') {
        input.remove_prefix(1);
        sign = -1;
    }

    if(input.empty() || (!IS_NUM(input.front()) && input.front() != '.'))
        return false;
    if(IS_NUM(input.front())) {
        do {
            integer = static_cast<T>(10) * integer + (input.front() - '0');
            input.remove_prefix(1);
        } while(!input.empty() && IS_NUM(input.front()));
    }

    if(!input.empty() && input.front() == '.') {
        input.remove_prefix(1);
        if(input.empty() || !IS_NUM(input.front()))
            return false;
        T divisor = static_cast<T>(1);
        do {
            fraction = static_cast<T>(10) * fraction + (input.front() - '0');
            divisor *= static_cast<T>(10);
            input.remove_prefix(1);
        } while(!input.empty() && IS_NUM(input.front()));
        fraction /= divisor;
    }

    if(input.size() > 1 && (input[0] == 'e' || input[0] == 'E')
        && (input[1] != 'x' && input[1] != 'm'))
    {
        input.remove_prefix(1);
        if(!input.empty() && input.front() == '+')
            input.remove_prefix(1);
        else if(!input.empty() && input.front() == '-') {
            input.remove_prefix(1);
            expsign = -1;
        }

        if(input.empty() || !IS_NUM(input.front()))
            return false;
        do {
            exponent = 10 * exponent + (input.front() - '0');
            input.remove_prefix(1);
        } while(!input.empty() && IS_NUM(input.front()));
    }

    output = sign * (integer + fraction);
    if(exponent)
        output *= std::pow<T>(10, expsign * exponent);
    return output >= -maxValue && output <= maxValue;
}

bool SVGString::parse(std::string_view input)
{
    m_value.assign(input);
    return true;
}

bool SVGEnumerationBase::parse(std::string_view input)
{
    skipLeadingAndTrailingSpaces(input);
    for(const auto& entry : m_entries) {
        if(input == entry.second) {
            m_value = entry.first;
            return true;
        }
    }

    return false;
}

template<>
const SVGEnumerationEntries& getEnumerationEntries<SVGUnitsType>()
{
    static const SVGEnumerationEntries entries = {
        {SVGUnitsTypeUserSpaceOnUse, "userSpaceOnUse"},
        {SVGUnitsTypeObjectBoundingBox, "objectBoundingBox"}
    };

    return entries;
}

template<>
const SVGEnumerationEntries& getEnumerationEntries<SVGMarkerUnitsType>()
{
    static const SVGEnumerationEntries entries = {
        {SVGMarkerUnitsTypeUserSpaceOnUse, "userSpaceOnUse"},
        {SVGMarkerUnitsTypeStrokeWidth, "strokeWidth"}
    };

    return entries;
}

template<>
const SVGEnumerationEntries& getEnumerationEntries<SVGSpreadMethodType>()
{
    static const SVGEnumerationEntries entries = {
        {SVGSpreadMethodTypePad, "pad"},
        {SVGSpreadMethodTypeReflect, "reflect"},
        {SVGSpreadMethodTypeRepeat, "repeat"}
    };

    return entries;
}

bool SVGAngle::parse(std::string_view input)
{
    skipLeadingAndTrailingSpaces(input);
    if(input == "auto") {
        m_value = 0.f;
        m_orientType = OrientType::Auto;
        return true;
    }

    if(input == "auto-start-reverse") {
        m_value = 0.f;
        m_orientType = OrientType::AutoStartReverse;
        return true;
    }

    float value = 0.f;
    if(!parseNumber(input, value))
        return false;
    if(!input.empty()) {
        if(input == "rad")
            value *= 180.f / kPi;
        else if(input == "grad")
            value *= 360.f / 400.f;
        else if(input == "turn")
            value *= 360.f;
        else if(input != "deg") {
            return false;
        }
    }

    m_value = value;
    m_orientType = OrientType::Angle;
    return true;
}

bool SVGLength::parse(std::string_view input)
{
    float value = 0.f;
    skipLeadingAndTrailingSpaces(input);
    if(!parseNumber(input, value))
        return false;
    if(value < 0.f && m_negativeMode == SVGLengthNegativeValuesMode::Forbid)
        return false;
    if(input.empty()) {
        m_value = value;
        m_type = SVGLengthType::Number;
        return true;
    }

    static const struct {
        std::string_view name;
        SVGLengthType type;
    } entries[] = {
        {"%", SVGLengthType::Percentage},
        {"em", SVGLengthType::Ems},
        {"ex", SVGLengthType::Exs},
        {"px", SVGLengthType::Pixels},
        {"cm", SVGLengthType::Centimeters},
        {"mm", SVGLengthType::Millimeters},
        {"in", SVGLengthType::Inches},
        {"pt", SVGLengthType::Points},
        {"pc", SVGLengthType::Picas},
        {"rem", SVGLengthType::Rems},
        {"ch", SVGLengthType::Chs}
    };

    for(const auto& entry : entries) {
        if(input == entry.name) {
            m_value = value;
            m_type = entry.type;
            return true;
        }
    }

    return false;
}

static const BoxStyle* styleForLengthResolving(const Element* element)
{
    const ContainerNode* current = element;
    while(current) {
        if(auto style = current->style())
            return style;
        current = current->parentNode();
    }

    return nullptr;
}

float SVGLengthContext::valueForLength(const SVGLength& length) const
{
    if(length.type() == SVGLengthType::Percentage) {
        if(m_unitType == SVGUnitsTypeUserSpaceOnUse)
            return length.value() * viewportDimension(length.direction()) / 100.f;
        return length.value() / 100.f;
    }

    constexpr auto dpi = 96.f;
    switch(length.type()) {
    case SVGLengthType::Number:
    case SVGLengthType::Pixels:
        return length.value();
    case SVGLengthType::Inches:
        return length.value() * dpi;
    case SVGLengthType::Centimeters:
        return length.value() * dpi / 2.54f;
    case SVGLengthType::Millimeters:
        return length.value() * dpi / 25.4f;
    case SVGLengthType::Points:
        return length.value() * dpi / 72.f;
    case SVGLengthType::Picas:
        return length.value() * dpi / 6.f;
    default:
        break;
    }

    if(auto style = styleForLengthResolving(m_element)) {
        switch(length.type()) {
        case SVGLengthType::Ems:
            return length.value() * style->fontSize();
        case SVGLengthType::Exs:
            return length.value() * style->exFontSize();
        case SVGLengthType::Rems:
            return length.value() * style->remFontSize();
        case SVGLengthType::Chs:
            return length.value() * style->chFontSize();
        default:
            assert(false);
        }
    }

    return 0.f;
}

float SVGLengthContext::valueForLength(const Length& length, SVGLengthDirection direction) const
{
    if(length.isPercent()) {
        if(m_unitType == SVGUnitsTypeUserSpaceOnUse)
            return length.value() * viewportDimension(direction) / 100.f;
        return length.value() / 100.f;
    }

    if(length.isFixed())
        return length.value();
    return 0.f;
}

float SVGLengthContext::viewportDimension(SVGLengthDirection direction) const
{
    auto viewportSize = m_element->currentViewportSize();
    switch(direction) {
    case SVGLengthDirection::Horizontal:
        return viewportSize.w;
    case SVGLengthDirection::Vertical:
        return viewportSize.h;
    default:
        return std::sqrt(viewportSize.w * viewportSize.w + viewportSize.h * viewportSize.h) / kSqrt2;
    }
}

bool SVGLengthList::parse(std::string_view input)
{
    m_values.clear();
    skipLeadingSpaces(input);
    while(!input.empty()) {
        size_t count = 0;
        while(count < input.length() && input[count] != ',' && !IS_WS(input[count]))
            ++count;
        if(count == 0)
            break;
        SVGLength value(m_direction, m_negativeMode);
        if(!value.parse(input.substr(0, count)))
            return false;
        input.remove_prefix(count);
        skipOptionalSpacesOrComma(input);
        m_values.push_back(std::move(value));
    }

    return true;
}

bool SVGNumber::parse(std::string_view input)
{
    float value = 0.f;
    skipLeadingAndTrailingSpaces(input);
    if(!parseNumber(input, value))
        return false;
    if(!input.empty())
        return false;
    m_value = value;
    return true;
}

bool SVGNumberPercentage::parse(std::string_view input)
{
    float value = 0.f;
    skipLeadingAndTrailingSpaces(input);
    if(!parseNumber(input, value))
        return false;
    if(!input.empty() && input.front() == '%') {
        value /= 100.f;
        input.remove_prefix(1);
    }

    if(!input.empty())
        return false;
    m_value = value;
    return true;
}

bool SVGNumberList::parse(std::string_view input)
{
    m_values.clear();
    skipLeadingSpaces(input);
    while(!input.empty()) {
        float value = 0.f;
        if(!parseNumber(input, value))
            return false;
        skipOptionalSpaces(input);
        m_values.push_back(value);
    }

    return true;
}

static void decomposeQuadToCubic(Path& path, const Point& currentPoint, float x1, float y1, float x2, float y2)
{
    auto cp1x = 2.f / 3.f * x1 + 1.f / 3.f * currentPoint.x;
    auto cp1y = 2.f / 3.f * y1 + 1.f / 3.f * currentPoint.y;
    auto cp2x = 2.f / 3.f * x1 + 1.f / 3.f * x2;
    auto cp2y = 2.f / 3.f * y1 + 1.f / 3.f * y2;

    path.cubicTo(cp1x, cp1y, cp2x, cp2y, x2, y2);
}

static void decomposeArcSegmentToCubic(Path& path, float xc, float yc, float th0, float th1, float rx, float ry, float sinTh, float cosTh)
{
    auto a00 =  cosTh * rx;
    auto a01 = -sinTh * ry;
    auto a10 =  sinTh * rx;
    auto a11 =  cosTh * ry;

    auto thHalf = 0.5f * (th1 - th0);
    auto t = (8.f / 3.f) * std::sin(thHalf * 0.5f) * std::sin(thHalf * 0.5f) / std::sin(thHalf);

    auto x1 = xc + std::cos(th0) - t * std::sin(th0);
    auto y1 = yc + std::sin(th0) + t * std::cos(th0);

    auto x3 = xc + std::cos(th1);
    auto y3 = yc + std::sin(th1);

    auto x2 = x3 + t * std::sin(th1);
    auto y2 = y3 - t * std::cos(th1);

    auto cp1x = a00 * x1 + a01 * y1;
    auto cp1y = a10 * x1 + a11 * y1;
    auto cp2x = a00 * x2 + a01 * y2;
    auto cp2y = a10 * x2 + a11 * y2;
    auto cp3x = a00 * x3 + a01 * y3;
    auto cp3y = a10 * x3 + a11 * y3;

    path.cubicTo(cp1x, cp1y, cp2x, cp2y, cp3x, cp3y);
}

static void decomposeArcToCubic(Path& path, const Point& currentPoint, float rx, float ry, float xAxisRotation, bool largeArcFlag, bool sweepFlag, float x, float y)
{
    if(rx == 0.f || ry == 0.f || (currentPoint.x == x && currentPoint.y == y)) {
        path.lineTo(x, y);
        return;
    }

    if(rx < 0.f) rx = -rx;
    if(ry < 0.f) ry = -ry;

    auto sin_th = std::sin(deg2rad(xAxisRotation));
    auto cos_th = std::cos(deg2rad(xAxisRotation));

    auto dx = (currentPoint.x - x) / 2.f;
    auto dy = (currentPoint.y - y) / 2.f;

    auto dx1 = cos_th * dx + sin_th * dy;
    auto dy1 = -sin_th * dx + cos_th * dy;

    auto dx1dx1 = dx1 * dx1;
    auto dy1dy1 = dy1 * dy1;

    auto rxrx = rx * rx;
    auto ryry = ry * ry;

    auto check = dx1dx1 / rxrx + dy1dy1 / ryry;
    if(check > 1.f) {
        rx = rx * std::sqrt(check);
        ry = ry * std::sqrt(check);
    }

    auto a00 = cos_th / rx;
    auto a01 = sin_th / rx;
    auto a10 = -sin_th / ry;
    auto a11 = cos_th / ry;

    auto x0 = a00 * currentPoint.x + a01 * currentPoint.y;
    auto y0 = a10 * currentPoint.x + a11 * currentPoint.y;

    auto x1 = a00 * x + a01 * y;
    auto y1 = a10 * x + a11 * y;

    auto d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    auto sfactor_sq = 1.f / d - 0.25f;
    if(sfactor_sq < 0.f) sfactor_sq = 0.f;
    auto sfactor = std::sqrt(sfactor_sq);
    if(sweepFlag == largeArcFlag) {
        sfactor = -sfactor;
    }

    auto xc = 0.5f * (x0 + x1) - sfactor * (y1 - y0);
    auto yc = 0.5f * (y0 + y1) + sfactor * (x1 - x0);

    auto th0 = std::atan2(y0 - yc, x0 - xc);
    auto th1 = std::atan2(y1 - yc, x1 - xc);

    auto th_arc = th1 - th0;
    if(th_arc < 0.f && sweepFlag) {
        th_arc += kTwoPi;
    } else if(th_arc > 0.f && !sweepFlag) {
        th_arc -= kTwoPi;
    }

    int segments = std::ceil(std::fabs(th_arc / (kHalfPi + 0.001f)));
    for(int i = 0; i < segments; i++) {
        decomposeArcSegmentToCubic(path, xc, yc, th0 + i * th_arc / segments, th0 + (i + 1) * th_arc / segments, rx, ry, sin_th, cos_th);
    }
}

static bool parseNumberList(std::string_view& input, float values[6], int offset, int count)
{
    for(int i = 0; i < count; i++) {
        if(!parseNumber(input, values[i + offset]))
            return false;
        skipOptionalSpacesOrComma(input);
    }

    return true;
}

static bool parseArcFlag(std::string_view& input, bool& flag)
{
    if(!input.empty() && input.front() == '0')
        flag = false;
    else if(!input.empty() && input.front() == '1')
        flag = true;
    else
        return false;
    input.remove_prefix(1);
    skipOptionalSpacesOrComma(input);
    return true;
}

bool SVGPath::parse(std::string_view input)
{
    m_value.clear();
    skipLeadingSpaces(input);

    float values[6];
    bool flags[2];

    Point startPoint;
    Point currentPoint;
    Point controlPoint;

    int command = 0;
    int lastCommand = 0;
    while(!input.empty()) {
        if(IS_ALPHA(input.front())) {
            command = input.front();
            input.remove_prefix(1);
            skipOptionalSpaces(input);
        }

        if(!lastCommand && !(command == 'M' || command == 'm'))
            return false;
        if(command == 'M' || command == 'm') {
            if(!parseNumberList(input, values, 0, 2))
                return false;
            if(command == 'm') {
                values[0] += currentPoint.x;
                values[1] += currentPoint.y;
            }

            m_value.moveTo(values[0], values[1]);
            startPoint.x = currentPoint.x = values[0];
            startPoint.y = currentPoint.y = values[1];
            command = command == 'm' ? 'l' : 'L';
        } else if(command == 'L' || command == 'l') {
            if(!parseNumberList(input, values, 0, 2))
                return false;
            if(command == 'l') {
                values[0] += currentPoint.x;
                values[1] += currentPoint.y;
            }

            m_value.lineTo(values[0], values[1]);
            currentPoint.x = values[0];
            currentPoint.y = values[1];
        } else if(command == 'H' || command == 'h') {
            if(!parseNumberList(input, values, 0, 1))
                return false;
            if(command == 'h') {
                values[0] += currentPoint.x;
            }

            m_value.lineTo(values[0], currentPoint.y);
            currentPoint.x = values[0];
        } else if(command == 'V' || command == 'v') {
            if(!parseNumberList(input, values, 1, 1))
                return false;
            if(command == 'v') {
                values[1] += currentPoint.y;
            }

            m_value.lineTo(currentPoint.x, values[1]);
            currentPoint.y = values[1];
        } else if(command == 'Q' || command == 'q') {
            if(!parseNumberList(input, values, 0, 4))
                return false;
            if(command == 'q') {
                values[0] += currentPoint.x;
                values[1] += currentPoint.y;
                values[2] += currentPoint.x;
                values[3] += currentPoint.y;
            }

            decomposeQuadToCubic(m_value, currentPoint, values[0], values[1], values[2], values[3]);
            controlPoint.x = values[0];
            controlPoint.y = values[1];
            currentPoint.x = values[2];
            currentPoint.y = values[3];
        } else if(command == 'C' || command == 'c') {
            if(!parseNumberList(input, values, 0, 6))
                return false;
            if(command == 'c') {
                values[0] += currentPoint.x;
                values[1] += currentPoint.y;
                values[2] += currentPoint.x;
                values[3] += currentPoint.y;
                values[4] += currentPoint.x;
                values[5] += currentPoint.y;
            }

            m_value.cubicTo(values[0], values[1], values[2], values[3], values[4], values[5]);
            controlPoint.x = values[2];
            controlPoint.y = values[3];
            currentPoint.x = values[4];
            currentPoint.y = values[5];
        } else if(command == 'T' || command == 't') {
            if(lastCommand != 'Q' && lastCommand != 'q' && lastCommand != 'T' && lastCommand != 't') {
                values[0] = currentPoint.x;
                values[1] = currentPoint.y;
            } else {
                values[0] = 2.f * currentPoint.x - controlPoint.x;
                values[1] = 2.f * currentPoint.y - controlPoint.y;
            }

            if(!parseNumberList(input, values, 2, 2))
                return false;
            if(command == 't') {
                values[2] += currentPoint.x;
                values[3] += currentPoint.y;
            }

            decomposeQuadToCubic(m_value, currentPoint, values[0], values[1], values[2], values[3]);
            controlPoint.x = values[0];
            controlPoint.y = values[1];
            currentPoint.x = values[2];
            currentPoint.y = values[3];
        } else if(command == 'S' || command == 's') {
            if(lastCommand != 'C' && lastCommand != 'c' && lastCommand != 'S' && lastCommand != 's') {
                values[0] = currentPoint.x;
                values[1] = currentPoint.y;
            } else {
                values[0] = 2.f * currentPoint.x - controlPoint.x;
                values[1] = 2.f * currentPoint.y - controlPoint.y;
            }

            if(!parseNumberList(input, values, 2, 4))
                return false;
            if(command == 's') {
                values[2] += currentPoint.x;
                values[3] += currentPoint.y;
                values[4] += currentPoint.x;
                values[5] += currentPoint.y;
            }

            m_value.cubicTo(values[0], values[1], values[2], values[3], values[4], values[5]);
            controlPoint.x = values[2];
            controlPoint.y = values[3];
            currentPoint.x = values[4];
            currentPoint.y = values[5];
        } else if(command == 'A' || command == 'a') {
            if(!parseNumberList(input, values,0 , 3)
                || !parseArcFlag(input, flags[0])
                || !parseArcFlag(input, flags[1])
                || !parseNumberList(input, values, 3, 2)) {
                return false;
            }

            if(command == 'a') {
                values[3] += currentPoint.x;
                values[4] += currentPoint.y;
            }

            decomposeArcToCubic(m_value, currentPoint, values[0], values[1], values[2], flags[0], flags[1], values[3], values[4]);
            currentPoint.x = values[3];
            currentPoint.y = values[4];
        } else if(command == 'Z' || command == 'z') {
            if(lastCommand == 'Z' || lastCommand == 'z')
                return false;
            m_value.close();
            currentPoint.x = startPoint.x;
            currentPoint.y = startPoint.y;
        } else {
            return false;
        }

        lastCommand = command;
    }

    return true;
}

bool SVGPoint::parse(std::string_view input)
{
    Point value;
    skipLeadingAndTrailingSpaces(input);
    if(!parseNumber(input, value.x)
        || !skipOptionalSpaces(input)
        || !parseNumber(input, value.y)
        || !input.empty()) {
        return false;
    }

    m_value = value;
    return true;
}

bool SVGPointList::parse(std::string_view input)
{
    m_values.clear();
    skipLeadingSpaces(input);
    while(!input.empty()) {
        Point value;
        if(!parseNumber(input, value.x)
            || !skipOptionalSpacesOrComma(input)
            || !parseNumber(input, value.y)) {
            return false;
        }

        m_values.push_back(value);
        skipOptionalSpacesOrComma(input);
    }

    return true;
}

bool SVGRect::parse(std::string_view input)
{
    Rect value;
    skipLeadingAndTrailingSpaces(input);
    if(!parseNumber(input, value.x)
        || !skipOptionalSpacesOrComma(input)
        || !parseNumber(input, value.y)
        || !skipOptionalSpacesOrComma(input)
        || !parseNumber(input, value.w)
        || !skipOptionalSpacesOrComma(input)
        || !parseNumber(input, value.h)
        || !input.empty()) {
        return false;
    }

    if(value.w < 0.f || value.h < 0.f)
        return false;
    m_value = value;
    return true;
}

static int parseTransform(std::string_view& input, float values[6], int required, int optional)
{
    skipOptionalSpaces(input);
    if(input.empty() || input.front() != '(')
        return 0;
    input.remove_prefix(1);
    skipOptionalSpaces(input);

    int count = 0;
    int maxCount = required + optional;
    for(; count < maxCount; ++count) {
        if(!parseNumber(input, values[count]))
            break;
        skipOptionalSpacesOrComma(input);
    }

    if(input.empty() || input.front() != ')' || !(count == required || count == maxCount))
        return 0;
    input.remove_prefix(1);
    return count;
}

bool SVGTransform::parse(std::string_view input)
{
    m_value = Transform::Identity;
    float values[6];
    skipLeadingSpaces(input);
    while(!input.empty()) {
        if(skipString(input, "matrix")) {
            auto count = parseTransform(input, values, 6, 0);
            if(count == 0)
                return false;
            m_value.multiply(Transform(values[0], values[1], values[2], values[3], values[4], values[5]));
        } else if(skipString(input, "rotate")) {
            auto count = parseTransform(input, values, 1, 2);
            if(count == 0)
                return false;
            if(count == 1) {
                m_value.rotate(values[0], 0, 0);
            } else {
                m_value.rotate(values[0], values[1], values[2]);
            }
        } else if(skipString(input, "scale")) {
            auto count = parseTransform(input, values, 1, 1);
            if(count == 0)
                return false;
            if(count == 1) {
                m_value.scale(values[0], values[0]);
            } else {
                m_value.scale(values[0], values[1]);
            }
        } else if(skipString(input, "skewX")) {
            auto count = parseTransform(input, values, 1, 0);
            if(count == 0)
                return false;
            m_value.shear(values[0], 0);
        } else if(skipString(input, "skewY")) {
            auto count = parseTransform(input, values, 1, 0);
            if(count == 0)
                return false;
            m_value.shear(0, values[0]);
        } else if(skipString(input, "translate")) {
            auto count = parseTransform(input, values, 1, 1);
            if(count == 0)
                return false;
            if(count == 1) {
                m_value.translate(values[0], 0);
            } else {
                m_value.translate(values[0], values[1]);
            }
        } else {
            return false;
        }

        skipOptionalSpacesOrComma(input);
    }

    return true;
}

bool SVGPreserveAspectRatio::parse(std::string_view input)
{
    auto alignType = AlignType::xMidYMid;
    skipLeadingSpaces(input);
    if(skipString(input, "none"))
        alignType = AlignType::None;
    else if(skipString(input, "xMinYMin"))
        alignType = AlignType::xMinYMin;
    else if(skipString(input, "xMidYMin"))
        alignType = AlignType::xMidYMin;
    else if(skipString(input, "xMaxYMin"))
        alignType = AlignType::xMaxYMin;
    else if(skipString(input, "xMinYMid"))
        alignType = AlignType::xMinYMid;
    else if(skipString(input, "xMidYMid"))
        alignType = AlignType::xMidYMid;
    else if(skipString(input, "xMaxYMid"))
        alignType = AlignType::xMaxYMid;
    else if(skipString(input, "xMinYMax"))
        alignType = AlignType::xMinYMax;
    else if(skipString(input, "xMidYMax"))
        alignType = AlignType::xMidYMax;
    else if(skipString(input, "xMaxYMax"))
        alignType = AlignType::xMaxYMax;
    else {
        return false;
    }

    auto meetOrSlice = MeetOrSlice::Meet;
    skipOptionalSpaces(input);
    if(skipString(input, "meet")) {
        meetOrSlice = MeetOrSlice::Meet;
    } else if(skipString(input, "slice")) {
        meetOrSlice = MeetOrSlice::Slice;
    }

    if(alignType == AlignType::None)
        meetOrSlice = MeetOrSlice::Meet;
    skipOptionalSpaces(input);
    if(!input.empty())
        return false;
    m_alignType = alignType;
    m_meetOrSlice = meetOrSlice;
    return true;
}

Rect SVGPreserveAspectRatio::getClipRect(const Rect& viewBoxRect, const Size& viewportSize) const
{
    assert(!viewBoxRect.isEmpty() && !viewportSize.isEmpty());
    if(m_meetOrSlice == MeetOrSlice::Meet)
        return viewBoxRect;
    auto scale = std::max(viewportSize.w / viewBoxRect.w, viewportSize.h / viewBoxRect.h);
    auto xOffset = -viewBoxRect.x * scale;
    auto yOffset = -viewBoxRect.y * scale;
    auto viewWidth = viewBoxRect.w * scale;
    auto viewHeight = viewBoxRect.h * scale;
    switch(m_alignType) {
    case AlignType::xMidYMin:
    case AlignType::xMidYMid:
    case AlignType::xMidYMax:
        xOffset += (viewportSize.w - viewWidth) * 0.5f;
        break;
    case AlignType::xMaxYMin:
    case AlignType::xMaxYMid:
    case AlignType::xMaxYMax:
        xOffset += (viewportSize.w - viewWidth);
        break;
    default:
        break;
    }

    switch(m_alignType) {
    case AlignType::xMinYMid:
    case AlignType::xMidYMid:
    case AlignType::xMaxYMid:
        yOffset += (viewportSize.h - viewHeight) * 0.5f;
        break;
    case AlignType::xMinYMax:
    case AlignType::xMidYMax:
    case AlignType::xMaxYMax:
        yOffset += (viewportSize.h - viewHeight);
        break;
    default:
        break;
    }

    return Rect(-xOffset / scale, -yOffset / scale, viewportSize.w / scale, viewportSize.h / scale);
}

Transform SVGPreserveAspectRatio::getTransform(const Rect& viewBoxRect, const Size& viewportSize) const
{
    assert(!viewBoxRect.isEmpty() && !viewportSize.isEmpty());
    auto xScale = viewportSize.w / viewBoxRect.w;
    auto yScale = viewportSize.h / viewBoxRect.h;
    if(m_alignType == AlignType::None) {
        return Transform(xScale, 0, 0, yScale, -viewBoxRect.x * xScale, -viewBoxRect.y * yScale);
    }

    auto scale = (m_meetOrSlice == MeetOrSlice::Meet) ? std::min(xScale, yScale) : std::max(xScale, yScale);
    auto xOffset = -viewBoxRect.x * scale;
    auto yOffset = -viewBoxRect.y * scale;
    auto viewWidth = viewBoxRect.w * scale;
    auto viewHeight = viewBoxRect.h * scale;
    switch(m_alignType) {
    case AlignType::xMidYMin:
    case AlignType::xMidYMid:
    case AlignType::xMidYMax:
        xOffset += (viewportSize.w - viewWidth) * 0.5f;
        break;
    case AlignType::xMaxYMin:
    case AlignType::xMaxYMid:
    case AlignType::xMaxYMax:
        xOffset += (viewportSize.w - viewWidth);
        break;
    default:
        break;
    }

    switch(m_alignType) {
    case AlignType::xMinYMid:
    case AlignType::xMidYMid:
    case AlignType::xMaxYMid:
        yOffset += (viewportSize.h - viewHeight) * 0.5f;
        break;
    case AlignType::xMinYMax:
    case AlignType::xMidYMax:
    case AlignType::xMaxYMax:
        yOffset += (viewportSize.h - viewHeight);
        break;
    default:
        break;
    }

    return Transform(scale, 0, 0, scale, xOffset, yOffset);
}

void SVGPreserveAspectRatio::transformRect(Rect& dstRect, Rect& srcRect) const
{
    if(m_alignType == AlignType::None)
        return;
    auto viewSize = dstRect.size();
    auto imageSize = srcRect.size();
    if(m_meetOrSlice == MeetOrSlice::Meet) {
        auto scale = imageSize.h / imageSize.w;
        if(viewSize.h > viewSize.w * scale) {
            dstRect.h = viewSize.w * scale;
            switch(m_alignType) {
            case AlignType::xMinYMid:
            case AlignType::xMidYMid:
            case AlignType::xMaxYMid:
                dstRect.y += (viewSize.h - dstRect.h) * 0.5f;
                break;
            case AlignType::xMinYMax:
            case AlignType::xMidYMax:
            case AlignType::xMaxYMax:
                dstRect.y += viewSize.h - dstRect.h;
                break;
            default:
                break;
            }
        }

        if(viewSize.w > viewSize.h / scale) {
            dstRect.w = viewSize.h / scale;
            switch(m_alignType) {
            case AlignType::xMidYMin:
            case AlignType::xMidYMid:
            case AlignType::xMidYMax:
                dstRect.x += (viewSize.w - dstRect.w) * 0.5f;
                break;
            case AlignType::xMaxYMin:
            case AlignType::xMaxYMid:
            case AlignType::xMaxYMax:
                dstRect.x += viewSize.w - dstRect.w;
                break;
            default:
                break;
            }
        }
    } else if(m_meetOrSlice == MeetOrSlice::Slice) {
        auto scale = imageSize.h / imageSize.w;
        if(viewSize.h < viewSize.w * scale) {
            srcRect.h = viewSize.h * (imageSize.w / viewSize.w);
            switch(m_alignType) {
            case AlignType::xMinYMid:
            case AlignType::xMidYMid:
            case AlignType::xMaxYMid:
                srcRect.y += (imageSize.h - srcRect.h) * 0.5f;
                break;
            case AlignType::xMinYMax:
            case AlignType::xMidYMax:
            case AlignType::xMaxYMax:
                srcRect.y += imageSize.h - srcRect.h;
                break;
            default:
                break;
            }
        }

        if(viewSize.w < viewSize.h / scale) {
            srcRect.w = viewSize.w * (imageSize.h / viewSize.h);
            switch(m_alignType) {
            case AlignType::xMidYMin:
            case AlignType::xMidYMid:
            case AlignType::xMidYMax:
                srcRect.x += (imageSize.w - srcRect.w) * 0.5f;
                break;
            case AlignType::xMaxYMin:
            case AlignType::xMaxYMid:
            case AlignType::xMaxYMax:
                srcRect.x += imageSize.w - srcRect.w;
                break;
            default:
                break;
            }
        }
    }
}

} // namespace plutobook
