#include "svgreplacedbox.h"
#include "svgresourcebox.h"
#include "imageresource.h"

namespace plutobook {

SVGRootBox::SVGRootBox(SVGSVGElement* element, const RefPtr<BoxStyle>& style)
    : ReplacedBox(element, style)
{
}

bool SVGRootBox::requiresLayer() const
{
    return isPositioned() || isRelPositioned() || hasTransform() || style()->zIndex();
}

const Rect& SVGRootBox::fillBoundingBox() const
{
    if(m_fillBoundingBox.isValid())
        return m_fillBoundingBox;
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        auto& transform = child->localTransform();
        if(child->isSVGHiddenContainerBox())
            continue;
        m_fillBoundingBox.unite(transform.mapRect(child->fillBoundingBox()));
    }

    if(!m_fillBoundingBox.isValid())
        m_fillBoundingBox = Rect::Empty;
    return m_fillBoundingBox;
}

const Rect& SVGRootBox::strokeBoundingBox() const
{
    if(m_strokeBoundingBox.isValid())
        return m_strokeBoundingBox;
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        auto& transform = child->localTransform();
        if(child->isSVGHiddenContainerBox())
            continue;
        m_strokeBoundingBox.unite(transform.mapRect(child->strokeBoundingBox()));
    }

    if(!m_strokeBoundingBox.isValid())
        m_strokeBoundingBox = Rect::Empty;
    return m_strokeBoundingBox;
}

const Rect& SVGRootBox::paintBoundingBox() const
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

void SVGRootBox::build()
{
    m_clipper = element()->getClipper(style()->clipPath());
    m_masker = element()->getMasker(style()->mask());
    ReplacedBox::build();
}

void SVGRootBox::updateOverflowRect()
{
    ReplacedBox::updateOverflowRect();
    auto contentRect = contentBoxRect();
    auto localTransform = Transform::translated(contentRect.x, contentRect.y);
    localTransform.multiply(element()->viewBoxToViewTransform(contentRect.size()));
    addOverflowRect(localTransform.mapRect(paintBoundingBox()));
}

void SVGRootBox::computeIntrinsicRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const
{
    element()->computeIntrinsicRatioInformation(intrinsicWidth, intrinsicHeight, intrinsicRatio);
}

void SVGRootBox::paintReplaced(const PaintInfo& info, const Point& offset)
{
    Rect borderRect(offset, size());
    if(borderRect.isEmpty())
        return;
    auto topWidth = borderTop() + paddingTop();
    auto bottomWidth = borderBottom() + paddingBottom();
    auto leftWidth = borderLeft() + paddingLeft();
    auto rightWidth = borderRight() + paddingRight();

    SVGBlendInfo blendInfo(m_clipper, m_masker, style());
    SVGRenderState newState(blendInfo, this, nullptr, SVGRenderMode::Painting, *info, info->getTransform());
    if(isOverflowHidden()) {
        auto clipRect = style()->getBorderRoundedRect(borderRect, true, true);
        clipRect.shrink(topWidth, bottomWidth, leftWidth, rightWidth);
        newState->clipRoundedRect(clipRect);
    }

    borderRect.shrink(topWidth, bottomWidth, leftWidth, rightWidth);
    newState->translate(borderRect.x, borderRect.y);
    newState->addTransform(element()->viewBoxToViewTransform(borderRect.size()));
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        if(auto box = to<SVGBoxModel>(child)) {
            box->render(newState);
        }
    }
}

SVGImageBox::SVGImageBox(SVGImageElement* element, const RefPtr<BoxStyle>& style)
    : SVGBoxModel(element, style)
    , m_image(element->image())
{
}

void SVGImageBox::render(const SVGRenderState& state) const
{
    if(m_image == nullptr || state.mode() != SVGRenderMode::Painting || style()->visibility() != Visibility::Visible)
        return;
    auto& preserveAspectRatio = element()->preserveAspectRatio();
    Rect dstRect(m_viewportRect);
    Rect srcRect(0, 0, m_image->width(), m_image->height());
    preserveAspectRatio.transformRect(dstRect, srcRect);

    SVGBlendInfo blendInfo(m_clipper, m_masker, style());
    SVGRenderState newState(blendInfo, this, state, element()->transform());
    m_image->draw(*newState, dstRect, srcRect);
}

void SVGImageBox::build()
{
    SVGLengthContext lengthContext(element());
    m_viewportRect.x = lengthContext.valueForLength(element()->x());
    m_viewportRect.y = lengthContext.valueForLength(element()->y());
    m_viewportRect.w = lengthContext.valueForLength(element()->width());
    m_viewportRect.h = lengthContext.valueForLength(element()->height());
    SVGBoxModel::build();
}

} // namespace plutobook
