#include "pagebox.h"
#include "document.h"

namespace plutobook {

std::unique_ptr<PageBox> PageBox::create(const RefPtr<BoxStyle>& style, const PageSize& pageSize, const GlobalString& pageName, uint32_t pageIndex)
{
    return std::unique_ptr<PageBox>(new (style->heap()) PageBox(style, pageSize, pageName, pageIndex));
}

void PageBox::updateOverflowRect()
{
    BlockBox::updateOverflowRect();
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        addOverflowRect(child, child->x(), child->y());
    }
}

void PageBox::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    assert(false);
}

void PageBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
}

void PageBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
}

void PageBox::layout(FragmentBuilder* fragmentainer)
{
    updateWidth();
    updateHeight();
}

void PageBox::build()
{
    BlockBox::build();
}

PageBox::PageBox(const RefPtr<BoxStyle>& style, const PageSize& pageSize, const GlobalString& pageName, uint32_t pageIndex)
    : BlockBox(nullptr, style)
    , m_pageSize(pageSize)
    , m_pageName(pageName)
    , m_pageIndex(pageIndex)
{
}

PageMarginBox::PageMarginBox(const RefPtr<BoxStyle>& style, PageMarginType marginType)
    : BlockFlowBox(nullptr, style)
    , m_marginType(marginType)
{
}

} // namespace plutobook
