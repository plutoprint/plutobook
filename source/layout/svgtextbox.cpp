/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "svgtextbox.h"

namespace plutobook {

SVGInlineTextBox::SVGInlineTextBox(TextNode* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
    setIsInline(true);
}

SVGInlineBox::SVGInlineBox(SVGTextContentElement* element, const RefPtr<BoxStyle>& style)
    : Box(element, style)
{
    setIsInline(true);
}

void SVGInlineBox::build()
{
    m_fill = element()->getFillPaintServer(style());
    m_stroke = element()->getStrokePaintServer(style());
    Box::build();
}

SVGTSpanBox::SVGTSpanBox(SVGTSpanElement* element, const RefPtr<BoxStyle>& style)
    : SVGInlineBox(element, style)
{
}

SVGTextPathBox::SVGTextPathBox(SVGTextPathElement* element, const RefPtr<BoxStyle>& style)
    : SVGInlineBox(element, style)
{
}

Path SVGTextPathBox::textPath() const
{
    auto targetElement = element()->getTargetElement(document());
    if(targetElement && targetElement->tagName() == pathTag) {
        auto pathElement = static_cast<SVGPathElement*>(targetElement);
        return pathElement->path().transformed(pathElement->transform());
    }

    return Path();
}

SVGTextBox::SVGTextBox(SVGTextElement* element, const RefPtr<BoxStyle>& style)
    : SVGBoxModel(element, style)
    , m_lineLayout(this)
{
}

Rect SVGTextBox::fillBoundingBox() const
{
    if(!m_fillBoundingBox.isValid())
        m_fillBoundingBox = m_lineLayout.boundingRect(false);
    return m_fillBoundingBox;
}

Rect SVGTextBox::strokeBoundingBox() const
{
    if(!m_strokeBoundingBox.isValid())
        m_strokeBoundingBox = m_lineLayout.boundingRect(true);
    return m_strokeBoundingBox;
}

void SVGTextBox::render(const SVGRenderState& state) const
{
    if(style()->visibility() != Visibility::Visible)
        return;
    SVGBlendInfo blendInfo(m_clipper, m_masker, style());
    SVGRenderState newState(blendInfo, this, state, element()->transform());
    m_lineLayout.render(newState);
}

void SVGTextBox::layout()
{
    m_fillBoundingBox = Rect::Invalid;
    m_strokeBoundingBox = Rect::Invalid;
    m_lineLayout.layout();
    SVGBoxModel::layout();
}

void SVGTextBox::build()
{
    m_fill = element()->getFillPaintServer(style());
    m_stroke = element()->getStrokePaintServer(style());
    m_lineLayout.build();
    SVGBoxModel::build();
}

void SVGTextBox::serializeChildren(std::ostream& o, int indent) const
{
    m_lineLayout.serialize(o, indent);
}

} // namespace plutobook
