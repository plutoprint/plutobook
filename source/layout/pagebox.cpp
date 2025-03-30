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
    std::array<PageMarginBox*, 16> entries = {};
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

void PageBox::layoutCornerPageMargin(PageMarginBox* cornerBox, const Rect& cornerRect)
{
    if(cornerBox == nullptr) {
        return;
    }

    cornerBox->resolvePaddings(cornerRect.size());
    cornerBox->layoutContents(cornerRect.size());
    cornerBox->resolveMargins(cornerRect.size());

    cornerBox->setX(cornerRect.x + cornerBox->marginLeft());
    cornerBox->setY(cornerRect.y + cornerBox->marginTop());
}

void PageBox::layoutEdgePageMargin(PageMarginBox* edgeBox, const Rect& edgeRect, float mainAxisSize)
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

bool PageMarginBox::isHorizontalFlow() const
{
    switch(m_marginType) {
    case PageMarginType::TopLeftCorner:
    case PageMarginType::TopRightCorner:
    case PageMarginType::RightTop:
    case PageMarginType::RightMiddle:
    case PageMarginType::RightBottom:
    case PageMarginType::BottomRightCorner:
    case PageMarginType::BottomLeftCorner:
    case PageMarginType::LeftTop:
    case PageMarginType::LeftMiddle:
    case PageMarginType::LeftBottom:
        return true;
    default:
        return false;
    }
}

bool PageMarginBox::isVerticalFlow() const
{
    switch(m_marginType) {
    case PageMarginType::TopLeftCorner:
    case PageMarginType::TopLeft:
    case PageMarginType::TopCenter:
    case PageMarginType::TopRight:
    case PageMarginType::TopRightCorner:
    case PageMarginType::BottomRightCorner:
    case PageMarginType::BottomLeft:
    case PageMarginType::BottomCenter:
    case PageMarginType::BottomRight:
    case PageMarginType::BottomLeftCorner:
        return true;
    default:
        return false;
    }
}

void PageMarginBox::resolvePaddings(const Size& availableSize)
{
    auto paddingTopLength = style()->paddingTop();
    auto paddingRightLength = style()->paddingRight();
    auto paddingBottomLength = style()->paddingBottom();
    auto paddingLeftLength = style()->paddingLeft();

    auto paddingTop = paddingTopLength.calcMin(availableSize.h);
    auto paddingRight = paddingRightLength.calcMin(availableSize.w);
    auto paddingBottom = paddingBottomLength.calcMin(availableSize.h);
    auto paddingLeft = paddingLeftLength.calcMin(availableSize.w);

    m_paddingTop = paddingTop;
    m_paddingRight = paddingRight;
    m_paddingBottom = paddingBottom;
    m_paddingLeft = paddingLeft;
}

void PageMarginBox::resolveMargins(const Size& availableSize)
{
    auto marginTopLength = style()->marginTop();
    auto marginRightLength = style()->marginRight();
    auto marginBottomLength = style()->marginBottom();
    auto marginLeftLength = style()->marginLeft();

    auto marginTop = marginTopLength.calcMin(availableSize.h);
    auto marginRight = marginRightLength.calcMin(availableSize.w);
    auto marginBottom = marginBottomLength.calcMin(availableSize.h);
    auto marginLeft = marginLeftLength.calcMin(availableSize.w);

    if(isVerticalFlow()) {
        auto availableSpace = std::max(0.f, availableSize.h - marginTop - marginBottom - height());

        float autoMarginOffset = 0.f;
        if(marginTopLength.isAuto() && marginBottomLength.isAuto())
            autoMarginOffset += availableSpace / 2.f;
        else
            autoMarginOffset += availableSpace;
        if(marginTopLength.isAuto())
            marginTop += autoMarginOffset;
        if(marginBottomLength.isAuto()) {
            marginBottom += autoMarginOffset;
        }

        auto additionalSpace = availableSize.h - marginTop - marginBottom - height();
        switch(m_marginType) {
        case PageMarginType::TopLeftCorner:
        case PageMarginType::TopLeft:
        case PageMarginType::TopCenter:
        case PageMarginType::TopRight:
        case PageMarginType::TopRightCorner:
            marginTop += additionalSpace;
            break;
        default:
            marginBottom += additionalSpace;
            break;
        }
    }

    if(isHorizontalFlow()) {
        auto availableSpace = std::max(0.f, availableSize.w - marginLeft - marginRight - width());

        float autoMarginOffset = 0.f;
        if(marginLeftLength.isAuto() && marginRightLength.isAuto())
            autoMarginOffset += availableSpace / 2.f;
        else
            autoMarginOffset += availableSpace;
        if(marginLeftLength.isAuto())
            marginLeft += autoMarginOffset;
        if(marginRightLength.isAuto()) {
            marginRight += autoMarginOffset;
        }

        auto additionalSpace = availableSize.h - marginLeft - marginRight - width();
        switch(m_marginType) {
        case PageMarginType::TopLeftCorner:
        case PageMarginType::BottomLeftCorner:
        case PageMarginType::LeftTop:
        case PageMarginType::LeftMiddle:
        case PageMarginType::LeftBottom:
            marginLeft += additionalSpace;
            break;
        default:
            marginRight += additionalSpace;
            break;
        }
    }

    m_marginTop = marginTop;
    m_marginRight = marginRight;
    m_marginBottom = marginBottom;
    m_marginLeft = marginLeft;
}

void PageMarginBox::layoutContents(const Size& availableSize)
{
    auto widthLength = style()->width();
    auto minWidthLength = style()->minWidth();
    auto maxWidthLength = style()->maxWidth();

    auto heightLength = style()->height();
    auto minHeightLength = style()->minHeight();
    auto maxHeightLength = style()->maxHeight();

    auto width = adjustBorderBoxWidth(widthLength.calc(availableSize.w));
    if(!maxWidthLength.isNone())
        width = std::min(width, adjustBorderBoxWidth(maxWidthLength.calc(availableSize.w)));
    if(!minWidthLength.isAuto()) {
        width = std::max(width, adjustBorderBoxWidth(minWidthLength.calc(availableSize.w)));
    }

    auto height = adjustBorderBoxHeight(heightLength.calc(availableSize.h));
    if(!maxHeightLength.isNone())
        height = std::min(height, adjustBorderBoxHeight(maxHeightLength.calc(availableSize.h)));
    if(!minHeightLength.isAuto()) {
        height = std::max(height, adjustBorderBoxHeight(minHeightLength.calc(availableSize.h)));
    }

    setWidth(width);
    layout(nullptr);
    setHeight(height);
}

void PageMarginBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
}

void PageMarginBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
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

    auto marginBox = new (m_document->heap()) PageMarginBox(marginStyle, marginType);
    Counters marginCounters(counters);
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
