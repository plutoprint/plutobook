/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_WRITINGMODE_H
#define PLUTOBOOK_WRITINGMODE_H

#include <cstdint>

namespace plutobook {

enum class WritingMode : uint8_t {
    HorizontalTb,
    VerticalRl,
    VerticalLr
};

enum class Direction : uint8_t {
    Ltr,
    Rtl
};

enum BoxSide {
    BoxSideTop,
    BoxSideRight,
    BoxSideBottom,
    BoxSideLeft
};

enum BoxCorner {
    BoxCornerTopLeft,
    BoxCornerTopRight,
    BoxCornerBottomRight,
    BoxCornerBottomLeft
};

class WritingDirection {
public:
    constexpr WritingDirection(WritingMode mode, Direction direction)
        : m_mode(mode), m_direction(direction)
    {}

    constexpr WritingMode mode() const { return m_mode; }
    constexpr Direction direction() const { return m_direction; }

    constexpr bool isHorizontal() const { return m_mode == WritingMode::HorizontalTb; }

    constexpr BoxSide inlineStart() const;
    constexpr BoxSide inlineEnd() const;
    constexpr BoxSide blockStart() const;
    constexpr BoxSide blockEnd() const;

    constexpr BoxCorner startStart() const;
    constexpr BoxCorner startEnd() const;
    constexpr BoxCorner endStart() const;
    constexpr BoxCorner endEnd() const;

private:
    WritingMode m_mode;
    Direction m_direction;
};

constexpr BoxSide WritingDirection::inlineStart() const
{
    if(m_mode == WritingMode::HorizontalTb)
        return m_direction == Direction::Ltr ? BoxSideLeft : BoxSideRight;
    return m_direction == Direction::Ltr ? BoxSideTop : BoxSideBottom;
}

constexpr BoxSide WritingDirection::inlineEnd() const
{
    if(m_mode == WritingMode::HorizontalTb)
        return m_direction == Direction::Ltr ? BoxSideRight : BoxSideLeft;
    return m_direction == Direction::Ltr ? BoxSideBottom : BoxSideTop;
}

constexpr BoxSide WritingDirection::blockStart() const
{
    if(m_mode == WritingMode::HorizontalTb)
        return BoxSideTop;
    if(m_mode == WritingMode::VerticalRl)
        return BoxSideRight;
    return BoxSideLeft;
}

constexpr BoxSide WritingDirection::blockEnd() const
{
    if(m_mode == WritingMode::HorizontalTb)
        return BoxSideBottom;
    if(m_mode == WritingMode::VerticalRl)
        return BoxSideLeft;
    return BoxSideRight;
}

constexpr BoxCorner WritingDirection::startStart() const
{
    if(m_mode == WritingMode::HorizontalTb)
        return m_direction == Direction::Ltr ? BoxCornerTopLeft : BoxCornerTopRight;
    if(m_mode == WritingMode::VerticalRl)
        return m_direction == Direction::Ltr ? BoxCornerTopRight : BoxCornerBottomRight;
    return m_direction == Direction::Ltr ? BoxCornerTopLeft : BoxCornerBottomLeft;
}

constexpr BoxCorner WritingDirection::startEnd() const
{
    if(m_mode == WritingMode::HorizontalTb)
        return m_direction == Direction::Ltr ? BoxCornerTopRight : BoxCornerTopLeft;
    if(m_mode == WritingMode::VerticalRl)
        return m_direction == Direction::Ltr ? BoxCornerBottomRight : BoxCornerTopRight;
    return m_direction == Direction::Ltr ? BoxCornerBottomLeft : BoxCornerTopLeft;
}

constexpr BoxCorner WritingDirection::endStart() const
{
    if(m_mode == WritingMode::HorizontalTb)
        return m_direction == Direction::Ltr ? BoxCornerBottomLeft : BoxCornerBottomRight;
    if(m_mode == WritingMode::VerticalRl)
        return m_direction == Direction::Ltr ? BoxCornerTopLeft : BoxCornerBottomLeft;
    return m_direction == Direction::Ltr ? BoxCornerTopRight : BoxCornerBottomRight;
}

constexpr BoxCorner WritingDirection::endEnd() const
{
    if(m_mode == WritingMode::HorizontalTb)
        return m_direction == Direction::Ltr ? BoxCornerBottomRight : BoxCornerBottomLeft;
    if(m_mode == WritingMode::VerticalRl)
        return m_direction == Direction::Ltr ? BoxCornerBottomLeft : BoxCornerTopLeft;
    return m_direction == Direction::Ltr ? BoxCornerBottomRight : BoxCornerTopRight;
}

} // namespace plutobook

#endif // PLUTOBOOK_WRITINGMODE_H
