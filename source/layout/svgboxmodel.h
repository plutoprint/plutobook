#ifndef PLUTOBOOK_SVGBOXMODEL_H
#define PLUTOBOOK_SVGBOXMODEL_H

#include "box.h"
#include "svgdocument.h"
#include "graphicscontext.h"

namespace plutobook {

enum class SVGRenderMode {
    Display,
    Clipping
};

class SVGRenderState {
public:
    SVGRenderState(const Box* box, const SVGRenderState& parent, const Transform& transform)
        : m_box(box), m_parent(&parent), m_context(parent.context())
        , m_mode(parent.mode()), m_currentTransform(m_context.getTransform() * transform)
    {}

    SVGRenderState(const Box* box, const SVGRenderState& parent, GraphicsContext& context, SVGRenderMode mode)
        : m_box(box), m_parent(&parent), m_context(context)
        , m_mode(mode), m_currentTransform(context.getTransform())
    {}

    SVGRenderState(const Box* box, GraphicsContext& context)
        : m_box(box), m_parent(nullptr), m_context(context)
        , m_mode(SVGRenderMode::Display), m_currentTransform(context.getTransform())
    {}

    GraphicsContext& operator*() const { return m_context; }
    GraphicsContext* operator->() const { return &m_context; }

    const Box* box() const { return m_box; }
    const SVGRenderState* parent() const { return m_parent; }
    GraphicsContext& context() const { return m_context; }
    SVGRenderMode mode() const { return m_mode; }
    const Rect& fillBoundingBox() const { return m_box->fillBoundingBox(); }
    const Rect& paintBoundingBox() const { return m_box->paintBoundingBox(); }
    const Transform& currentTransform() const { return m_currentTransform; }
    Rect mapRect(const Rect& rect) const { return m_currentTransform.mapRect(rect); }

    bool hasCycleReference(const Box* box) const;

private:
    const Box* m_box;
    const SVGRenderState* m_parent;
    GraphicsContext& m_context;
    SVGRenderMode m_mode;
    Transform m_currentTransform;
};

class SVGBlendInfo {
public:
    SVGBlendInfo(const SVGRenderState& state, const SVGResourceClipperBox* clipper, const SVGResourceMaskerBox* masker, const BoxStyle* style)
        : SVGBlendInfo(state, clipper, masker, style->opacity(), style->blendMode())
    {}

    SVGBlendInfo(const SVGRenderState& state, const SVGResourceClipperBox* clipper, const SVGResourceMaskerBox* masker, float opacity, BlendMode blendMode)
        : m_state(state), m_clipper(clipper), m_masker(masker), m_opacity(opacity)
        , m_blendMode(blendMode), m_requiresCompositing(requiresCompositing(state.mode()))
    {}

    void beginGroup() const;
    void endGroup() const;

private:
    bool requiresCompositing(SVGRenderMode mode) const;
    const SVGRenderState& m_state;
    const SVGResourceClipperBox* m_clipper;
    const SVGResourceMaskerBox* m_masker;
    const float m_opacity;
    const BlendMode m_blendMode;
    const bool m_requiresCompositing;
};

class SVGPaintServer {
public:
    SVGPaintServer() = default;
    SVGPaintServer(const SVGResourcePaintServerBox* painter, const Color& color, float opacity)
        : m_painter(painter), m_color(color), m_opacity(opacity)
    {}

    bool isRenderable() const { return m_opacity > 0.f && (m_painter || m_color.alpha() > 0); }

    const SVGResourcePaintServerBox* painter() const { return m_painter; }
    const Color& color() const { return  m_color; }
    float opacity() const { return m_opacity; }

    void applyPaint(const SVGRenderState& state) const;

private:
    const SVGResourcePaintServerBox* m_painter{nullptr};
    Color m_color;
    float m_opacity{0.f};
};

class SVGBoxModel : public Box {
public:
    SVGBoxModel(SVGElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGBoxModel() const final { return true; }

    SVGElement* element() const;
    const Rect& paintBoundingBox() const override;
    virtual void render(const SVGRenderState& state) const = 0;
    void build() override;

    const SVGResourceClipperBox* clipper() const { return m_clipper; }
    const SVGResourceMaskerBox* masker() const { return m_masker; }

protected:
    mutable Rect m_paintBoundingBox = Rect::Invalid;
    const SVGResourceClipperBox* m_clipper = nullptr;
    const SVGResourceMaskerBox* m_masker = nullptr;
};

template<>
struct is_a<SVGBoxModel> {
    static bool check(const Box& box) { return box.isSVGBoxModel(); }
};

inline SVGElement* SVGBoxModel::element() const
{
    return static_cast<SVGElement*>(node());
}

} // namespace plutobook

#endif // PLUTOBOOK_SVGBOXMODEL_H
