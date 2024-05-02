#include "svgcontainerbox.h"

namespace plutobook {

SVGContainerBox::SVGContainerBox(SVGElement* element, const RefPtr<BoxStyle>& style)
    : SVGBoxModel(element, style)
{
}

const Rect& SVGContainerBox::fillBoundingBox() const
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

const Rect& SVGContainerBox::strokeBoundingBox() const
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

void SVGContainerBox::renderChildren(const SVGRenderState& state) const
{
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        if(auto box = to<SVGBoxModel>(child)) {
            box->render(state);
        }
    }
}

SVGHiddenContainerBox::SVGHiddenContainerBox(SVGElement* element, const RefPtr<BoxStyle>& style)
    : SVGContainerBox(element, style)
{
}

void SVGHiddenContainerBox::render(const SVGRenderState& state) const
{
}

SVGTransformableContainerBox::SVGTransformableContainerBox(SVGGraphicsElement* element, const RefPtr<BoxStyle>& style)
    : SVGContainerBox(element, style)
{
}

inline const SVGUseElement* toSVGUseElement(const SVGElement* element)
{
    if(element->tagName() == useTag)
        return static_cast<const SVGUseElement*>(element);
    return nullptr;
}

void SVGTransformableContainerBox::build()
{
    m_localTransform = element()->transform();
    if(auto useElement = toSVGUseElement(element())) {
        SVGLengthContext lengthContext(useElement);
        auto tx = lengthContext.valueForLength(useElement->x());
        auto ty = lengthContext.valueForLength(useElement->y());
        m_localTransform.translate(tx, ty);
    }

    SVGContainerBox::build();
}

void SVGTransformableContainerBox::render(const SVGRenderState& state) const
{
    SVGRenderState newState(this, state, m_localTransform);
    SVGBlendInfo blendInfo(newState, m_clipper, m_masker, style());
    blendInfo.beginGroup();
    renderChildren(newState);
    blendInfo.endGroup();
}

SVGViewportContainerBox::SVGViewportContainerBox(SVGSVGElement* element, const RefPtr<BoxStyle>& style)
    : SVGContainerBox(element, style)
{
    setOverflowHidden(style->isOverflowHidden());
}

void SVGViewportContainerBox::render(const SVGRenderState& state) const
{
    if(m_clipRect.isEmpty())
        return;
    SVGRenderState newState(this, state, m_localTransform);
    SVGBlendInfo blendInfo(newState, m_clipper, m_masker, style());
    blendInfo.beginGroup();
    if(isOverflowHidden())
        newState->clipRect(m_clipRect);
    renderChildren(newState);
    blendInfo.endGroup();
}

void SVGViewportContainerBox::build()
{
    SVGLengthContext lengthContext(element());
    const Rect viewportRect = {
        lengthContext.valueForLength(element()->x()),
        lengthContext.valueForLength(element()->y()),
        lengthContext.valueForLength(element()->width()),
        lengthContext.valueForLength(element()->height())
    };

    m_clipRect = element()->getClipRect(viewportRect.size());
    m_localTransform = element()->transform() * Transform::translated(viewportRect.x, viewportRect.y) * element()->viewBoxToViewTransform(viewportRect.size());
    SVGContainerBox::build();
}

SVGResourceContainerBox::SVGResourceContainerBox(SVGElement* element, const RefPtr<BoxStyle>& style)
    : SVGHiddenContainerBox(element, style)
{
}

} // namespace plutobook
