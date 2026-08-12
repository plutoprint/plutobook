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

GradientImage::ResolvedGradient GradientImage::resolveGradient(float lineLength) const
{
    ResolvedGradient gradient;
    buildColorStops(lineLength, gradient.stops);

    auto first = gradient.stops.front().first;
    auto last = gradient.stops.back().first;
    auto span = last - first;
    if(span <= 0.f) {
        if(m_repeating) {
            gradient.color = averageColor(gradient.stops);
            gradient.degenerate = true;
            return gradient;
        }

        gradient.start = first;
        gradient.end = first + kHardTransitionSpan;

        GradientStops stops;
        stops.emplace_back(0.f, gradient.stops.front().second);
        stops.emplace_back(1.f, gradient.stops.back().second);
        gradient.stops = std::move(stops);
        return gradient;
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
    auto gradient = resolveGradient(lineLength);
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

void GradientImage::apply(GraphicsContext& context) const
{
    switch(m_gradientType) {
    case CSSGradientType::Linear:
        applyLinearGradient(context);
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
