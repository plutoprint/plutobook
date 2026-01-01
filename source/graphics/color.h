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
    constexpr Color(int r, int g, int b, int a = 255) : m_value(a << 24 | r << 16 | g << 8 | b) {}

    constexpr uint8_t alpha() const { return (m_value >> 24) & 0xff; }
    constexpr uint8_t red() const { return (m_value >> 16) & 0xff; }
    constexpr uint8_t green() const { return (m_value >> 8) & 0xff; }
    constexpr uint8_t blue() const { return (m_value >> 0) & 0xff; }
    constexpr uint32_t value() const { return m_value; }

    constexpr bool isOpaque() const { return alpha() == 255; }
    constexpr bool isVisible() const { return alpha() > 0; }

    constexpr Color opaqueColor() const { return Color(m_value | 0xFF000000); }
    constexpr Color colorWithAlpha(float opacity) const;

    Color lighten() const;
    Color darken() const;

    static const Color Transparent;
    static const Color Black;
    static const Color White;

    static std::optional<Color> named(const std::string_view& name);

private:
    uint32_t m_value{0};
};

constexpr Color Color::colorWithAlpha(float opacity) const
{
    auto rgb = m_value & 0x00FFFFFF;
    auto a = static_cast<int>(alpha() * std::clamp(opacity, 0.f, 1.f));
    return Color(rgb | a << 24);
}

constexpr bool operator==(const Color& a, const Color& b)
{
    return a.value() == b.value();
}

constexpr bool operator!=(const Color& a, const Color& b)
{
    return a.value() != b.value();
}

constexpr bool operator<(const Color& a, const Color& b)
{
    return a.value() < b.value();
}

constexpr bool operator>(const Color& a, const Color& b)
{
    return a.value() > b.value();
}

constexpr bool operator<=(const Color& a, const Color& b)
{
    return a.value() <= b.value();
}

constexpr bool operator>=(const Color& a, const Color& b)
{
    return a.value() >= b.value();
}

} // namespace plutobook

#endif // PLUTOBOOK_COLOR_H
