/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "pagebox.h"
#include "boxview.h"
#include "contentbox.h"
#include "counters.h"
#include "document.h"
#include "graphicscontext.h"
#include "plutobook.hpp"
#include "cssrule.h"

#include <cmath>

namespace plutobook {

std::unique_ptr<PageBox> PageBox::create(const RefPtr<BoxStyle>& style, const GlobalString& pageName, uint32_t pageIndex, float pageWidth, float pageHeight, float pageScale)
{
    return std::unique_ptr<PageBox>(new (style->heap()) PageBox(style, pageName, pageIndex, pageWidth, pageHeight, pageScale));
}

PageSize PageBox::pageSize() const
{
    return PageSize(m_pageWidth * units::px, m_pageHeight * units::px);
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

static void layoutPageMarginBox(PageMarginBox* marginBox, float availableWidth, float availableHeight, bool fixedWidth, bool fixedHeight)
{
    if(fixedWidth) {
        marginBox->setOverrideWidth(availableWidth);
    } else {
        auto widthLength = marginBox->style()->width();
        auto minWidthLength = marginBox->style()->minWidth();
        auto maxWidthLength = marginBox->style()->maxWidth();

        auto width = std::max(0.f, availableWidth - marginBox->marginWidth());
        if(!widthLength.isAuto())
            width = marginBox->adjustBorderBoxWidth(widthLength.calc(availableWidth));
        if(!maxWidthLength.isNone())
            width = std::min(width, marginBox->adjustBorderBoxWidth(maxWidthLength.calc(availableWidth)));
        if(!minWidthLength.isAuto()) {
            width = std::max(width, marginBox->adjustBorderBoxWidth(minWidthLength.calc(availableWidth)));
        }

        marginBox->setOverrideWidth(width);
    }

    if(fixedHeight) {
        marginBox->setOverrideHeight(availableHeight);
    } else {
        auto heightLength = marginBox->style()->height();
        auto minHeightLength = marginBox->style()->minHeight();
        auto maxHeightLength = marginBox->style()->maxHeight();

        auto height = std::max(0.f, availableHeight - marginBox->marginHeight());
        if(!heightLength.isAuto())
            height = marginBox->adjustBorderBoxHeight(heightLength.calc(availableHeight));
        if(!maxHeightLength.isNone())
            height = std::min(height, marginBox->adjustBorderBoxHeight(maxHeightLength.calc(availableHeight)));
        if(!minHeightLength.isAuto()) {
            height = std::max(height, marginBox->adjustBorderBoxHeight(minHeightLength.calc(availableHeight)));
        }

        marginBox->setOverrideHeight(height);
    }

    marginBox->layout(nullptr);
}

static void layoutCornerPageMargin(PageMarginBox* cornerBox, const Rect& cornerRect)
{
    if(cornerBox == nullptr) {
        return;
    }

    cornerBox->updateMargins(cornerRect.size());
    cornerBox->updatePaddings(cornerRect.size());
    layoutPageMarginBox(cornerBox, cornerRect.w, cornerRect.h, false, false);
    cornerBox->updateAutoMargins(cornerRect.size());

    cornerBox->setX(cornerRect.x + cornerBox->marginLeft());
    cornerBox->setY(cornerRect.y + cornerBox->marginTop());
}

constexpr bool isHorizontalEdge(BoxSide side) { return side == BoxSideTop || side == BoxSideBottom; }

static void layoutEdgePageMargin(PageMarginBox* edgeBox, const Rect& edgeRect, BoxSide edgeSide, float mainAxisSize)
{
    if(edgeBox == nullptr) {
        return;
    }

    if(isHorizontalEdge(edgeSide)) {
        layoutPageMarginBox(edgeBox, mainAxisSize, edgeRect.h, true, false);
    } else {
        layoutPageMarginBox(edgeBox, edgeRect.w, mainAxisSize, false, true);
    }

    edgeBox->updateAutoMargins(edgeRect.size());

    auto edgeOffset = edgeRect.origin();
    if(isHorizontalEdge(edgeSide)) {
        auto availableSpace = edgeRect.w - edgeBox->marginBoxWidth();
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
        auto availableSpace = edgeRect.h - edgeBox->marginBoxHeight();
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
        auto widthLength = edgeBox->style()->width();
        auto minWidthLength = edgeBox->style()->minWidth();
        auto maxWidthLength = edgeBox->style()->maxWidth();

        auto width = std::max(0.f, edgeRect.w - edgeBox->marginWidth());
        if(!widthLength.isAuto())
            width = edgeBox->adjustBorderBoxWidth(widthLength.calc(edgeRect.w));
        if(!maxWidthLength.isNone())
            width = std::min(width, edgeBox->adjustBorderBoxWidth(maxWidthLength.calc(edgeRect.w)));
        if(!minWidthLength.isAuto()) {
            width = std::max(width, edgeBox->adjustBorderBoxWidth(minWidthLength.calc(edgeRect.w)));
        }

        edgeBox->setOverrideWidth(width);
        edgeBox->layout(nullptr);

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

static void layoutEdgePageMargins(PageMarginBox* edgeStartBox, PageMarginBox* edgeCenterBox, PageMarginBox* edgeEndBox, const Rect& edgeRect, BoxSide edgeSide)
{
    if(edgeStartBox == nullptr && edgeCenterBox == nullptr && edgeEndBox == nullptr)
        return;
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

    auto availableMainAxisSize = isHorizontalEdge(edgeSide) ? edgeRect.w : edgeRect.h;
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

    auto leftWidth = marginLeft();
    auto rightWidth = marginRight();

    auto topHeight = marginTop();
    auto bottomHeight = marginBottom();

    Rect topLeftCornerRect(0, 0, leftWidth, topHeight);
    Rect topRightCornerRect(m_pageWidth - rightWidth, 0, rightWidth, topHeight);
    Rect bottomRightCornerRect(m_pageWidth - rightWidth, m_pageHeight - bottomHeight, rightWidth, bottomHeight);
    Rect bottomLeftCornerRect(0, m_pageHeight - bottomHeight, leftWidth, bottomHeight);

    Rect topEdgeRect(leftWidth, 0, m_pageWidth - leftWidth - rightWidth, topHeight);
    Rect rightEdgeRect(m_pageWidth - rightWidth, topHeight, rightWidth, m_pageHeight - topHeight - bottomHeight);
    Rect bottomEdgeRect(leftWidth, m_pageHeight - bottomHeight, m_pageWidth - leftWidth - rightWidth, bottomHeight);
    Rect leftEdgeRect(0, topHeight, leftWidth, m_pageHeight - topHeight - bottomHeight);

    const auto inv_scale = 1.f / m_pageScale;

    topLeftCornerRect.scale(inv_scale);
    topRightCornerRect.scale(inv_scale);
    bottomRightCornerRect.scale(inv_scale);
    bottomLeftCornerRect.scale(inv_scale);

    topEdgeRect.scale(inv_scale);
    rightEdgeRect.scale(inv_scale);
    bottomEdgeRect.scale(inv_scale);
    leftEdgeRect.scale(inv_scale);

    layoutCornerPageMargin(margins[PageMarginType::TopLeftCorner], topLeftCornerRect);
    layoutEdgePageMargins(margins[PageMarginType::TopLeft], margins[PageMarginType::TopCenter], margins[PageMarginType::TopRight], topEdgeRect, BoxSideTop);

    layoutCornerPageMargin(margins[PageMarginType::TopRightCorner], topRightCornerRect);
    layoutEdgePageMargins(margins[PageMarginType::RightTop], margins[PageMarginType::RightMiddle], margins[PageMarginType::RightBottom], rightEdgeRect, BoxSideRight);

    layoutCornerPageMargin(margins[PageMarginType::BottomRightCorner], bottomRightCornerRect);
    layoutEdgePageMargins(margins[PageMarginType::BottomLeft], margins[PageMarginType::BottomCenter], margins[PageMarginType::BottomRight], bottomEdgeRect, BoxSideBottom);

    layoutCornerPageMargin(margins[PageMarginType::BottomLeftCorner], bottomLeftCornerRect);
    layoutEdgePageMargins(margins[PageMarginType::LeftTop], margins[PageMarginType::LeftMiddle], margins[PageMarginType::LeftBottom], leftEdgeRect, BoxSideLeft);

    updateOverflowRect();
    updateLayerPosition();
}

void PageBox::paintRootBackground(const PaintInfo& info) const
{
    paintBackgroundStyle(info, pageRect(), style());
}

void PageBox::paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    auto contentRect = document()->pageContentRectAt(m_pageIndex);
    if(phase == PaintPhase::Contents && !contentRect.isEmpty()) {
        info->save();
        info->translate(offset.x, offset.y);
        info->scale(m_pageScale, m_pageScale);
        info->translate(-contentRect.x, -contentRect.y);
        info->clipRect(contentRect);
        document()->render(*info, contentRect);
        info->restore();
    }
}

PageBox::PageBox(const RefPtr<BoxStyle>& style, const GlobalString& pageName, uint32_t pageIndex, float pageWidth, float pageHeight, float pageScale)
    : BlockBox(nullptr, style)
    , m_pageName(pageName)
    , m_pageIndex(pageIndex)
    , m_pageWidth(pageWidth)
    , m_pageHeight(pageHeight)
    , m_pageScale(pageScale)
{
    setIsBackgroundStolen(true);
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

float PageMarginBox::computeVerticalAlignShift() const
{
    auto availableHeight = overrideHeight();
    if(availableHeight < height())
        return 0.f;
    switch(style()->verticalAlignType()) {
    case VerticalAlignType::Middle:
        return (availableHeight - height()) / 2.f;
    case VerticalAlignType::Bottom:
        return availableHeight - height();
    default:
        return 0.f;
    }
}

void PageMarginBox::updatePaddings(const Size& availableSize)
{
    auto paddingTopLength = style()->paddingTop();
    auto paddingBottomLength = style()->paddingBottom();
    auto paddingLeftLength = style()->paddingLeft();
    auto paddingRightLength = style()->paddingRight();

    auto paddingTop = paddingTopLength.calcMin(availableSize.h);
    auto paddingBottom = paddingBottomLength.calcMin(availableSize.h);
    auto paddingLeft = paddingLeftLength.calcMin(availableSize.w);
    auto paddingRight = paddingRightLength.calcMin(availableSize.w);

    setPaddingTop(paddingTop);
    setPaddingBottom(paddingBottom);
    setPaddingLeft(paddingLeft);
    setPaddingRight(paddingRight);
}

void PageMarginBox::updateMargins(const Size& availableSize)
{
    auto marginTopLength = style()->marginTop();
    auto marginBottomLength = style()->marginBottom();
    auto marginLeftLength = style()->marginLeft();
    auto marginRightLength = style()->marginRight();

    auto marginTop = marginTopLength.calcMin(availableSize.h);
    auto marginBottom = marginBottomLength.calcMin(availableSize.h);
    auto marginLeft = marginLeftLength.calcMin(availableSize.w);
    auto marginRight = marginRightLength.calcMin(availableSize.w);

    setMarginTop(marginTop);
    setMarginBottom(marginBottom);
    setMarginLeft(marginLeft);
    setMarginRight(marginRight);
}

void PageMarginBox::updateAutoMargins(const Size& availableSize)
{
    if(isHorizontalFlow()) {
        auto availableSpace = std::max(0.f, availableSize.h - marginBoxHeight());

        auto marginTopLength = style()->marginTop();
        auto marginBottomLength = style()->marginBottom();

        float autoMarginOffset = 0.f;
        if(marginTopLength.isAuto() && marginBottomLength.isAuto())
            autoMarginOffset += availableSpace / 2.f;
        else
            autoMarginOffset += availableSpace;
        if(marginTopLength.isAuto())
            setMarginTop(autoMarginOffset);
        if(marginBottomLength.isAuto()) {
            setMarginBottom(autoMarginOffset);
        }

        auto additionalSpace = availableSize.h - marginBoxHeight();
        switch(m_marginType) {
        case PageMarginType::TopLeftCorner:
        case PageMarginType::TopLeft:
        case PageMarginType::TopCenter:
        case PageMarginType::TopRight:
        case PageMarginType::TopRightCorner:
            setMarginTop(additionalSpace + marginTop());
            break;
        default:
            setMarginBottom(additionalSpace + marginBottom());
            break;
        }
    }

    if(isVerticalFlow()) {
        auto availableSpace = std::max(0.f, availableSize.w - marginBoxWidth());

        auto marginRightLength = style()->marginRight();
        auto marginLeftLength = style()->marginLeft();

        float autoMarginOffset = 0.f;
        if(marginLeftLength.isAuto() && marginRightLength.isAuto())
            autoMarginOffset += availableSpace / 2.f;
        else
            autoMarginOffset += availableSpace;
        if(marginLeftLength.isAuto())
            setMarginLeft(autoMarginOffset);
        if(marginRightLength.isAuto()) {
            setMarginRight(autoMarginOffset);
        }

        auto additionalSpace = availableSize.w - marginBoxWidth();
        switch(m_marginType) {
        case PageMarginType::TopLeftCorner:
        case PageMarginType::BottomLeftCorner:
        case PageMarginType::LeftBottom:
        case PageMarginType::LeftMiddle:
        case PageMarginType::LeftTop:
            setMarginLeft(additionalSpace + marginLeft());
            break;
        default:
            setMarginRight(additionalSpace + marginRight());
            break;
        }
    }
}

PageLayout::PageLayout(Document* document)
    : m_document(document)
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

constexpr auto kMinPageScaleFactor = 1.f / 100.f;

void PageLayout::layout()
{
    auto& pages = m_document->pages();
    if(!pages.empty()) {
        const auto& pageBox = pages.front();
        const auto pageWidth = pageBox->width() / pageBox->pageScale();
        const auto pageHeight = pageBox->height() / pageBox->pageScale();
        m_document->setContainerSize(pageWidth, pageHeight);
        m_document->box()->layout(m_document);
        return;
    }

    auto book = m_document->book();
    auto box = m_document->box();

    auto pageStyle = m_document->styleForPage(emptyGlo, 0, PseudoType::FirstPage);
    auto pageSize = pageStyle->getPageSize(book->pageSize());
    auto pageScale = pageStyle->pageScale();

    auto pageWidth = pageStyle->width().calc(pageSize.width() / units::px);
    auto pageHeight = pageStyle->height().calc(pageSize.height() / units::px);

    auto marginLeftLength = pageStyle->marginLeft();
    auto marginRightLength = pageStyle->marginRight();
    auto marginTopLength = pageStyle->marginTop();
    auto marginBottomLength = pageStyle->marginBottom();

    const auto& deviceMargins = book->pageMargins();
    auto marginTop = marginTopLength.isAuto() ? deviceMargins.top() / units::px : marginTopLength.calcMin(pageHeight);
    auto marginRight = marginRightLength.isAuto() ? deviceMargins.right() / units::px : marginRightLength.calcMin(pageWidth);
    auto marginBottom = marginBottomLength.isAuto() ? deviceMargins.bottom() / units::px : marginBottomLength.calcMin(pageHeight);
    auto marginLeft = marginLeftLength.isAuto() ? deviceMargins.left() / units::px : marginLeftLength.calcMin(pageWidth);

    auto width = std::max(0.f, pageWidth - marginLeft - marginRight);
    auto height = std::max(0.f, pageHeight - marginTop - marginBottom);

    auto pageScaleFactor = std::max(kMinPageScaleFactor, pageScale.value_or(1.f));
    if(m_document->setContainerSize(width / pageScaleFactor, height / pageScaleFactor)) {
        box->layout(m_document);
    }

    if(!pageScale.has_value() && m_document->containerWidth() < m_document->width()) {
        pageScaleFactor = std::max(kMinPageScaleFactor, m_document->containerWidth() / m_document->width());
        if(m_document->setContainerSize(width / pageScaleFactor, height / pageScaleFactor)) {
            box->layout(m_document);
        }
    }

    if(m_document->containerHeight() > 0.f) {
        Counters counters(m_document, std::ceil(m_document->height() / m_document->containerHeight()));
        for(uint32_t pageIndex = 0; pageIndex < counters.pageCount(); ++pageIndex) {
            if(pageIndex > 0) pageStyle = m_document->styleForPage(emptyGlo, pageIndex, pagePseudoType(pageIndex));
            auto pageBox = PageBox::create(pageStyle, emptyGlo, pageIndex, pageWidth, pageHeight, pageScaleFactor);

            pageBox->setX(marginLeft);
            pageBox->setY(marginTop);

            pageBox->setWidth(width);
            pageBox->setHeight(height);

            pageBox->setMarginTop(marginTop);
            pageBox->setMarginRight(marginRight);
            pageBox->setMarginBottom(marginBottom);
            pageBox->setMarginLeft(marginLeft);

            counters.update(pageBox.get());
            buildPageMargins(counters, pageBox.get());

            pageBox->build();
            pageBox->layout(nullptr);

            pages.push_back(std::move(pageBox));
        }
    }
}

void PageLayout::buildPageMargin(const Counters& counters, PageBox* pageBox, PageMarginType marginType)
{
    auto marginStyle = m_document->styleForPageMargin(pageBox->pageName(), pageBox->pageIndex(), marginType, pageBox->style());
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
    ContentBoxBuilder(marginCounters, nullptr, marginBox).build(*content);
    pageBox->addChild(marginBox);
}

void PageLayout::buildPageMargins(const Counters& counters, PageBox* pageBox)
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
