#ifndef PLUTOBOOK_SVGSHAPEBOX_H
#define PLUTOBOOK_SVGSHAPEBOX_H

#include "svgboxmodel.h"

namespace plutobook {

class SVGMarkerPosition {
public:
    SVGMarkerPosition(const SVGResourceMarkerBox* marker, const Point& origin, float angle)
        : m_marker(marker), m_origin(origin), m_angle(angle)
    {}

    const SVGResourceMarkerBox* marker() const { return m_marker; }
    const Point& origin() const { return m_origin; }
    float angle() const { return m_angle; }

    Rect markerBoundingBox(float strokeWidth) const;
    void renderMarker(const SVGRenderState& state, float strokeWidth) const;

private:
    const SVGResourceMarkerBox* m_marker;
    Point m_origin;
    float m_angle;
};

using SVGMarkerPositionList = std::vector<SVGMarkerPosition>;

class SVGMarkerData {
public:
    SVGMarkerData() = default;
    SVGMarkerData(float strokeWidth, SVGMarkerPositionList&& positions)
        : m_strokeWidth(strokeWidth), m_positions(std::move(positions))
    {}

    float strokeWidth() const { return m_strokeWidth; }
    const SVGMarkerPositionList& positions() const { return m_positions; }

private:
    float m_strokeWidth{1.f};
    SVGMarkerPositionList m_positions;
};

class SVGShapeBox final : public SVGBoxModel {
public:
    SVGShapeBox(SVGGeometryElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGShapeBox() const final { return true; }

    SVGGeometryElement* element() const;
    const Path& path() const { return m_path; }
    const Transform& localTransform() const final { return element()->transform(); }
    const Rect& fillBoundingBox() const final;
    const Rect& strokeBoundingBox() const final;
    void render(const SVGRenderState& state) const final;
    void build() final;

    const char* name() const final { return "SVGShapeBox"; }

private:
    Path m_path;
    StrokeData m_strokeData;
    SVGMarkerData m_markerData;
    SVGPaintServer m_fill;
    SVGPaintServer m_stroke;
    mutable Rect m_fillBoundingBox = Rect::Invalid;
    mutable Rect m_strokeBoundingBox = Rect::Invalid;
};

template<>
struct is_a<SVGShapeBox> {
    static bool check(const Box& box) { return box.isSVGShapeBox(); }
};

inline SVGGeometryElement* SVGShapeBox::element() const
{
    return static_cast<SVGGeometryElement*>(node());
}

} // namespace plutobook

#endif // PLUTOBOOK_SVGSHAPEBOX_H
