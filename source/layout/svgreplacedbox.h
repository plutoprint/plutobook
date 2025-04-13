#ifndef PLUTOBOOK_SVGREPLACEDBOX_H
#define PLUTOBOOK_SVGREPLACEDBOX_H

#include "replacedbox.h"
#include "svgboxmodel.h"

namespace plutobook {

class SVGSVGElement;

class SVGRootBox final : public ReplacedBox {
public:
    SVGRootBox(SVGSVGElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGRootBox() const final { return true; }
    bool requiresLayer() const final;

    SVGSVGElement* element() const;
    const Rect& fillBoundingBox() const final;
    const Rect& strokeBoundingBox() const final;
    const Rect& paintBoundingBox() const final;
    void layout(FragmentBuilder* fragmentainer) final;
    void build() final;

    float intrinsicReplacedWidth() const final { return 300.f; }
    float intrinsicReplacedHeight() const final { return 150.f; }

    void updateOverflowRect() final;
    void computeIntrinsicRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const final;
    void paintReplaced(const PaintInfo& info, const Point& offset) final;

    const char* name() const final { return "SVGRootBox"; }

private:
    mutable Rect m_fillBoundingBox = Rect::Invalid;
    mutable Rect m_strokeBoundingBox = Rect::Invalid;
    mutable Rect m_paintBoundingBox = Rect::Invalid;
    const SVGResourceClipperBox* m_clipper = nullptr;
    const SVGResourceMaskerBox* m_masker = nullptr;
};

template<>
struct is_a<SVGRootBox> {
    static bool check(const Box& box) { return box.isSVGRootBox(); }
};

inline SVGSVGElement* SVGRootBox::element() const
{
    return static_cast<SVGSVGElement*>(node());
}

class SVGImageElement;

class SVGImageBox final : public SVGBoxModel {
public:
    SVGImageBox(SVGImageElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGImageBox() const final { return true; }

    SVGImageElement* element() const;
    const RefPtr<Image>& image() const { return m_image; }
    const Transform& localTransform() const final { return element()->transform(); }
    const Rect& fillBoundingBox() const final;
    const Rect& strokeBoundingBox() const final;
    void render(const SVGRenderState& state) const final;
    void layout() final;
    void build() final;

    const char* name() const final { return "SVGImageBox"; }

private:
    RefPtr<Image> m_image;
    mutable Rect m_fillBoundingBox;
};

template<>
struct is_a<SVGImageBox> {
    static bool check(const Box& box) { return box.isSVGImageBox(); }
};

inline SVGImageElement* SVGImageBox::element() const
{
    return static_cast<SVGImageElement*>(node());
}

} // namespace plutobook

#endif // PLUTOBOOK_SVGREPLACEDBOX_H
