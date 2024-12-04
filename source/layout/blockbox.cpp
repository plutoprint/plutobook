#include "blockbox.h"
#include "multicolumnbox.h"
#include "linebox.h"
#include "linelayout.h"
#include "boxlayer.h"
#include "fragmentbuilder.h"

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
    if(!m_positionedBoxes)
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
            box->layout();
        }
    }
}

std::optional<float> BlockBox::availableHeight() const
{
    if(isBoxView())
        return style()->viewportHeight();
    if(hasOverrideHeight())
        return overrideHeight() - borderAndPaddingHeight();
    if(isAnonymous())
        return containingBlock()->availableHeight();
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
    auto availableWidth = container->availableWidthForLine(y(), false) - marginLeft - marginRight;
    auto marginStart = style()->isLeftToRightDirection() ? marginLeft : marginRight;
    auto marginEnd = style()->isLeftToRightDirection() ? marginRight : marginLeft;
    if(marginStart > 0) {
        auto lineStartOffset = container->startOffsetForLine(y(), false);
        auto contentStartOffset = container->startOffsetForContent();
        auto marginStartOffset = contentStartOffset + marginStart;
        if(lineStartOffset > marginStartOffset) {
            availableWidth += marginStart;
        } else {
            availableWidth += lineStartOffset - contentStartOffset;
        }
    }

    if(marginEnd > 0) {
        auto lineEndOffset = container->endOffsetForLine(y(), false);
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
        if(auto availableHeight = containingBlock()->availableHeight()) {
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
    auto containerWidth = container->availableWidthForPositioned();
    auto containerDirection = container->direction();

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
    auto containerHeight = container->availableHeightForPositioned();
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
    auto containerWidth = std::max(0.f, container->availableWidth());
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
        for(auto& item : *m_floatingBoxes) {
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
        if(!child->isFloatingOrPositioned()) {
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
        if(child->isPositioned())
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
    auto& lines = m_lineLayout->lines();
    if(lines.empty())
        return std::nullopt;
    auto& firstLine = lines.front();
    return firstLine->y() + style()->fontAscent();
}

std::optional<float> BlockFlowBox::lastLineBaseline() const
{
    if(!isChildrenInline())
        return BlockBox::lastLineBaseline();
    auto& lines = m_lineLayout->lines();
    if(lines.empty())
        return std::nullopt;
    auto& lastLine = lines.back();
    return lastLine->y() + style()->fontAscent();
}

std::optional<float> BlockFlowBox::inlineBlockBaseline() const
{
    if(!isChildrenInline())
        return BlockBox::inlineBlockBaseline();
    return lastLineBaseline();
}

void BlockFlowBox::insertFloatingBox(BoxFrame* box)
{
    if(containsFloat(box))
        return;

    box->layout();

    FloatingBox floatingBox(box);
    floatingBox.setWidth(box->width() + box->marginWidth());
    floatingBox.setHeight(box->height() + box->marginHeight());
    floatingBox.setIsIntruding(false);
    floatingBox.setIsPlaced(false);
    if(!m_floatingBoxes)
        m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
    m_floatingBoxes->push_back(floatingBox);
}

void BlockFlowBox::removeFloatingBox(BoxFrame* box)
{
    if(!m_floatingBoxes)
        return;
    for(auto it = m_floatingBoxes->begin(); it != m_floatingBoxes->end(); ++it) {
        if(box == it->box()) {
            m_floatingBoxes->erase(it);
            break;
        }
    }
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
    auto prev = prevSibling();
    auto parentHasFloats = false;
    while(prev && (prev->avoidsFloats() || !prev->isBlockFlowBox())) {
        if(prev->isFloating())
            parentHasFloats = true;
        prev = prev->prevSibling();
    }

    auto offsetX = parentBlock->leftOffsetForContent();
    auto offsetY = y();
    if(parentHasFloats)
        addIntrudingFloats(parentBlock, offsetX, offsetY);
    auto prevBlock = to<BlockFlowBox>(prev);
    if(prevBlock == nullptr) {
        prevBlock = parentBlock;
    } else {
        offsetX = 0;
        offsetY -= prevBlock->y();
    }

    if(prevBlock->floatBottom() > offsetY) {
        addIntrudingFloats(prevBlock, offsetX, offsetY);
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
    for(auto& item : *prevBlock->floatingBoxes()) {
        if(item.bottom() > offsetY && !containsFloat(item.box())) {
            FloatingBox floatingBox(item.box());
            floatingBox.setX(offsetX + marginLeft());
            if(prevBlock != parentBox())
                floatingBox.setX(floatingBox.x() - prevBlock->marginLeft());
            floatingBox.setX(item.x() - floatingBox.x());
            floatingBox.setY(item.y() - offsetY);
            floatingBox.setWidth(item.width());
            floatingBox.setHeight(item.height());
            floatingBox.setIsIntruding(true);
            floatingBox.setIsPlaced(true);
            if(!m_floatingBoxes)
                m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
            m_floatingBoxes->push_back(floatingBox);
        }
    }
}

void BlockFlowBox::addOverhangingFloats(BlockFlowBox* childBlock)
{
    if(!childBlock->containsFloats() || childBlock->avoidsFloats())
        return;
    for(auto& item : *childBlock->floatingBoxes()) {
        auto floatBottom = item.bottom() + childBlock->y();
        if(floatBottom > height() && !containsFloat(item.box())) {
            FloatingBox floatingBox(item.box());
            floatingBox.setX(item.x() + childBlock->x());
            floatingBox.setY(item.y() + childBlock->y());
            floatingBox.setWidth(item.width());
            floatingBox.setHeight(item.height());
            floatingBox.setIsIntruding(true);
            floatingBox.setIsPlaced(true);
            if(!m_floatingBoxes)
                m_floatingBoxes = std::make_unique<FloatingBoxList>(heap());
            m_floatingBoxes->push_back(floatingBox);
        }
    }
}

void BlockFlowBox::positionNewFloats()
{
    if(!containsFloats())
        return;
    auto floatTop = height();
    for(auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced()) {
            floatTop = std::max(floatTop, floatingBox.y());
            continue;
        }

        auto child = floatingBox.box();
        assert(this == child->containingBlock());
        auto childStyle = child->style();
        if(childStyle->isClearLeft())
            floatTop = std::max(floatTop, leftFloatBottom());
        if(childStyle->isClearRight()) {
            floatTop = std::max(floatTop, rightFloatBottom());
        }

        auto leftOffset = leftOffsetForContent();
        auto rightOffset = rightOffsetForContent();
        auto floatWidth = std::min(rightOffset - leftOffset, floatingBox.width());

        float floatLeft = 0;
        if(childStyle->floating() == Float::Left) {
            float heightRemainingLeft = 1;
            float heightRemainingRight = 1;
            floatLeft = leftOffsetForFloat(floatTop, leftOffset, false, &heightRemainingLeft);
            while(rightOffsetForFloat(floatTop, rightOffset, false, &heightRemainingRight) - floatLeft < floatWidth) {
                floatTop += std::min(heightRemainingLeft, heightRemainingRight);
                floatLeft = leftOffsetForFloat(floatTop, leftOffset, false, &heightRemainingLeft);
            }

            floatLeft = std::max(0.f, floatLeft);
        } else {
            float heightRemainingLeft = 1;
            float heightRemainingRight = 1;
            floatLeft = rightOffsetForFloat(floatTop, rightOffset, false, &heightRemainingRight);
            while(floatLeft - leftOffsetForFloat(floatTop, leftOffset, false, &heightRemainingLeft) < floatWidth) {
                floatTop += std::min(heightRemainingLeft, heightRemainingRight);
                floatLeft = rightOffsetForFloat(floatTop, rightOffset, false, &heightRemainingRight);
            }

            floatLeft -= floatingBox.width();
        }

        child->setX(floatLeft + child->marginLeft());
        child->setY(floatTop + child->marginTop());

        floatingBox.setX(floatLeft);
        floatingBox.setY(floatTop);
        floatingBox.setIsPlaced(true);
    }
}

bool BlockFlowBox::containsFloat(Box* box) const
{
    if(!m_floatingBoxes)
        return false;
    for(auto& floatingBox : *m_floatingBoxes) {
        if(box == floatingBox.box()) {
            return true;
        }
    }

    return false;
}

float BlockFlowBox::leftFloatBottom() const
{
    if(!m_floatingBoxes)
        return 0;
    float bottom = 0;
    for(auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced() && floatingBox.type() == Float::Left) {
            bottom = std::max(bottom, floatingBox.bottom());
        }
    }

    return bottom;
}

float BlockFlowBox::rightFloatBottom() const
{
    if(!m_floatingBoxes)
        return 0;
    float bottom = 0;
    for(auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced() && floatingBox.type() == Float::Right) {
            bottom = std::max(bottom, floatingBox.bottom());
        }
    }

    return bottom;
}

float BlockFlowBox::floatBottom() const
{
    if(!m_floatingBoxes)
        return 0;
    float bottom = 0;
    for(auto& floatingBox : *m_floatingBoxes) {
        if(floatingBox.isPlaced()) {
            bottom = std::max(bottom, floatingBox.bottom());
        }
    }

    return bottom;
}

float BlockFlowBox::nextFloatBottom(float y) const
{
    if(!m_floatingBoxes)
        return 0;
    std::optional<float> bottom;
    for(auto& floatingBox : *m_floatingBoxes) {
        assert(floatingBox.isPlaced());
        auto floatBottom = floatingBox.bottom();
        if(floatBottom > y) {
            bottom = std::min(floatBottom, bottom.value_or(floatBottom));
        }
    }

    return bottom.value_or(0.f);
}

float BlockFlowBox::leftOffsetForFloat(float y, float offset, bool indent, float* heightRemaining) const
{
    if(heightRemaining) *heightRemaining = 1;
    if(m_floatingBoxes) {
        for(auto& item : *m_floatingBoxes) {
            if(item.type() != Float::Left || !item.isPlaced())
                continue;
            if(item.y() <= y && item.bottom() > y && item.right() > offset) {
                if(heightRemaining) *heightRemaining = item.bottom() - y;
                offset = item.right();
            }
        }
    }

    if(indent && style()->isLeftToRightDirection()) {
        float availableWidth = 0;
        auto textIndentLength = style()->textIndent();
        if(textIndentLength.isPercent())
            availableWidth = containingBlock()->availableWidth();
        offset += textIndentLength.calcMin(availableWidth);
    }

    return offset;
}

float BlockFlowBox::rightOffsetForFloat(float y, float offset, bool indent, float* heightRemaining) const
{
    if(heightRemaining) *heightRemaining = 1;
    if(m_floatingBoxes) {
        for(auto& item : *m_floatingBoxes) {
            if(item.type() != Float::Right || !item.isPlaced())
                continue;
            if(item.y() <= y && item.bottom() > y && item.x() < offset) {
                if(heightRemaining) *heightRemaining = item.bottom() - y;
                offset = item.x();
            }
        }
    }

    if(indent && style()->isRightToLeftDirection()) {
        float availableWidth = 0;
        auto textIndentLength = style()->textIndent();
        if(textIndentLength.isPercent())
            availableWidth = containingBlock()->availableWidth();
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

float BlockFlowBox::startAlignedOffsetForLine(float y, bool indent) const
{
    auto leftOffset = leftOffsetForLine(y, indent);
    auto rightOffset = rightOffsetForLine(y, indent);
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

void BlockFlowBox::adjustFloatingBox(const MarginInfo& marginInfo)
{
    float marginOffset = 0;
    if(!marginInfo.canCollapseWithMarginTop())
        marginOffset = marginInfo.margin();
    setHeight(height() + marginOffset);
    positionNewFloats();
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
        childLayer->setStaticLeft(startAlignedOffsetForLine(height(), false));
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

float BlockFlowBox::collapseMargins(BoxFrame* child, MarginInfo& marginInfo)
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

    auto top = height();
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

    return top;
}

void BlockFlowBox::updateMaxMargins()
{
    if(isTableCellBox()) {
        m_maxPositiveMarginTop = 0.f;
        m_maxNegativeMarginTop = 0.f;
        m_maxPositiveMarginBottom = 0.f;
        m_maxNegativeMarginBottom = 0.f;
        return;
    }

    m_maxPositiveMarginTop = std::max(0.f, m_marginTop);
    m_maxNegativeMarginTop = std::max(0.f, -m_marginTop);
    m_maxPositiveMarginBottom = std::max(0.f, m_marginBottom);
    m_maxNegativeMarginBottom = std::max(0.f, -m_marginBottom);
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
            auto availableWidth = availableWidthForLine(top, false);
            if(availableWidth == availableWidthForContent())
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

void BlockFlowBox::clearFloats(Clear clear)
{
    positionNewFloats();
    float y = 0;
    switch(clear) {
    case Clear::Left:
        y = leftFloatBottom();
        break;
    case Clear::Right:
        y = rightFloatBottom();
        break;
    case Clear::Both:
        y = floatBottom();
        break;
    case Clear::None:
        break;
    }

    if(height() < y) {
        setHeight(y);
    }
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
        if(!grandChild->isFloatingOrPositioned()) {
            break;
        }
    }

    if(grandChild && grandChild->style()->clear() == Clear::None) {
        grandChild->updateVerticalMargins();
        estimateMarginTop(grandChild, positiveMarginTop, negativeMarginTop);
    }
}

float BlockFlowBox::estimateVerticalPosition(BoxFrame* child, MultiColumnFlowBox* column, const MarginInfo& marginInfo) const
{
    auto estimatedTop = height();
    if(!marginInfo.canCollapseWithMarginTop()) {
        float positiveMarginTop = child->maxMarginTop(true);
        float negativeMarginTop = child->maxMarginTop(false);
        if(positiveMarginTop < 0.f && negativeMarginTop < 0.f)
            estimateMarginTop(child, positiveMarginTop, negativeMarginTop);
        estimatedTop += std::max(positiveMarginTop, marginInfo.positiveMargin()) - std::max(negativeMarginTop, marginInfo.negativeMargin());
    }

    estimatedTop += getClearDelta(child, estimatedTop);
    if(isInsideColumnFlow()) {
        estimatedTop = column->applyColumnBreakBefore(child, estimatedTop);
        estimatedTop = column->applyColumnBreakInside(child, estimatedTop);
    }

    return estimatedTop;
}

void BlockFlowBox::determineHorizontalPosition(BoxFrame* child) const
{
    if(style()->isLeftToRightDirection()) {
        auto offsetX = borderLeft() + paddingLeft() + child->marginLeft();
        if(containsFloats() && child->avoidsFloats()) {
            auto startOffset = startOffsetForLine(child->y(), false);
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
            auto startOffset = startOffsetForLine(child->y(), false);
            if(child->style()->marginRight().isAuto())
                offsetX = std::max(offsetX, startOffset + child->marginRight());
            else if(startOffset > borderAndPaddingRight()) {
                offsetX = std::max(offsetX, startOffset);
            }
        }

        child->setX(width() - offsetX - child->width());
    }
}

void BlockFlowBox::adjustBlockChildInColumnFlow(BoxFrame* child, MultiColumnFlowBox* column)
{
    auto newOffset = column->applyColumnBreakBefore(child, child->y());
    auto adjustedOffset = column->applyColumnBreakInside(child, newOffset);

    auto childHeight = child->height();
    if(adjustedOffset > newOffset) {
        auto delta = adjustedOffset - newOffset;
        column->setColumnBreak(newOffset, childHeight - delta);
        newOffset += delta;
    } else {
        auto columnHeight = column->columnHeightForOffset(newOffset);
        if(columnHeight > 0.f) {
            auto remainingHeight = column->columnRemainingHeightForOffset(newOffset, AssociateWithLatterColumn);
            if(remainingHeight < childHeight) {
                column->setColumnBreak(newOffset, childHeight - remainingHeight);
            } else if(columnHeight == remainingHeight && child->y() > 0.f) {
                column->setColumnBreak(newOffset, childHeight);
            }
        }
    }

    setHeight(height() + (newOffset - child->y()));
    child->setY(newOffset);
}

void BlockFlowBox::layoutBlockChild(BoxFrame* child, MultiColumnFlowBox* column, MarginInfo& marginInfo)
{
    auto posTop = m_maxPositiveMarginTop;
    auto negTop = m_maxNegativeMarginTop;

    child->updateVerticalMargins();

    auto estimatedTop = estimateVerticalPosition(child, column, marginInfo);
    if(isInsideColumnFlow())
        column->enterChild(estimatedTop);
    child->setY(estimatedTop);
    child->layout();

    auto offsetY = collapseMargins(child, marginInfo);
    auto clearDelta = getClearDelta(child, offsetY);
    if(clearDelta && child->isSelfCollapsingBlock()) {
        marginInfo.setPositiveMargin(std::max(child->maxMarginTop(true), child->maxMarginBottom(true)));
        marginInfo.setNegativeMargin(std::max(child->maxMarginTop(false), child->maxMarginBottom(false)));

        setHeight(child->y() + child->maxMarginTop(false));
    } else {
        setHeight(clearDelta + height());
    }

    if(clearDelta && marginInfo.atTopOfBlock()) {
        m_maxPositiveMarginTop = posTop;
        m_maxNegativeMarginTop = negTop;
        marginInfo.setAtTopOfBlock(false);
    }

    child->setY(offsetY + clearDelta);
    if(isInsideColumnFlow())
        adjustBlockChildInColumnFlow(child, column);
    if(marginInfo.atTopOfBlock() && !child->isSelfCollapsingBlock()) {
        marginInfo.setAtTopOfBlock(false);
    }

    determineHorizontalPosition(child);
    if(auto spanner = to<MultiColumnSpanBox>(child)) {
        spanner->box()->setX(child->x());
        spanner->box()->setY(child->y());
    }

    setHeight(height() + child->height());
    if(isInsideColumnFlow()) {
        setHeight(column->applyColumnBreakAfter(child, height()));
        column->leaveChild(estimatedTop);
    }

    if(auto childBlock = to<BlockFlowBox>(child)) {
        addOverhangingFloats(childBlock);
    }
}

void BlockFlowBox::layoutBlockChildren()
{
    auto column = containingColumn();

    auto top = borderTop() + paddingTop();
    auto bottom = borderBottom() + paddingBottom();

    MarginInfo marginInfo(this, top, bottom);
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned()) {
            child->containingBlock()->insertPositonedBox(child);
            adjustPositionedBox(child, marginInfo);
        } else if(child->isFloating()) {
            insertFloatingBox(child);
            adjustFloatingBox(marginInfo);
        } else if(child->hasColumnSpanBox()) {
            setHeight(height() + marginInfo.margin());
            child->columnSpanBox()->columnFlowBox()->skipColumnSpanBox(child, height());
            marginInfo.clearMargin();
        } else if(child->isMultiColumnFlowBox()) {
            assert(child == m_columnFlowBox);
            child->setY(top);
            child->layout();
            determineHorizontalPosition(child);
        } else {
            layoutBlockChild(child, column, marginInfo);
        }
    }

    handleBottomOfBlock(top, bottom, marginInfo);
}

void BlockFlowBox::layout()
{
    if(isChildrenInline()) {
        m_lineLayout->updateWidth();
    } else {
        updateWidth();
    }

    collectIntrudingFloats();
    updateMaxMargins();

    setHeight(borderAndPaddingTop());
    if(isChildrenInline()) {
        m_lineLayout->layout();
    } else {
        layoutBlockChildren();
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
    if(!m_floatingBoxes)
        return;
    for(auto& item : *m_floatingBoxes) {
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

void BlockFlowBox::fragmentize(FragmentBuilder& builder, float top) const
{
    builder.enterBox(this, top);
    if(isChildrenInline()) {
        m_lineLayout->fragmentize(builder, top + y());
    } else {
        for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
            if(!child->isFloatingOrPositioned()) {
                child->fragmentize(builder, top + y());
            }
        }
    }

    builder.exitBox(this, top);
}

} // namespace plutobook
