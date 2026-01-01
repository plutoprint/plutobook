/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_BORDERPAINTER_H
#define PLUTOBOOK_BORDERPAINTER_H

#include "geometry.h"
#include "boxstyle.h"

namespace plutobook {

class GraphicsContext;
class PaintInfo;

enum class BorderPainterType {
    Border,
    Outline
};

using BorderEdgeFlags = unsigned int;

class BorderPainter {
public:
    static void paintBorder(const PaintInfo& info, const Rect& borderRect, const BoxStyle* style, bool includeLeftEdge, bool includeRightEdge);
    static void paintOutline(const PaintInfo& info, const Rect& borderRect, const BoxStyle* style);

    static void paintBoxSide(GraphicsContext& context, BoxSide side, LineStyle style, const Color& color, const Rect& rect);

private:
    BorderPainter(BorderPainterType type, const Rect& borderRect, const BoxStyle* style, bool includeLeftEdge, bool includeRightEdge);

    void paint(const PaintInfo& info) const;

    void paintTranslucentSides(GraphicsContext& context, BorderEdgeFlags visibleEdgeSet) const;
    void paintSides(GraphicsContext& context, BorderEdgeFlags visibleEdgeSet, const Color* commonColor = nullptr) const;
    void paintSide(GraphicsContext& context, BoxSide side, BoxSide adjacentSide1, BoxSide adjacentSide2, const Color& color, const Rect& rect) const;
    void paintSide(GraphicsContext& context, BoxSide side, BoxSide adjacentSide1, BoxSide adjacentSide2, const Color& color, const Path& path) const;
    void paintBoxSide(GraphicsContext& context, BoxSide side, LineStyle style, const Color& color, float thickness, const Path& path) const;
    void clipBoxSide(GraphicsContext& context, BoxSide side) const;

    BorderEdge m_edges[4];

    BorderEdgeFlags m_visibleEdgeSet{0};
    unsigned m_visibleEdgeCount{0};
    unsigned m_firstVisibleEdge{0};

    bool m_isUniformStyle{true};
    bool m_isUniformColor{true};

    bool m_isOpaque{true};
    bool m_isRounded{false};

    RoundedRect m_inner;
    RoundedRect m_outer;
};

inline void BorderPainter::paintBorder(const PaintInfo& info, const Rect& borderRect, const BoxStyle* style, bool includeLeftEdge, bool includeRightEdge)
{
    BorderPainter(BorderPainterType::Border, borderRect, style, includeLeftEdge, includeRightEdge).paint(info);
}

inline void BorderPainter::paintOutline(const PaintInfo& info, const Rect& borderRect, const BoxStyle* style)
{
    BorderPainter(BorderPainterType::Outline, borderRect, style, true, true).paint(info);
}

} // namespace plutobook

#endif // PLUTOBOOK_BORDERPAINTER_H
