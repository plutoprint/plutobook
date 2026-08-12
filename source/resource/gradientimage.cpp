/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gradientimage.h"
#include "boxstyle.h"

#include <cairo.h>

#include <cmath>
#include <numbers>

namespace plutobook {

// The CSS gradient line is described by a fraction of its total length; a
// gradient whose stops all coincide is turned into a hard transition over
// this fraction, which is small enough to be invisible at any scale.
constexpr float kHardTransitionSpan = 0.0001f;

// Number of samples used to approximate a transition hint. Cairo has no
// notion of a color midpoint, so the non linear ramp it describes has to be
// flattened into ordinary stops.
constexpr int kHintSampleCount = 12;

// Number of samples used between two stops of differing opacity.
constexpr int kAlphaSampleCount = 12;

static Color interpolateColor(const Color& from, const Color& to, float t)
{
    auto fromAlpha = from.alpha() / 255.f;
    auto toAlpha = to.alpha() / 255.f;
    auto alpha = fromAlpha + (toAlpha - fromAlpha) * t;

    // CSS interpolates gradient colors in premultiplied sRGB, so that a
    // transition to a transparent color does not darken the ramp.
    auto interpolate = [&](uint8_t fromValue, uint8_t toValue) {
        auto fromPremultiplied = fromValue / 255.f * fromAlpha;
        auto toPremultiplied = toValue / 255.f * toAlpha;
        auto value = fromPremultiplied + (toPremultiplied - fromPremultiplied) * t;
        if(alpha > 0.f)
            value /= alpha;
        return static_cast<int>(std::lround(std::clamp(value, 0.f, 1.f) * 255.f));
    };

    return Color(interpolate(from.red(), to.red()), interpolate(from.green(), to.green()),
        interpolate(from.blue(), to.blue()), static_cast<int>(std::lround(std::clamp(alpha, 0.f, 1.f) * 255.f)));
}

static Color averageColor(const GradientStops& stops)
{
    float red = 0.f;
    float green = 0.f;
    float blue = 0.f;
    float alpha = 0.f;
    for(const auto& stop : stops) {
        auto stopAlpha = stop.second.alpha() / 255.f;
        red += stop.second.red() / 255.f * stopAlpha;
        green += stop.second.green() / 255.f * stopAlpha;
        blue += stop.second.blue() / 255.f * stopAlpha;
        alpha += stopAlpha;
    }

    if(alpha <= 0.f)
        return Color::Transparent;
    auto toByte = [](float value) { return static_cast<int>(std::lround(std::clamp(value, 0.f, 1.f) * 255.f)); };
    return Color(toByte(red / alpha), toByte(green / alpha), toByte(blue / alpha), toByte(alpha / stops.size()));
}

static void appendHintStops(GradientStops& stops, float fromOffset, const Color& fromColor, float hintOffset, float toOffset, const Color& toColor)
{
    auto span = toOffset - fromOffset;
    if(span <= 0.f)
        return;
    auto ratio = (hintOffset - fromOffset) / span;
    if(ratio <= 0.f || ratio >= 1.f) {
        // A hint sitting on either end collapses the ramp onto one color.
        stops.emplace_back(hintOffset, ratio <= 0.f ? toColor : fromColor);
        return;
    }

    if(std::abs(ratio - 0.5f) < 0.001f)
        return;
    auto exponent = std::log(0.5f) / std::log(ratio);
    for(int index = 1; index < kHintSampleCount; ++index) {
        auto position = static_cast<float>(index) / kHintSampleCount;
        stops.emplace_back(fromOffset + span * position, interpolateColor(fromColor, toColor, std::pow(position, exponent)));
    }
}

void GradientImage::buildColorStops(float lineLength, GradientStops& stops) const
{
    const auto count = m_stops.size();
    std::vector<float> offsets(count, 0.f);
    std::vector<bool> resolved(count, false);
    for(size_t index = 0; index < count; ++index) {
        const auto& position = m_stops[index].position;
        if(position.isAuto())
            continue;
        if(position.isPercent()) {
            offsets[index] = position.value() / 100.f;
        } else if(lineLength > 0.f) {
            offsets[index] = position.value() / lineLength;
        }

        resolved[index] = true;
    }

    if(!resolved.front()) {
        offsets.front() = 0.f;
        resolved.front() = true;
    }

    if(!resolved.back()) {
        offsets.back() = 1.f;
        resolved.back() = true;
    }

    // Stop positions may not decrease; a smaller one is pulled up to its
    // predecessor, which is what produces a hard color transition.
    auto maximum = offsets.front();
    for(size_t index = 0; index < count; ++index) {
        if(resolved[index]) {
            maximum = std::max(maximum, offsets[index]);
            offsets[index] = maximum;
        }
    }

    // Runs of stops without a position spread evenly between the surrounding
    // positioned ones.
    for(size_t index = 1; index < count; ++index) {
        if(resolved[index])
            continue;
        size_t next = index;
        while(!resolved[next])
            ++next;
        for(size_t current = index; current < next; ++current)
            offsets[current] = offsets[index - 1] + (offsets[next] - offsets[index - 1]) * (current - index + 1) / (next - index + 1);
        index = next;
    }

    GradientStops resolvedStops;
    for(size_t index = 0; index < count; ++index) {
        if(m_stops[index].hint)
            continue;
        if(index >= 2 && m_stops[index - 1].hint) {
            appendHintStops(resolvedStops, offsets[index - 2], m_stops[index - 2].color, offsets[index - 1], offsets[index], m_stops[index].color);
        }

        resolvedStops.emplace_back(offsets[index], m_stops[index].color);
    }

    // Cairo blends between two stops without premultiplying their alpha, so
    // a ramp towards a translucent color has to be sampled by hand to avoid
    // the color of that stop bleeding into the transparent end.
    for(size_t index = 0; index < resolvedStops.size(); ++index) {
        if(index > 0) {
            const auto& from = resolvedStops[index - 1];
            const auto& to = resolvedStops[index];
            if(from.second.alpha() != to.second.alpha() && from.first < to.first) {
                for(int sample = 1; sample < kAlphaSampleCount; ++sample) {
                    auto position = static_cast<float>(sample) / kAlphaSampleCount;
                    stops.emplace_back(from.first + (to.first - from.first) * position, interpolateColor(from.second, to.second, position));
                }
            }
        }

        stops.push_back(resolvedStops[index]);
    }
}

static void clampColorStops(GradientStops& stops)
{
    if(stops.front().first >= 0.f)
        return;
    size_t index = 0;
    while(index + 1 < stops.size() && stops[index + 1].first <= 0.f)
        ++index;
    if(index + 1 == stops.size()) {
        // The whole ramp lies before the origin, only its end is visible.
        auto color = stops.back().second;
        stops.clear();
        stops.emplace_back(0.f, color);
        stops.emplace_back(1.f, color);
        return;
    }

    const auto& from = stops[index];
    const auto& to = stops[index + 1];
    auto color = interpolateColor(from.second, to.second, -from.first / (to.first - from.first));
    stops.erase(stops.begin(), stops.begin() + index + 1);
    stops.insert(stops.begin(), GradientStop(0.f, color));
}

GradientImage::ResolvedGradient GradientImage::resolveGradient(float lineLength, bool positiveOnly) const
{
    ResolvedGradient gradient;
    buildColorStops(lineLength, gradient.stops);
    if(positiveOnly && !m_repeating)
        clampColorStops(gradient.stops);

    auto first = gradient.stops.front().first;
    auto last = gradient.stops.back().first;
    auto span = last - first;
    if(span <= 0.f) {
        if(m_repeating) {
            gradient.color = averageColor(gradient.stops);
            gradient.degenerate = true;
            return gradient;
        }

        gradient.start = std::max(first, 0.f);
        gradient.end = gradient.start + kHardTransitionSpan;

        GradientStops stops;
        stops.emplace_back(0.f, gradient.stops.front().second);
        stops.emplace_back(1.f, gradient.stops.back().second);
        gradient.stops = std::move(stops);
        return gradient;
    }

    if(positiveOnly && first < 0.f) {
        // A repeating ramp is periodic, so sliding it by whole periods keeps
        // the rendering identical while moving it onto the positive ray.
        auto shift = std::ceil(-first / span) * span;
        for(auto& stop : gradient.stops)
            stop.first += shift;
        first += shift;
        last += shift;
    }

    // Cairo clamps stop offsets to [0, 1], so the gradient geometry is moved
    // onto the span actually covered by the stops instead.
    for(auto& stop : gradient.stops)
        stop.first = (stop.first - first) / span;
    gradient.start = first;
    gradient.end = last;
    gradient.method = m_repeating ? SpreadMethod::Repeat : SpreadMethod::Pad;
    return gradient;
}

static float directionAngle(CSSValueID horizontal, CSSValueID vertical, float width, float height)
{
    if(horizontal == CSSValueID::Center)
        return vertical == CSSValueID::Top ? 0.f : 180.f;
    if(vertical == CSSValueID::Center)
        return horizontal == CSSValueID::Right ? 90.f : 270.f;
    // The gradient line of a corner keyword is perpendicular to the diagonal
    // joining the two neighbouring corners.
    auto angle = rad2deg(std::atan2(height, width));
    if(vertical == CSSValueID::Top)
        return horizontal == CSSValueID::Right ? angle : 360.f - angle;
    return horizontal == CSSValueID::Right ? 180.f - angle : 180.f + angle;
}

void GradientImage::applyLinearGradient(GraphicsContext& context) const
{
    auto width = m_containerSize.w;
    auto height = m_containerSize.h;
    auto angle = m_angle;
    if(m_directionX != CSSValueID::Unknown)
        angle = directionAngle(m_directionX, m_directionY, width, height);
    auto radians = deg2rad(angle);
    auto dx = std::sin(radians);
    auto dy = -std::cos(radians);

    // An angle of 0deg points to the top and grows clockwise; the line is
    // long enough for the box corners to fall on the 0% and 100% stops.
    auto lineLength = std::abs(width * dx) + std::abs(height * dy);
    auto gradient = resolveGradient(lineLength, false);
    if(gradient.degenerate) {
        context.setColor(gradient.color);
        return;
    }

    Point origin(width / 2.f - dx * lineLength / 2.f, height / 2.f - dy * lineLength / 2.f);

    LinearGradientValues values;
    values.x1 = origin.x + dx * lineLength * gradient.start;
    values.y1 = origin.y + dy * lineLength * gradient.start;
    values.x2 = origin.x + dx * lineLength * gradient.end;
    values.y2 = origin.y + dy * lineLength * gradient.end;
    context.setLinearGradient(values, gradient.stops, Transform(), gradient.method, 1.f);
}

void GradientImage::applyRadialGradient(GraphicsContext& context) const
{
    auto width = m_containerSize.w;
    auto height = m_containerSize.h;

    Point center(m_position.x().calc(width), m_position.y().calc(height));
    auto left = std::abs(center.x);
    auto right = std::abs(width - center.x);
    auto top = std::abs(center.y);
    auto bottom = std::abs(height - center.y);

    float radiusX = 0.f;
    float radiusY = 0.f;
    switch(m_sizeType) {
    case CSSValueID::ClosestSide:
    case CSSValueID::ClosestCorner:
        radiusX = std::min(left, right);
        radiusY = std::min(top, bottom);
        break;
    case CSSValueID::FarthestSide:
    case CSSValueID::FarthestCorner:
        radiusX = std::max(left, right);
        radiusY = std::max(top, bottom);
        break;
    default:
        radiusX = m_radiusX.calc(width);
        radiusY = m_radiusY.calc(height);
        break;
    }

    if(m_sizeType == CSSValueID::ClosestSide || m_sizeType == CSSValueID::FarthestSide) {
        if(m_circle) {
            radiusX = radiusY = m_sizeType == CSSValueID::ClosestSide
                ? std::min(radiusX, radiusY) : std::max(radiusX, radiusY);
        }
    } else if(m_sizeType == CSSValueID::ClosestCorner || m_sizeType == CSSValueID::FarthestCorner) {
        if(m_circle) {
            radiusX = radiusY = std::hypot(radiusX, radiusY);
        } else {
            // The ending shape keeps the aspect ratio of the matching side
            // sized ellipse while passing through the corner.
            radiusX *= std::numbers::sqrt2_v<float>;
            radiusY *= std::numbers::sqrt2_v<float>;
        }
    }

    if(radiusX <= 0.f || radiusY <= 0.f) {
        context.setColor(m_stops.back().color);
        return;
    }

    // The gradient ray runs from the center towards the right edge of the
    // ending shape, so its length is the horizontal radius.
    auto gradient = resolveGradient(radiusX, true);
    if(gradient.degenerate) {
        context.setColor(gradient.color);
        return;
    }

    // Cairo only draws circular gradients; the ellipse comes from scaling the
    // pattern space, which keeps the shading vectorial.
    auto transform = Transform::makeTranslate(center.x, center.y);
    transform.scale(1.f, radiusY / radiusX);

    RadialGradientValues values;
    values.r0 = radiusX * gradient.start;
    values.r = radiusX * gradient.end;
    context.setRadialGradient(values, gradient.stops, transform, gradient.method, 1.f);
}

void GradientImage::apply(GraphicsContext& context) const
{
    switch(m_gradientType) {
    case CSSGradientType::Linear:
        applyLinearGradient(context);
        break;
    case CSSGradientType::Radial:
        applyRadialGradient(context);
        break;
    default:
        context.setColor(Color::Transparent);
        break;
    }
}

void GradientImage::draw(GraphicsContext& context, const Rect& dstRect, const Rect& srcRect)
{
    if(dstRect.isEmpty() || srcRect.isEmpty() || m_containerSize.isEmpty()) {
        return;
    }

    auto xScale = dstRect.w / srcRect.w;
    auto yScale = dstRect.h / srcRect.h;

    auto xOffset = dstRect.x - (srcRect.x * xScale);
    auto yOffset = dstRect.y - (srcRect.y * yScale);

    context.save();
    context.clipRect(dstRect);
    context.translate(xOffset, yOffset);
    context.scale(xScale, yScale);
    apply(context);
    context.fillRect(Rect(0, 0, m_containerSize.w, m_containerSize.h));
    context.restore();
}

void GradientImage::drawPattern(GraphicsContext& context, const Rect& destRect, const Size& size, const Size& scale, const Point& phase)
{
    assert(!destRect.isEmpty() && !size.isEmpty() && !scale.isEmpty());

    cairo_matrix_t pattern_matrix;
    cairo_matrix_init(&pattern_matrix, 1, 0, 0, 1, -phase.x, -phase.y);

    cairo_rectangle_t pattern_rectangle = {0, 0, size.w * scale.w, size.h * scale.h};
    auto pattern_surface = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &pattern_rectangle);
    auto pattern_canvas = cairo_create(pattern_surface);

    auto pattern = cairo_pattern_create_for_surface(pattern_surface);
    cairo_pattern_set_matrix(pattern, &pattern_matrix);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

    GraphicsContext pattern_context(pattern_canvas);
    pattern_context.scale(scale.w, scale.h);
    apply(pattern_context);
    pattern_context.fillRect(Rect(0, 0, size.w, size.h));

    auto canvas = context.canvas();
    cairo_save(canvas);
    cairo_set_fill_rule(canvas, CAIRO_FILL_RULE_WINDING);
    cairo_rectangle(canvas, destRect.x, destRect.y, destRect.w, destRect.h);
    cairo_set_source(canvas, pattern);
    cairo_fill(canvas);
    cairo_restore(canvas);

    cairo_pattern_destroy(pattern);
    cairo_destroy(pattern_canvas);
    cairo_surface_destroy(pattern_surface);
}

void GradientImage::computeIntrinsicDimensions(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio)
{
    // A gradient has no intrinsic dimensions at all; it always takes the size
    // of the area it is painted into.
    intrinsicWidth = 0.f;
    intrinsicHeight = 0.f;
    intrinsicRatio = 0.0;
}

RefPtr<GradientImage> GradientImage::create(const BoxStyle& style, const CSSGradientValue& value)
{
    GradientColorStopList stops;
    stops.reserve(value.stops().size());
    for(const auto& stop : value.stops()) {
        GradientColorStop resolved;
        resolved.hint = stop.isHint();
        if(!resolved.hint)
            resolved.color = style.convertColor(*stop.color());
        if(stop.position() == nullptr) {
            resolved.position = Length::Auto;
        } else if(is<CSSAngleValue>(*stop.position())) {
            resolved.position = Length(Length::Type::Percent, to<CSSAngleValue>(*stop.position()).valueInDegrees() / 3.6f);
        } else {
            resolved.position = style.convertLengthOrPercent(*stop.position());
        }

        stops.push_back(resolved);
    }

    auto image = adoptPtr(new (style.heap()) GradientImage(value.gradientType(), value.repeating(), std::move(stops)));
    if(const auto& angle = value.angle())
        image->m_angle = to<CSSAngleValue>(*angle).valueInDegrees();
    if(const auto& direction = value.direction()) {
        const auto& pair = to<CSSPairValue>(*direction);
        image->m_directionX = pair.first()->id();
        image->m_directionY = pair.second()->id();
    }

    if(const auto& shape = value.shape())
        image->m_circle = shape->id() == CSSValueID::Circle;
    if(const auto& size = value.size()) {
        if(is<CSSIdentValue>(*size)) {
            image->m_sizeType = size->id();
        } else if(is<CSSPairValue>(*size)) {
            const auto& pair = to<CSSPairValue>(*size);
            image->m_sizeType = CSSValueID::Unknown;
            image->m_radiusX = style.convertLengthOrPercent(*pair.first());
            image->m_radiusY = style.convertLengthOrPercent(*pair.second());
        } else {
            image->m_sizeType = CSSValueID::Unknown;
            image->m_radiusX = style.convertLength(*size);
            image->m_radiusY = image->m_radiusX;
        }
    }

    if(const auto& position = value.position())
        image->m_position = style.convertPositionCoordinate(*position);
    return image;
}

GradientImage::GradientImage(CSSGradientType gradientType, bool repeating, GradientColorStopList stops)
    : m_gradientType(gradientType), m_repeating(repeating), m_stops(std::move(stops))
{
    if(gradientType == CSSGradientType::Conic) {
        m_angle = 0.f;
    }
}

} // namespace plutobook
