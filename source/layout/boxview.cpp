/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "boxview.h"
#include "pagebox.h"
#include "document.h"

namespace plutobook {

BoxView::BoxView(Document* document, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(document, style)
{
}

Rect BoxView::backgroundRect() const
{
    if(m_currentPage)
        return document()->pageContentRectAt(m_currentPage->pageIndex());
    return Rect(0, 0, document()->width(), document()->height());
}

void BoxView::paintRootBackground(const PaintInfo& info) const
{
    if(m_backgroundStyle) {
        paintBackgroundStyle(info, backgroundRect(), m_backgroundStyle);
    }
}

void BoxView::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
}

void BoxView::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
}

void BoxView::layout(FragmentBuilder* fragmentainer)
{
    setWidth(document()->containerWidth());
    BlockFlowBox::layout(fragmentainer);
    updateLayerPosition();
}

void BoxView::build()
{
    auto bodyStyle = document()->bodyStyle();
    if(bodyStyle)
        style()->setDirection(bodyStyle->direction());
    m_backgroundStyle = document()->rootStyle();
    if(m_backgroundStyle && !m_backgroundStyle->hasBackground()
        && bodyStyle && bodyStyle->hasBackground()) {
        m_backgroundStyle = bodyStyle;
    }

    if(m_backgroundStyle) {
        auto node = m_backgroundStyle->node();
        if(auto box = node->box()) {
            box->setIsBackgroundStolen(true);
        }
    }

    BlockFlowBox::build();
}

} // namespace plutobook
