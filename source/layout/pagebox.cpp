#include "pagebox.h"
#include "contentbox.h"
#include "counters.h"
#include "document.h"
#include "graphicscontext.h"
#include "cssrule.h"

#include <cmath>

namespace plutobook {

std::unique_ptr<PageBox> PageBox::create(const RefPtr<BoxStyle>& style, const PageSize& pageSize, const GlobalString& pageName, uint32_t pageIndex)
{
    return std::unique_ptr<PageBox>(new (style->heap()) PageBox(style, pageSize, pageName, pageIndex));
}

void PageBox::updateOverflowRect()
{
    BlockBox::updateOverflowRect();
}

void PageBox::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    assert(false);
}

void PageBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    assert(false);
}

void PageBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    assert(false);
}

class PageMarginMap {
public:
    PageMarginMap() = default;

    PageMarginBox* operator[](PageMarginType type) const {
        return entries[static_cast<size_t>(type)];
    }

    PageMarginBox*& operator[](PageMarginType type) {
        return entries[static_cast<size_t>(type)];
    }

private:
    std::array<PageMarginBox*, 16> entries;
};

void PageBox::layout(FragmentBuilder* fragmentainer)
{
    PageMarginMap margins;
    for(auto child = firstMarginBox(); child; child = child->nextMarginBox()) {
        margins[child->marginType()] = child;
    }

    auto pageWidth = width();
    auto pageHeight = height();

    auto leftWidth = marginLeft();
    auto rightWidth = marginRight();

    auto topHeight = marginTop();
    auto bottomHeight = marginBottom();

    Rect topLeftCornerRect(0, 0, leftWidth, topHeight);
    Rect topRightCornerRect(pageWidth - rightWidth, 0, rightWidth, topHeight);
    Rect bottomRightCornerRect(pageWidth - rightWidth, pageHeight - bottomHeight, rightWidth, bottomHeight);
    Rect bottomLeftCornerRect(0, pageHeight - bottomHeight, leftWidth, bottomHeight);

    Rect topEdgeRect(leftWidth, 0, pageWidth - leftWidth - rightWidth, topHeight);
    Rect rightEdgeRect(pageWidth - rightWidth, topHeight, rightWidth, pageHeight - topHeight - bottomHeight);
    Rect bottomEdgeRect(leftWidth, pageHeight - bottomHeight, pageWidth - leftWidth - rightWidth, bottomHeight);
    Rect leftEdgeRect(0, topHeight, leftWidth, pageHeight - topHeight - bottomHeight);

    layoutCornerPageMargin(margins[PageMarginType::TopLeftCorner], topLeftCornerRect);
    layoutEdgePageMargins(margins[PageMarginType::TopLeft], margins[PageMarginType::TopCenter], margins[PageMarginType::TopRight], topEdgeRect);

    layoutCornerPageMargin(margins[PageMarginType::TopRightCorner], topRightCornerRect);
    layoutEdgePageMargins(margins[PageMarginType::RightTop], margins[PageMarginType::RightMiddle], margins[PageMarginType::RightBottom], rightEdgeRect);

    layoutCornerPageMargin(margins[PageMarginType::BottomRightCorner], bottomRightCornerRect);
    layoutEdgePageMargins(margins[PageMarginType::BottomLeft], margins[PageMarginType::BottomCenter], margins[PageMarginType::BottomRight], bottomEdgeRect);

    layoutCornerPageMargin(margins[PageMarginType::BottomLeftCorner], bottomLeftCornerRect);
    layoutEdgePageMargins(margins[PageMarginType::LeftTop], margins[PageMarginType::LeftMiddle], margins[PageMarginType::LeftBottom], leftEdgeRect);

    updateOverflowRect();
}

void PageBox::build()
{
    BlockBox::build();
}

void PageBox::layoutCornerPageMargin(PageMarginBox* cornerBox, const Rect& cornerRect)
{
}

void PageBox::layoutEdgePageMargins(PageMarginBox* edgeStartBox, PageMarginBox* edgeCenterBox, PageMarginBox* edgeEndBox, const Rect& edgeRect)
{
}

void PageBox::paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    auto pageContentRect = document()->pageContentRectAt(m_pageIndex);
    if(phase == PaintPhase::Contents && !pageContentRect.isEmpty()) {
        info->save();
        info->translate(marginLeft(), marginTop());
        info->scale(document()->pageContentScale(), document()->pageContentScale());
        info->translate(-pageContentRect.x, -pageContentRect.y);
        info->clipRect(pageContentRect);
        document()->render(*info, pageContentRect);
        info->restore();
    }
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

void PageMarginBox::layout(FragmentBuilder* fragmentainer)
{
    BlockFlowBox::layout(fragmentainer);
}

void PageMarginBox::build()
{
    BlockFlowBox::build();
}

PageBoxBuilder::PageBoxBuilder(Document* document, const PageSize& pageSize, float pageWidth, float pageHeight, float marginTop, float marginRight, float marginBottom, float marginLeft)
    : m_document(document)
    , m_pageSize(pageSize)
    , m_pageWidth(pageWidth)
    , m_pageHeight(pageHeight)
    , m_marginTop(marginTop)
    , m_marginRight(marginRight)
    , m_marginBottom(marginBottom)
    , m_marginLeft(marginLeft)
{
}

constexpr PseudoType pagePseudoType(uint32_t pageIndex)
{
    if(pageIndex == 0)
        return PseudoType::FirstPage;
    if(pageIndex % 2 == 0)
        return PseudoType::RightPage;
    return PseudoType::LeftPage;
}

void PageBoxBuilder::build()
{
    Counters counters(m_document, std::ceil(m_document->height() / m_document->pageContentHeight()));
    for(uint32_t pageIndex = 0; pageIndex < counters.pageCount(); ++pageIndex) {
        auto pageStyle = m_document->styleForPage(emptyGlo, pageIndex, pagePseudoType(pageIndex));
        auto pageBox = PageBox::create(pageStyle, m_pageSize, emptyGlo, pageIndex);

        pageBox->setWidth(m_pageWidth);
        pageBox->setHeight(m_pageHeight);

        pageBox->setMarginTop(m_marginTop);
        pageBox->setMarginRight(m_marginRight);
        pageBox->setMarginBottom(m_marginBottom);
        pageBox->setMarginLeft(m_marginLeft);

        counters.update(pageBox.get());
        buildPageMargins(counters, pageBox.get());

        pageBox->build();
        pageBox->layout(nullptr);

        m_document->pages().push_back(std::move(pageBox));
    }
}

void PageBoxBuilder::buildPageMargin(const Counters& counters, PageBox* pageBox, PageMarginType marginType)
{
    auto marginStyle = m_document->styleForPageMargin(pageBox->pageName(), pageBox->pageIndex(), marginType, *pageBox->style());
    if(marginStyle == nullptr) {
        return;
    }

    Counters marginCounters(counters);
    auto marginBox = new (m_document->heap()) PageMarginBox(marginStyle, marginType);
    marginCounters.update(marginBox);
    ContentBoxBuilder(marginCounters, nullptr, marginBox).build();
    pageBox->addChild(marginBox);
}

void PageBoxBuilder::buildPageMargins(const Counters& counters, PageBox* pageBox)
{
    buildPageMargin(counters, pageBox, PageMarginType::TopLeftCorner);
    buildPageMargin(counters, pageBox, PageMarginType::TopLeft);
    buildPageMargin(counters, pageBox, PageMarginType::TopCenter);
    buildPageMargin(counters, pageBox, PageMarginType::TopRight);

    buildPageMargin(counters, pageBox, PageMarginType::TopRightCorner);
    buildPageMargin(counters, pageBox, PageMarginType::RightTop);
    buildPageMargin(counters, pageBox, PageMarginType::RightMiddle);
    buildPageMargin(counters, pageBox, PageMarginType::RightBottom);

    buildPageMargin(counters, pageBox, PageMarginType::BottomRightCorner);
    buildPageMargin(counters, pageBox, PageMarginType::BottomLeft);
    buildPageMargin(counters, pageBox, PageMarginType::BottomCenter);
    buildPageMargin(counters, pageBox, PageMarginType::BottomRight);

    buildPageMargin(counters, pageBox, PageMarginType::BottomRightCorner);
    buildPageMargin(counters, pageBox, PageMarginType::LeftTop);
    buildPageMargin(counters, pageBox, PageMarginType::LeftMiddle);
    buildPageMargin(counters, pageBox, PageMarginType::LeftBottom);
}

} // namespace plutobook
