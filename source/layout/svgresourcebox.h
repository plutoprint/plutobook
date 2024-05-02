#ifndef PLUTOBOOK_SVGRESOURCEBOX_H
#define PLUTOBOOK_SVGRESOURCEBOX_H

#include "svgcontainerbox.h"

namespace plutobook {

class SVGResourceMarkerBox final : public SVGResourceContainerBox {
public:
    SVGResourceMarkerBox(SVGMarkerElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGResourceMarkerBox() const final { return true; }

    SVGMarkerElement* element() const;
    const Transform& localTransform() const final { return m_localTransform; }
    Transform markerTransform(const Point& origin, float angle, float strokeWidth) const;
    Rect markerBoundingBox(const Point& origin, float angle, float strokeWidth) const;
    void renderMarker(const SVGRenderState& state, const Point& origin, float angle, float strokeWidth) const;
    void build() final;

    const char* name() const final { return "SVGResourceMarkerBox"; }

private:
    Point m_refPoint;
    Rect m_clipRect;
    Transform m_localTransform;
};

template<>
struct is_a<SVGResourceMarkerBox> {
    static bool check(const Box& box) { return box.isSVGResourceMarkerBox(); }
};

inline SVGMarkerElement* SVGResourceMarkerBox::element() const
{
    return static_cast<SVGMarkerElement*>(node());
}

class SVGResourceClipperBox final : public SVGResourceContainerBox {
public:
    SVGResourceClipperBox(SVGClipPathElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGResourceClipperBox() const final { return true; }
    bool requiresMasking() const;

    SVGClipPathElement* element() const;
    Rect clipBoundingBox(const Box* box) const;
    void applyClipPath(const SVGRenderState& state) const;
    void applyClipMask(const SVGRenderState& state) const;

    const char* name() const final { return "SVGResourceClipperBox"; }
};

template<>
struct is_a<SVGResourceClipperBox> {
    static bool check(const Box& box) { return box.isSVGResourceClipperBox(); }
};

inline SVGClipPathElement* SVGResourceClipperBox::element() const
{
    return static_cast<SVGClipPathElement*>(node());
}

class SVGResourceMaskerBox final : public SVGResourceContainerBox {
public:
    SVGResourceMaskerBox(SVGMaskElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGResourceMaskerBox() const final { return true; }

    SVGMaskElement* element() const;
    Rect maskBoundingBox(const Box* box) const;
    void applyMask(const SVGRenderState& state) const;
    void build() final;

    const char* name() const final { return "SVGResourceMaskerBox"; }

private:
    Rect m_maskRect;
};

template<>
struct is_a<SVGResourceMaskerBox> {
    static bool check(const Box& box) { return box.isSVGResourceMaskerBox(); }
};

inline SVGMaskElement* SVGResourceMaskerBox::element() const
{
    return static_cast<SVGMaskElement*>(node());
}

class SVGResourcePaintServerBox : public SVGResourceContainerBox {
public:
    SVGResourcePaintServerBox(SVGElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGResourcePaintServerBox() const final { return true; }

    virtual void applyPaint(const SVGRenderState& state, float opacity) const = 0;

    const char* name() const override { return "SVGResourcePaintServerBox"; }
};

template<>
struct is_a<SVGResourcePaintServerBox> {
    static bool check(const Box& box) { return box.isSVGResourcePaintServerBox(); }
};

class SVGResourcePatternBox final : public SVGResourcePaintServerBox {
public:
    SVGResourcePatternBox(SVGPatternElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGResourcePatternBox() const final { return true; }

    SVGPatternElement* element() const;
    void applyPaint(const SVGRenderState& state, float opacity) const final;
    void build() final;

    const char* name() const final { return "SVGResourcePatternBox"; }

private:
    const SVGResourcePatternBox* m_patternContentBox;
    Transform m_patternTransform;
    SVGUnitsType m_patternUnits;
    SVGUnitsType m_patternContentUnits;
    SVGPreserveAspectRatio m_preserveAspectRatio;
    Rect m_viewBox;
    Rect m_patternRect;
};

template<>
struct is_a<SVGResourcePatternBox> {
    static bool check(const Box& box) { return box.isSVGResourcePatternBox(); }
};

inline SVGPatternElement* SVGResourcePatternBox::element() const
{
    return static_cast<SVGPatternElement*>(node());
}

class SVGGradientStopBox final : public Box {
public:
    SVGGradientStopBox(SVGStopElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGGradientStopBox() const final { return true; }
    SVGStopElement* element() const;

    const char* name() const final { return "SVGGradientStopBox"; }
};

template<>
struct is_a<SVGGradientStopBox> {
    static bool check(const Box& box) { return box.isSVGGradientStopBox(); }
};

inline SVGStopElement* SVGGradientStopBox::element() const
{
    return static_cast<SVGStopElement*>(node());
}

class SVGResourceGradientBox : public SVGResourcePaintServerBox {
public:
    SVGResourceGradientBox(SVGGradientElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGResourceGradientBox() const final { return true; }
    SVGGradientElement* element() const;

    const char* name() const override { return "SVGResourceGradientBox"; }

protected:
    Transform m_gradientTransform;
    GradientStops m_gradientStops;
    SVGUnitsType m_gradientUnits;
    SpreadMethod m_spreadMethod;
};

template<>
struct is_a<SVGResourceGradientBox> {
    static bool check(const Box& box) { return box.isSVGResourceGradientBox(); }
};

inline SVGGradientElement* SVGResourceGradientBox::element() const
{
    return static_cast<SVGGradientElement*>(node());
}

class SVGResourceLinearGradientBox final : public SVGResourceGradientBox {
public:
    SVGResourceLinearGradientBox(SVGLinearGradientElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGResourceLinearGradientBox() const final { return true; }
    SVGLinearGradientElement* element() const;
    void applyPaint(const SVGRenderState& state, float opacity) const final;
    void build() final;

    const char* name() const final { return "SVGResourceLinearGradientBox"; }

private:
    LinearGradientValues m_values;
};

template<>
struct is_a<SVGResourceLinearGradientBox> {
    static bool check(const Box& box) { return box.isSVGResourceLinearGradientBox(); }
};

inline SVGLinearGradientElement* SVGResourceLinearGradientBox::element() const
{
    return static_cast<SVGLinearGradientElement*>(node());
}

class SVGResourceRadialGradientBox final : public SVGResourceGradientBox {
public:
    SVGResourceRadialGradientBox(SVGRadialGradientElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGResourceRadialGradientBox() const final { return true; }

    SVGRadialGradientElement* element() const;
    void applyPaint(const SVGRenderState& state, float opacity) const final;
    void build() final;

    const char* name() const final { return "SVGResourceRadialGradientBox"; }

private:
    RadialGradientValues m_values;
};

template<>
struct is_a<SVGResourceRadialGradientBox> {
    static bool check(const Box& box) { return box.isSVGResourceRadialGradientBox(); }
};

inline SVGRadialGradientElement* SVGResourceRadialGradientBox::element() const
{
    return static_cast<SVGRadialGradientElement*>(node());
}

} // namespace plutobook

#endif // PLUTOBOOK_SVGRESOURCEBOX_H
