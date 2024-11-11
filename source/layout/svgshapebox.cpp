#include "svgshapebox.h"
#include "svgresourcebox.h"

#include <numbers>

namespace plutobook {

Rect SVGMarkerPosition::markerBoundingBox(float strokeWidth) const
{
    return m_marker->markerBoundingBox(m_origin, m_angle, strokeWidth);
}

void SVGMarkerPosition::renderMarker(const SVGRenderState& state, float strokeWidth) const
{
    m_marker->renderMarker(state, m_origin, m_angle, strokeWidth);
}

SVGShapeBox::SVGShapeBox(SVGGeometryElement* element, const RefPtr<BoxStyle>& style)
    : SVGBoxModel(element, style)
{
}

const Rect& SVGShapeBox::fillBoundingBox() const
{
    if(!m_fillBoundingBox.isValid())
        m_fillBoundingBox = m_path.boundingRect();
    return m_fillBoundingBox;
}

const Rect& SVGShapeBox::strokeBoundingBox() const
{
    if(m_strokeBoundingBox.isValid())
        return m_strokeBoundingBox;
    m_strokeBoundingBox = fillBoundingBox();
    if(style()->hasStroke()) {
        float caplimit = m_strokeData.lineWidth() / 2.f;
        if(m_strokeData.lineCap() == LineCap::Square)
            caplimit *= std::numbers::sqrt2;
        float joinlimit = m_strokeData.lineWidth() / 2.f;
        if(m_strokeData.lineJoin() == LineJoin::Miter) {
            joinlimit *= m_strokeData.miterLimit();
        }

        m_strokeBoundingBox.inflate(std::max(caplimit, joinlimit));
    }

    for(auto& markerPosition : m_markerData.positions())
        m_strokeBoundingBox.unite(markerPosition.markerBoundingBox(m_markerData.strokeWidth()));
    return m_strokeBoundingBox;
}

void SVGShapeBox::render(const SVGRenderState& state) const
{
    if(style()->visibility() != Visibility::Visible)
        return;
    SVGBlendInfo blendInfo(m_clipper, m_masker, style());
    SVGRenderState newState(blendInfo, this, state, element()->transform());
    if(newState.mode() == SVGRenderMode::Clipping) {
        newState->setColor(Color::White);
        newState->fillPath(m_path, style()->clipRule());
    } else {
        if(m_fill.isRenderable()) {
            m_fill.applyPaint(newState);
            newState->fillPath(m_path, style()->fillRule());
        }

        if(m_stroke.isRenderable()) {
            m_stroke.applyPaint(newState);
            newState->strokePath(m_path, m_strokeData);
        }

        for(auto& markerPosition : m_markerData.positions()) {
            markerPosition.renderMarker(newState, m_markerData.strokeWidth());
        }
    }
}

void SVGShapeBox::build()
{
    m_path = element()->path();
    m_strokeData = element()->getStrokeData(style());
    m_markerData = element()->getMarkerData(m_path, style());
    m_fill = element()->getPaintServer(style()->fill(), style()->fillOpacity());
    m_stroke = element()->getPaintServer(style()->stroke(), style()->strokeOpacity());
    SVGBoxModel::build();
}

} // namespace plutobook
