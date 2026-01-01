/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "svgboxmodel.h"
#include "svgresourcebox.h"

namespace plutobook {

bool SVGBlendInfo::requiresCompositing(SVGRenderMode mode) const
{
    return (m_clipper && m_clipper->requiresMasking()) || (mode == SVGRenderMode::Painting && (m_masker || m_opacity < 1.f || m_blendMode > BlendMode::Normal));
}

SVGRenderState::SVGRenderState(const SVGBlendInfo& info, const Box* box, const SVGRenderState& parent, const Transform& localTransform)
    : SVGRenderState(info, box, &parent, parent.mode(), parent.context(), parent.currentTransform() * localTransform)
{
}

SVGRenderState::SVGRenderState(const SVGBlendInfo& info, const Box* box, const SVGRenderState& parent, SVGRenderMode mode, GraphicsContext& context)
    : SVGRenderState(info, box, &parent, mode, context, context.getTransform())
{
}

SVGRenderState::SVGRenderState(const SVGBlendInfo& info, const Box* box, const SVGRenderState* parent, SVGRenderMode mode, GraphicsContext& context, const Transform& currentTransform)
    : m_box(box), m_parent(parent), m_info(info), m_context(context), m_currentTransform(currentTransform)
    , m_mode(mode), m_requiresCompositing(info.requiresCompositing(mode))
{
    if(m_requiresCompositing) {
        m_context.pushGroup();
    } else {
        m_context.save();
    }

    m_context.setTransform(m_currentTransform);
    if(m_info.clipper() && !m_requiresCompositing) {
        m_info.clipper()->applyClipPath(*this);
    }
}

SVGRenderState::~SVGRenderState()
{
    const auto& element = to<SVGElement>(*m_box->node());
    if(m_parent && (element.isLinkSource() || element.isLinkDestination()))
        m_box->paintAnnotation(m_context, m_box->paintBoundingBox());
    if(m_requiresCompositing) {
        if(m_info.clipper())
            m_info.clipper()->applyClipMask(*this);
        if(m_mode == SVGRenderMode::Painting) {
            if(m_info.masker())
                m_info.masker()->applyMask(*this);
            m_context.popGroup(m_info.opacity(), m_info.blendMode());
        } else {
            m_context.popGroup(1.0);
        }
    } else {
        m_context.restore();
    }
}

bool SVGRenderState::hasCycleReference(const Box* box) const
{
    auto current = this;
    do {
        if(box == current->box())
            return true;
        current = current->parent();
    } while(current);
    return false;
}

void SVGPaintServer::applyPaint(const SVGRenderState& state) const
{
    if(m_painter) {
        m_painter->applyPaint(state, m_opacity);
    } else {
        state->setColor(m_color.colorWithAlpha(m_opacity));
    }
}

SVGBoxModel::SVGBoxModel(SVGElement* element, const RefPtr<BoxStyle>& style)
    : Box(element, style)
{
    setIsInline(false);
}

void SVGBoxModel::layout()
{
    m_paintBoundingBox = Rect::Invalid;
}

void SVGBoxModel::build()
{
    m_clipper = element()->getClipper(style()->clipPath());
    m_masker = element()->getMasker(style()->mask());
    Box::build();
}

Rect SVGBoxModel::paintBoundingBox() const
{
    if(m_paintBoundingBox.isValid())
        return m_paintBoundingBox;
    m_paintBoundingBox = strokeBoundingBox();
    assert(m_paintBoundingBox.isValid());
    if(m_clipper)
        m_paintBoundingBox.intersect(m_clipper->clipBoundingBox(this));
    if(m_masker) {
        m_paintBoundingBox.intersect(m_masker->maskBoundingBox(this));
    }

    return m_paintBoundingBox;
}

} // namespace plutobook
