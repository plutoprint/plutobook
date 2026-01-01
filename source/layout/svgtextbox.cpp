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

SVGTSpanBox::SVGTSpanBox(SVGTSpanElement* element, const RefPtr<BoxStyle>& style)
    : Box(element, style)
{
    setIsInline(true);
}

void SVGTSpanBox::build()
{
    m_fill = element()->getPaintServer(style()->fill(), style()->fillOpacity());
    Box::build();
}

SVGTextBox::SVGTextBox(SVGTextElement* element, const RefPtr<BoxStyle>& style)
    : SVGBoxModel(element, style)
    , m_lineLayout(this)
{
}

Rect SVGTextBox::fillBoundingBox() const
{
    if(!m_fillBoundingBox.isValid())
        m_fillBoundingBox = m_lineLayout.boundingRect();
    return m_fillBoundingBox;
}

void SVGTextBox::render(const SVGRenderState& state) const
{
    if(style()->visibility() != Visibility::Visible)
        return;
    SVGBlendInfo blendInfo(m_clipper, m_masker, style());
    SVGRenderState newState(blendInfo, this, state, element()->transform());
    if(newState.mode() == SVGRenderMode::Clipping) {
        newState->setColor(Color::White);
    } else {
        m_fill.applyPaint(newState);
    }

    m_lineLayout.render(newState);
}

void SVGTextBox::layout()
{
    m_fillBoundingBox = Rect::Invalid;
    m_lineLayout.layout();
    SVGBoxModel::layout();
}

void SVGTextBox::build()
{
    m_fill = element()->getPaintServer(style()->fill(), style()->fillOpacity());
    m_lineLayout.build();
    SVGBoxModel::build();
}

} // namespace plutobook
