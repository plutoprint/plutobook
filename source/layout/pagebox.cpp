#include "pagebox.h"
#include "document.h"

namespace plutobook {

std::unique_ptr<PageBox> PageBox::create(const RefPtr<BoxStyle>& style, const GlobalString& pageName, uint32_t pageIndex)
{
    return std::unique_ptr<PageBox>(new (style->heap()) PageBox(style, pageName, pageIndex));
}

const PageSize& PageBox::pageSize() const
{
    return document()->book()->pageSize();
}

void PageBox::updateOverflowRect()
{
    BlockBox::updateOverflowRect();
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        addOverflowRect(child, child->x(), child->y());
    }
}

void PageBox::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    assert(false);
}

void PageBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto book = document()->book();
    width = book->pageSize().height() / units::px;
    marginLeft = book->pageMargins().left() / units::px;
    marginRight = book->pageMargins().right() / units::px;
}

void PageBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    auto book = document()->book();
    height = book->pageSize().height() / units::px;
    marginTop = book->pageMargins().top() / units::px;
    marginBottom = book->pageMargins().bottom() / units::px;
}

void PageBox::layout()
{
    updateWidth();
    updateHeight();
}

void PageBox::build()
{
    BlockBox::build();
}

PageBox::PageBox(const RefPtr<BoxStyle>& style, const GlobalString& pageName, uint32_t pageIndex)
    : BlockBox(nullptr, style)
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
