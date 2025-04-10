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
    layoutEdgePageMargins(margins[PageMarginType::TopLeft], margins[PageMarginType::TopCenter], margins[PageMarginType::TopRight], topEdgeRect, BoxSideTop);

    layoutCornerPageMargin(margins[PageMarginType::TopRightCorner], topRightCornerRect);
    layoutEdgePageMargins(margins[PageMarginType::RightTop], margins[PageMarginType::RightMiddle], margins[PageMarginType::RightBottom], rightEdgeRect, BoxSideRight);

    layoutCornerPageMargin(margins[PageMarginType::BottomRightCorner], bottomRightCornerRect);
    layoutEdgePageMargins(margins[PageMarginType::BottomLeft], margins[PageMarginType::BottomCenter], margins[PageMarginType::BottomRight], bottomEdgeRect, BoxSideBottom);

    layoutCornerPageMargin(margins[PageMarginType::BottomLeftCorner], bottomLeftCornerRect);
    layoutEdgePageMargins(margins[PageMarginType::LeftTop], margins[PageMarginType::LeftMiddle], margins[PageMarginType::LeftBottom], leftEdgeRect, BoxSideLeft);

    updateOverflowRect();
    updateLayerPositions();
}

void PageBox::layoutCornerPageMargin(PageMarginBox* cornerBox, const Rect& cornerRect)
{
    if(cornerBox == nullptr) {
        return;
    }

    cornerBox->updateMargins(cornerRect.size());
    cornerBox->updatePaddings(cornerRect.size());
    cornerBox->layoutContent(cornerRect.size());
    cornerBox->updateAutoMargins(cornerRect.size());

    cornerBox->setX(cornerRect.x + cornerBox->marginLeft());
    cornerBox->setY(cornerRect.y + cornerBox->marginTop());
}

constexpr bool isHorizontalEdge(BoxSide side) { return side == BoxSideTop || side == BoxSideBottom; }

void PageBox::layoutEdgePageMargin(PageMarginBox* edgeBox, const Rect& edgeRect, BoxSide edgeSide, float mainAxisSize)
{
    if(edgeBox == nullptr) {
        return;
    }

    if(isHorizontalEdge(edgeSide)) {
        edgeBox->layoutContent(mainAxisSize, edgeRect.h, true, false);
    } else {
        edgeBox->layoutContent(edgeRect.w, mainAxisSize, false, true);
    }

    edgeBox->updateAutoMargins(edgeRect.size());

    auto edgeOffset = edgeRect.origin();
    if(isHorizontalEdge(edgeSide)) {
        auto availableSpace = edgeRect.w - edgeBox->width() - edgeBox->marginWidth();
        switch(edgeBox->marginType()) {
        case PageMarginType::TopCenter:
        case PageMarginType::BottomCenter:
            edgeOffset.x += availableSpace / 2.f;
            break;
        case PageMarginType::TopRight:
        case PageMarginType::BottomRight:
            edgeOffset.x += availableSpace;
            break;
        default:
            break;
        }
    } else {
        auto availableSpace = edgeRect.h - edgeBox->height() - edgeBox->marginHeight();
        switch(edgeBox->marginType()) {
        case PageMarginType::RightMiddle:
        case PageMarginType::LeftMiddle:
            edgeOffset.y += availableSpace / 2.f;
            break;
        case PageMarginType::RightBottom:
        case PageMarginType::LeftBottom:
            edgeOffset.y += availableSpace;
            break;
        default:
            break;
        }
    }

    edgeBox->setX(edgeOffset.x + edgeBox->marginLeft());
    edgeBox->setY(edgeOffset.y + edgeBox->marginTop());
}

class PreferredSizeInfo {
public:
    enum Type {
        Fixed,
        Auto
    };

    PreferredSizeInfo() = default;
    PreferredSizeInfo(Type type, float minSize, float maxSize, float marginSize)
        : m_type(type), m_minSize(minSize), m_maxSize(maxSize), m_marginSize(marginSize)
    {}

    bool isAuto() const { return m_type == Type::Auto; }

    float minSize() const { return m_minSize; }
    float maxSize() const { return m_maxSize; }
    float marginSize() const { return m_marginSize; }

    float minLength() const { return m_minSize + m_marginSize; }
    float maxLength() const { return m_maxSize + m_marginSize; }

    PreferredSizeInfo doubled() const {
        return PreferredSizeInfo(m_type, m_minSize * 2.f, m_maxSize * 2.f, m_marginSize * 2.f);
    }

private:
    Type m_type = Type::Fixed;
    float m_minSize = 0.f;
    float m_maxSize = 0.f;
    float m_marginSize = 0.f;
};

static PreferredSizeInfo computeEdgePreferredSize(PageMarginBox* edgeBox, const Rect& edgeRect, BoxSide edgeSide)
{
    if(edgeBox == nullptr) {
        return PreferredSizeInfo();
    }

    edgeBox->updateMargins(edgeRect.size());
    edgeBox->updatePaddings(edgeRect.size());

    if(isHorizontalEdge(edgeSide)) {
        auto widthLength = edgeBox->style()->width();
        if(widthLength.isAuto()) {
            return PreferredSizeInfo(PreferredSizeInfo::Auto, edgeBox->minPreferredWidth(), edgeBox->maxPreferredWidth(), edgeBox->marginWidth());
        }

        auto minWidthLength = edgeBox->style()->minWidth();
        auto maxWidthLength = edgeBox->style()->maxWidth();

        auto width = edgeBox->adjustBorderBoxWidth(widthLength.calc(edgeRect.w));
        if(!maxWidthLength.isNone())
            width = std::min(width, edgeBox->adjustBorderBoxWidth(maxWidthLength.calc(edgeRect.w)));
        if(!minWidthLength.isAuto()) {
            width = std::max(width, edgeBox->adjustBorderBoxWidth(minWidthLength.calc(edgeRect.w)));
        }

        return PreferredSizeInfo(PreferredSizeInfo::Fixed, width, width, edgeBox->marginWidth());
    }

    auto heightLength = edgeBox->style()->height();
    if(heightLength.isAuto()) {
        edgeBox->layoutWidth(edgeRect.w, false);
        return PreferredSizeInfo(PreferredSizeInfo::Auto, edgeBox->height(), edgeBox->height(), edgeBox->marginHeight());
    }

    auto minHeightLength = edgeBox->style()->minHeight();
    auto maxHeightLength = edgeBox->style()->maxHeight();

    auto height = edgeBox->adjustBorderBoxHeight(heightLength.calc(edgeRect.h));
    if(!maxHeightLength.isNone())
        height = std::min(height, edgeBox->adjustBorderBoxHeight(maxHeightLength.calc(edgeRect.h)));
    if(!minHeightLength.isAuto()) {
        height = std::max(height, edgeBox->adjustBorderBoxHeight(minHeightLength.calc(edgeRect.h)));
    }

    return PreferredSizeInfo(PreferredSizeInfo::Fixed, height, height, edgeBox->marginHeight());
}

static void resolveTwoEdgePageMarginLengths(const std::array<PreferredSizeInfo, 3>& preferredMainAxisSizes, float availableMainAxisSize, float& firstMainAxisSize, float* secondMainAxisSize)
{
    enum { FirstResolvee = 0, NonResolvee = 1, SecondResolvee = 2 };

    assert(!preferredMainAxisSizes[NonResolvee].isAuto());
    float availableMainAxisSizeForFlex = availableMainAxisSize;
    float totalAutoMinSize = 0.f;
    float totalAutoMaxSize = 0.f;
    for(int i = 0; i < 3; i++) {
        if(preferredMainAxisSizes[i].isAuto()) {
            totalAutoMinSize += preferredMainAxisSizes[i].minLength();
            totalAutoMaxSize += preferredMainAxisSizes[i].maxLength();
        } else {
            availableMainAxisSizeForFlex -= preferredMainAxisSizes[i].minLength();
        }
    }

    std::array<float, 3> unflexedSizes = {};
    std::array<float, 3> flexFactors = {};

    float flexSpace = 0.f;
    if(availableMainAxisSizeForFlex > totalAutoMaxSize) {
        flexSpace = availableMainAxisSizeForFlex - totalAutoMaxSize;
        for(int i = 0; i < 3; i++) {
            unflexedSizes[i] = preferredMainAxisSizes[i].maxLength();
            flexFactors[i] = unflexedSizes[i];
        }
    } else {
        flexSpace = availableMainAxisSizeForFlex - totalAutoMinSize;
        for(int i = 0; i < 3; i++) {
            unflexedSizes[i] = preferredMainAxisSizes[i].minLength();
        }

        if(flexSpace > 0.f) {
            for(int i = 0; i < 3; i++) {
                flexFactors[i] = preferredMainAxisSizes[i].maxLength() - preferredMainAxisSizes[i].minLength();
            }
        } else {
            for(int i = 0; i < 3; i++) {
                flexFactors[i] = preferredMainAxisSizes[i].minLength();
            }
        }
    }

    firstMainAxisSize = unflexedSizes[FirstResolvee];
    if(preferredMainAxisSizes[FirstResolvee].isAuto()) {
        if(preferredMainAxisSizes[SecondResolvee].isAuto()) {
            auto totalFlex = flexFactors[FirstResolvee] + flexFactors[SecondResolvee];
            if(totalFlex > 0.f) {
                firstMainAxisSize += flexSpace * flexFactors[FirstResolvee] / totalFlex;
            }
        } else {
            firstMainAxisSize = availableMainAxisSize - unflexedSizes[SecondResolvee];
        }
    }

    if(secondMainAxisSize) {
        *secondMainAxisSize = unflexedSizes[SecondResolvee];
        if(preferredMainAxisSizes[SecondResolvee].isAuto()) {
            *secondMainAxisSize = availableMainAxisSize - firstMainAxisSize;
        }
    }
}

void PageBox::layoutEdgePageMargins(PageMarginBox* edgeStartBox, PageMarginBox* edgeCenterBox, PageMarginBox* edgeEndBox, const Rect& edgeRect, BoxSide edgeSide)
{
    auto availableMainAxisSize = isHorizontalEdge(edgeSide) ? edgeRect.w : edgeRect.h;
    std::array<PreferredSizeInfo, 3> preferredMainAxisSizes = {
        computeEdgePreferredSize(edgeStartBox, edgeRect, edgeSide),
        computeEdgePreferredSize(edgeCenterBox, edgeRect, edgeSide),
        computeEdgePreferredSize(edgeEndBox, edgeRect, edgeSide)
    };

    enum { StartMargin = 0, CenterMargin = 1, EndMargin = 2 };

    std::array<float, 3> mainAxisSizes = {
        preferredMainAxisSizes[StartMargin].maxLength(),
        preferredMainAxisSizes[CenterMargin].maxLength(),
        preferredMainAxisSizes[EndMargin].maxLength()
    };

    if(edgeCenterBox == nullptr) {
        resolveTwoEdgePageMarginLengths(preferredMainAxisSizes, availableMainAxisSize, mainAxisSizes[StartMargin], &mainAxisSizes[EndMargin]);
    } else {
        if(preferredMainAxisSizes[CenterMargin].isAuto()) {
            std::array<PreferredSizeInfo, 3> acSizesForStart = { preferredMainAxisSizes[CenterMargin], PreferredSizeInfo(), preferredMainAxisSizes[StartMargin].doubled() };
            std::array<PreferredSizeInfo, 3> acSizesForEnd = { preferredMainAxisSizes[CenterMargin], PreferredSizeInfo(), preferredMainAxisSizes[EndMargin].doubled() };

            float centerSize1;
            float centerSize2;

            resolveTwoEdgePageMarginLengths(acSizesForStart, availableMainAxisSize, centerSize1, nullptr);
            resolveTwoEdgePageMarginLengths(acSizesForEnd, availableMainAxisSize, centerSize2, nullptr);

            mainAxisSizes[CenterMargin] = std::min(centerSize1, centerSize2);
        }

        auto sideSpace = availableMainAxisSize - mainAxisSizes[CenterMargin];
        if(preferredMainAxisSizes[StartMargin].isAuto()) {
            mainAxisSizes[StartMargin] = sideSpace / 2.f;
        }

        if(preferredMainAxisSizes[EndMargin].isAuto()) {
            mainAxisSizes[EndMargin] = sideSpace - sideSpace / 2.f;
        }
    }

    for(int i = 0; i < 3; i++) {
        mainAxisSizes[i] = std::max(0.f, mainAxisSizes[i] - preferredMainAxisSizes[i].marginSize());
    }

    layoutEdgePageMargin(edgeStartBox, edgeRect, edgeSide, mainAxisSizes[StartMargin]);
    layoutEdgePageMargin(edgeCenterBox, edgeRect, edgeSide, mainAxisSizes[CenterMargin]);
    layoutEdgePageMargin(edgeEndBox, edgeRect, edgeSide, mainAxisSizes[EndMargin]);
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
    case PageMarginType::TopLeft:
    case PageMarginType::TopCenter:
    case PageMarginType::TopRight:
    case PageMarginType::TopRightCorner:
    case PageMarginType::BottomRightCorner:
    case PageMarginType::BottomRight:
    case PageMarginType::BottomCenter:
    case PageMarginType::BottomLeft:
    case PageMarginType::BottomLeftCorner:
        return true;
    default:
        return false;
    }
}

bool PageMarginBox::isVerticalFlow() const
{
    switch(m_marginType) {
    case PageMarginType::TopLeftCorner:
    case PageMarginType::TopRightCorner:
    case PageMarginType::RightTop:
    case PageMarginType::RightMiddle:
    case PageMarginType::RightBottom:
    case PageMarginType::BottomRightCorner:
    case PageMarginType::BottomLeftCorner:
    case PageMarginType::LeftBottom:
    case PageMarginType::LeftMiddle:
    case PageMarginType::LeftTop:
        return true;
    default:
        return false;
    }
}

void PageMarginBox::updatePaddingWidths() const
{
}

bool PageMarginBox::updateIntrinsicPaddings(float availableHeight)
{
    float intrinsicPaddingTop = 0.f;
    switch(style()->verticalAlignType()) {
    case VerticalAlignType::Middle:
        intrinsicPaddingTop = (availableHeight - height()) / 2.f;
        break;
    case VerticalAlignType::Bottom:
        intrinsicPaddingTop = availableHeight - height();
        break;
    default:
        return false;
    }

    auto intrinsicPaddingBottom = availableHeight - intrinsicPaddingTop - height();
    m_paddingTop += intrinsicPaddingTop;
    m_paddingBottom += intrinsicPaddingBottom;
    return intrinsicPaddingTop || intrinsicPaddingBottom;
}

void PageMarginBox::updatePaddings(const Size& availableSize)
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

void PageMarginBox::updateMargins(const Size& availableSize)
{
    auto marginTopLength = style()->marginTop();
    auto marginRightLength = style()->marginRight();
    auto marginBottomLength = style()->marginBottom();
    auto marginLeftLength = style()->marginLeft();

    auto marginTop = marginTopLength.calcMin(availableSize.h);
    auto marginRight = marginRightLength.calcMin(availableSize.w);
    auto marginBottom = marginBottomLength.calcMin(availableSize.h);
    auto marginLeft = marginLeftLength.calcMin(availableSize.w);

    m_marginTop = marginTop;
    m_marginRight = marginRight;
    m_marginBottom = marginBottom;
    m_marginLeft = marginLeft;
}

void PageMarginBox::updateAutoMargins(const Size& availableSize)
{
    if(isHorizontalFlow()) {
        auto availableSpace = std::max(0.f, availableSize.h - m_marginTop - m_marginBottom - height());

        auto marginTopLength = style()->marginTop();
        auto marginBottomLength = style()->marginBottom();

        float autoMarginOffset = 0.f;
        if(marginTopLength.isAuto() && marginBottomLength.isAuto())
            autoMarginOffset += availableSpace / 2.f;
        else
            autoMarginOffset += availableSpace;
        if(marginTopLength.isAuto())
            m_marginTop += autoMarginOffset;
        if(marginBottomLength.isAuto()) {
            m_marginBottom += autoMarginOffset;
        }

        auto additionalSpace = availableSize.h - m_marginTop - m_marginBottom - height();
        switch(m_marginType) {
        case PageMarginType::TopLeftCorner:
        case PageMarginType::TopLeft:
        case PageMarginType::TopCenter:
        case PageMarginType::TopRight:
        case PageMarginType::TopRightCorner:
            m_marginTop += additionalSpace;
            break;
        default:
            m_marginBottom += additionalSpace;
            break;
        }
    }

    if(isVerticalFlow()) {
        auto availableSpace = std::max(0.f, availableSize.w - m_marginLeft - m_marginRight - width());

        auto marginRightLength = style()->marginRight();
        auto marginLeftLength = style()->marginLeft();

        float autoMarginOffset = 0.f;
        if(marginLeftLength.isAuto() && marginRightLength.isAuto())
            autoMarginOffset += availableSpace / 2.f;
        else
            autoMarginOffset += availableSpace;
        if(marginLeftLength.isAuto())
            m_marginLeft += autoMarginOffset;
        if(marginRightLength.isAuto()) {
            m_marginRight += autoMarginOffset;
        }

        auto additionalSpace = availableSize.w - m_marginLeft - m_marginRight - width();
        switch(m_marginType) {
        case PageMarginType::TopLeftCorner:
        case PageMarginType::BottomLeftCorner:
        case PageMarginType::LeftBottom:
        case PageMarginType::LeftMiddle:
        case PageMarginType::LeftTop:
            m_marginLeft += additionalSpace;
            break;
        default:
            m_marginRight += additionalSpace;
            break;
        }
    }
}

void PageMarginBox::layoutFixedWidth(float width)
{
    setWidth(width);
    layout(nullptr);
}

void PageMarginBox::layoutFixedHeight(float height)
{
    setOverrideHeight(height);
    if(updateIntrinsicPaddings(height))
        layout(nullptr);
    setHeight(height);
}

void PageMarginBox::layoutWidth(float availableWidth, bool fixedWidth)
{
    if(fixedWidth) {
        layoutFixedWidth(availableWidth);
        return;
    }

    auto widthLength = style()->width();
    auto minWidthLength = style()->minWidth();
    auto maxWidthLength = style()->maxWidth();

    auto width = std::max(0.f, availableWidth - marginWidth());
    if(!widthLength.isAuto())
        width = adjustBorderBoxWidth(widthLength.calc(availableWidth));
    if(!maxWidthLength.isNone())
        width = std::min(width, adjustBorderBoxWidth(maxWidthLength.calc(availableWidth)));
    if(!minWidthLength.isAuto()) {
        width = std::max(width, adjustBorderBoxWidth(minWidthLength.calc(availableWidth)));
    }

    layoutFixedWidth(width);
}

void PageMarginBox::layoutHeight(float availableHeight, bool fixedHeight)
{
    if(fixedHeight) {
        layoutFixedHeight(availableHeight);
        return;
    }

    auto heightLength = style()->height();
    auto minHeightLength = style()->minHeight();
    auto maxHeightLength = style()->maxHeight();

    auto height = std::max(0.f, availableHeight - marginHeight());
    if(!heightLength.isAuto())
        height = adjustBorderBoxHeight(heightLength.calc(availableHeight));
    if(!maxHeightLength.isNone())
        height = std::min(height, adjustBorderBoxHeight(maxHeightLength.calc(availableHeight)));
    if(!minHeightLength.isAuto()) {
        height = std::max(height, adjustBorderBoxHeight(minHeightLength.calc(availableHeight)));
    }

    layoutFixedHeight(height);
}

void PageMarginBox::layoutContent(float availableWidth, float availableHeight, bool fixedWidth, bool fixedHeight)
{
    layoutWidth(availableWidth, fixedWidth);
    layoutHeight(availableHeight, fixedHeight);
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

    auto content = marginStyle->get(CSSPropertyID::Content);
    if(content == nullptr || content->id() == CSSValueID::None || content->id() == CSSValueID::Normal) {
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
    buildPageMargin(counters, pageBox, PageMarginType::BottomRight);
    buildPageMargin(counters, pageBox, PageMarginType::BottomCenter);
    buildPageMargin(counters, pageBox, PageMarginType::BottomLeft);

    buildPageMargin(counters, pageBox, PageMarginType::BottomLeftCorner);
    buildPageMargin(counters, pageBox, PageMarginType::LeftBottom);
    buildPageMargin(counters, pageBox, PageMarginType::LeftMiddle);
    buildPageMargin(counters, pageBox, PageMarginType::LeftTop);
}

} // namespace plutobook
