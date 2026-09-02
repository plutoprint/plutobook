/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_COLOR_H
#define PLUTOBOOK_COLOR_H

#include <optional>
#include <string_view>
#include <algorithm>
#include <cstdint>

namespace plutobook {

class Color {
public:
    constexpr Color() = default;
    constexpr explicit Color(uint32_t value) : m_value(value) {}
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    constexpr Color(float r, float g, float b, float a = 1.f);

    constexpr uint8_t alpha() const { return (m_value >> 24) & 0xff; }
    constexpr uint8_t red() const { return (m_value >> 16) & 0xff; }
    constexpr uint8_t green() const { return (m_value >> 8) & 0xff; }
    constexpr uint8_t blue() const { return (m_value >> 0) & 0xff; }

    constexpr uint32_t value() const { return m_value; }

    constexpr bool isOpaque() const { return alpha() == 255; }
    constexpr bool isVisible() const { return alpha() > 0; }

    constexpr Color opaqueColor() const { return Color(m_value | 0xFF000000); }
    constexpr Color colorWithAlpha(float opacity) const;

    Color lightened() const;
    Color darkened() const;

    static const Color Transparent;
    static const Color Black;
    static const Color White;

    static std::optional<Color> fromName(std::string_view name);

private:
    static constexpr uint8_t scaleToByte(float v);
    uint32_t m_value{0};
};

constexpr uint8_t Color::scaleToByte(float v)
{
    return static_cast<uint8_t>(std::clamp(v, 0.f, 1.f) * 255.f + 0.5f);
}

constexpr Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    : m_value(static_cast<uint32_t>(a) << 24 | r << 16 | g << 8 | b)
{}

constexpr Color::Color(float r, float g, float b, float a)
    : Color(scaleToByte(r), scaleToByte(g), scaleToByte(b), scaleToByte(a))
{}

constexpr Color Color::colorWithAlpha(float opacity) const
{
    auto rgb = m_value & 0x00FFFFFF;
    auto a = static_cast<uint32_t>(std::clamp(opacity, 0.f, 1.f) * alpha() + 0.5f);
    return Color(rgb | a << 24);
}

constexpr bool operator==(const Color& a, const Color& b)
{
    return a.value() == b.value();
}

constexpr bool operator<(const Color& a, const Color& b)
{
    return a.value() < b.value();
}

} // namespace plutobook

#endif // PLUTOBOOK_COLOR_H
