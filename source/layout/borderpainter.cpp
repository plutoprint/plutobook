/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "borderpainter.h"
#include "graphicscontext.h"
#include "box.h"

#include <cmath>

namespace plutobook {

enum BorderEdgeFlag {
    TopBorderEdge = 1 << BoxSideTop,
    RightBorderEdge = 1 << BoxSideRight,
    BottomBorderEdge = 1 << BoxSideBottom,
    LeftBorderEdge = 1 << BoxSideLeft,
    AllBorderEdges = TopBorderEdge | BottomBorderEdge | LeftBorderEdge | RightBorderEdge
};

constexpr BorderEdgeFlag edgeFlagForSide(BoxSide side)
{
    return BorderEdgeFlag(1 << side);
}

constexpr bool includesEdge(BorderEdgeFlags flags, BoxSide side)
{
    return flags & edgeFlagForSide(side);
}

constexpr bool includesAdjacentEdges(BorderEdgeFlags flags)
{
    return (flags & (TopBorderEdge | BottomBorderEdge)) && (flags & (LeftBorderEdge | RightBorderEdge));
}

static void paintDashedOrDottedBoxSide(GraphicsContext& context, BoxSide side, LineStyle style, const Color& color, float x1, float y1, float x2, float y2, float thickness, float length)
{
    auto cornerWidth = style == LineStyle::Dotted ? thickness : std::min(2.f * thickness, std::max(thickness, length / 3.f));
    context.setColor(color);
    if(side == BoxSideLeft || side == BoxSideRight) {
        context.fillRect(Rect(x1, y1, thickness, cornerWidth));
        context.fillRect(Rect(x1, y2 - cornerWidth, thickness, cornerWidth));
    } else {
        context.fillRect(Rect(x1, y1, cornerWidth, thickness));
        context.fillRect(Rect(x2 - cornerWidth, y1, cornerWidth, thickness));
    }

    auto strokeWidth = length - cornerWidth * 2.f;
    auto patternWidth = style == LineStyle::Dotted ? thickness : std::min(3.f * thickness, std::max(thickness, strokeWidth / 3.f));
    if(strokeWidth <= patternWidth)
        return;
    int numberOfSegments = std::floor(strokeWidth / patternWidth);
    bool oddNumberOfSegments = numberOfSegments % 2;
    float remainingWidth = strokeWidth - (numberOfSegments * patternWidth);
    float patternOffset = patternWidth;
    if(oddNumberOfSegments && remainingWidth) {
        patternOffset -= remainingWidth / 2.f;
    } else if(!oddNumberOfSegments) {
        if(remainingWidth)
            patternOffset += patternOffset - (patternWidth + remainingWidth) / 2.f;
        else {
            patternOffset += patternWidth / 2.f;
        }
    }

    Point p1(x1, y1);
    Point p2(x2, y2);
    if(side == BoxSideLeft || side == BoxSideRight) {
        auto centerOffset = (p2.x - p1.x) / 2.f;
        p1.translate(centerOffset, cornerWidth);
        p2.translate(-centerOffset, -cornerWidth);
    } else {
        auto centerOffset = (p2.y - p1.y) / 2.f;
        p1.translate(cornerWidth, centerOffset);
        p2.translate(-cornerWidth, -centerOffset);
    }

    Path path;
    path.moveTo(p1.x, p1.y);
    path.lineTo(p2.x, p2.y);

    StrokeData strokeData(thickness);
    strokeData.setDashOffset(patternOffset);
    strokeData.setDashArray({patternWidth, patternWidth});
    context.strokePath(path, strokeData);
}

static void paintDoubleBoxSide(GraphicsContext& context, BoxSide side, const Color& color, float x1, float y1, float x2, float y2, float thickness, float length)
{
    auto thirdOfThickness = std::ceil(thickness / 3.f);
    context.setColor(color);
    switch(side) {
    case BoxSideTop:
    case BoxSideBottom:
        context.fillRect(Rect(x1, y1, length, thirdOfThickness));
        context.fillRect(Rect(x1, y2 - thirdOfThickness, length, thirdOfThickness));
        break;
    case BoxSideLeft:
    case BoxSideRight:
        context.fillRect(Rect(x1, y1, thirdOfThickness, length));
        context.fillRect(Rect(x2 - thirdOfThickness, y1, thirdOfThickness, length));
        break;
    }
}

static void paintRidgeOrGrooveBoxSide(GraphicsContext& context, BoxSide side, LineStyle style, const Color& color, float x1, float y1, float x2, float y2, float thickness, float length)
{
    LineStyle s1;
    LineStyle s2;
    if(style == LineStyle::Groove) {
        s1 = LineStyle::Inset;
        s2 = LineStyle::Outset;
    } else {
        s1 = LineStyle::Outset;
        s2 = LineStyle::Inset;
    }

    auto secondOfThickness = std::ceil(thickness / 2.f);
    switch(side) {
    case BoxSideTop:
        BorderPainter::paintBoxSide(context, side, s1, color, Rect(x1, y1, length, secondOfThickness));
        BorderPainter::paintBoxSide(context, side, s2, color, Rect(x1, y2 - secondOfThickness, length, secondOfThickness));
        break;
    case BoxSideLeft:
        BorderPainter::paintBoxSide(context, side, s1, color, Rect(x1, y1, secondOfThickness, length));
        BorderPainter::paintBoxSide(context, side, s2, color, Rect(x2 - secondOfThickness, y1, secondOfThickness, length));
        break;
    case BoxSideBottom:
        BorderPainter::paintBoxSide(context, side, s2, color, Rect(x1, y1, length, secondOfThickness));
        BorderPainter::paintBoxSide(context, side, s1, color, Rect(x1, y2 - secondOfThickness, length, secondOfThickness));
        break;
    case BoxSideRight:
        BorderPainter::paintBoxSide(context, side, s2, color, Rect(x1, y1, secondOfThickness, length));
        BorderPainter::paintBoxSide(context, side, s1, color, Rect(x2 - secondOfThickness, y1, secondOfThickness, length));
        break;
    }
}

static void paintSolidBoxSide(GraphicsContext& context, const Color& color, float x1, float y1, float x2, float y2)
{
    context.setColor(color);
    context.fillRect(Rect(x1, y1, x2 - x1, y2 - y1));
}

static void paintInsetOrOutsetBoxSide(GraphicsContext& context, BoxSide side, LineStyle style, const Color& color, float x1, float y1, float x2, float y2)
{
    if((side == BoxSideTop || side == BoxSideLeft) == (style == LineStyle::Inset)) {
        context.setColor(color.darken());
    } else {
        context.setColor(color.lighten());
    }

    context.fillRect(Rect(x1, y1, x2 - x1, y2 - y1));
}

void BorderPainter::paintBoxSide(GraphicsContext& context, BoxSide side, LineStyle style, const Color& color, const Rect& rect)
{
    auto x1 = rect.x;
    auto x2 = rect.x + rect.w;
    auto y1 = rect.y;
    auto y2 = rect.y + rect.h;

    float thickness;
    float length;
    if(side == BoxSideTop || side == BoxSideBottom) {
        thickness = y2 - y1;
        length = x2 - x1;
    } else {
        thickness = x2 - x1;
        length = y2 - y1;
    }

    if(thickness <= 0.f || length <= 0.f )
        return;
    if(style == LineStyle::Double && thickness < 3.f)
        style = LineStyle::Solid;
    switch(style) {
    case LineStyle::Dashed:
    case LineStyle::Dotted:
        paintDashedOrDottedBoxSide(context, side, style, color, x1, y1, x2, y2, thickness, length);
        break;
    case LineStyle::Double:
        paintDoubleBoxSide(context, side, color, x1, y1, x2, y2, thickness, length);
        break;
    case LineStyle::Ridge:
    case LineStyle::Groove:
        paintRidgeOrGrooveBoxSide(context, side, style, color, x1, y1, x2, y2, thickness, length);
        break;
    case LineStyle::Solid:
        paintSolidBoxSide(context, color, x1, y1, x2, y2);
        break;
    case LineStyle::Inset:
    case LineStyle::Outset:
        paintInsetOrOutsetBoxSide(context, side, style, color, x1, y1, x2, y2);
        break;
    default:
        break;
    }
}

static RectOutsets edgeOutsets(const BorderEdge edges[4], float scale)
{
    auto topWidth = edges[BoxSideTop].width() * scale;
    auto rightWidth = edges[BoxSideRight].width() * scale;
    auto bottomWidth = edges[BoxSideBottom].width() * scale;
    auto leftWidth = edges[BoxSideLeft].width() * scale;

    return RectOutsets(topWidth, rightWidth, bottomWidth, leftWidth);
}

BorderPainter::BorderPainter(BorderPainterType type, const Rect& borderRect, const BoxStyle* style, bool includeLeftEdge, bool includeRightEdge)
{
    if(type == BorderPainterType::Border) {
        style->getBorderEdgeInfo(m_edges, includeLeftEdge, includeRightEdge);
    } else {
        assert(includeLeftEdge && includeRightEdge);
        auto outlineEdge = style->getOutlineEdge();
        for(auto& edge : m_edges) {
            edge = outlineEdge;
        }
    }

    for(auto side : { BoxSideTop, BoxSideRight, BoxSideBottom, BoxSideLeft }) {
        const auto& edge = m_edges[side];
        if(edge.isRenderable()) {
            assert(edge.color().alpha() > 0);
            if(!edge.color().isOpaque())
                m_isOpaque = false;
            m_visibleEdgeSet |= edgeFlagForSide(side);
            m_visibleEdgeCount++;
            if(m_visibleEdgeCount == 1) {
                m_firstVisibleEdge = side;
                continue;
            }

            m_isUniformStyle &= edge.style() == m_edges[m_firstVisibleEdge].style();
            m_isUniformColor &= edge.color() == m_edges[m_firstVisibleEdge].color();
        }
    }

    if(m_visibleEdgeCount == 0)
        return;
    m_outer = style->getBorderRoundedRect(borderRect, includeLeftEdge, includeRightEdge);
    if(type == BorderPainterType::Outline) {
        m_outer += RectOutsets(style->outlineWidth() + style->outlineOffset());
    }

    m_inner = m_outer - edgeOutsets(m_edges, 1.f);
    m_isRounded = m_outer.isRounded();
}

void BorderPainter::paint(const PaintInfo& info) const
{
    if(m_visibleEdgeCount == 0 || !m_outer.rect().intersects(info.rect()))
        return;
    const auto& firstEdge = m_edges[m_firstVisibleEdge];
    if(m_isUniformStyle && m_isUniformColor && (firstEdge.style() == LineStyle::Solid || firstEdge.style() == LineStyle::Double)) {
        if(m_visibleEdgeSet == AllBorderEdges) {
            Path path;
            path.addRoundedRect(m_outer);
            if(firstEdge.style() == LineStyle::Double) {
                RoundedRect outerThirdRect(m_outer - edgeOutsets(m_edges, 1.f / 3.f));
                RoundedRect innerThirdRect(m_outer - edgeOutsets(m_edges, 2.f / 3.f));

                path.addRoundedRect(outerThirdRect);
                path.addRoundedRect(innerThirdRect);
            }

            path.addRoundedRect(m_inner);
            info->setColor(firstEdge.color());
            info->fillPath(path, FillRule::EvenOdd);
            return;
        }

        if(!m_isRounded && firstEdge.style() == LineStyle::Solid) {
            Path path;
            for(auto side : { BoxSideTop, BoxSideRight, BoxSideBottom, BoxSideLeft }) {
                const auto& edge = m_edges[side];
                if(edge.isRenderable()) {
                    Rect sideRect(m_outer.rect());
                    switch(side) {
                    case BoxSideTop:
                        sideRect.h = edge.width();
                        break;
                    case BoxSideRight:
                        sideRect.x = sideRect.right() - edge.width();
                        sideRect.w = edge.width();
                        break;
                    case BoxSideBottom:
                        sideRect.y = sideRect.bottom() - edge.width();
                        sideRect.h = edge.width();
                        break;
                    case BoxSideLeft:
                        sideRect.w = edge.width();
                        break;
                    }

                    path.addRect(sideRect);
                }
            }

            info->setColor(firstEdge.color());
            info->fillPath(path);
            return;
        }
    }

    if(m_isRounded) {
        info->save();
        info->clipRoundedRect(m_outer);
        info->clipOutRoundedRect(m_inner);
    }

    if(m_isOpaque) {
        paintSides(*info, m_visibleEdgeSet);
    } else {
        paintTranslucentSides(*info, m_visibleEdgeSet);
    }

    if(m_isRounded) {
        info->restore();
    }
}

void BorderPainter::paintTranslucentSides(GraphicsContext& context, BorderEdgeFlags visibleEdgeSet) const
{
    while(visibleEdgeSet) {
        Color commonColor;
        BorderEdgeFlags commonColorEdgeSet = 0;
        for(auto side : { BoxSideTop, BoxSideBottom, BoxSideLeft, BoxSideRight }) {
            if(includesEdge(visibleEdgeSet, side)) {
                const auto& edge = m_edges[side];
                if(commonColorEdgeSet == 0) {
                    commonColor = edge.color();
                    commonColorEdgeSet = edgeFlagForSide(side);
                } else if(commonColor == edge.color()) {
                    commonColorEdgeSet |= edgeFlagForSide(side);
                }
            }
        }

        auto opacity = commonColor.alpha() / 255.f;
        auto compositing = includesAdjacentEdges(commonColorEdgeSet) && opacity < 1.f;
        if(compositing) {
            context.pushGroup();
            commonColor = commonColor.opaqueColor();
        }

        paintSides(context, commonColorEdgeSet, &commonColor);
        if(compositing)
            context.popGroup(opacity);
        visibleEdgeSet &= ~commonColorEdgeSet;
    }
}

constexpr bool borderWillArcInnerEdge(const Size& firstRadius, const Size& secondRadius)
{
    return !firstRadius.isZero() || !secondRadius.isZero();
}

constexpr bool borderStyleHasInnerDetail(LineStyle style)
{
    return style == LineStyle::Groove || style == LineStyle::Ridge || style == LineStyle::Double;
}

void BorderPainter::paintSides(GraphicsContext& context, BorderEdgeFlags visibleEdgeSet, const Color* commonColor) const
{
    Path path;
    if(m_isRounded) {
        path.addRoundedRect(m_outer);
    }

    const auto& innerRadii = m_inner.radii();
    for(auto side : { BoxSideTop, BoxSideBottom, BoxSideLeft, BoxSideRight }) {
        const auto& edge = m_edges[side];
        if(!edge.isRenderable() || !includesEdge(visibleEdgeSet, side))
            continue;
        auto color = edge.color();
        if(commonColor) {
            color = *commonColor;
        }

        Rect sideRect(m_outer.rect());
        if(side == BoxSideTop) {
            if(m_isRounded && (borderStyleHasInnerDetail(edge.style()) || borderWillArcInnerEdge(innerRadii.tl, innerRadii.tr))) {
                paintSide(context, side, BoxSideLeft, BoxSideRight, color, path);
            } else {
                sideRect.h = edge.width();
                paintSide(context, side, BoxSideLeft, BoxSideRight, color, sideRect);
            }
        } else if(side == BoxSideBottom) {
            if(m_isRounded && (borderStyleHasInnerDetail(edge.style()) || borderWillArcInnerEdge(innerRadii.bl, innerRadii.br))) {
                paintSide(context, side, BoxSideLeft, BoxSideRight, color, path);
            } else {
                sideRect.y = sideRect.bottom() - edge.width();
                sideRect.h = edge.width();
                paintSide(context, side, BoxSideLeft, BoxSideRight, color, sideRect);
            }
        } else if(side == BoxSideLeft) {
            if(m_isRounded && (borderStyleHasInnerDetail(edge.style()) || borderWillArcInnerEdge(innerRadii.bl, innerRadii.tl))) {
                paintSide(context, side, BoxSideTop, BoxSideBottom, color, path);
            } else {
                sideRect.w = edge.width();
                paintSide(context, side, BoxSideTop, BoxSideBottom, color, sideRect);
            }
        } else if(side == BoxSideRight) {
            if(m_isRounded && (borderStyleHasInnerDetail(edge.style()) || borderWillArcInnerEdge(innerRadii.br, innerRadii.tr))) {
                paintSide(context, side, BoxSideTop, BoxSideBottom, color, path);
            } else {
                sideRect.x = sideRect.right() - edge.width();
                sideRect.w = edge.width();
                paintSide(context, side, BoxSideTop, BoxSideBottom, color, sideRect);
            }
        }
    }
}

constexpr bool borderStyleHasUnmatchedColorsAtCorner(BoxSide side, BoxSide adjacentSide, LineStyle style)
{
    if(style == LineStyle::Inset || style == LineStyle::Outset) {
        BorderEdgeFlags topRightFlags = edgeFlagForSide(BoxSideTop) | edgeFlagForSide(BoxSideRight);
        BorderEdgeFlags bottomLeftFlags = edgeFlagForSide(BoxSideBottom) | edgeFlagForSide(BoxSideLeft);
        BorderEdgeFlags flags = edgeFlagForSide(side) | edgeFlagForSide(adjacentSide);
        return flags == topRightFlags || flags == bottomLeftFlags;
    }

    return style == LineStyle::Groove || style == LineStyle::Ridge;
}

void BorderPainter::paintSide(GraphicsContext& context, BoxSide side, BoxSide adjacentSide1, BoxSide adjacentSide2, const Color& color, const Rect& rect) const
{
    const auto& edge = m_edges[side];
    auto joinRequiresMitre = [&](BoxSide side, BoxSide adjacentSide) {
        const auto& adjacentEdge = m_edges[adjacentSide];
        if(!adjacentEdge.width())
            return false;
        if(edge.color() != adjacentEdge.color())
            return true;
        if(edge.style() != adjacentEdge.style())
            return true;
        return borderStyleHasUnmatchedColorsAtCorner(side, adjacentSide, edge.style());
    };

    auto clipping = joinRequiresMitre(side, adjacentSide1) || joinRequiresMitre(side, adjacentSide2);
    if(clipping) {
        context.save();
        clipBoxSide(context, side);
    }

    paintBoxSide(context, side, edge.style(), color, rect);
    if(clipping) {
        context.restore();
    }
}

void BorderPainter::paintSide(GraphicsContext& context, BoxSide side, BoxSide adjacentSide1, BoxSide adjacentSide2, const Color& color, const Path& path) const
{
    const auto& edge = m_edges[side];
    const auto& adjacentEdge1 = m_edges[adjacentSide1];
    const auto& adjacentEdge2 = m_edges[adjacentSide2];
    auto thickness = std::max(adjacentEdge2.width(), std::max(edge.width(), adjacentEdge1.width()));

    context.save();
    clipBoxSide(context, side);
    paintBoxSide(context, side, edge.style(), color, thickness, path);
    context.restore();
}

void BorderPainter::paintBoxSide(GraphicsContext& context, BoxSide side, LineStyle style, const Color& color, float thickness, const Path& path) const
{
    switch(style) {
    case LineStyle::Dashed:
    case LineStyle::Dotted: {
        StrokeData strokeData(thickness * 2.f);
        if(style == LineStyle::Dashed)
            strokeData.setDashArray({thickness * 3.f});
        else {
            strokeData.setDashArray({thickness});
        }

        context.setColor(color);
        context.strokePath(path, strokeData);
        break;
    }

    case LineStyle::Double: {
        RoundedRect outerClipRect(m_outer - edgeOutsets(m_edges, 1.f / 3.f));
        RoundedRect innerClipRect(m_outer - edgeOutsets(m_edges, 2.f / 3.f));

        context.save();
        context.clipRoundedRect(innerClipRect);
        paintBoxSide(context, side, LineStyle::Solid, color, thickness, path);
        context.restore();

        context.save();
        context.clipOutRoundedRect(outerClipRect);
        paintBoxSide(context, side, LineStyle::Solid, color, thickness, path);
        context.restore();
        break;
    }

    case LineStyle::Ridge:
    case LineStyle::Groove: {
        LineStyle s1;
        LineStyle s2;
        if(style == LineStyle::Groove) {
            s1 = LineStyle::Inset;
            s2 = LineStyle::Outset;
        } else {
            s1 = LineStyle::Outset;
            s2 = LineStyle::Inset;
        }

        RoundedRect clipRect(m_outer - edgeOutsets(m_edges, 1.f / 2.f));
        paintBoxSide(context, side, s1, color, thickness, path);
        context.save();
        context.clipRoundedRect(clipRect);
        paintBoxSide(context, side, s2, color, thickness, path);
        context.restore();
        break;
    }

    case LineStyle::Solid:
        context.setColor(color);
        context.fillRect(m_outer.rect());
        break;
    case LineStyle::Inset:
    case LineStyle::Outset:
        if((side == BoxSideTop || side == BoxSideLeft) == (style == LineStyle::Inset)) {
            context.setColor(color.darken());
        } else {
            context.setColor(color.lighten());
        }

        context.fillRect(m_outer.rect());
        break;
    default:
        assert(false);
    }
}

static void findIntersection(const Point& p1, const Point& p2, const Point& d1, const Point& d2, Point& intersection)
{
    auto pxLength = p2.x - p1.x;
    auto pyLength = p2.y - p1.y;

    auto dxLength = d2.x - d1.x;
    auto dyLength = d2.y - d1.y;

    auto denom = pxLength * dyLength - pyLength * dxLength;
    if(denom == 0.f)
        return;
    auto param = ((d1.x - p1.x) * dyLength - (d1.y - p1.y) * dxLength) / denom;
    intersection.x = p1.x + param * pxLength;
    intersection.y = p1.y + param * pyLength;
}

void BorderPainter::clipBoxSide(GraphicsContext& context, BoxSide side) const
{
    const auto& outerRect = m_outer.rect();
    const auto& innerRect = m_inner.rect();
    const auto& innerRadii = m_inner.radii();

    Point quad[4];
    switch(side) {
    case BoxSideTop:
        quad[0] = outerRect.topLeft();
        quad[1] = innerRect.topLeft();
        quad[2] = innerRect.topRight();
        quad[3] = outerRect.topRight();
        if(!innerRadii.tl.isZero())
            findIntersection(outerRect.topLeft(), innerRect.topLeft(), innerRect.bottomLeft(), innerRect.topRight(), quad[1]);
        if(!innerRadii.tr.isZero())
            findIntersection(outerRect.topRight(), innerRect.topRight(), innerRect.topLeft(), innerRect.bottomRight(), quad[2]);
        break;
    case BoxSideLeft:
        quad[0] = outerRect.topLeft();
        quad[1] = innerRect.topLeft();
        quad[2] = innerRect.bottomLeft();
        quad[3] = outerRect.bottomLeft();
        if(!innerRadii.tl.isZero())
            findIntersection(outerRect.topLeft(), innerRect.topLeft(), innerRect.bottomLeft(), innerRect.topRight(), quad[1]);
        if(!innerRadii.bl.isZero())
            findIntersection(outerRect.bottomLeft(), innerRect.bottomLeft(), innerRect.topLeft(), innerRect.bottomRight(), quad[2]);
        break;
    case BoxSideBottom:
        quad[0] = outerRect.bottomLeft();
        quad[1] = innerRect.bottomLeft();
        quad[2] = innerRect.bottomRight();
        quad[3] = outerRect.bottomRight();
        if(!innerRadii.bl.isZero())
            findIntersection(outerRect.bottomLeft(), innerRect.bottomLeft(), innerRect.topLeft(), innerRect.bottomRight(), quad[1]);
        if(!innerRadii.br.isZero())
            findIntersection(outerRect.bottomRight(), innerRect.bottomRight(), innerRect.topRight(), innerRect.bottomLeft(), quad[2]);
        break;
    case BoxSideRight:
        quad[0] = outerRect.topRight();
        quad[1] = innerRect.topRight();
        quad[2] = innerRect.bottomRight();
        quad[3] = outerRect.bottomRight();
        if(!innerRadii.tr.isZero())
            findIntersection(outerRect.topRight(), innerRect.topRight(), innerRect.topLeft(), innerRect.bottomRight(), quad[1]);
        if(!innerRadii.br.isZero())
            findIntersection(outerRect.bottomRight(), innerRect.bottomRight(), innerRect.topRight(), innerRect.bottomLeft(), quad[2]);
        break;
    }

    Path path;
    path.moveTo(quad[0].x, quad[0].y);
    for(size_t i = 1; i < 4; i++)
        path.lineTo(quad[i].x, quad[i].y);
    context.clipPath(path, FillRule::NonZero);
}

} // namespace plutobook
