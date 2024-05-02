#ifndef PLUTOBOOK_BORDERPAINTER_H
#define PLUTOBOOK_BORDERPAINTER_H

#include "geometry.h"
#include "boxstyle.h"

namespace plutobook {

using BorderEdgeFlags = unsigned int;

class GraphicsContext;

enum class BorderPainterType {
    Border,
    Outline
};

class BorderPainter {
public:
    BorderPainter(BorderPainterType type, const Rect& borderRect, const BoxStyle& style, bool includeLeftEdge, bool includeRightEdge);

    void paint(GraphicsContext& context, const Rect& rect) const;

    static void paintBoxSide(GraphicsContext& context, BoxSide side, LineStyle style, const Color& color, const Rect& rect);

private:
    void paintTranslucentSides(GraphicsContext& context) const;
    void paintSides(GraphicsContext& context, BorderEdgeFlags visibleEdgeSet, const Color* commonColor = nullptr) const;
    void paintSide(GraphicsContext& context, BoxSide side, BoxSide adjacentSide1, BoxSide adjacentSide2, const Color& color, const Rect& rect) const;
    void paintSide(GraphicsContext& context, BoxSide side, BoxSide adjacentSide1, BoxSide adjacentSide2, const Color& color, const Path& path) const;
    void paintBoxSide(GraphicsContext& context, BoxSide side, LineStyle style, const Color& color, float thickness, const Path& path) const;
    void clipBoxSide(GraphicsContext& context, BoxSide side) const;

    BorderEdge m_edges[4];
    bool m_isUniformStyle{true};
    bool m_isUniformWidth{true};
    bool m_isUniformColor{true};
    bool m_isOpaque{true};
    bool m_isRounded{false};

    unsigned m_visibleEdgeCount{0};
    unsigned m_firstVisibleEdge{0};
    BorderEdgeFlags m_visibleEdgeSet{0};

    RoundedRect m_inner;
    RoundedRect m_outer;
};

} // namespace plutobook

#endif // PLUTOBOOK_BORDERPAINTER_H
