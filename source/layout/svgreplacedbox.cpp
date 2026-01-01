/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "svgreplacedbox.h"
#include "svgresourcebox.h"
#include "imageresource.h"

namespace plutobook {

SVGRootBox::SVGRootBox(SVGSVGElement* element, const RefPtr<BoxStyle>& style)
    : ReplacedBox(element, style)
{
    setIntrinsicSize(Size(300, 150));
}

bool SVGRootBox::requiresLayer() const
{
    return isPositioned() || isRelativePositioned() || hasTransform() || style()->zIndex();
}

Rect SVGRootBox::fillBoundingBox() const
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

Rect SVGRootBox::strokeBoundingBox() const
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

Rect SVGRootBox::paintBoundingBox() const
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

float SVGRootBox::computePreferredReplacedWidth() const
{
    if(document()->isSVGImageDocument())
        return document()->containerWidth();
    return ReplacedBox::computePreferredReplacedWidth();
}

float SVGRootBox::computeReplacedWidth() const
{
    if(document()->isSVGImageDocument())
        return document()->containerWidth();
    return ReplacedBox::computeReplacedWidth();
}

float SVGRootBox::computeReplacedHeight() const
{
    if(document()->isSVGImageDocument())
        return document()->containerHeight();
    return ReplacedBox::computeReplacedHeight();
}

void SVGRootBox::computeIntrinsicRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const
{
    element()->computeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
}

void SVGRootBox::paintReplaced(const PaintInfo& info, const Point& offset)
{
    const RectOutsets outsets = {
        borderTop() + paddingTop(),
        borderRight() + paddingRight(),
        borderBottom() + paddingBottom(),
        borderLeft() + paddingLeft()
    };

    Rect borderRect(offset, size());
    Rect contentRect(borderRect - outsets);
    if(contentRect.isEmpty()) {
        return;
    }

    if(isOverflowHidden()) {
        auto clipRect = style()->getBorderRoundedRect(borderRect, true, true);
        info->save();
        info->clipRoundedRect(clipRect - outsets);
    }

    auto currentTransform = info->getTransform();
    currentTransform.translate(contentRect.x, contentRect.y);
    currentTransform.multiply(element()->viewBoxToViewTransform(contentRect.size()));

    {
        SVGBlendInfo blendInfo(m_clipper, m_masker, style());
        SVGRenderState newState(blendInfo, this, nullptr, SVGRenderMode::Painting, *info, currentTransform);
        for(auto child = firstChild(); child; child = child->nextSibling()) {
            if(auto box = to<SVGBoxModel>(child)) {
                box->render(newState);
            }
        }
    }

    if(isOverflowHidden()) {
        info->restore();
    }
}

void SVGRootBox::layout(FragmentBuilder* fragmentainer)
{
    ReplacedBox::layout(fragmentainer);

    m_fillBoundingBox = Rect::Invalid;
    m_strokeBoundingBox = Rect::Invalid;
    m_paintBoundingBox = Rect::Invalid;
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        if(auto box = to<SVGBoxModel>(child)) {
            box->layout();
        }
    }

    if(!isOverflowHidden()) {
        auto contentRect = contentBoxRect();
        auto localTransform = Transform::makeTranslate(contentRect.x, contentRect.y);
        localTransform.multiply(element()->viewBoxToViewTransform(contentRect.size()));
        addOverflowRect(localTransform.mapRect(paintBoundingBox()));
    }
}

void SVGRootBox::build()
{
    m_clipper = element()->getClipper(style()->clipPath());
    m_masker = element()->getMasker(style()->mask());
    ReplacedBox::build();
}

SVGImageBox::SVGImageBox(SVGImageElement* element, const RefPtr<BoxStyle>& style)
    : SVGBoxModel(element, style)
{
}

Rect SVGImageBox::fillBoundingBox() const
{
    if(m_fillBoundingBox.isValid())
        return m_fillBoundingBox;
    SVGLengthContext lengthContext(element());
    m_fillBoundingBox.x = lengthContext.valueForLength(element()->x());
    m_fillBoundingBox.y = lengthContext.valueForLength(element()->y());
    m_fillBoundingBox.w = lengthContext.valueForLength(element()->width());
    m_fillBoundingBox.h = lengthContext.valueForLength(element()->height());
    return m_fillBoundingBox;
}

Rect SVGImageBox::strokeBoundingBox() const
{
    return fillBoundingBox();
}

void SVGImageBox::render(const SVGRenderState& state) const
{
    if(m_image == nullptr || state.mode() != SVGRenderMode::Painting || style()->visibility() != Visibility::Visible)
        return;
    Rect dstRect(fillBoundingBox());
    m_image->setContainerSize(dstRect.size());

    Rect srcRect(m_image->size());
    element()->preserveAspectRatio().transformRect(dstRect, srcRect);

    SVGBlendInfo blendInfo(m_clipper, m_masker, style());
    SVGRenderState newState(blendInfo, this, state, element()->transform());
    m_image->draw(*newState, dstRect, srcRect);
}

void SVGImageBox::layout()
{
    m_fillBoundingBox = Rect::Invalid;
    SVGBoxModel::layout();
}

void SVGImageBox::build()
{
    m_image = element()->image();
    SVGBoxModel::build();
}

} // namespace plutobook
