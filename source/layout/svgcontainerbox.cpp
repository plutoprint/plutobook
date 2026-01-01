/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "svgcontainerbox.h"

namespace plutobook {

SVGContainerBox::SVGContainerBox(SVGElement* element, const RefPtr<BoxStyle>& style)
    : SVGBoxModel(element, style)
{
}

Rect SVGContainerBox::fillBoundingBox() const
{
    if(m_fillBoundingBox.isValid())
        return m_fillBoundingBox;
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        const auto& transform = child->localTransform();
        if(child->isSVGHiddenContainerBox())
            continue;
        m_fillBoundingBox.unite(transform.mapRect(child->fillBoundingBox()));
    }

    if(!m_fillBoundingBox.isValid())
        m_fillBoundingBox = Rect::Empty;
    return m_fillBoundingBox;
}

Rect SVGContainerBox::strokeBoundingBox() const
{
    if(m_strokeBoundingBox.isValid())
        return m_strokeBoundingBox;
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        const auto& transform = child->localTransform();
        if(child->isSVGHiddenContainerBox())
            continue;
        m_strokeBoundingBox.unite(transform.mapRect(child->strokeBoundingBox()));
    }

    if(!m_strokeBoundingBox.isValid())
        m_strokeBoundingBox = Rect::Empty;
    return m_strokeBoundingBox;
}

void SVGContainerBox::layout()
{
    SVGBoxModel::layout();

    m_fillBoundingBox = Rect::Invalid;
    m_strokeBoundingBox = Rect::Invalid;
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        if(auto box = to<SVGBoxModel>(child)) {
            box->layout();
        }
    }
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

void SVGTransformableContainerBox::render(const SVGRenderState& state) const
{
    SVGBlendInfo blendInfo(m_clipper, m_masker, style());
    SVGRenderState newState(blendInfo, this, state, m_localTransform);
    renderChildren(newState);
}

inline const SVGUseElement* toSVGUseElement(const SVGElement* element)
{
    if(element->tagName() == useTag)
        return static_cast<const SVGUseElement*>(element);
    return nullptr;
}

void SVGTransformableContainerBox::layout()
{
    m_localTransform = element()->transform();
    if(auto useElement = toSVGUseElement(element())) {
        SVGLengthContext lengthContext(useElement);
        const Point translation = {
            lengthContext.valueForLength(useElement->x()),
            lengthContext.valueForLength(useElement->y())
        };

        m_localTransform.translate(translation.x, translation.y);
    }

    SVGContainerBox::layout();
}

SVGViewportContainerBox::SVGViewportContainerBox(SVGSVGElement* element, const RefPtr<BoxStyle>& style)
    : SVGContainerBox(element, style)
{
    setIsOverflowHidden(style->isOverflowHidden());
}

void SVGViewportContainerBox::render(const SVGRenderState& state) const
{
    SVGBlendInfo blendInfo(m_clipper, m_masker, style());
    SVGRenderState newState(blendInfo, this, state, m_localTransform);
    if(isOverflowHidden()) {
        SVGLengthContext lengthContext(element());
        const Size viewportSize = {
            lengthContext.valueForLength(element()->width()),
            lengthContext.valueForLength(element()->height())
        };

        newState->clipRect(element()->getClipRect(viewportSize));
    }

    renderChildren(newState);
}

void SVGViewportContainerBox::layout()
{
    SVGLengthContext lengthContext(element());
    const Rect viewportRect = {
        lengthContext.valueForLength(element()->x()),
        lengthContext.valueForLength(element()->y()),
        lengthContext.valueForLength(element()->width()),
        lengthContext.valueForLength(element()->height())
    };

    m_localTransform = element()->transform() * Transform::makeTranslate(viewportRect.x, viewportRect.y) * element()->viewBoxToViewTransform(viewportRect.size());
    SVGContainerBox::layout();
}

SVGResourceContainerBox::SVGResourceContainerBox(SVGElement* element, const RefPtr<BoxStyle>& style)
    : SVGHiddenContainerBox(element, style)
{
}

} // namespace plutobook
