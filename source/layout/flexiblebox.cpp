/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "flexiblebox.h"
#include "boxlayer.h"

#include <span>
#include <ranges>
#include <list>

namespace plutobook {

FlexItem::FlexItem(BoxFrame* box, int order, float flexGrow, float flexShrink, AlignItem alignSelf)
    : m_box(box)
    , m_order(order)
    , m_flexGrow(flexGrow)
    , m_flexShrink(flexShrink)
    , m_alignSelf(alignSelf)
{
}

std::optional<float> FlexItem::computeWidthUsing(const Length& widthLength) const
{
    if(widthLength.isFixed())
        return m_box->adjustContentBoxWidth(widthLength.value());
    if(widthLength.isPercent() || widthLength.isIntrinsic()) {
        auto containerWidth = m_box->containingBlockWidthForContent(flexBox());
        if(widthLength.isPercent())
            return m_box->adjustContentBoxWidth(widthLength.calc(containerWidth));
        return m_box->computeIntrinsicWidthUsing(widthLength, containerWidth) - m_box->borderAndPaddingWidth();
    }

    return std::nullopt;
}

std::optional<float> FlexItem::computeHeightUsing(const Length& heightLength) const
{
    if(heightLength.isFixed())
        return m_box->adjustContentBoxHeight(heightLength.value());
    if(heightLength.isPercent()) {
        if(auto availableHeight = m_box->containingBlockHeightForContent(flexBox())) {
            return m_box->adjustContentBoxHeight(heightLength.calc(availableHeight.value()));
        }
    }

    return std::nullopt;
}

float FlexItem::constrainWidth(float width) const
{
    if(auto maxWidth = computeWidthUsing(m_box->style()->maxWidth()))
        width = std::min(width, *maxWidth);
    if(auto minWidth = computeWidthUsing(m_box->style()->minWidth()))
        width = std::max(width, *minWidth);
    if(m_box->isTableBox())
        width = std::max(width, m_box->minPreferredWidth());
    return std::max(0.f, width);
}

float FlexItem::constrainHeight(float height) const
{
    if(auto maxHeight = computeHeightUsing(m_box->style()->maxHeight()))
        height = std::min(height, *maxHeight);
    if(auto minHeight = computeHeightUsing(m_box->style()->minHeight()))
        height = std::max(height, *minHeight);
    return std::max(0.f, height);
}

float FlexItem::constrainMainSize(float size) const
{
    if(isHorizontalFlow())
        return constrainWidth(size);
    return constrainHeight(size);
}

float FlexItem::constrainCrossSize(float size) const
{
    if(isHorizontalFlow())
        return constrainHeight(size);
    return constrainWidth(size);
}

float FlexItem::computeFlexBaseSize() const
{
    auto flexBasis = m_box->style()->flexBasis();
    if(isHorizontalFlow()) {
        if(flexBasis.isAuto())
            flexBasis = m_box->style()->width();
        if(auto width = computeWidthUsing(flexBasis))
            return width.value();
        return m_box->maxPreferredWidth() - m_box->borderAndPaddingWidth();
    }

    if(flexBasis.isAuto())
        flexBasis = m_box->style()->height();
    auto height = computeHeightUsing(flexBasis);
    if(height == std::nullopt)
        m_box->layout(nullptr);
    return height.value_or(m_box->height() - m_box->borderAndPaddingHeight());
}

float FlexItem::flexBaseMarginBoxSize() const
{
    if(isHorizontalFlow())
        return m_flexBaseSize + m_box->marginWidth() + m_box->borderAndPaddingWidth();
    return m_flexBaseSize + m_box->marginHeight() + m_box->borderAndPaddingHeight();
}

float FlexItem::flexBaseBorderBoxSize() const
{
    if(isHorizontalFlow())
        return m_flexBaseSize + m_box->borderAndPaddingWidth();
    return m_flexBaseSize + m_box->borderAndPaddingHeight();
}

float FlexItem::targetMainMarginBoxSize() const
{
    if(isHorizontalFlow())
        return m_targetMainSize + m_box->marginWidth() + m_box->borderAndPaddingWidth();
    return m_targetMainSize + m_box->marginHeight() + m_box->borderAndPaddingHeight();
}

float FlexItem::targetMainBorderBoxSize() const
{
    if(isHorizontalFlow())
        return m_targetMainSize + m_box->borderAndPaddingWidth();
    return m_targetMainSize + m_box->borderAndPaddingHeight();
}

float FlexItem::marginBoxMainSize() const
{
    if(isHorizontalFlow())
        return m_box->marginBoxWidth();
    return m_box->marginBoxHeight();
}

float FlexItem::marginBoxCrossSize() const
{
    if(isHorizontalFlow())
        return m_box->marginBoxHeight();
    return m_box->marginBoxWidth();
}

float FlexItem::marginBoxCrossBaseline() const
{
    assert(isHorizontalFlow());
    if(auto baseline = m_box->firstLineBaseline())
        return baseline.value() + m_box->marginTop();
    return m_box->height() + m_box->marginTop();
}

float FlexItem::borderBoxMainSize() const
{
    if(isHorizontalFlow())
        return m_box->width();
    return m_box->height();
}

float FlexItem::borderBoxCrossSize() const
{
    if(isHorizontalFlow())
        return m_box->height();
    return m_box->width();
}

float FlexItem::marginStart() const
{
    switch(flexDirection()) {
    case FlexDirection::Row:
        return m_box->marginStart(direction());
    case FlexDirection::RowReverse:
        return m_box->marginEnd(direction());
    case FlexDirection::Column:
        return m_box->marginTop();
    case FlexDirection::ColumnReverse:
        return m_box->marginBottom();
    default:
        assert(false);
    }

    return m_box->marginLeft();
}

float FlexItem::marginEnd() const
{
    switch(flexDirection()) {
    case FlexDirection::Row:
        return m_box->marginEnd(direction());
    case FlexDirection::RowReverse:
        return m_box->marginStart(direction());
    case FlexDirection::Column:
        return m_box->marginBottom();
    case FlexDirection::ColumnReverse:
        return m_box->marginTop();
    default:
        assert(false);
    }

    return m_box->marginRight();
}

float FlexItem::marginBefore() const
{
    if(isHorizontalFlow())
        return m_box->marginTop();
    return m_box->marginStart(direction());
}

float FlexItem::marginAfter() const
{
    if(isHorizontalFlow())
        return m_box->marginBottom();
    return m_box->marginEnd(direction());
}

FlexibleBox::FlexibleBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
    , m_flexDirection(style->flexDirection())
    , m_flexWrap(style->flexWrap())
    , m_justifyContent(style->justifyContent())
    , m_alignContent(style->alignContent())
    , m_items(style->heap())
{
}

void FlexibleBox::addChild(Box* newChild)
{
    if(newChild->isPositioned() || !newChild->isInline()) {
        BlockBox::addChild(newChild);
        return;
    }

    auto lastBlock = lastChild();
    if(lastBlock && lastBlock->isAnonymousBlock()) {
        lastBlock->addChild(newChild);
        return;
    }

    auto newBlock = createAnonymousBlock(style());
    appendChild(newBlock);
    newBlock->addChild(newChild);
}

void FlexibleBox::updateOverflowRect()
{
    BlockBox::updateOverflowRect();
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(!child->isPositioned()) {
            addOverflowRect(child, child->x(), child->y());
        }
    }
}

void FlexibleBox::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned()) {
            continue;
        }

        child->updateHorizontalMargins(nullptr);
        child->updateHorizontalPaddings(nullptr);

        auto childMinWidth = child->minPreferredWidth() + child->marginWidth();
        auto childMaxWidth = child->maxPreferredWidth() + child->marginWidth();

        if(isVerticalFlow()) {
            minWidth = std::max(minWidth, childMinWidth);
            maxWidth = std::max(maxWidth, childMaxWidth);
        } else {
            maxWidth += childMaxWidth;
            if(isMultiLine()) {
                minWidth = std::max(minWidth, childMinWidth);
            } else {
                minWidth += childMinWidth;
            }
        }
    }

    if(m_items.size() > 1 && isHorizontalFlow()) {
        auto gapWidth = m_gapBetweenItems * (m_items.size() - 1);
        maxWidth += gapWidth;
        if(!isMultiLine()) {
            minWidth += gapWidth;
        }
    }

    minWidth = std::max(0.f, minWidth);
    maxWidth = std::max(minWidth, maxWidth);
}

std::optional<float> FlexibleBox::firstLineBaseline() const
{
    const BoxFrame* baselineChild = nullptr;
    for(const auto& item : m_items) {
        auto child = item.box();
        if(!baselineChild)
            baselineChild = child;
        if(item.alignSelf() == AlignItem::Baseline) {
            baselineChild = child;
            break;
        }
    }

    if(!baselineChild)
        return std::nullopt;
    if(auto baseline = baselineChild->firstLineBaseline())
        return baseline.value() + baselineChild->y();
    return height() + baselineChild->y();
}

std::optional<float> FlexibleBox::lastLineBaseline() const
{
    const BoxFrame* baselineChild = nullptr;
    for(const auto& item : m_items | std::views::reverse) {
        auto child = item.box();
        if(!baselineChild)
            baselineChild = child;
        if(item.alignSelf() == AlignItem::Baseline) {
            baselineChild = child;
            break;
        }
    }

    if(!baselineChild)
        return std::nullopt;
    if(auto baseline = baselineChild->lastLineBaseline())
        return baseline.value() + baselineChild->y();
    return height() + baselineChild->y();
}

std::optional<float> FlexibleBox::inlineBlockBaseline() const
{
    return firstLineBaseline();
}

float FlexibleBox::computeMainContentSize(float hypotheticalMainSize) const
{
    if(isHorizontalFlow())
        return contentBoxWidth();
    float y = 0;
    float height = hypotheticalMainSize + borderAndPaddingHeight();
    float marginTop = 0;
    float marginBottom = 0;
    computeHeight(y, height, marginTop, marginBottom);
    return height - borderAndPaddingHeight();
}

float FlexibleBox::availableCrossSize() const
{
    if(isHorizontalFlow())
        return contentBoxHeight();
    return contentBoxWidth();
}

float FlexibleBox::borderAndPaddingStart() const
{
    switch(m_flexDirection) {
    case FlexDirection::Row:
        return borderStart() + paddingStart();
    case FlexDirection::RowReverse:
        return borderEnd() + paddingEnd();
    case FlexDirection::Column:
        return borderTop() + paddingTop();
    case FlexDirection::ColumnReverse:
        return borderBottom() + paddingBottom();
    default:
        assert(false);
    }

    return borderStart() + paddingStart();
}

float FlexibleBox::borderAndPaddingEnd() const
{
    switch(m_flexDirection) {
    case FlexDirection::Row:
        return borderEnd() + paddingEnd();
    case FlexDirection::RowReverse:
        return borderStart() + paddingStart();
    case FlexDirection::Column:
        return borderBottom() + paddingBottom();
    case FlexDirection::ColumnReverse:
        return borderTop() + paddingTop();
    default:
        assert(false);
    }

    return borderEnd() + paddingEnd();
}

float FlexibleBox::borderAndPaddingBefore() const
{
    if(isHorizontalFlow())
        return borderTop() + paddingTop();
    return borderStart() + paddingStart();
}

float FlexibleBox::borderAndPaddingAfter() const
{
    if(isHorizontalFlow())
        return borderBottom() + paddingBottom();
    return borderEnd() + paddingEnd();
}

using FlexItemSpan = std::span<FlexItem>;

class FlexLine {
public:
    explicit FlexLine(const FlexItemSpan& items)
        : m_items(items)
    {}

    const FlexItemSpan& items() const { return m_items; }

    float crossOffset() const { return m_crossOffset; }
    float crossSize() const { return m_crossSize; }
    float crossBaseline() const { return m_crossBaseline; }

    void setCrossOffset(float offset) { m_crossOffset = offset; }
    void setCrossSize(float size) { m_crossSize = size; }
    void setCrossBaseline(float baseline) { m_crossBaseline = baseline; }

private:
    FlexItemSpan m_items;
    float m_crossOffset = 0;
    float m_crossSize = 0;
    float m_crossBaseline = 0;
};

using FlexLineList = std::vector<FlexLine>;

void FlexibleBox::layout(FragmentBuilder* fragmentainer)
{
    updateWidth();
    setHeight(borderAndPaddingHeight());

    float maxHypotheticalMainSize = 0;
    for(auto& item : m_items) {
        auto child = item.box();
        child->clearOverrideSize();
        child->updateMarginWidths(this);
        child->updatePaddingWidths(this);

        item.setFlexBaseSize(item.computeFlexBaseSize());
        item.setTargetMainSize(item.constrainMainSize(item.flexBaseSize()));
        maxHypotheticalMainSize += m_gapBetweenItems + item.targetMainMarginBoxSize();
    }

    const auto lineBreakLength = computeMainContentSize(maxHypotheticalMainSize);

    auto it = m_items.begin();
    auto end = m_items.end();

    FlexLineList lines;
    while(it != end) {
        float totalFlexGrow = 0;
        float totalFlexShrink = 0;
        float totalScaledFlexShrink = 0;
        float totalHypotheticalMainSize = 0;
        float totalFlexBaseSize = 0;

        auto begin = it;
        for(; it != end; ++it) {
            if(isMultiLine() && it != begin && totalHypotheticalMainSize + it->targetMainMarginBoxSize() > lineBreakLength)
                break;
            totalFlexGrow += it->flexGrow();
            totalFlexShrink += it->flexShrink();
            totalScaledFlexShrink += it->flexShrink() * it->flexBaseSize();
            totalHypotheticalMainSize += m_gapBetweenItems + it->targetMainMarginBoxSize();
            totalFlexBaseSize += m_gapBetweenItems + it->flexBaseMarginBoxSize();
        }

        totalHypotheticalMainSize -= m_gapBetweenItems;
        totalFlexBaseSize -= m_gapBetweenItems;

        auto mainContentSize = computeMainContentSize(totalHypotheticalMainSize);
        auto initialFreeSpace = mainContentSize - totalFlexBaseSize;
        auto sign = totalHypotheticalMainSize < mainContentSize ? FlexSign::Positive : FlexSign::Negative;

        FlexItemSpan items(begin, it);
        std::list<FlexItem*> unfrozenItems;
        for(auto& item : items) {
            if(item.flexFactor(sign) == 0 || (sign == FlexSign::Positive && item.flexBaseSize() > item.targetMainSize())
                || (sign == FlexSign::Negative && item.flexBaseSize() < item.targetMainSize())) {
                totalFlexGrow -= item.flexGrow();
                totalFlexShrink -= item.flexShrink();
                totalScaledFlexShrink -= item.flexShrink() * item.flexBaseSize();
                initialFreeSpace -= item.targetMainSize() - item.flexBaseSize();
            } else {
                unfrozenItems.push_back(&item);
            }
        }

        auto remainingFreeSpace = initialFreeSpace;
        while(!unfrozenItems.empty()) {
            auto totalFlexFactor = sign == FlexSign::Positive ? totalFlexGrow : totalFlexShrink;
            if(totalFlexFactor > 0.f && totalFlexFactor < 1.f) {
                auto scaledInitialFreeSpace = initialFreeSpace * totalFlexFactor;
                if(std::abs(scaledInitialFreeSpace) < std::abs(remainingFreeSpace)) {
                    remainingFreeSpace = scaledInitialFreeSpace;
                }
            }

            float totalViolation = 0;
            for(const auto& item : unfrozenItems) {
                if(remainingFreeSpace > 0.f && totalFlexGrow > 0.f && sign == FlexSign::Positive) {
                    auto extraSpace = remainingFreeSpace * item->flexGrow() / totalFlexGrow;
                    item->setTargetMainSize(extraSpace + item->flexBaseSize());
                } else if(remainingFreeSpace < 0.f && totalScaledFlexShrink > 0.f && sign == FlexSign::Negative) {
                    auto extraSpace = remainingFreeSpace * item->flexBaseSize() * item->flexShrink() / totalScaledFlexShrink;
                    item->setTargetMainSize(extraSpace + item->flexBaseSize());
                } else {
                    item->setTargetMainSize(item->flexBaseSize());
                }

                auto unclampedSize = item->targetMainSize();
                auto clampedSize = item->constrainMainSize(unclampedSize);
                auto violation = clampedSize - unclampedSize;
                if(violation > 0.f) {
                    item->setViolation(FlexViolation::Min);
                } else if(violation < 0.f) {
                    item->setViolation(FlexViolation::Max);
                } else {
                    item->setViolation(FlexViolation::None);
                }

                item->setTargetMainSize(clampedSize);
                totalViolation += violation;
            }

            auto freezeMinViolations = totalViolation > 0.f;
            auto freezeMaxViolations = totalViolation < 0.f;
            auto freezeAllViolations = totalViolation == 0.f;

            auto itemIterator = unfrozenItems.begin();
            while(itemIterator != unfrozenItems.end()) {
                auto currentIterator = itemIterator++;
                auto item = *currentIterator;
                if(freezeAllViolations || (freezeMinViolations && item->minViolation())
                    || (freezeMaxViolations && item->maxViolation())) {
                    totalFlexGrow -= item->flexGrow();
                    totalFlexShrink -= item->flexShrink();
                    totalScaledFlexShrink -= item->flexShrink() * item->flexBaseSize();
                    remainingFreeSpace -= item->targetMainSize() - item->flexBaseSize();
                    unfrozenItems.erase(currentIterator);
                }
            }
        }

        auto availableSpace = mainContentSize;
        for(const auto& item : items)
            availableSpace -= item.targetMainMarginBoxSize();
        availableSpace -= m_gapBetweenItems * (items.size() - 1);

        size_t autoMarginCount = 0;
        if(availableSpace > 0.f) {
            for(const auto& item : items) {
                auto child = item.box();
                auto childStyle = child->style();
                if(isHorizontalFlow()) {
                    if(childStyle->marginLeft().isAuto())
                        ++autoMarginCount;
                    if(childStyle->marginRight().isAuto()) {
                        ++autoMarginCount;
                    }
                } else {
                    if(childStyle->marginTop().isAuto())
                        ++autoMarginCount;
                    if(childStyle->marginBottom().isAuto()) {
                        ++autoMarginCount;
                    }
                }
            }
        }

        float autoMarginOffset = 0;
        if(autoMarginCount > 0) {
            autoMarginOffset = availableSpace / autoMarginCount;
            availableSpace = 0.f;
        }

        auto mainOffset = borderAndPaddingStart();
        switch(m_justifyContent) {
        case AlignContent::FlexEnd:
            mainOffset += availableSpace;
            break;
        case AlignContent::Center:
            mainOffset += availableSpace / 2.f;
            break;
        case AlignContent::SpaceAround:
            if(availableSpace > 0)
                mainOffset += availableSpace / (2.f * items.size());
            break;
        case AlignContent::SpaceEvenly:
            if(availableSpace > 0)
                mainOffset += availableSpace / (items.size() + 1);
            break;
        default:
            break;
        }

        auto mainSize = mainContentSize + borderAndPaddingStart() + borderAndPaddingEnd();
        for(size_t i = 0; i < items.size(); i++) {
            const auto& item = items[i];
            auto child = item.box();
            if(isHorizontalFlow()) {
                child->setOverrideWidth(item.targetMainBorderBoxSize());
            } else {
                child->setOverrideHeight(item.targetMainBorderBoxSize());
            }

            child->layout(nullptr);

            if(autoMarginCount > 0) {
                auto childStyle = child->style();
                if(isHorizontalFlow()) {
                    if(childStyle->marginLeft().isAuto())
                        child->setMarginLeft(autoMarginOffset);
                    if(childStyle->marginRight().isAuto()) {
                        child->setMarginRight(autoMarginOffset);
                    }
                } else {
                    if(childStyle->marginTop().isAuto())
                        child->setMarginTop(autoMarginOffset);
                    if(childStyle->marginBottom().isAuto()) {
                        child->setMarginBottom(autoMarginOffset);
                    }
                }
            }

            mainOffset += item.marginStart();
            switch(m_flexDirection) {
            case FlexDirection::Row:
                child->setX(mainOffset);
                break;
            case FlexDirection::RowReverse:
                child->setX(mainSize - mainOffset - item.borderBoxMainSize());
                break;
            case FlexDirection::Column:
                child->setY(mainOffset);
                break;
            case FlexDirection::ColumnReverse:
                child->setY(mainSize - mainOffset - item.borderBoxMainSize());
                break;
            }

            mainOffset += item.borderBoxMainSize();
            mainOffset += item.marginEnd();
            if(i != items.size() - 1) {
                mainOffset += m_gapBetweenItems;
                if(availableSpace > 0 && items.size() > 1) {
                    switch(m_justifyContent) {
                    case AlignContent::SpaceAround:
                        mainOffset += availableSpace / items.size();
                        break;
                    case AlignContent::SpaceBetween:
                        mainOffset += availableSpace / (items.size() - 1);
                        break;
                    case AlignContent::SpaceEvenly:
                        mainOffset += availableSpace / (items.size() + 1);
                    default:
                        break;
                    }
                }
            }
        }

        mainOffset += borderAndPaddingEnd();
        if(isVerticalFlow())
            setHeight(std::max(mainOffset, height()));
        lines.emplace_back(items);
    }

    auto crossOffset = borderAndPaddingBefore();
    for(auto& line : lines) {
        float crossSize = 0;
        float maxCrossAscent = 0;
        float maxCrossDescent = 0;
        for(const auto& item : line.items()) {
            auto child = item.box();
            if(isHorizontalFlow()) {
                child->setY(crossOffset + item.marginBefore());
            } else {
                child->setX(crossOffset + item.marginBefore());
            }

            if(item.alignSelf() == AlignItem::Baseline && isHorizontalFlow()) {
                auto ascent = item.marginBoxCrossBaseline();
                auto descent = item.marginBoxCrossSize() - ascent;
                maxCrossAscent = std::max(maxCrossAscent, ascent);
                maxCrossDescent = std::max(maxCrossDescent, descent);
                crossSize = std::max(crossSize, maxCrossAscent + maxCrossDescent);
            } else {
                crossSize = std::max(crossSize, item.marginBoxCrossSize());
            }
        }

        line.setCrossOffset(crossOffset);
        line.setCrossSize(crossSize);
        line.setCrossBaseline(maxCrossAscent);
        crossOffset += crossSize;
    }

    if(lines.size() > 1)
        crossOffset += m_gapBetweenLines * (lines.size() - 1);
    crossOffset += borderAndPaddingAfter();
    if(isHorizontalFlow())
        setHeight(std::max(crossOffset, height()));
    updateHeight();

    if(!isMultiLine() && !lines.empty())
        lines.front().setCrossSize(availableCrossSize());
    if(isMultiLine() && !lines.empty()) {
        auto availableSpace = availableCrossSize();
        for(const auto& line : lines)
            availableSpace -= line.crossSize();
        availableSpace -= m_gapBetweenLines * (lines.size() - 1);

        float lineOffset = 0;
        switch(m_alignContent) {
        case AlignContent::FlexEnd:
            lineOffset += availableSpace;
            break;
        case AlignContent::Center:
            lineOffset += availableSpace / 2.f;
            break;
        case AlignContent::SpaceAround:
            if(availableSpace > 0)
                lineOffset += availableSpace / (2.f * lines.size());
            break;
        case AlignContent::SpaceEvenly:
            if(availableSpace > 0)
                lineOffset += availableSpace / (lines.size() + 1);
            break;
        default:
            break;
        }

        for(auto& line : lines) {
            line.setCrossOffset(lineOffset + line.crossOffset());
            for(const auto& item : line.items()) {
                auto child = item.box();
                if(isHorizontalFlow()) {
                    child->setY(lineOffset + child->y());
                } else {
                    child->setX(lineOffset + child->x());
                }
            }

            if(m_alignContent == AlignContent::Stretch && availableSpace > 0) {
                auto lineSize = availableSpace / lines.size();
                line.setCrossSize(lineSize + line.crossSize());
                lineOffset += lineSize;
            }

            if(lines.size() > 1) {
                lineOffset += m_gapBetweenLines;
                if(availableSpace > 0) {
                    switch(m_alignContent) {
                    case AlignContent::SpaceAround:
                        lineOffset += availableSpace / lines.size();
                        break;
                    case AlignContent::SpaceBetween:
                        lineOffset += availableSpace / (lines.size() - 1);
                        break;
                    case AlignContent::SpaceEvenly:
                        lineOffset += availableSpace / (lines.size() + 1);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }

    for(const auto& line : lines) {
        for(const auto& item : line.items()) {
            auto child = item.box();
            auto childStyle = child->style();
            if(isHorizontalFlow()) {
                auto marginTopLength = childStyle->marginTop();
                auto marginBottomLength = childStyle->marginBottom();
                if(marginTopLength.isAuto() || marginBottomLength.isAuto()) {
                    float autoMarginOffset = 0;
                    auto availableSpace = line.crossSize() - item.marginBoxCrossSize();
                    if(marginTopLength.isAuto() && marginBottomLength.isAuto()) {
                        autoMarginOffset += availableSpace / 2.f;
                    } else {
                        autoMarginOffset += availableSpace;
                    }

                    if(marginTopLength.isAuto())
                        child->setMarginTop(autoMarginOffset);
                    if(marginBottomLength.isAuto()) {
                        child->setMarginBottom(autoMarginOffset);
                    }

                    if(marginTopLength.isAuto())
                        child->setY(autoMarginOffset + child->y());
                    continue;
                }
            } else {
                auto marginLeftLength = childStyle->marginLeft();
                auto marginRightLength = childStyle->marginRight();
                if(marginLeftLength.isAuto() || marginRightLength.isAuto()) {
                    float autoMarginOffset = 0;
                    auto availableSpace = line.crossSize() - item.marginBoxCrossSize();
                    if(marginLeftLength.isAuto() && marginRightLength.isAuto()) {
                        autoMarginOffset += availableSpace / 2.f;
                    } else {
                        autoMarginOffset += availableSpace;
                    }

                    if(marginLeftLength.isAuto())
                        child->setMarginLeft(autoMarginOffset);
                    if(marginRightLength.isAuto()) {
                        child->setMarginRight(autoMarginOffset);
                    }

                    auto marginStartLength = style()->isLeftToRightDirection() ? marginLeftLength : marginRightLength;
                    if(marginStartLength.isAuto())
                        child->setX(autoMarginOffset + child->x());
                    continue;
                }
            }

            auto align = item.alignSelf();
            if(align == AlignItem::Stretch) {
                if(isHorizontalFlow() && childStyle->height().isAuto()) {
                    auto childHeight = line.crossSize() - child->marginHeight() - child->borderAndPaddingHeight();
                    childHeight = item.constrainHeight(childHeight) + child->borderAndPaddingHeight();
                    if(!isNearlyEqual(childHeight, child->height())) {
                        child->setOverrideHeight(childHeight);
                        child->layout(nullptr);
                    }
                } else if(isVerticalFlow() && childStyle->width().isAuto()) {
                    auto childWidth = line.crossSize() - child->marginWidth() - child->borderAndPaddingWidth();
                    childWidth = item.constrainWidth(childWidth) + child->borderAndPaddingWidth();
                    if(!isNearlyEqual(childWidth, child->width())) {
                        child->setOverrideWidth(childWidth);
                        child->layout(nullptr);
                    }
                }
            }

            if(align == AlignItem::Stretch || (align == AlignItem::Baseline && !isHorizontalFlow()))
                align = AlignItem::FlexStart;
            if(m_flexWrap == FlexWrap::WrapReverse) {
                if(align == AlignItem::FlexStart) {
                    align = AlignItem::FlexEnd;
                } else if(align == AlignItem::FlexEnd) {
                    align = AlignItem::FlexStart;
                }
            }

            float alignOffset = 0;
            auto availableSpace = line.crossSize() - item.marginBoxCrossSize();
            if(align == AlignItem::FlexEnd) {
                alignOffset += availableSpace;
            } else if(align == AlignItem::Center) {
                alignOffset += availableSpace / 2.f;
            } else if(align == AlignItem::Baseline) {
                alignOffset += line.crossBaseline() - item.marginBoxCrossBaseline();
            }

            if(isHorizontalFlow()) {
                child->setY(alignOffset + child->y());
            } else {
                child->setX(alignOffset + child->x());
            }
        }
    }

    if(m_flexWrap == FlexWrap::WrapReverse) {
        auto availableSpace = availableCrossSize();
        for(const auto& line : lines) {
            auto originalOffset = line.crossOffset() - borderAndPaddingBefore();
            auto newOffset = availableSpace - originalOffset - line.crossSize();
            auto delta = newOffset - originalOffset;
            for(const auto& item : line.items()) {
                auto child = item.box();
                if(isHorizontalFlow()) {
                    child->setY(delta + child->y());
                } else {
                    child->setX(delta + child->x());
                }
            }
        }
    }

    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned()) {
            auto childLayer = child->layer();
            childLayer->setStaticLeft(borderAndPaddingStart());
            childLayer->setStaticTop(borderAndPaddingBefore());
            child->containingBlock()->insertPositonedBox(child);
        } else if(style()->isRightToLeftDirection()) {
            child->setX(width() - child->width() - child->x());
        }
    }

    layoutPositionedBoxes();
    updateOverflowRect();
}

void FlexibleBox::build()
{
    auto alignItems = style()->alignItems();
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned())
            continue;
        auto childStyle = child->style();
        auto order = childStyle->order();
        auto flexGlow = childStyle->flexGrow();
        auto flexShrink = childStyle->flexShrink();
        auto alignSelf = childStyle->alignSelf();
        if(alignSelf == AlignItem::Auto)
            alignSelf = alignItems;
        m_items.emplace_back(child, order, flexGlow, flexShrink, alignSelf);
    }

    auto rowGap = style()->rowGap().value_or(0);
    auto columnGap = style()->columnGap().value_or(0);

    m_gapBetweenItems = isVerticalFlow() ? rowGap : columnGap;
    m_gapBetweenLines = isVerticalFlow() ? columnGap : rowGap;

    auto compare_func = [](const auto& a, const auto& b) { return a.order() < b.order(); };
    std::stable_sort(m_items.begin(), m_items.end(), compare_func);
    BlockBox::build();
}

void FlexibleBox::paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(phase == PaintPhase::Contents) {
        for(const auto& item : m_items) {
            auto child = item.box();
            if(!child->hasLayer()) {
                child->paint(info, offset, PaintPhase::Decorations);
                child->paint(info, offset, PaintPhase::Floats);
                child->paint(info, offset, PaintPhase::Contents);
                child->paint(info, offset, PaintPhase::Outlines);
            }
        }
    }
}

} // namespace plutobook
