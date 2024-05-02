#include "svgboxmodel.h"
#include "svgresourcebox.h"

namespace plutobook {

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

void SVGBlendInfo::beginGroup() const
{
    if(m_requiresCompositing) {
        m_state->pushGroup();
    } else {
        m_state->save();
    }

    m_state->setTransform(m_state.currentTransform());
    if(m_clipper && !m_requiresCompositing) {
        m_clipper->applyClipPath(m_state);
    }
}

void SVGBlendInfo::endGroup() const
{
    if(m_requiresCompositing) {
        if(m_clipper)
            m_clipper->applyClipMask(m_state);
        if(m_state.mode() == SVGRenderMode::Display) {
            if(m_masker)
                m_masker->applyMask(m_state);
            m_state->popGroup(m_opacity, m_blendMode);
        } else {
            m_state->popGroup(1.0);
        }
    } else {
        m_state->restore();
    }
}

bool SVGBlendInfo::requiresCompositing(SVGRenderMode mode) const
{
    return (m_clipper && m_clipper->requiresMasking()) || (mode == SVGRenderMode::Display && (m_masker || m_opacity < 1.f || m_blendMode > BlendMode::Normal));
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
    setInline(false);
}

void SVGBoxModel::build()
{
    m_clipper = element()->getClipper(style()->clipPath());
    m_masker = element()->getMasker(style()->mask());
    Box::build();
}

const Rect& SVGBoxModel::paintBoundingBox() const
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
