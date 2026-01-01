/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_SVGGEOMETRYBOX_H
#define PLUTOBOOK_SVGGEOMETRYBOX_H

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

class SVGGeometryBox : public SVGBoxModel {
public:
    SVGGeometryBox(SVGGeometryElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGGeometryBox() const final { return true; }

    virtual const Path& path() const = 0;

    SVGGeometryElement* element() const;
    Transform localTransform() const override { return element()->transform(); }
    Rect fillBoundingBox() const override;
    Rect strokeBoundingBox() const override;

    void render(const SVGRenderState& state) const override;
    void layout() override;
    void build() override;

    const char* name() const override { return "SVGGeometryBox"; }

protected:
    void updateMarkerPositions();

    SVGPaintServer m_fill;
    SVGPaintServer m_stroke;
    SVGMarkerPositionList m_markerPositions;

    const SVGResourceMarkerBox* m_markerStart = nullptr;
    const SVGResourceMarkerBox* m_markerMid = nullptr;
    const SVGResourceMarkerBox* m_markerEnd = nullptr;

    mutable Rect m_fillBoundingBox = Rect::Invalid;
    mutable Rect m_strokeBoundingBox = Rect::Invalid;
};

template<>
struct is_a<SVGGeometryBox> {
    static bool check(const Box& box) { return box.isSVGGeometryBox(); }
};

inline SVGGeometryElement* SVGGeometryBox::element() const
{
    return static_cast<SVGGeometryElement*>(node());
}

class SVGPathBox final : public SVGGeometryBox {
public:
    SVGPathBox(SVGPathElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGPathBox() const final { return true; }

    SVGPathElement* element() const;
    const Path& path() const final { return element()->path(); }

    const char* name() const final { return "SVGPathBox"; }
};

template<>
struct is_a<SVGPathBox> {
    static bool check(const Box& box) { return box.isSVGPathBox(); }
};

inline SVGPathElement* SVGPathBox::element() const
{
    return static_cast<SVGPathElement*>(node());
}

class SVGShapeBox final : public SVGGeometryBox {
public:
    SVGShapeBox(SVGShapeElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGShapeBox() const final { return true; }

    SVGShapeElement* element() const;
    const Path& path() const final { return m_path; }
    void layout() final;

    const char* name() const final { return "SVGShapeBox"; }

private:
    Path m_path;
};

template<>
struct is_a<SVGShapeBox> {
    static bool check(const Box& box) { return box.isSVGShapeBox(); }
};

inline SVGShapeElement* SVGShapeBox::element() const
{
    return static_cast<SVGShapeElement*>(node());
}

} // namespace plutobook

#endif // PLUTOBOOK_SVGGEOMETRYBOX_H
