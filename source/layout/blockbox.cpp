#include "blockbox.h"
#include "multicolumnbox.h"
#include "linebox.h"
#include "linelayout.h"
#include "boxlayer.h"
#include "document.h"

namespace plutobook {

BlockBox::BlockBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
{
    setReplaced(style->isDisplayInlineType());
}

void BlockBox::computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const
{
    minPreferredWidth = 0;
    maxPreferredWidth = 0;

    auto widthLength = style()->width();
    if(widthLength.isFixed() && !isTableCellBox()) {
        minPreferredWidth = maxPreferredWidth = adjustContentBoxWidth(widthLength.value());
    } else {
        computeIntrinsicWidths(minPreferredWidth, maxPreferredWidth);
    }

    auto maxWidthLength = style()->maxWidth();
    if(maxWidthLength.isFixed()) {
        minPreferredWidth = std::min(minPreferredWidth, adjustContentBoxWidth(maxWidthLength.value()));
        maxPreferredWidth = std::min(maxPreferredWidth, adjustContentBoxWidth(maxWidthLength.value()));
    }

    auto minWidthLength = style()->minWidth();
    if(minWidthLength.isFixed() && minWidthLength.value() > 0) {
        minPreferredWidth = std::max(minPreferredWidth, adjustContentBoxWidth(minWidthLength.value()));
        maxPreferredWidth = std::max(maxPreferredWidth, adjustContentBoxWidth(minWidthLength.value()));
    }

    minPreferredWidth += borderAndPaddingWidth();
    maxPreferredWidth += borderAndPaddingWidth();
}

void BlockBox::insertPositonedBox(BoxFrame* box)
{
    if(m_positionedBoxes == nullptr)
        m_positionedBoxes = std::make_unique<PositionedBoxList>(heap());
    m_positionedBoxes->insert(box);
}

void BlockBox::removePositonedBox(BoxFrame* box)
{
    if(m_positionedBoxes) {
        m_positionedBoxes->erase(box);
    }
}

void BlockBox::layoutPositionedBoxes()
{
    if(m_positionedBoxes) {
        for(auto box : *m_positionedBoxes) {
            box->layout(nullptr);
        }
    }
}

std::optional<float> BlockBox::availableHeight() const
{
    if(isBoxView())
        return document()->availableHeight();
    if(hasOverrideHeight())
        return overrideHeight() - borderAndPaddingHeight();
    if(isAnonymous())
        return containingBlockHeightForContent();
    if(isPositioned() && (!style()->height().isAuto() || (!style()->top().isAuto() && !style()->bottom().isAuto()))) {
        float y = 0;
        float height = 0;
        float marginTop = 0;
        float marginBottom = 0;
        computePositionedHeight(y, height, marginTop, marginBottom);
        return height - borderAndPaddingHeight();
    }

    if(auto height = computeHeightUsing(style()->height()))
        return constrainContentBoxHeight(adjustContentBoxHeight(height.value()));
    return std::nullopt;
}

bool BlockBox::shrinkToAvoidFloats() const
{
    if(isInline() || isFloating() || !avoidsFloats())
        return false;
    return style()->width().isAuto();
}

float BlockBox::shrinkWidthToAvoidFloats(float marginLeft, float marginRight, const BlockFlowBox* container) const
{
    auto availableWidth = container->availableWidthForLine(y()) - marginLeft - marginRight;
    auto marginStart = style()->isLeftToRightDirection() ? marginLeft : marginRight;
    auto marginEnd = style()->isLeftToRightDirection() ? marginRight : marginLeft;
    if(marginStart > 0) {
        auto lineStartOffset = container->startOffsetForLine(y());
        auto contentStartOffset = container->startOffsetForContent();
        auto marginStartOffset = contentStartOffset + marginStart;
        if(lineStartOffset > marginStartOffset) {
            availableWidth += marginStart;
        } else {
            availableWidth += lineStartOffset - contentStartOffset;
        }
    }

    if(marginEnd > 0) {
        auto lineEndOffset = container->endOffsetForLine(y());
        auto contentEndOffset = container->endOffsetForContent();
        auto marginEndOffset = contentEndOffset + marginEnd;
        if(lineEndOffset > marginEndOffset) {
            availableWidth += marginEnd;
        } else {
            availableWidth += lineEndOffset - contentEndOffset;
        }
    }

    return availableWidth;
}

float BlockBox::computeWidthUsing(const Length& widthLength, const BlockBox* container, float containerWidth) const
{
    if(widthLength.isIntrinsic())
        return computeIntrinsicWidthUsing(widthLength, containerWidth);
    if(!widthLength.isAuto())
        return adjustBorderBoxWidth(widthLength.calc(containerWidth));
    auto marginLeft = style()->marginLeft().calcMin(containerWidth);
    auto marginRight = style()->marginRight().calcMin(containerWidth);
    auto width = containerWidth - marginLeft - marginRight;
    auto containerBlock = to<BlockFlowBox>(container);
    if(containerBlock && containerBlock->containsFloats() && shrinkToAvoidFloats())
        width = std::min(width, shrinkWidthToAvoidFloats(marginLeft, marginRight, containerBlock));
    if(isFloating() || isInline() || isFlexItem() || isTableBox()) {
        width = std::min(width, maxPreferredWidth());
        width = std::max(width, minPreferredWidth());
    }

    return width;
}

std::optional<float> BlockBox::computeHeightUsing(const Length& heightLength) const
{
    if(heightLength.isFixed())
        return heightLength.value();
    if(heightLength.isPercent()) {
        if(auto availableHeight = containingBlockHeightForContent()) {
            return heightLength.calc(availableHeight.value());
        }
    }

    return std::nullopt;
}

float BlockBox::constrainWidth(float width, const BlockBox* container, float containerWidth) const
{
    auto minWidthLength = style()->minWidth();
    auto maxWidthLength = style()->maxWidth();

    if(!maxWidthLength.isNone())
        width = std::min(width, computeWidthUsing(maxWidthLength, container, containerWidth));
    if(!minWidthLength.isAuto())
        return std::max(width, computeWidthUsing(minWidthLength, container, containerWidth));
    return std::max(width, adjustBorderBoxWidth(0));
}

float BlockBox::constrainBorderBoxHeight(float height) const
{
    if(auto maxHeight = computeHeightUsing(style()->maxHeight()))
        height = std::min(height, adjustBorderBoxHeight(*maxHeight));
    if(auto minHeight = computeHeightUsing(style()->minHeight()))
        return std::max(height, adjustBorderBoxHeight(*minHeight));
    return std::max(height, adjustBorderBoxHeight(0));
}

float BlockBox::constrainContentBoxHeight(float height) const
{
    if(auto maxHeight = computeHeightUsing(style()->maxHeight()))
        height = std::min(height, adjustContentBoxHeight(*maxHeight));
    if(auto minHeight = computeHeightUsing(style()->minHeight()))
        height = std::max(height, adjustContentBoxHeight(*minHeight));
    return std::max(0.f, height);
}

void BlockBox::computePositionedWidthUsing(const Length& widthLength, const BoxModel* container, Direction containerDirection, float containerWidth,
    const Length& leftLength, const Length& rightLength, const Length& marginLeftLength, const Length& marginRightLength,
    float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto widthLenghtIsAuto = widthLength.isAuto();
    auto leftLenghtIsAuto = leftLength.isAuto();
    auto rightLenghtIsAuto = rightLength.isAuto();

    float leftLengthValue = 0;
    float widthLengthValue = 0;
    if(widthLength.isIntrinsic()) {
        widthLengthValue = computeIntrinsicWidthUsing(widthLength, containerWidth) - borderAndPaddingWidth();
    } else {
        widthLengthValue = adjustContentBoxWidth(widthLength.calc(containerWidth));
    }

    if(!leftLenghtIsAuto && !widthLenghtIsAuto && !rightLenghtIsAuto) {
        width = widthLengthValue;
        leftLengthValue = leftLength.calc(containerWidth);

        auto availableSpace = containerWidth - (leftLengthValue + width + rightLength.calc(containerWidth) + borderAndPaddingWidth());
        if(marginLeftLength.isAuto() && marginRightLength.isAuto()) {
            if(availableSpace >= 0) {
                marginLeft = availableSpace / 2.f;
                marginRight = availableSpace - marginLeft;
            } else {
                if(containerDirection == Direction::Ltr) {
                    marginLeft = 0;
                    marginRight = availableSpace;
                } else {
                    marginLeft = availableSpace;
                    marginRight = 0;
                }
            }
        } else if(marginLeftLength.isAuto()) {
            marginRight = marginRightLength.calc(containerWidth);
            marginLeft = availableSpace - marginRight;
        } else if(marginRightLength.isAuto()) {
            marginLeft = marginLeftLength.calc(containerWidth);
            marginRight = availableSpace - marginLeft;
        } else {
            marginLeft = marginLeftLength.calc(containerWidth);
            marginRight = marginRightLength.calc(containerWidth);
            if(containerDirection == Direction::Rtl) {
                leftLengthValue = (availableSpace + leftLengthValue) - marginLeft - marginRight;
            }
        }
    } else {
        marginLeft = marginLeftLength.calcMin(containerWidth);
        marginRight = marginRightLength.calcMin(containerWidth);

        auto availableSpace = containerWidth - (marginLeft + marginRight + borderAndPaddingWidth());
        if(leftLenghtIsAuto && widthLenghtIsAuto && !rightLenghtIsAuto) {
            auto rightLengthValue = rightLength.calc(containerWidth);

            auto preferredMaxWidth = maxPreferredWidth() - borderAndPaddingWidth();
            auto preferredMinWidth = minPreferredWidth() - borderAndPaddingWidth();
            auto availableWidth = availableSpace - rightLengthValue;
            width = std::min(preferredMaxWidth, std::max(preferredMinWidth, availableWidth));
            leftLengthValue = availableSpace - (width + rightLengthValue);
        } else if(!leftLenghtIsAuto && widthLenghtIsAuto && rightLenghtIsAuto) {
            leftLengthValue = leftLength.calc(containerWidth);

            auto preferredMaxWidth = maxPreferredWidth() - borderAndPaddingWidth();
            auto preferredMinWidth = minPreferredWidth() - borderAndPaddingWidth();
            auto availableWidth = availableSpace - leftLengthValue;
            width = std::min(preferredMaxWidth, std::max(preferredMinWidth, availableWidth));
        } else if(leftLenghtIsAuto && !widthLenghtIsAuto && !rightLenghtIsAuto) {
            width = widthLengthValue;
            leftLengthValue = availableSpace - (width + rightLength.calc(containerWidth));
        } else if(!leftLenghtIsAuto && widthLenghtIsAuto && !rightLenghtIsAuto) {
            leftLengthValue = leftLength.calc(containerWidth);
            width = availableSpace - (leftLengthValue + rightLength.calc(containerWidth));
        } else if (!leftLenghtIsAuto && !widthLenghtIsAuto && rightLenghtIsAuto) {
            width = widthLengthValue;
            leftLengthValue = leftLength.calc(containerWidth);
        }
    }

    x = leftLengthValue + marginLeft + container->borderLeft();
}

void BlockBox::computePositionedWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto container = containingBox();
    auto containerWidth = containingBlockWidthForPositioned(container);
    auto containerDirection = container->style()->direction();

    auto marginLeftLength = style()->marginLeft();
    auto marginRightLength = style()->marginRight();

    auto leftLength = style()->left();
    auto rightLength = style()->right();
    computeHorizontalStaticDistance(leftLength, rightLength, container, containerWidth);

    auto widthLength = style()->width();
    auto minWidthLength = style()->minWidth();
    auto maxWidthLength = style()->maxWidth();
    computePositionedWidthUsing(widthLength, container, containerDirection, containerWidth,
        leftLength, rightLength, marginLeftLength, marginRightLength, x, width, marginLeft, marginRight);
    if(!maxWidthLength.isNone()) {
        float maxX = 0;
        float maxWidth = 0;
        float maxMarginLeft = 0;
        float maxMarginRight = 0;
        computePositionedWidthUsing(maxWidthLength, container, containerDirection, containerWidth,
            leftLength, rightLength, marginLeftLength, marginRightLength, maxX, maxWidth, maxMarginLeft, maxMarginRight);
        if(width > maxWidth) {
            x = maxX;
            width = maxWidth;
            marginLeft = maxMarginLeft;
            marginRight = maxMarginRight;
        }
    }

    if(!minWidthLength.isZero() || minWidthLength.isIntrinsic()) {
        float minX = 0;
        float minWidth = 0;
        float minMarginLeft = 0;
        float minMarginRight = 0;
        computePositionedWidthUsing(minWidthLength, container, containerDirection, containerWidth,
            leftLength, rightLength, marginLeftLength, marginRightLength, minX, minWidth, minMarginLeft, minMarginRight);
        if(width < minWidth) {
            x = minX;
            width = minWidth;
            marginLeft = minMarginLeft;
            marginRight = minMarginRight;
        }
    }

    width += borderAndPaddingWidth();
}

void BlockBox::computePositionedHeightUsing(const Length& heightLength, const BoxModel* container, float containerHeight, float contentHeight,
    const Length& topLength, const Length& bottomLength, const Length& marginTopLength, const Length& marginBottomLength,
    float& y, float& height, float& marginTop, float& marginBottom) const
{
    auto heightLenghtIsAuto = heightLength.isAuto() || heightLength.isIntrinsic();
    auto topLenghtIsAuto = topLength.isAuto();
    auto bottomLenghtIsAuto = bottomLength.isAuto();

    float topLengthValue = 0;
    float heightLengthValue = 0;
    if(isTableBox()) {
        heightLengthValue = contentHeight;
        heightLenghtIsAuto = true;
    } else {
        heightLengthValue = heightLength.calc(containerHeight);
        heightLengthValue = adjustContentBoxHeight(heightLengthValue);
    }

    if(!topLenghtIsAuto && !heightLenghtIsAuto && !bottomLenghtIsAuto) {
        height = heightLengthValue;
        topLengthValue = topLength.calc(containerHeight);

        auto availableSpace = containerHeight - (height + topLengthValue + bottomLength.calc(containerHeight) + borderAndPaddingHeight());
        if(marginTopLength.isAuto() && marginBottomLength.isAuto()) {
            marginTop = availableSpace / 2.f;
            marginBottom = availableSpace - marginTop;
        } else if(marginTopLength.isAuto()) {
            marginBottom = marginBottomLength.calc(containerHeight);
            marginTop = availableSpace - marginBottom;
        } else if(marginBottomLength.isAuto()) {
            marginTop = marginTopLength.calc(containerHeight);
            marginBottom = availableSpace - marginTop;
        } else {
            marginTop = marginTopLength.calc(containerHeight);
            marginBottom = marginBottomLength.calc(containerHeight);
        }
    } else {
        marginTop = marginTopLength.calcMin(containerHeight);
        marginBottom = marginBottomLength.calcMin(containerHeight);

        auto availableSpace = containerHeight - (marginTop + marginBottom + borderAndPaddingHeight());
        if(topLenghtIsAuto && heightLenghtIsAuto && !bottomLenghtIsAuto) {
            height = contentHeight;
            topLengthValue = availableSpace - (height + bottomLength.calc(containerHeight));
        } else if(!topLenghtIsAuto && heightLenghtIsAuto && bottomLenghtIsAuto) {
            topLengthValue = topLength.calc(containerHeight);
            height = contentHeight;
        } else if(topLenghtIsAuto && !heightLenghtIsAuto && !bottomLenghtIsAuto) {
            height = heightLengthValue;
            topLengthValue = availableSpace - (height + bottomLength.calc(containerHeight));
        } else if(!topLenghtIsAuto && heightLenghtIsAuto && !bottomLenghtIsAuto) {
            topLengthValue = topLength.calc(containerHeight);
            height = std::max(0.f, availableSpace - (topLengthValue + bottomLength.calc(containerHeight)));
        } else if(!topLenghtIsAuto && !heightLenghtIsAuto && bottomLenghtIsAuto) {
            height = heightLengthValue;
            topLengthValue = topLength.calc(containerHeight);
        }
    }

    y = topLengthValue + marginTop + container->borderTop();
}

void BlockBox::computePositionedHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    auto container = containingBox();
    auto containerHeight = containingBlockHeightForPositioned(container);
    auto contentHeight = height - borderAndPaddingHeight();

    auto marginTopLength = style()->marginTop();
    auto marginBottomLength = style()->marginBottom();

    auto topLength = style()->top();
    auto bottomLength = style()->bottom();
    computeVerticalStaticDistance(topLength, bottomLength, container);

    auto heightLength = style()->height();
    auto minHeightLength = style()->minHeight();
    auto maxHeightLength = style()->maxHeight();
    computePositionedHeightUsing(heightLength, container, containerHeight, contentHeight,
        topLength, bottomLength, marginTopLength, marginBottomLength, y, height, marginTop, marginBottom);
    if(!maxHeightLength.isNone()) {
        float maxY = 0;
        float maxHeight = 0;
        float maxMarginTop = 0;
        float maxMarginBottom = 0;
        computePositionedHeightUsing(maxHeightLength, container, containerHeight, contentHeight,
            topLength, bottomLength, marginTopLength, marginBottomLength, maxY, maxHeight, maxMarginTop, maxMarginBottom);
        if(height > maxHeight) {
            y = maxY;
            height = maxHeight;
            marginTop = maxMarginTop;
            marginBottom = maxMarginBottom;
        }
    }

    if(!minHeightLength.isZero()) {
        float minY = 0;
        float minHeight = 0;
        float minMarginTop = 0;
        float minMarginBottom = 0;
        computePositionedHeightUsing(minHeightLength, container, containerHeight, contentHeight,
            topLength, bottomLength, marginTopLength, marginBottomLength, minY, minHeight, minMarginTop, minMarginBottom);
        if(height < minHeight) {
            y = minY;
            height = minHeight;
            marginTop = minMarginTop;
            marginBottom = minMarginBottom;
        }
    }

    height += borderAndPaddingHeight();
}

void BlockBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    if(hasOverrideWidth()) {
        width = overrideWidth();
        return;
    }

    if(isTableCellBox())
        return;
    if(isPositioned()) {
        computePositionedWidth(x, width, marginLeft, marginRight);
        if(!isTableBox()) {
            return;
        }
    }

    auto container = containingBlock();
    auto containerWidth = std::max(0.f, containingBlockWidthForContent(container));
    width = computeWidthUsing(style()->width(), container, containerWidth);
    width = constrainWidth(width, container, containerWidth);
    if(isTableBox())
        width = std::max(width, minPreferredWidth());
    computeHorizontalMargins(marginLeft, marginRight, width, container, containerWidth);
}

void BlockBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    if(isTableCellBox())
        return;
    if(hasOverrideHeight()) {
        height = overrideHeight();
        return;
    }

    if(isPositioned()) {
        computePositionedHeight(y, height, marginTop, marginBottom);
        return;
    }

    computeVerticalMargins(marginTop, marginBottom);
    if(isTableBox())
        return;
    if(auto computedHeight = computeHeightUsing(style()->height()))
        height = adjustBorderBoxHeight(computedHeight.value());
    height = constrainBorderBoxHeight(height);
}

std::optional<float> BlockBox::firstLineBaseline() const
{
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isFloatingOrPositioned())
            continue;
        if(auto baseline = child->firstLineBaseline()) {
            return baseline.value() + child->y();
        }
    }

    return std::nullopt;
}

std::optional<float> BlockBox::lastLineBaseline() const
{
    for(auto child = lastBoxFrame(); child; child = child->prevBoxFrame()) {
        if(child->isFloatingOrPositioned())
            continue;
        if(auto baseline = child->lastLineBaseline()) {
            return baseline.value() + child->y();
        }
    }

    return std::nullopt;
}

std::optional<float> BlockBox::inlineBlockBaseline() const
{
    for(auto child = lastBoxFrame(); child; child = child->prevBoxFrame()) {
        if(child->isFloatingOrPositioned() || child->isTableBox())
            continue;
        if(auto baseline = child->inlineBlockBaseline()) {
            return baseline.value() + child->y();
        }
    }

    return std::nullopt;
}

void BlockBox::paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(!child->isFloating() && !child->hasLayer() && !child->hasColumnSpanBox()) {
            child->paint(info, offset, phase);
        }
    }
}

void BlockBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    auto overflowRect = visualOverflowRect();
    overflowRect.move(offset + location());
    if(!overflowRect.intersects(info.rect())) {
        return;
    }

    Point adjustedOffset(offset + location());
    if(phase == PaintPhase::Decorations && style()->visibility() == Visibility::Visible)
        paintDecorations(info, adjustedOffset);
    paintContents(info, adjustedOffset, phase);
    if(phase == PaintPhase::Outlines && style()->visibility() == Visibility::Visible) {
        paintOutlines(info, adjustedOffset);
    }
}

BlockFlowBox::BlockFlowBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
{
    setChildrenInline(true);
}

BlockFlowBox::~BlockFlowBox() = default;

bool BlockFlowBox::avoidsFloats() const
{
    return isInline() || isFloating() || isPositioned() || isOverflowHidden()
        || hasColumnFlowBox() || hasColumnSpanBox() || isRootBox() || isFlexItem();
}

void BlockFlowBox::addChild(Box* newChild)
{
    if(isChildrenInline() && !newChild->isInline() && !newChild->isFloatingOrPositioned()) {
        for(auto child = firstChild(); child; child = child->nextSibling()) {
            if(child->isFloatingOrPositioned())
                continue;
            auto newBlock = createAnonymousBlock(style());
            moveChildrenTo(newBlock);
            appendChild(newBlock);
            break;
        }

        setChildrenInline(false);
    } else if(!isChildrenInline() && (newChild->isInline() || newChild->isFloatingOrPositioned())) {
        auto lastBlock = lastChild();
        if(lastBlock && lastBlock->isAnonymousBlock()) {
            lastBlock->addChild(newChild);
            return;
        }

        if(newChild->isInline()) {
            auto newBlock = createAnonymousBlock(style());
            appendChild(newBlock);

            auto child = newBlock->prevSibling();
            while(child && child->isFloatingOrPositioned()) {
                auto prevChild = child->prevSibling();
                removeChild(child);
                newBlock->insertChild(child, newBlock->firstChild());
                child = prevChild;
            }

            newBlock->addChild(newChild);
            return;
        }
    }

    BlockBox::addChild(newChild);
}

void BlockFlowBox::updateOverflowRect()
{
    BlockBox::updateOverflowRect();
    if(m_floatingBoxes) {
        for(const auto& item : *m_floatingBoxes) {
            auto child = item.box();
            if(!item.isIntruding()) {
                addOverflowRect(child, item.x() + child->marginLeft(), item.y() + child->marginTop());
            }
        }
    }

    if(isChildrenInline()) {
        m_lineLayout->updateOverflowRect();
        return;
    }

    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(!child->isFloatingOrPositioned() && !child->hasColumnSpanBox()) {
            addOverflowRect(child, child->x(), child->y());
        }
    }
}

void BlockFlowBox::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    if(isChildrenInline()) {
        m_lineLayout->computeIntrinsicWidths(minWidth, maxWidth);
        return;
    }

    float floatLeftWidth = 0;
    float floatRightWidth = 0;
    const auto nowrap = style()->whiteSpace() == WhiteSpace::Nowrap;
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned() || child->hasColumnSpanBox())
            continue;
        auto childStyle = child->style();
        if(child->isFloating() || child->avoidsFloats()) {
            auto floatWidth = floatLeftWidth + floatRightWidth;
            if(childStyle->isClearLeft()) {
                maxWidth = std::max(floatWidth, maxWidth);
                floatLeftWidth = 0;
            }

            if(childStyle->isClearRight()) {
                maxWidth = std::max(floatWidth, maxWidth);
                floatRightWidth = 0;
            }
        }

        auto marginLeftLength = childStyle->marginLeft();
        auto marginRightLength = childStyle->marginRight();

        float marginLeft = 0;
        float marginRight = 0;
        if(marginLeftLength.isFixed())
            marginLeft = marginLeftLength.value();
        if(marginRightLength.isFixed()) {
            marginRight = marginRightLength.value();
        }

        auto childMinWidth = child->minPreferredWidth();
        auto childMaxWidth = child->maxPreferredWidth();

        auto marginWidth = marginLeft + marginRight;
        auto width = childMinWidth + marginWidth;

        minWidth = std::max(width, minWidth);
        if(nowrap && !child->isTableBox())
            maxWidth = std::max(width, maxWidth);
        width = childMaxWidth + marginWidth;
        if(child->isFloating()) {
            if(childStyle->floating() == Float::Left) {
                floatLeftWidth += width;
            } else {
                floatRightWidth += width;
            }
        } else {
            if(child->avoidsFloats()) {
                if(marginLeft > 0) {
                    marginLeft = std::max(floatLeftWidth, marginLeft);
                } else {
                    marginLeft += floatLeftWidth;
                }

                if(marginRight > 0)
                    marginRight = std::max(floatRightWidth, marginRight);
                else
                    marginRight += floatRightWidth;
                width = std::max(childMaxWidth + marginLeft + marginRight, floatLeftWidth + floatRightWidth);
            } else {
                maxWidth = std::max(maxWidth, floatLeftWidth + floatRightWidth);
            }

            maxWidth = std::max(width, maxWidth);
            floatLeftWidth = 0;
            floatRightWidth = 0;
        }
    }

    minWidth = std::max(0.f, minWidth);
    maxWidth = std::max(0.f, maxWidth);

    maxWidth = std::max(maxWidth, floatLeftWidth + floatRightWidth);
    maxWidth = std::max(maxWidth, minWidth);
}

std::optional<float> BlockFlowBox::firstLineBaseline() const
{
    if(!isChildrenInline())
        return BlockBox::firstLineBaseline();
    const auto& lines = m_lineLayout->lines();
    if(lines.empty())
        return std::nullopt;
    const auto& firstLine = lines.front();
    return firstLine->y() + style()->fontAscent();
}

std::optional<float> BlockFlowBox::lastLineBaseline() const
{
    if(!isChildrenInline())
        return BlockBox::lastLineBaseline();
    const auto& lines = m_lineLayout->lines();
    if(lines.empty())
        return std::nullopt;
    const auto& lastLine = lines.back();
    return lastLine->y() + style()->fontAscent();
}

std::optional<float> BlockFlowBox::inlineBlockBaseline() const
{
    if(!isChildrenInline())
        return BlockBox::inlineBlockBaseline();
    return lastLineBaseline();
}

void BlockFlowBox::collectIntrudingFloats()
{
    if(m_floatingBoxes)
        m_floatingBoxes->clear();
    if(isFloating() || isPositioned() || avoidsFloats()) {
        return;
    }

    auto parentBlock = to<BlockFlowBox>(parentBox());
    if(parentBlock == nullptr)
        return;
    bool parentHasFloats = false;
    BlockFlowBox* prevBlock = nullptr;
    for(auto sibling = prevSibling(); sibling; sibling = sibling->prevSibling()) {
        auto siblingBlock = to<BlockFlowBox>(sibling);
        if(siblingBlock && !siblingBlock->avoidsFloats()) {
            prevBlock = siblingBlock;
            break;
        }

        if(sibling->isFloating()) {
            parentHasFloats = true;
        }
    }

    if(parentHasFloats || (!prevBlock && parentBlock->floatBottom() > y()))
        addIntrudingFloats(parentBlock, parentBlock->leftOffsetForContent(), y());
    if(prevBlock) {
        auto offsetY = y() - prevBlock->y();
        if(prevBlock->floatBottom() > offsetY) {
            addIntrudingFloats(prevBlock, 0, offsetY);
        }
    }
}

void BlockFlowBox::collectOverhangingFloats()
{
    if(isChildrenInline())
        return;
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        if(child->isFloatingOrPositioned())
            continue;
        if(auto block = to<BlockFlowBox>(child)) {
            if(block->floatBottom() + block->y() > height()) {
                addOverhangingFloats(block);
            }
        }
    }
}

void BlockFlowBox::addIntrudingFloats(BlockFlowBox* prevBlock, float offsetX, float offsetY)
{
    if(!prevBlock->containsFloats())
        return;
    for(const auto& item : *prevBlock->floatingBoxes()) {
        if(item.bottom() > offsetY && !containsFloat(item.box())) {
            auto leftOffset = offsetX + marginLeft();
            if(prevBlock != parentBox())
                leftOffset -= prevBlock->marginLeft();
            FloatingBox floatingBox(item.box());
            floatingBox.setX(item.x() - leftOffset);
            floatingBox.setY(item.y() - offsetY);
            floatingBox.setWidth(item.width());
            floatingBox.setHeight(item.height());
            floatingBox.setIsIntruding(true);
            floatingBox.setIsPlaced(true);
            if(m_floatingBoxes == nullptr)
                m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
            m_floatingBoxes->push_back(floatingBox);
        }
    }
}

void BlockFlowBox::addOverhangingFloats(BlockFlowBox* childBlock)
{
    if(!childBlock->containsFloats() || childBlock->avoidsFloats())
        return;
    for(const auto& item : *childBlock->floatingBoxes()) {
        auto floatBottom = item.bottom() + childBlock->y();
        if(floatBottom > height() && !containsFloat(item.box())) {
            FloatingBox floatingBox(item.box());
            floatingBox.setX(item.x() + childBlock->x());
            floatingBox.setY(item.y() + childBlock->y());
            floatingBox.setWidth(item.width());
            floatingBox.setHeight(item.height());
            floatingBox.setIsIntruding(true);
            floatingBox.setIsPlaced(true);
            if(m_floatingBoxes == nullptr)
                m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
            m_floatingBoxes->push_back(floatingBox);
        }
    }
}

void BlockFlowBox::positionFloatingBox(FloatingBox& floatingBox, FragmentBuilder* fragmentainer, float top)
{
    auto child = floatingBox.box();
    auto leftOffset = leftOffsetForContent();
    auto rightOffset = rightOffsetForContent();
    auto floatWidth = std::min(rightOffset - leftOffset, child->marginBoxWidth());

    float floatLeft = 0;
    float floatTop = top;
    if(child->style()->floating() == Float::Left) {
        float heightRemainingLeft = 1;
        float heightRemainingRight = 1;
        floatLeft = leftOffsetForFloat(floatTop, floatTop, leftOffset, &heightRemainingLeft);
        while(rightOffsetForFloat(floatTop, floatTop, rightOffset, &heightRemainingRight) - floatLeft < floatWidth) {
            floatTop += std::min(heightRemainingLeft, heightRemainingRight);
            floatLeft = leftOffsetForFloat(floatTop, floatTop, leftOffset, &heightRemainingLeft);
        }

        floatLeft = std::max(0.f, floatLeft);
    } else {
        float heightRemainingLeft = 1;
        float heightRemainingRight = 1;
        floatLeft = rightOffsetForFloat(floatTop, floatTop, rightOffset, &heightRemainingRight);
        while(floatLeft - leftOffsetForFloat(floatTop, floatTop, leftOffset, &heightRemainingLeft) < floatWidth) {
            floatTop += std::min(heightRemainingLeft, heightRemainingRight);
            floatLeft = rightOffsetForFloat(floatTop, floatTop, rightOffset, &heightRemainingRight);
        }

        floatLeft -= child->marginBoxWidth();
    }

    if(fragmentainer) {
        floatTop = fragmentainer->applyFragmentBreakInside(child, floatTop);
        if(!isNearlyEqual(top, floatTop)) {
            auto newTop = floatTop + child->marginTop();
            fragmentainer->enterFragment(newTop);
            child->layout(fragmentainer);
            fragmentainer->leaveFragment(newTop);
        }
    }

    child->setX(floatLeft + child->marginLeft());
    child->setY(floatTop + child->marginTop());

    floatingBox.setX(floatLeft);
    floatingBox.setY(floatTop);
    floatingBox.setWidth(child->marginBoxWidth());
    floatingBox.setHeight(child->marginBoxHeight());
    floatingBox.setIsPlaced(true);
}

void BlockFlowBox::positionNewFloats(FragmentBuilder* fragmentainer)
{
    if(m_floatingBoxes == nullptr)
        return;
    auto floatTop = height();
    for(auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced()) {
            floatTop = std::max(floatTop, floatingBox.y());
            continue;
        }

        auto child = floatingBox.box();
        if(child->style()->isClearLeft())
            floatTop = std::max(floatTop, leftFloatBottom());
        if(child->style()->isClearRight()) {
            floatTop = std::max(floatTop, rightFloatBottom());
        }

        auto estimatedTop = floatTop + child->computeMarginTop();
        if(fragmentainer)
            fragmentainer->enterFragment(estimatedTop);
        child->layout(fragmentainer);
        if(fragmentainer) {
            fragmentainer->leaveFragment(estimatedTop);
        }

        positionFloatingBox(floatingBox, fragmentainer, floatTop);
    }
}

FloatingBox& BlockFlowBox::insertFloatingBox(BoxFrame* box)
{
    if(m_floatingBoxes) {
        for(auto& floatingBox : *m_floatingBoxes) {
            if(box == floatingBox.box()) {
                return floatingBox;
            }
        }
    }

    if(m_floatingBoxes == nullptr)
        m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
    m_floatingBoxes->emplace_back(box);
    return m_floatingBoxes->back();
}

bool BlockFlowBox::containsFloat(Box* box) const
{
    if(m_floatingBoxes == nullptr)
        return false;
    for(const auto& floatingBox : *m_floatingBoxes) {
        if(box == floatingBox.box()) {
            return true;
        }
    }

    return false;
}

float BlockFlowBox::leftFloatBottom() const
{
    if(m_floatingBoxes == nullptr)
        return 0;
    float bottom = 0;
    for(const auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced() && floatingBox.type() == Float::Left) {
            bottom = std::max(bottom, floatingBox.bottom());
        }
    }

    return bottom;
}

float BlockFlowBox::rightFloatBottom() const
{
    if(m_floatingBoxes == nullptr)
        return 0;
    float bottom = 0;
    for(const auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced() && floatingBox.type() == Float::Right) {
            bottom = std::max(bottom, floatingBox.bottom());
        }
    }

    return bottom;
}

float BlockFlowBox::floatBottom() const
{
    if(m_floatingBoxes == nullptr)
        return 0;
    float bottom = 0;
    for(const auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced()) {
            bottom = std::max(bottom, floatingBox.bottom());
        }
    }

    return bottom;
}

float BlockFlowBox::nextFloatBottom(float y) const
{
    if(m_floatingBoxes == nullptr)
        return 0;
    std::optional<float> bottom;
    for(const auto& floatingBox : *m_floatingBoxes) {
        assert(floatingBox.isPlaced());
        auto floatBottom = floatingBox.bottom();
        if(floatBottom > y) {
            bottom = std::min(floatBottom, bottom.value_or(floatBottom));
        }
    }

    return bottom.value_or(0.f);
}

constexpr bool rangesIntersect(float objectTop, float objectBottom, float floatTop, float floatBottom)
{
    if(objectTop >= floatBottom || objectBottom < floatTop)
        return false;
    if(objectTop >= floatTop)
        return true;
    if(objectTop < floatTop && objectBottom > floatBottom)
        return true;
    if(objectBottom > objectTop && objectBottom > floatTop && objectBottom <= floatBottom)
        return true;
    return false;
}

float BlockFlowBox::leftOffsetForFloat(float top, float bottom, float offset, float* heightRemaining) const
{
    if(heightRemaining) *heightRemaining = 1;
    if(m_floatingBoxes) {
        for(const auto& item : *m_floatingBoxes) {
            if(item.type() == Float::Left && item.isPlaced()
                && item.right() > offset && rangesIntersect(top, bottom, item.y(), item.bottom())) {
                if(heightRemaining) *heightRemaining = item.bottom() - top;
                offset = std::max(offset, item.right());
            }
        }
    }

    return offset;
}

float BlockFlowBox::rightOffsetForFloat(float top, float bottom, float offset, float* heightRemaining) const
{
    if(heightRemaining) *heightRemaining = 1;
    if(m_floatingBoxes) {
        for(const auto& item : *m_floatingBoxes) {
            if(item.type() == Float::Right && item.isPlaced()
                && item.x() < offset && rangesIntersect(top, bottom, item.y(), item.bottom())) {
                if(heightRemaining) *heightRemaining = item.bottom() - top;
                offset = std::min(offset, item.x());
            }
        }
    }

    return offset;
}

float BlockFlowBox::leftOffsetForLine(float y, float height, bool indent) const
{
    auto offset = leftOffsetForFloat(y, y + height, leftOffsetForContent());
    if(indent && style()->isLeftToRightDirection()) {
        float availableWidth = 0;
        auto textIndentLength = style()->textIndent();
        if(textIndentLength.isPercent())
            availableWidth = containingBlockWidthForContent();
        offset += textIndentLength.calcMin(availableWidth);
    }

    return offset;
}

float BlockFlowBox::rightOffsetForLine(float y, float height, bool indent) const
{
    auto offset = rightOffsetForFloat(y, y + height, rightOffsetForContent());
    if(indent && style()->isRightToLeftDirection()) {
        float availableWidth = 0;
        auto textIndentLength = style()->textIndent();
        if(textIndentLength.isPercent())
            availableWidth = containingBlockWidthForContent();
        offset -= textIndentLength.calcMin(availableWidth);
    }

    return offset;
}

float BlockFlowBox::lineOffsetForAlignment(float remainingWidth) const
{
    auto textAlign = style()->textAlign();
    auto direction = style()->direction();
    if(textAlign == TextAlign::Start || textAlign == TextAlign::Justify)
        textAlign = direction == Direction::Ltr ? TextAlign::Left : TextAlign::Right;
    else if(textAlign == TextAlign::End) {
        textAlign = direction == Direction::Ltr ? TextAlign::Right : TextAlign::Left;
    }

    switch(textAlign) {
    case TextAlign::Left:
        if(direction == Direction::Ltr)
            return 0.f;
        if(remainingWidth < 0.f)
            return remainingWidth;
        return 0.f;
    case TextAlign::Right:
        if(direction == Direction::Rtl)
            return remainingWidth;
        if(remainingWidth > 0.f)
            return remainingWidth;
        return 0.f;
    case TextAlign::Center:
        if(remainingWidth > 0.f)
            return remainingWidth / 2.f;
        if(direction == Direction::Rtl)
            return remainingWidth;
        return 0.f;
    default:
        assert(false);
    }

    return 0.f;
}

float BlockFlowBox::startAlignedOffsetForLine(float y, float height, bool indent) const
{
    auto leftOffset = leftOffsetForLine(y, height, indent);
    auto rightOffset = rightOffsetForLine(y, height, indent);
    if(style()->isLeftToRightDirection())
        return leftOffset + lineOffsetForAlignment(rightOffset - leftOffset);
    return width() - leftOffset - lineOffsetForAlignment(rightOffset - leftOffset);
}

class MarginInfo {
public:
    MarginInfo(const BlockFlowBox* block, float top, float bottom);

    bool atTopOfBlock() const { return m_atTopOfBlock; }
    bool atBottomOfBlock() const { return m_atBottomOfBlock; }

    bool canCollapseWithChildren() const { return m_canCollapseWithChildren; }
    bool canCollapseMarginTopWithChildren() const { return m_canCollapseMarginTopWithChildren; }
    bool canCollapseMarginBottomWithChildren() const { return m_canCollapseMarginBottomWithChildren; }

    bool canCollapseWithMarginTop() const { return m_atTopOfBlock && m_canCollapseMarginTopWithChildren; }
    bool canCollapseWithMarginBottom() const { return m_atBottomOfBlock && m_canCollapseMarginBottomWithChildren; }

    float positiveMargin() const { return m_positiveMargin; }
    float negativeMargin() const { return m_negativeMargin; }
    float margin() const { return m_positiveMargin - m_negativeMargin; }

    void setAtTopOfBlock(bool value) { m_atTopOfBlock = value; }
    void setAtBottomOfBlock(bool value) { m_atBottomOfBlock = value; }

    void setPositiveMargin(float value) { m_positiveMargin = value; }
    void setNegativeMargin(float value) { m_negativeMargin = value; }

    void setPositiveMarginIfLarger(float value) { if(value > m_positiveMargin) { m_positiveMargin = value; } }
    void setNegativeMarginIfLarger(float value) { if(value > m_negativeMargin) { m_negativeMargin = value; } }

    void clearMargin() { m_positiveMargin = 0.f; m_negativeMargin = 0.f; }

private:
    bool m_atTopOfBlock;
    bool m_atBottomOfBlock;

    bool m_canCollapseWithChildren;
    bool m_canCollapseMarginTopWithChildren;
    bool m_canCollapseMarginBottomWithChildren;

    float m_positiveMargin;
    float m_negativeMargin;
};

inline MarginInfo::MarginInfo(const BlockFlowBox* block, float top, float bottom)
    : m_atTopOfBlock(true)
    , m_atBottomOfBlock(false)
{
    m_canCollapseWithChildren = !block->avoidsFloats();
    m_canCollapseMarginTopWithChildren = m_canCollapseWithChildren && !top;
    m_canCollapseMarginBottomWithChildren = m_canCollapseWithChildren && !bottom && block->style()->height().isAuto();

    m_positiveMargin = m_canCollapseMarginTopWithChildren ? block->maxPositiveMarginTop() : 0.f;
    m_negativeMargin = m_canCollapseMarginTopWithChildren ? block->maxNegativeMarginTop() : 0.f;
}

void BlockFlowBox::adjustFloatingBox(FragmentBuilder* fragmentainer, const MarginInfo& marginInfo)
{
    float marginOffset = 0;
    if(!marginInfo.canCollapseWithMarginTop())
        marginOffset = marginInfo.margin();
    setHeight(height() + marginOffset);
    positionNewFloats(fragmentainer);
    setHeight(height() - marginOffset);
}

void BlockFlowBox::adjustPositionedBox(BoxFrame* child, const MarginInfo& marginInfo)
{
    auto staticTop = height();
    if(!marginInfo.canCollapseWithMarginTop())
        staticTop += marginInfo.margin();
    auto childLayer = child->layer();
    childLayer->setStaticTop(staticTop);
    if(child->style()->isOriginalDisplayInlineType()) {
        childLayer->setStaticLeft(startAlignedOffsetForLine(height()));
    } else {
        childLayer->setStaticLeft(startOffsetForContent());
    }
}

void BlockFlowBox::handleBottomOfBlock(float top, float bottom, MarginInfo& marginInfo)
{
    marginInfo.setAtBottomOfBlock(true);
    if(!marginInfo.canCollapseWithMarginBottom() && !marginInfo.canCollapseWithMarginTop())
        setHeight(height() + marginInfo.margin());
    setHeight(bottom + height());
    setHeight(std::max(top + bottom, height()));
    if(marginInfo.canCollapseWithMarginBottom() && !marginInfo.canCollapseWithMarginTop()) {
        m_maxPositiveMarginBottom = std::max(m_maxPositiveMarginBottom, marginInfo.positiveMargin());
        m_maxNegativeMarginBottom = std::max(m_maxNegativeMarginBottom, marginInfo.negativeMargin());
    }
}

float BlockFlowBox::collapseMargins(BoxFrame* child, FragmentBuilder* fragmentainer, MarginInfo& marginInfo)
{
    auto posTop = child->maxMarginTop(true);
    auto negTop = child->maxMarginTop(false);
    if(child->isSelfCollapsingBlock()) {
        posTop = std::max(posTop, child->maxMarginBottom(true));
        negTop = std::max(negTop, child->maxMarginBottom(false));
    }

    if(marginInfo.canCollapseWithMarginTop()) {
        m_maxPositiveMarginTop = std::max(posTop, m_maxPositiveMarginTop);
        m_maxNegativeMarginTop = std::max(negTop, m_maxNegativeMarginTop);
    }

    auto beforeCollapseTop = height();
    auto top = beforeCollapseTop;
    if(child->isSelfCollapsingBlock()) {
        auto collapsedPosTop = std::max(marginInfo.positiveMargin(), child->maxMarginTop(true));
        auto collapsedNegTop = std::max(marginInfo.negativeMargin(), child->maxMarginTop(false));
        if(!marginInfo.canCollapseWithMarginTop()) {
            top = height() + collapsedPosTop - collapsedNegTop;
        }

        marginInfo.setPositiveMargin(collapsedPosTop);
        marginInfo.setNegativeMargin(collapsedNegTop);

        marginInfo.setPositiveMarginIfLarger(child->maxMarginBottom(true));
        marginInfo.setNegativeMarginIfLarger(child->maxMarginBottom(false));
    } else {
        if(!marginInfo.atTopOfBlock() || !marginInfo.canCollapseMarginTopWithChildren()) {
            setHeight(height() + std::max(posTop, marginInfo.positiveMargin()) - std::max(negTop, marginInfo.negativeMargin()));
            top = height();
        }

        marginInfo.setPositiveMargin(child->maxMarginBottom(true));
        marginInfo.setNegativeMargin(child->maxMarginBottom(false));
    }

    if(fragmentainer &&  top > beforeCollapseTop) {
        auto fragmentHeight = fragmentainer->fragmentHeightForOffset(beforeCollapseTop);
        if(fragmentHeight > 0.f) {
            auto newTop = std::min(top, beforeCollapseTop + fragmentainer->fragmentRemainingHeightForOffset(beforeCollapseTop, AssociateWithLatterFragment));
            setHeight(height() + (newTop - top));
            top = newTop;
        }
    }

    return top;
}

void BlockFlowBox::updateMaxMargins()
{
    if(isTableCellBox()) {
        m_maxPositiveMarginTop = m_maxNegativeMarginTop = 0.f;
        m_maxPositiveMarginBottom = m_maxNegativeMarginBottom = 0.f;
        return;
    }

    m_maxPositiveMarginTop = std::max(0.f, marginTop());
    m_maxNegativeMarginTop = std::max(0.f, -marginTop());
    m_maxPositiveMarginBottom = std::max(0.f, marginBottom());
    m_maxNegativeMarginBottom = std::max(0.f, -marginBottom());
}

bool BlockFlowBox::isSelfCollapsingBlock() const
{
    if(height() || avoidsFloats())
        return false;
    if(isChildrenInline())
        return m_lineLayout->isBlockLevel();
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isFloatingOrPositioned())
            continue;
        if(!child->isSelfCollapsingBlock()) {
            return false;
        }
    }

    return true;
}

float BlockFlowBox::maxMarginTop(bool positive) const
{
    return positive ? m_maxPositiveMarginTop : m_maxNegativeMarginTop;
}

float BlockFlowBox::maxMarginBottom(bool positive) const
{
    return positive ? m_maxPositiveMarginBottom : m_maxNegativeMarginBottom;
}

float BlockFlowBox::getClearDelta(BoxFrame* child, float y) const
{
    if(!containsFloats())
        return 0;
    float delta = 0;
    switch(child->style()->clear()) {
    case Clear::Left:
        delta = std::max(0.f, leftFloatBottom() - y);
        break;
    case Clear::Right:
        delta = std::max(0.f, rightFloatBottom() - y);
        break;
    case Clear::Both:
        delta = std::max(0.f, floatBottom() - y);
        break;
    case Clear::None:
        break;
    }

    if(!delta && child->avoidsFloats()) {
        auto top = y;
        while(true) {
            auto availableWidth = availableWidthForLine(top);
            if(isNearlyEqual(availableWidth, availableWidthForContent()))
                return top - y;
            auto childX = child->x();
            auto childY = child->y();
            auto childWidth = child->width();
            auto childMarginLeft = child->marginLeft();
            auto childMarginRight = child->marginRight();

            child->setY(top);
            child->computeWidth(childX, childWidth, childMarginLeft, childMarginRight);
            child->setY(childY);
            if(childWidth <= availableWidth)
                return top - y;
            top = nextFloatBottom(top);
        }
    }

    return delta;
}

void BlockFlowBox::estimateMarginTop(BoxFrame* child, float& positiveMarginTop, float& negativeMarginTop) const
{
    positiveMarginTop = std::max(positiveMarginTop, child->marginTop());
    negativeMarginTop = std::max(negativeMarginTop, -child->marginTop());

    auto childBlock = to<BlockFlowBox>(child);
    if(childBlock == nullptr || childBlock->isChildrenInline())
        return;
    MarginInfo childMarginInfo(childBlock, childBlock->borderAndPaddingTop(), childBlock->borderAndPaddingBottom());
    if(!childMarginInfo.canCollapseMarginTopWithChildren()) {
        return;
    }

    auto grandChild = childBlock->firstBoxFrame();
    for(; grandChild; grandChild = grandChild->nextBoxFrame()) {
        if(!grandChild->isFloatingOrPositioned() && !grandChild->hasColumnSpanBox()) {
            break;
        }
    }

    if(grandChild && grandChild->style()->clear() == Clear::None) {
        grandChild->updateVerticalMargins();
        estimateMarginTop(grandChild, positiveMarginTop, negativeMarginTop);
    }
}

float BlockFlowBox::estimateVerticalPosition(BoxFrame* child, FragmentBuilder* fragmentainer, const MarginInfo& marginInfo) const
{
    auto estimatedTop = height();
    if(!marginInfo.canCollapseWithMarginTop()) {
        float positiveMarginTop = child->maxMarginTop(true);
        float negativeMarginTop = child->maxMarginTop(false);
        if(positiveMarginTop < 0.f && negativeMarginTop < 0.f)
            estimateMarginTop(child, positiveMarginTop, negativeMarginTop);
        estimatedTop += std::max(positiveMarginTop, marginInfo.positiveMargin()) - std::max(negativeMarginTop, marginInfo.negativeMargin());
    }

    if(fragmentainer && estimatedTop > height()) {
        auto fragmentHeight = fragmentainer->fragmentHeightForOffset(height());
        if(fragmentHeight > 0.f) {
            estimatedTop = std::min(estimatedTop, height() + fragmentainer->fragmentRemainingHeightForOffset(height(), AssociateWithLatterFragment));
        }
    }

    estimatedTop += getClearDelta(child, estimatedTop);
    if(fragmentainer) {
        estimatedTop = fragmentainer->applyFragmentBreakBefore(child, estimatedTop);
        estimatedTop = fragmentainer->applyFragmentBreakInside(child, estimatedTop);
    }

    return estimatedTop;
}

float BlockFlowBox::determineVerticalPosition(BoxFrame* child, FragmentBuilder* fragmentainer, MarginInfo& marginInfo)
{
    auto posTop = m_maxPositiveMarginTop;
    auto negTop = m_maxNegativeMarginTop;

    auto offset = collapseMargins(child, fragmentainer, marginInfo);
    auto clearDelta = getClearDelta(child, offset);
    if(clearDelta == 0.f) {
        return offset;
    }

    if(child->isSelfCollapsingBlock()) {
        marginInfo.setPositiveMargin(std::max(child->maxMarginTop(true), child->maxMarginBottom(true)));
        marginInfo.setNegativeMargin(std::max(child->maxMarginTop(false), child->maxMarginBottom(false)));

        setHeight(child->y() + child->maxMarginTop(false));
    } else {
        setHeight(clearDelta + height());
    }

    if(marginInfo.atTopOfBlock()) {
        m_maxPositiveMarginTop = posTop;
        m_maxNegativeMarginTop = negTop;
        marginInfo.setAtTopOfBlock(false);
    }

    return offset + clearDelta;
}

void BlockFlowBox::determineHorizontalPosition(BoxFrame* child) const
{
    if(style()->isLeftToRightDirection()) {
        auto offsetX = borderLeft() + paddingLeft() + child->marginLeft();
        if(containsFloats() && child->avoidsFloats()) {
            auto startOffset = startOffsetForLine(child->y());
            if(child->style()->marginLeft().isAuto())
                offsetX = std::max(offsetX, startOffset + child->marginLeft());
            else if(startOffset > borderAndPaddingLeft()) {
                offsetX = std::max(offsetX, startOffset);
            }
        }

        child->setX(offsetX);
    } else {
        auto offsetX = borderRight() + paddingRight() + child->marginRight();
        if(containsFloats() && child->avoidsFloats()) {
            auto startOffset = startOffsetForLine(child->y());
            if(child->style()->marginRight().isAuto())
                offsetX = std::max(offsetX, startOffset + child->marginRight());
            else if(startOffset > borderAndPaddingRight()) {
                offsetX = std::max(offsetX, startOffset);
            }
        }

        child->setX(width() - offsetX - child->width());
    }
}

float BlockFlowBox::adjustBlockChildInFragmentFlow(BoxFrame* child, FragmentBuilder* fragmentainer, float top)
{
    auto newTop = fragmentainer->applyFragmentBreakBefore(child, top);
    auto adjustedTop = fragmentainer->applyFragmentBreakInside(child, newTop);

    auto childHeight = child->height();
    if(adjustedTop > newTop) {
        auto delta = adjustedTop - newTop;
        fragmentainer->setFragmentBreak(newTop, childHeight - delta);
        newTop += delta;
    } else {
        auto fragmentHeight = fragmentainer->fragmentHeightForOffset(newTop);
        if(fragmentHeight > 0.f) {
            auto remainingHeight = fragmentainer->fragmentRemainingHeightForOffset(newTop, AssociateWithLatterFragment);
            if(remainingHeight < childHeight) {
                fragmentainer->setFragmentBreak(newTop, childHeight - remainingHeight);
            } else if(isNearlyEqual(fragmentHeight, remainingHeight) && !isNearlyZero(top + fragmentainer->fragmentOffset())) {
                fragmentainer->setFragmentBreak(newTop, childHeight);
            }
        }
    }

    setHeight(height() + (newTop - top));
    return newTop;
}

void BlockFlowBox::layoutBlockChild(BoxFrame* child, FragmentBuilder* fragmentainer, MarginInfo& marginInfo)
{
    child->updateVerticalMargins();

    auto estimatedTop = estimateVerticalPosition(child, fragmentainer, marginInfo);
    if(fragmentainer)
        fragmentainer->enterFragment(estimatedTop);
    child->setY(estimatedTop);
    child->layout(fragmentainer);
    if(fragmentainer) {
        fragmentainer->leaveFragment(estimatedTop);
    }

    auto newTop = determineVerticalPosition(child, fragmentainer, marginInfo);
    if(fragmentainer) {
        newTop = adjustBlockChildInFragmentFlow(child, fragmentainer, newTop);
    }

    if(!isNearlyEqual(newTop, estimatedTop)) {
        if(fragmentainer)
            fragmentainer->enterFragment(newTop);
        child->setY(newTop);
        child->layout(fragmentainer);
        if(fragmentainer) {
            fragmentainer->leaveFragment(newTop);
        }
    }

    if(marginInfo.atTopOfBlock() && !child->isSelfCollapsingBlock()) {
        marginInfo.setAtTopOfBlock(false);
    }

    determineHorizontalPosition(child);
    if(auto spanner = to<MultiColumnSpanBox>(child)) {
        spanner->box()->setX(child->x());
        spanner->box()->setY(child->y());
    }

    setHeight(height() + child->height());
    if(fragmentainer) {
        auto newHeight = fragmentainer->applyFragmentBreakAfter(child, height());
        if(newHeight > height())
            marginInfo.clearMargin();
        setHeight(newHeight);
    }

    if(auto childBlock = to<BlockFlowBox>(child)) {
        addOverhangingFloats(childBlock);
    }
}

void BlockFlowBox::layoutBlockChildren(FragmentBuilder* fragmentainer)
{
    auto top = borderTop() + paddingTop();
    auto bottom = borderBottom() + paddingBottom();

    MarginInfo marginInfo(this, top, bottom);
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned()) {
            child->containingBlock()->insertPositonedBox(child);
            adjustPositionedBox(child, marginInfo);
        } else if(child->isFloating()) {
            insertFloatingBox(child);
            adjustFloatingBox(fragmentainer, marginInfo);
        } else if(child->hasColumnSpanBox()) {
            setHeight(height() + marginInfo.margin());
            child->columnSpanBox()->columnFlowBox()->skipColumnSpanBox(child, height());
            marginInfo.clearMargin();
        } else if(child->isMultiColumnFlowBox()) {
            assert(child == m_columnFlowBox);
            child->setY(top);
            child->layout(nullptr);
            determineHorizontalPosition(child);
        } else {
            layoutBlockChild(child, fragmentainer, marginInfo);
        }
    }

    handleBottomOfBlock(top, bottom, marginInfo);
}

void BlockFlowBox::layout(FragmentBuilder* fragmentainer)
{
    if(isChildrenInline()) {
        m_lineLayout->updateWidth();
    } else {
        updateWidth();
    }

    updateMaxMargins();
    collectIntrudingFloats();

    setHeight(borderAndPaddingTop());
    if(isChildrenInline()) {
        m_lineLayout->layout(fragmentainer);
    } else {
        layoutBlockChildren(fragmentainer);
    }

    if(avoidsFloats() && floatBottom() > (height() - borderAndPaddingBottom()))
        setHeight(floatBottom() + borderAndPaddingBottom());
    updateHeight();
    collectOverhangingFloats();
    layoutPositionedBoxes();
    updateOverflowRect();
}

void BlockFlowBox::build()
{
    auto child = firstChild();
    while(child && child->isFloatingOrPositioned())
        child = child->nextSibling();
    if(child == nullptr)
        setChildrenInline(false);
    if(child == nullptr)
        setChildrenInline(false);
    if(child && style()->hasColumns()) {
        auto columnFlowBox = MultiColumnFlowBox::create(style());
        moveChildrenTo(columnFlowBox);
        appendChild(columnFlowBox);
        columnFlowBox->setChildrenInline(isChildrenInline());
        setChildrenInline(false);
        setHasColumnFlowBox(true);
        m_columnFlowBox = columnFlowBox;
    }

    if(isChildrenInline()) {
        m_lineLayout = LineLayout::create(this);
        m_lineLayout->build();
    }

    BlockBox::build();
}

void BlockFlowBox::paintFloats(const PaintInfo& info, const Point& offset)
{
    if(m_floatingBoxes == nullptr)
        return;
    for(const auto& item : *m_floatingBoxes) {
        auto child = item.box();
        if(!item.isIntruding() && !child->hasLayer()) {
            Point adjustedOffset = {
                offset.x + item.x() - child->x() + child->marginLeft(),
                offset.y + item.y() - child->y() + child->marginTop()
            };

            child->paint(info, adjustedOffset, PaintPhase::Decorations);
            child->paint(info, adjustedOffset, PaintPhase::Floats);
            child->paint(info, adjustedOffset, PaintPhase::Contents);
            child->paint(info, adjustedOffset, PaintPhase::Outlines);
        }
    }
}

void BlockFlowBox::paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(isChildrenInline())
        m_lineLayout->paint(info, offset, phase);
    else
        BlockBox::paintContents(info, offset, phase);
    if(phase == PaintPhase::Floats) {
        paintFloats(info, offset);
    }
}

} // namespace plutobook
