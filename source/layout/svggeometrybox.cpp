/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "svggeometrybox.h"
#include "svgresourcebox.h"

#include <cmath>

namespace plutobook {

Rect SVGMarkerPosition::markerBoundingBox(float strokeWidth) const
{
    return m_marker->markerBoundingBox(m_origin, m_angle, strokeWidth);
}

void SVGMarkerPosition::renderMarker(const SVGRenderState& state, float strokeWidth) const
{
    m_marker->renderMarker(state, m_origin, m_angle, strokeWidth);
}

SVGGeometryBox::SVGGeometryBox(SVGGeometryElement* element, const RefPtr<BoxStyle>& style)
    : SVGBoxModel(element, style)
{
}

Rect SVGGeometryBox::fillBoundingBox() const
{
    if(!m_fillBoundingBox.isValid())
        m_fillBoundingBox = path().boundingRect();
    return m_fillBoundingBox;
}

Rect SVGGeometryBox::strokeBoundingBox() const
{
    if(m_strokeBoundingBox.isValid())
        return m_strokeBoundingBox;
    m_strokeBoundingBox = fillBoundingBox();
    if(style()->hasStroke()) {
        auto strokeData = element()->getStrokeData(style());
        auto caplimit = strokeData.lineWidth() / 2.f;
        if(strokeData.lineCap() == LineCap::Square)
            caplimit *= kSqrt2;
        auto joinlimit = strokeData.lineWidth() / 2.f;
        if(strokeData.lineJoin() == LineJoin::Miter) {
            joinlimit *= strokeData.miterLimit();
        }

        m_strokeBoundingBox.inflate(std::max(caplimit, joinlimit));
    }

    if(!m_markerPositions.empty()) {
        SVGLengthContext lengthContext(element());
        auto strokeWidth = lengthContext.valueForLength(style()->strokeWidth());
        for(const auto& markerPosition : m_markerPositions) {
            m_strokeBoundingBox.unite(markerPosition.markerBoundingBox(strokeWidth));
        }
    }

    return m_strokeBoundingBox;
}

void SVGGeometryBox::render(const SVGRenderState& state) const
{
    if(style()->visibility() != Visibility::Visible)
        return;
    SVGBlendInfo blendInfo(m_clipper, m_masker, style());
    SVGRenderState newState(blendInfo, this, state, element()->transform());
    if(newState.mode() == SVGRenderMode::Clipping) {
        newState->setColor(Color::White);
        newState->fillPath(path(), style()->clipRule());
    } else {
        if(m_fill.isRenderable()) {
            m_fill.applyPaint(newState);
            newState->fillPath(path(), style()->fillRule());
        }

        if(m_stroke.isRenderable()) {
            m_stroke.applyPaint(newState);
            newState->strokePath(path(), element()->getStrokeData(style()));
        }

        if(!m_markerPositions.empty()) {
            SVGLengthContext lengthContext(element());
            auto strokeWidth = lengthContext.valueForLength(style()->strokeWidth());
            for(const auto& markerPosition : m_markerPositions) {
                markerPosition.renderMarker(newState, strokeWidth);
            }
        }
    }
}

void SVGGeometryBox::layout()
{
    m_strokeBoundingBox = Rect::Invalid;
    SVGBoxModel::layout();
    updateMarkerPositions();
}

void SVGGeometryBox::build()
{
    m_fill = element()->getPaintServer(style()->fill(), style()->fillOpacity());
    m_stroke = element()->getPaintServer(style()->stroke(), style()->strokeOpacity());
    m_markerStart = element()->getMarker(style()->markerStart());
    m_markerMid = element()->getMarker(style()->markerMid());
    m_markerEnd = element()->getMarker(style()->markerEnd());
    SVGBoxModel::build();
}

void SVGGeometryBox::updateMarkerPositions()
{
    m_markerPositions.clear();
    if(m_markerStart == nullptr && m_markerMid == nullptr && m_markerEnd == nullptr) {
        return;
    }

    Point origin;
    Point startPoint;
    Point inslopePoints[2];
    Point outslopePoints[2];

    int index = 0;
    std::array<Point, 3> points;
    PathIterator it(path());

    while(!it.isDone()) {
        switch(it.currentSegment(points)) {
        case PathCommand::MoveTo:
            startPoint = points[0];
            inslopePoints[0] = origin;
            inslopePoints[1] = points[0];
            origin = points[0];
            break;
        case PathCommand::LineTo:
            inslopePoints[0] = origin;
            inslopePoints[1] = points[0];
            origin = points[0];
            break;
        case PathCommand::CubicTo:
            inslopePoints[0] = points[1];
            inslopePoints[1] = points[2];
            origin = points[2];
            break;
        case PathCommand::Close:
            inslopePoints[0] = origin;
            inslopePoints[1] = points[0];
            origin = startPoint;
            startPoint = Point();
            break;
        }

        it.next();

        if(!it.isDone() && (m_markerStart || m_markerMid)) {
            it.currentSegment(points);
            outslopePoints[0] = origin;
            outslopePoints[1] = points[0];
            if(index == 0 && m_markerStart) {
                auto slope = outslopePoints[1] - outslopePoints[0];
                auto angle = rad2deg(std::atan2(slope.y, slope.x));
                const auto& orient = m_markerStart->element()->orient();
                if(orient.orientType() == SVGAngle::OrientType::AutoStartReverse)
                    angle -= 180.f;
                m_markerPositions.emplace_back(m_markerStart, origin, angle);
            }

            if(index > 0 && m_markerMid) {
                auto inslope = inslopePoints[1] - inslopePoints[0];
                auto outslope = outslopePoints[1] - outslopePoints[0];
                auto inangle = rad2deg(std::atan2(inslope.y, inslope.x));
                auto outangle = rad2deg(std::atan2(outslope.y, outslope.x));
                if(std::abs(inangle - outangle) > 180.f)
                    inangle += 360.f;
                auto angle = (inangle + outangle) * 0.5f;
                m_markerPositions.emplace_back(m_markerMid, origin, angle);
            }
        }

        if(m_markerEnd && it.isDone()) {
            auto slope = inslopePoints[1] - inslopePoints[0];
            auto angle = rad2deg(std::atan2(slope.y, slope.x));
            m_markerPositions.emplace_back(m_markerEnd, origin, angle);
        }

        index += 1;
    }
}

SVGPathBox::SVGPathBox(SVGPathElement* element, const RefPtr<BoxStyle>& style)
    : SVGGeometryBox(element, style)
{
}

SVGShapeBox::SVGShapeBox(SVGShapeElement* element, const RefPtr<BoxStyle>& style)
    : SVGGeometryBox(element, style)
{
}

void SVGShapeBox::layout()
{
    m_path.clear();
    m_fillBoundingBox = element()->getPath(m_path);
    SVGGeometryBox::layout();
}

} // namespace plutobook
