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
    assert(false);
}

void PageBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    assert(false);
}

void PageBox::layout(FragmentBuilder* fragmentainer)
{
    auto child = firstBoxFrame();
    while(child) {
        child->layout(nullptr);
        child = child->nextBoxFrame();
    }

    updateOverflowRect();
}

void PageBox::build()
{
    BlockBox::build();
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

constexpr PseudoType pagePseudoType(size_t pageIndex)
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
    for(size_t pageIndex = 0; pageIndex < counters.pageCount(); ++pageIndex) {
        auto pageStyle = m_document->styleForPage(emptyGlo, pageIndex, pagePseudoType(pageIndex));
        auto pageBox = PageBox::create(pageStyle, m_pageSize, emptyGlo, pageIndex);

        counters.update(pageBox.get());
        buildPageMargins(counters, pageBox.get());

        pageBox->setWidth(m_pageWidth);
        pageBox->setHeight(m_pageHeight);

        pageBox->setMarginTop(m_marginTop);
        pageBox->setMarginRight(m_marginRight);
        pageBox->setMarginBottom(m_marginBottom);
        pageBox->setMarginLeft(m_marginLeft);

        pageBox->build();
        pageBox->layout(nullptr);

        m_document->pages().push_back(std::move(pageBox));
    }
}

void PageBoxBuilder::buildPageMargin(const Counters& counters, PageBox* pageBox, PageMarginType marginType)
{
}

void PageBoxBuilder::buildPageMargins(const Counters& counters, PageBox* pageBox)
{
}

} // namespace plutobook
