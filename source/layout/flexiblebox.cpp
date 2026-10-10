/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "flexiblebox.h"
#include "boxlayer.h"
#include "fragmentbuilder.h"

#include <span>
#include <ranges>
#include <list>
#include <limits>

namespace plutobook {

FlexItem::FlexItem(BoxFrame* box, int order, float flexGrow, float flexShrink, AlignItem alignSelf)
    : m_box(box)
    , m_alignSelf(alignSelf)
    , m_order(order)
    , m_flexGrow(flexGrow)
    , m_flexShrink(flexShrink)
{
    box->setIsFlexItem(true);
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
        width = std::max(width, m_box->minPreferredWidth() - m_box->borderAndPaddingWidth());
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

std::optional<float> FlexItem::computeMainSizeUsing(const Length& length) const
{
    if(isHorizontalFlow())
        return computeWidthUsing(length);
    return computeHeightUsing(length);
}

float FlexItem::constrainMainSize(float size) const
{
    return std::max(m_minMainSize, std::min(size, m_maxMainSize));
}

float FlexItem::constrainCrossSize(float size) const
{
    if(isHorizontalFlow())
        return constrainHeight(size);
    return constrainWidth(size);
}

float FlexItem::computeContentHeight()
{
    if(!hasNaturalHeight()) {
        m_box->layout(nullptr);
        m_naturalHeight = m_box->height();
    }

    return std::max(0.f, m_naturalHeight - m_box->borderAndPaddingHeight());
}

float FlexItem::computeMinContentMainSize()
{
    if(isHorizontalFlow())
        return std::max(0.f, m_box->minPreferredWidth() - m_box->borderAndPaddingWidth());
    return computeContentHeight();
}

float FlexItem::computeMaxContentMainSize()
{
    if(isHorizontalFlow())
        return std::max(0.f, m_box->maxPreferredWidth() - m_box->borderAndPaddingWidth());
    return computeContentHeight();
}

void FlexItem::computeHypotheticalMainSize()
{
    auto style = m_box->style();
    auto mainLength = isHorizontalFlow() ? style->width() : style->height();
    auto maxLength = isHorizontalFlow() ? style->maxWidth() : style->maxHeight();
    auto minLength = isHorizontalFlow() ? style->minWidth() : style->minHeight();

    auto flexBasis = style->flexBasis();
    if(flexBasis.isAuto())
        flexBasis = mainLength;
    if(auto baseSize = computeMainSizeUsing(flexBasis)) {
        m_flexBaseSize = baseSize.value();
    } else {
        m_flexBaseSize = computeMaxContentMainSize();
    }

    m_maxMainSize = std::numeric_limits<float>::max();
    if(!maxLength.isNone() && !maxLength.isAuto()) {
        if(auto maxSize = computeMainSizeUsing(maxLength)) {
            m_maxMainSize = std::max(0.f, maxSize.value());
        }
    }

    if(!minLength.isAuto()) {
        m_minMainSize = std::max(0.f, computeMainSizeUsing(minLength).value_or(0.f));
    } else if(style->isOverflowHidden()) {
        m_minMainSize = 0.f;
    } else {
        auto contentSize = std::min(m_maxMainSize, computeMinContentMainSize());
        if(auto mainSize = computeMainSizeUsing(mainLength)) {
            m_minMainSize = std::min(contentSize, std::min(m_maxMainSize, mainSize.value()));
        } else {
            m_minMainSize = contentSize;
        }
    }

    if(isHorizontalFlow() && m_box->isTableBox())
        m_minMainSize = std::max(m_minMainSize, m_box->minPreferredWidth() - m_box->borderAndPaddingWidth());
    m_targetMainSize = constrainMainSize(m_flexBaseSize);
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
    if(isVerticalFlow())
        return 0.f;
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

        child->updateHorizontalMargins(0.f);
        child->updateHorizontalPaddings(0.f);

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

    const auto itemCount = m_items.size();
    if(itemCount > 1 && isHorizontalFlow()) {
        auto gapWidth = m_gapBetweenItems * (itemCount - 1);
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
    for(auto& item : m_items) {
        auto child = item.box();
        if(baselineChild == nullptr)
            baselineChild = child;
        if(item.alignSelf() == AlignItem::Baseline) {
            baselineChild = child;
            break;
        }
    }

    if(baselineChild == nullptr)
        return std::nullopt;
    if(auto baseline = baselineChild->firstLineBaseline())
        return baseline.value() + baselineChild->y();
    return baselineChild->y() + baselineChild->height();
}

std::optional<float> FlexibleBox::lastLineBaseline() const
{
    const BoxFrame* baselineChild = nullptr;
    for(auto& item : m_items | std::views::reverse) {
        auto child = item.box();
        if(baselineChild == nullptr)
            baselineChild = child;
        if(item.alignSelf() == AlignItem::Baseline) {
            baselineChild = child;
            break;
        }
    }

    if(baselineChild == nullptr)
        return std::nullopt;
    if(auto baseline = baselineChild->lastLineBaseline())
        return baseline.value() + baselineChild->y();
    return baselineChild->y() + baselineChild->height();
}

std::optional<float> FlexibleBox::inlineBlockBaseline() const
{
    return firstLineBaseline();
}

float FlexibleBox::unbreakableHeight(const FragmentBuilder* fragmentainer, float offset) const
{
    if(fragmentainer->avoidsBreakInside(this))
        return height();
    if(m_items.empty())
        return borderAndPaddingHeight();
    auto leadingHeight = borderAndPaddingTop();
    float unbreakableHeight = 0.f;
    for(auto& item : m_items) {
        auto child = item.box();
        if(item.naturalTop() > leadingHeight + kLayoutEpsilon)
            continue;
        unbreakableHeight = std::max(unbreakableHeight, child->unbreakableHeight(fragmentainer, offset + leadingHeight));
    }

    return leadingHeight + unbreakableHeight;
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
    switch(style()->flexDirection()) {
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
    switch(style()->flexDirection()) {
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

bool FlexibleBox::isHorizontalFlow() const
{
    switch(style()->flexDirection()) {
    case FlexDirection::Row:
    case FlexDirection::RowReverse:
        return true;
    default:
        return false;
    }
}

bool FlexibleBox::isVerticalFlow() const
{
    switch(style()->flexDirection()) {
    case FlexDirection::Column:
    case FlexDirection::ColumnReverse:
        return true;
    default:
        return false;
    }
}

bool FlexibleBox::isWrapReverse() const
{
    return style()->flexWrap() == FlexWrap::WrapReverse;
}

bool FlexibleBox::isMultiLine() const
{
    switch(style()->flexWrap()) {
    case FlexWrap::Wrap:
    case FlexWrap::WrapReverse:
        return true;
    default:
        return false;
    }
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
    float crossAscent() const { return m_crossAscent; }
    float crossDescent() const { return m_crossDescent; }

    void setCrossOffset(float offset) { m_crossOffset = offset; }
    void setCrossSize(float size) { m_crossSize = size; }
    void setCrossAscent(float ascent) { m_crossAscent = ascent; }
    void setCrossDescent(float descent) { m_crossDescent = descent; }

private:
    FlexItemSpan m_items;
    float m_crossOffset = 0;
    float m_crossSize = 0;
    float m_crossAscent = 0;
    float m_crossDescent = 0;
};

static float initialAlignmentOffset(AlignContent alignment, float availableSpace, size_t count)
{
    if(alignment == AlignContent::FlexEnd)
        return availableSpace;
    if(alignment == AlignContent::Center)
        return availableSpace / 2.f;
    if(availableSpace > 0.f) {
        if(alignment == AlignContent::SpaceAround)
            return availableSpace / (2.f * count);
        if(alignment == AlignContent::SpaceEvenly) {
            return availableSpace / (count + 1);
        }
    }

    if(alignment == AlignContent::SpaceAround
        || alignment == AlignContent::SpaceEvenly) {
        return availableSpace / 2.f;
    }

    return 0.f;
}

static float alignmentSpacing(AlignContent alignment, float availableSpace, size_t count)
{
    if(availableSpace > 0.f) {
        if(alignment == AlignContent::SpaceBetween)
            return availableSpace / (count - 1);
        if(alignment == AlignContent::SpaceAround)
            return availableSpace / count;
        if(alignment == AlignContent::SpaceEvenly) {
            return availableSpace / (count + 1);
        }
    }

    return 0.f;
}

void FlexibleBox::layout(FragmentBuilder* fragmentainer)
{
    updateWidth();
    setHeight(borderAndPaddingHeight());

    float maxHypotheticalMainSize = 0;
    for(auto& item : m_items) {
        auto child = item.box();
        child->clearOverrideSize();
        child->updateMarginWidths(availableWidth());
        child->updatePaddingWidths(availableWidth());

        item.setNaturalTop(0);
        item.setNaturalHeight(-1);

        item.computeHypotheticalMainSize();
        maxHypotheticalMainSize += m_gapBetweenItems + item.targetMainMarginBoxSize();
    }

    if(!m_items.empty()) {
        maxHypotheticalMainSize -= m_gapBetweenItems;
    }

    const auto lineBreakLength = computeMainContentSize(maxHypotheticalMainSize);
    const auto flexDirection = style()->flexDirection();
    const auto justifyContent = style()->justifyContent();
    const auto alignContent = style()->alignContent();

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
            if(isMultiLine() && it != begin && totalHypotheticalMainSize + it->targetMainMarginBoxSize() > lineBreakLength + kLayoutEpsilon)
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

        float frozenFreeSpace = 0;
        while(!unfrozenItems.empty()) {
            auto remainingFreeSpace = initialFreeSpace - frozenFreeSpace;
            auto totalFlexFactor = sign == FlexSign::Positive ? totalFlexGrow : totalFlexShrink;
            if(totalFlexFactor > 0.f && totalFlexFactor < 1.f) {
                auto scaledInitialFreeSpace = initialFreeSpace * totalFlexFactor;
                if(std::abs(scaledInitialFreeSpace) < std::abs(remainingFreeSpace)) {
                    remainingFreeSpace = scaledInitialFreeSpace;
                }
            }

            float totalViolation = 0;
            for(auto item : unfrozenItems) {
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
                    frozenFreeSpace += item->targetMainSize() - item->flexBaseSize();
                    unfrozenItems.erase(currentIterator);
                }
            }
        }

        const auto itemCount = items.size();

        auto availableSpace = mainContentSize;
        for(auto& item : items)
            availableSpace -= item.targetMainMarginBoxSize();
        availableSpace -= m_gapBetweenItems * (itemCount - 1);

        size_t autoMarginCount = 0;
        if(availableSpace > 0.f) {
            for(auto& item : items) {
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

        const auto mainSize = mainContentSize + borderAndPaddingStart() + borderAndPaddingEnd();

        auto mainOffset = borderAndPaddingStart() + initialAlignmentOffset(justifyContent, availableSpace, itemCount);
        for(size_t i = 0; i < itemCount; i++) {
            auto& item = items[i];
            auto child = item.box();
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

            if(isHorizontalFlow()) {
                child->setOverrideWidth(item.targetMainBorderBoxSize());
            } else {
                child->setOverrideHeight(item.targetMainBorderBoxSize());
            }

            child->layout(nullptr);
            if(!child->hasOverrideHeight()) {
                assert(isHorizontalFlow());
                item.setNaturalHeight(child->height());
            }

            mainOffset += item.marginStart();
            switch(flexDirection) {
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

            mainOffset += item.borderBoxMainSize() + item.marginEnd();
            if(i != itemCount - 1) {
                mainOffset += m_gapBetweenItems + alignmentSpacing(justifyContent, availableSpace, itemCount);
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
        float crossAscent = 0;
        float crossDescent = 0;
        for(auto& item : line.items()) {
            if(item.alignSelf() == AlignItem::Baseline && isHorizontalFlow()) {
                auto ascent = item.marginBoxCrossBaseline();
                auto descent = item.marginBoxCrossSize() - ascent;
                crossAscent = std::max(crossAscent, ascent);
                crossDescent = std::max(crossDescent, descent);
                crossSize = std::max(crossSize, crossAscent + crossDescent);
            } else {
                crossSize = std::max(crossSize, item.marginBoxCrossSize());
            }
        }

        line.setCrossOffset(crossOffset);
        line.setCrossSize(crossSize);
        line.setCrossAscent(crossAscent);
        line.setCrossDescent(crossDescent);
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

        auto lineOffset = initialAlignmentOffset(alignContent, availableSpace, lines.size());
        for(auto& line : lines) {
            line.setCrossOffset(lineOffset + line.crossOffset());
            if(alignContent == AlignContent::Stretch && availableSpace > 0) {
                auto lineSize = availableSpace / lines.size();
                line.setCrossSize(lineSize + line.crossSize());
                lineOffset += lineSize;
            }

            if(lines.size() > 1) {
                lineOffset += m_gapBetweenLines + alignmentSpacing(alignContent, availableSpace, lines.size());
            }
        }
    }

    if(isWrapReverse()) {
        auto crossStart = borderAndPaddingBefore();
        auto crossEnd = crossStart + availableCrossSize();
        for(auto& line : lines) {
            line.setCrossOffset(crossStart + crossEnd - line.crossOffset() - line.crossSize());
        }
    }

    for(auto& line : lines) {
        for(auto& item : line.items()) {
            auto child = item.box();
            alignItem(item, line, nullptr);
            item.setNaturalTop(child->y());
        }
    }

    if(fragmentainer) {
        adjustLinesInFragmentFlow(lines, fragmentainer);
    }

    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned()) {
            auto childLayer = child->layer();
            childLayer->setStaticLeft(borderAndPaddingLeft());
            childLayer->setStaticTop(borderAndPaddingTop());
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
        auto flexGrow = childStyle->flexGrow();
        auto flexShrink = childStyle->flexShrink();
        auto alignSelf = childStyle->alignSelf();
        if(alignSelf == AlignItem::Auto)
            alignSelf = alignItems;
        m_items.emplace_back(child, order, flexGrow, flexShrink, alignSelf);
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
        for(auto& item : m_items) {
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

void FlexibleBox::layoutItem(BoxFrame* child, FragmentBuilder* fragmentainer) const
{
    auto itemTop = child->y();
    if(fragmentainer)
        fragmentainer->enterFragment(itemTop);
    child->layout(fragmentainer);
    if(fragmentainer) {
        fragmentainer->leaveFragment(itemTop);
    }
}

void FlexibleBox::stretchItem(FlexItem& item, const FlexLine& line, FragmentBuilder* fragmentainer) const
{
    auto child = item.box();
    auto childStyle = child->style();
    if(isHorizontalFlow()) {
        if(!childStyle->height().isAuto() || childStyle->marginTop().isAuto() || childStyle->marginBottom().isAuto())
            return;
        auto childHeight = line.crossSize() - child->marginHeight() - child->borderAndPaddingHeight();
        childHeight = item.constrainHeight(childHeight) + child->borderAndPaddingHeight();
        if(!isNearlyEqual(childHeight, child->height())) {
            child->setOverrideHeight(childHeight);
            layoutItem(child, fragmentainer);
        }
    } else {
        if(!childStyle->width().isAuto() || childStyle->marginLeft().isAuto() || childStyle->marginRight().isAuto())
            return;
        auto childWidth = line.crossSize() - child->marginWidth() - child->borderAndPaddingWidth();
        childWidth = item.constrainWidth(childWidth) + child->borderAndPaddingWidth();
        if(!isNearlyEqual(childWidth, child->width())) {
            child->setOverrideWidth(childWidth);
            layoutItem(child, fragmentainer);
            item.setNaturalHeight(-1);
        }
    }
}

void FlexibleBox::alignItem(FlexItem& item, const FlexLine& line, FragmentBuilder* fragmentainer) const
{
    if(alignItemAutoMargins(item, line))
        return;
    auto align = item.alignSelf();
    if(align == AlignItem::Stretch)
        stretchItem(item, line, fragmentainer);
    if(align == AlignItem::Stretch || (align == AlignItem::Baseline && isVerticalFlow()))
        align = AlignItem::FlexStart;
    if(isWrapReverse()) {
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
        alignOffset += line.crossAscent() - item.marginBoxCrossBaseline();
        if(isWrapReverse()) {
            alignOffset += line.crossSize() - line.crossAscent() - line.crossDescent();
        }
    }

    auto child = item.box();
    if(isHorizontalFlow()) {
        child->setY(alignOffset + line.crossOffset() + item.marginBefore());
    } else {
        child->setX(alignOffset + line.crossOffset() + item.marginBefore());
    }
}

bool FlexibleBox::alignItemAutoMargins(FlexItem& item, const FlexLine& line) const
{
    auto child = item.box();
    auto childStyle = child->style();

    auto availableSpace = std::max(0.f, line.crossSize() - item.marginBoxCrossSize());
    if(isHorizontalFlow()) {
        auto marginTopLength = childStyle->marginTop();
        auto marginBottomLength = childStyle->marginBottom();
        if(!marginTopLength.isAuto() && !marginBottomLength.isAuto())
            return false;
        auto autoMarginOffset = availableSpace;
        if(marginTopLength.isAuto() && marginBottomLength.isAuto())
            autoMarginOffset /= 2.f;
        if(marginTopLength.isAuto())
            child->setMarginTop(autoMarginOffset);
        if(marginBottomLength.isAuto())
            child->setMarginBottom(autoMarginOffset);
        child->setY(line.crossOffset() + item.marginBefore());
        return true;
    }

    auto marginLeftLength = childStyle->marginLeft();
    auto marginRightLength = childStyle->marginRight();
    if(!marginLeftLength.isAuto() && !marginRightLength.isAuto())
        return false;
    auto autoMarginOffset = availableSpace;
    if(marginLeftLength.isAuto() && marginRightLength.isAuto())
        autoMarginOffset /= 2.f;
    if(marginLeftLength.isAuto())
        child->setMarginLeft(autoMarginOffset);
    if(marginRightLength.isAuto())
        child->setMarginRight(autoMarginOffset);
    child->setX(line.crossOffset() + item.marginBefore());
    return true;
}

static float lineUnbreakableHeight(const FlexLine& line, const FragmentBuilder* fragmentainer, float offset)
{
    float remainingHeight = 0.f;
    auto fragmentHeight = fragmentainer->fragmentHeightForOffset(offset);
    if(fragmentHeight > 0.f)
        remainingHeight = fragmentainer->fragmentRemainingHeightForOffset(offset, AssociateWithLatterFragment);
    float unbreakableHeight = 0.f;
    for(auto& item : line.items()) {
        auto child = item.box();
        auto itemOffset = item.naturalTop() - line.crossOffset();
        if(fragmentHeight > 0.f && remainingHeight <= itemOffset)
            continue;
        unbreakableHeight = std::max(unbreakableHeight, itemOffset + child->unbreakableHeight(fragmentainer, offset + itemOffset));
    }

    return unbreakableHeight;
}

static const BoxFrame* itemAlwaysBreakBefore(const FlexLine& line, const FragmentBuilder* fragmentainer)
{
    for(auto& item : line.items()) {
        auto child = item.box();
        if(fragmentainer->alwaysBreakBefore(child)) {
            return child;
        }
    }

    return nullptr;
}

static const BoxFrame* itemAlwaysBreakAfter(const FlexLine& line, const FragmentBuilder* fragmentainer)
{
    for(auto& item : line.items()) {
        auto child = item.box();
        if(fragmentainer->alwaysBreakAfter(child)) {
            return child;
        }
    }

    return nullptr;
}

static float fragmentationGrowth(float layoutHeight, float naturalHeight, float fragmentedHeight)
{
    return std::max(0.f, fragmentedHeight - std::max(layoutHeight, naturalHeight));
}

float FlexibleBox::adjustLineInFragmentFlow(FlexLine& line, FragmentBuilder* fragmentainer, float offset) const
{
    assert(isHorizontalFlow());
    auto lineTop = offset + line.crossOffset();
    auto lineHeight = line.crossSize();

    auto newTop = lineTop;
    if(auto child = itemAlwaysBreakBefore(line, fragmentainer))
        newTop = fragmentainer->applyFragmentBreakBefore(child, newTop);
    auto unbreakableHeight = lineUnbreakableHeight(line, fragmentainer, newTop);
    newTop = fragmentainer->adjustOffsetInFragmentFlow(newTop, lineHeight, unbreakableHeight);

    float crossAscent = 0;
    float crossDescent = 0;
    auto delta = newTop - line.crossOffset();
    for(auto& item : line.items()) {
        auto child = item.box();
        auto childStyle = child->style();
        auto height = child->height();

        child->setY(delta + child->y());
        child->setOverrideHeight(-1);

        if(childStyle->marginTop().isAuto())
            child->setMarginTop(0.f);
        if(childStyle->marginBottom().isAuto()) {
            child->setMarginBottom(0.f);
        }

        layoutItem(child, fragmentainer);

        assert(item.hasNaturalHeight());
        auto newHeight = height + fragmentationGrowth(height, item.naturalHeight(), child->height());
        auto marginBoxHeight = newHeight + child->marginTop() + child->marginBottom();
        if(item.alignSelf() == AlignItem::Baseline) {
            auto ascent = item.marginBoxCrossBaseline();
            auto descent = marginBoxHeight - ascent;
            crossAscent = std::max(crossAscent, ascent);
            crossDescent = std::max(crossDescent, descent);
            lineHeight = std::max(lineHeight, crossAscent + crossDescent);
        } else {
            lineHeight = std::max(lineHeight, marginBoxHeight);
        }
    }

    auto growth = lineHeight - line.crossSize();
    line.setCrossOffset(newTop);
    line.setCrossSize(lineHeight);
    line.setCrossAscent(crossAscent);
    line.setCrossDescent(crossDescent);
    for(auto& item : line.items()) {
        alignItem(item, line, fragmentainer);
    }

    auto lineBottom = newTop + lineHeight;
    auto newBottom = lineBottom;
    if(auto child = itemAlwaysBreakAfter(line, fragmentainer))
        newBottom = fragmentainer->applyFragmentBreakAfter(child, newBottom);
    return (newTop - lineTop) + growth + (newBottom - lineBottom);
}

float FlexibleBox::adjustItemInFragmentFlow(FlexItem& item, FragmentBuilder* fragmentainer, float offset) const
{
    assert(isVerticalFlow());
    auto child = item.box();
    auto top = offset + child->y();
    auto height = child->height();

    auto newTop = fragmentainer->applyFragmentBreakBefore(child, top);
    auto unbreakableHeight = child->unbreakableHeight(fragmentainer, newTop);
    newTop = fragmentainer->adjustOffsetInFragmentFlow(newTop, height, unbreakableHeight);

    child->setY(newTop);
    child->setOverrideHeight(-1);
    if(!item.hasNaturalHeight()) {
        child->layout(nullptr);
        item.setNaturalHeight(child->height());
    }

    layoutItem(child, fragmentainer);
    auto newHeight = height + fragmentationGrowth(height, item.naturalHeight(), child->height());
    if(!isNearlyEqual(newHeight, child->height())) {
        child->setOverrideHeight(newHeight);
        layoutItem(child, fragmentainer);
    }

    auto bottom = newTop + child->height();
    auto newBottom = fragmentainer->applyFragmentBreakAfter(child, bottom);
    return (newTop - top) + (newHeight - height) + (newBottom - bottom);
}

void FlexibleBox::adjustLinesInFragmentFlow(FlexLineList& lines, FragmentBuilder* fragmentainer)
{
    const auto lineCount = lines.size();
    for(size_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
        const auto& items = lines[lineIndex].items();
        const auto itemCount = items.size();
        for(size_t itemIndex = 0; itemIndex < itemCount; ++itemIndex) {
            bool isFirst = false;
            bool isLast = false;
            if(isHorizontalFlow()) {
                isFirst = lineIndex == 0;
                isLast = lineIndex + 1 == lineCount;
                if(style()->flexWrap() == FlexWrap::WrapReverse) {
                    std::swap(isFirst, isLast);
                }
            } else {
                isFirst = itemIndex == 0;
                isLast = itemIndex + 1 == itemCount;
                if(style()->flexDirection() == FlexDirection::ColumnReverse) {
                    std::swap(isFirst, isLast);
                }
            }

            auto child = items[itemIndex].box();
            child->setHasFlexItemBefore(!isFirst);
            child->setHasFlexItemAfter(!isLast);
        }
    }

    float offset = 0;
    if(isHorizontalFlow()) {
        if(style()->flexWrap() == FlexWrap::WrapReverse) {
            for(auto& line : lines | std::views::reverse) {
                offset += adjustLineInFragmentFlow(line, fragmentainer, offset);
            }
        } else {
            for(auto& line : lines) {
                offset += adjustLineInFragmentFlow(line, fragmentainer, offset);
            }
        }
    } else {
        for(auto& line : lines) {
            float lineOffset = 0;
            if(style()->flexDirection() == FlexDirection::ColumnReverse) {
                for(auto& item : line.items() | std::views::reverse) {
                    lineOffset += adjustItemInFragmentFlow(item, fragmentainer, lineOffset);
                }
            } else {
                for(auto& item : line.items()) {
                    lineOffset += adjustItemInFragmentFlow(item, fragmentainer, lineOffset);
                }
            }

            offset = std::max(offset, lineOffset);
        }
    }

    if(offset > 0.f) {
        setHeight(offset + height());
        updateHeight();
    }
}

} // namespace plutobook
