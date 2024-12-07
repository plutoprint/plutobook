#include "pagebox.h"
#include "document.h"

namespace plutobook {

std::unique_ptr<PageBox> PageBox::create(const RefPtr<BoxStyle>& style, const GlobalString& pageName, uint32_t pageIndex)
{
    return std::unique_ptr<PageBox>(new (style->heap()) PageBox(style, pageName, pageIndex));
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
    auto marginLeftLength = style()->marginLeft();
    auto marginRightLength = style()->marginRight();

    const auto& deviceMargins = document()->book()->pageMargins();
    marginLeft = marginLeftLength.isAuto() ? deviceMargins.left() / units::px : marginLeftLength.calcMin(m_pageSize.width() / units::px);
    marginRight = marginRightLength.isAuto() ? deviceMargins.right() / units::px : marginRightLength.calcMin(m_pageSize.width() / units::px);
    width = m_pageSize.width() / units::px;
}

void PageBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    auto marginTopLength = style()->marginTop();
    auto marginBottomLength = style()->marginBottom();

    const auto& deviceMargins = document()->book()->pageMargins();
    marginTop = marginTopLength.isAuto() ? deviceMargins.top() / units::px : marginTopLength.calcMin(m_pageSize.height() / units::px);
    marginBottom = marginBottomLength.isAuto() ? deviceMargins.bottom() / units::px : marginBottomLength.calcMin(m_pageSize.height() / units::px);
    height = m_pageSize.height() / units::px;
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

PageBox::PageBox(const RefPtr<BoxStyle>& style, const GlobalString& pageName, uint32_t pageIndex)
    : BlockBox(nullptr, style)
    , m_pageSize(style->getPageSize(style->book()->pageSize()))
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
