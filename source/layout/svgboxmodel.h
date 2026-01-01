/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_SVGBOXMODEL_H
#define PLUTOBOOK_SVGBOXMODEL_H

#include "box.h"
#include "svgdocument.h"
#include "graphicscontext.h"

namespace plutobook {

enum class SVGRenderMode {
    Painting,
    Clipping
};

class SVGBlendInfo {
public:
    SVGBlendInfo(const SVGResourceClipperBox* clipper, const SVGResourceMaskerBox* masker, const BoxStyle* style)
        : SVGBlendInfo(clipper, masker, style->opacity(), style->blendMode())
    {}

    SVGBlendInfo(const SVGResourceClipperBox* clipper, const SVGResourceMaskerBox* masker, float opacity, BlendMode blendMode)
        : m_clipper(clipper), m_masker(masker), m_opacity(opacity), m_blendMode(blendMode)
    {}

    const SVGResourceClipperBox* clipper() const { return m_clipper; }
    const SVGResourceMaskerBox* masker() const { return m_masker; }
    const float opacity() const { return m_opacity; }
    const BlendMode blendMode() const { return m_blendMode; }

    bool requiresCompositing(SVGRenderMode mode) const;

private:
    const SVGResourceClipperBox* m_clipper;
    const SVGResourceMaskerBox* m_masker;
    const float m_opacity;
    const BlendMode m_blendMode;
};

class SVGRenderState {
public:
    SVGRenderState(const SVGBlendInfo& info, const Box* box, const SVGRenderState& parent, const Transform& localTransform);
    SVGRenderState(const SVGBlendInfo& info, const Box* box, const SVGRenderState& parent, SVGRenderMode mode, GraphicsContext& context);
    SVGRenderState(const SVGBlendInfo& info, const Box* box, const SVGRenderState* parent, SVGRenderMode mode, GraphicsContext& context, const Transform& currentTransform);

    ~SVGRenderState();

    GraphicsContext& operator*() const { return m_context; }
    GraphicsContext* operator->() const { return &m_context; }

    const Box* box() const { return m_box; }
    const SVGRenderState* parent() const { return m_parent; }
    const SVGBlendInfo& info() const { return m_info; }
    GraphicsContext& context() const { return m_context; }
    const Transform& currentTransform() const { return m_currentTransform; }
    const SVGRenderMode mode() const { return m_mode; }

    Rect fillBoundingBox() const { return m_box->fillBoundingBox(); }
    Rect paintBoundingBox() const { return m_box->paintBoundingBox(); }

    bool hasCycleReference(const Box* box) const;

private:
    const Box* m_box;
    const SVGRenderState* m_parent;
    const SVGBlendInfo& m_info;
    GraphicsContext& m_context;
    const Transform m_currentTransform;
    const SVGRenderMode m_mode;
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
    Rect paintBoundingBox() const override;
    virtual void render(const SVGRenderState& state) const = 0;
    virtual void layout();
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
