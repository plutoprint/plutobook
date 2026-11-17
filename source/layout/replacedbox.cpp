/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "replacedbox.h"
#include "imageresource.h"
#include "graphicscontext.h"
#include "document.h"

namespace plutobook {

ReplacedBox::ReplacedBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
{
    setIsReplaced(true);
}

void ReplacedBox::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    minWidth = maxWidth = computeIntrinsicWidth();
}

std::pair<float, float> ReplacedBox::computePreferredWidths() const
{
    float minPreferredWidth = 0;
    float maxPreferredWidth = 0;

    auto widthLength = style()->width();
    if(widthLength.isFixed()) {
        minPreferredWidth = maxPreferredWidth = adjustContentBoxWidth(widthLength.value());
    } else {
        minPreferredWidth = minIntrinsicWidth();
        maxPreferredWidth = maxIntrinsicWidth();
    }

    auto maxWidthLength = style()->maxWidth();
    if(widthLength.isPercent() || maxWidthLength.isPercent()) {
        minPreferredWidth = 0;
    }

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
    return {minPreferredWidth, maxPreferredWidth};
}

void ReplacedBox::computePositionedReplacedWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto container = containingBox();
    auto containerWidth = containingBlockWidthForPositioned(container);
    auto containerDirection = container->style()->direction();

    auto marginLeftLength = style()->marginLeft();
    auto marginRightLength = style()->marginRight();

    auto leftLength = style()->left();
    auto rightLength = style()->right();
    computeHorizontalStaticDistance(leftLength, rightLength, container, containerWidth);
    if(leftLength.isAuto() || rightLength.isAuto()) {
        if(marginLeftLength.isAuto())
            marginLeftLength = Length::ZeroFixed;
        if(marginRightLength.isAuto()) {
            marginRightLength = Length::ZeroFixed;
        }
    }

    width = computeReplacedWidth() + borderAndPaddingWidth();
    auto availableSpace = containerWidth - width;

    float leftLengthValue = 0;
    float rightLengthValue = 0;
    if(marginLeftLength.isAuto() && marginRightLength.isAuto()) {
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        auto availableWidth = availableSpace - (leftLengthValue + rightLengthValue);
        if(availableWidth > 0) {
            marginLeft = availableWidth / 2.f;
            marginRight = availableWidth - marginLeft;
        } else {
            if(containerDirection == Direction::Ltr) {
                marginLeft = 0;
                marginRight = availableWidth;
            } else {
                marginLeft = availableWidth;
                marginRight = 0;
            }
        }
    } else if(leftLength.isAuto()) {
        marginLeft = marginLeftLength.calc(containerWidth);
        marginRight = marginRightLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        leftLengthValue = availableSpace - (rightLengthValue + marginLeft + marginRight);
    } else if(rightLength.isAuto()) {
        marginLeft = marginLeftLength.calc(containerWidth);
        marginRight = marginRightLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);
    } else if(marginLeftLength.isAuto()) {
        marginRight = marginRightLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        marginLeft = availableSpace - (leftLengthValue + rightLengthValue + marginRight);
    } else if(marginRightLength.isAuto()) {
        marginLeft = marginLeftLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);

        marginRight = availableSpace - (leftLengthValue + rightLengthValue + marginLeft);
    } else {
        marginLeft = marginLeftLength.calc(containerWidth);
        marginRight = marginRightLength.calc(containerWidth);
        leftLengthValue = leftLength.calc(containerWidth);
        rightLengthValue = rightLength.calc(containerWidth);
        if(containerDirection == Direction::Rtl) {
            auto totalWidth = width + leftLengthValue + rightLengthValue +  marginLeft + marginRight;
            leftLengthValue = containerWidth - (totalWidth - leftLengthValue);
        }
    }

    x = leftLengthValue + marginLeft + container->borderLeft();
}

void ReplacedBox::computePositionedReplacedHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    auto container = containingBox();
    auto containerWidth = containingBlockWidthForPositioned(container);
    auto containerHeight = containingBlockHeightForPositioned(container);

    auto marginTopLength = style()->marginTop();
    auto marginBottomLength = style()->marginBottom();

    auto topLength = style()->top();
    auto bottomLength = style()->bottom();
    computeVerticalStaticDistance(topLength, bottomLength, container);
    if(topLength.isAuto() || bottomLength.isAuto()) {
        if(marginTopLength.isAuto())
            marginTopLength = Length::ZeroFixed;
        if(marginBottomLength.isAuto()) {
            marginBottomLength = Length::ZeroFixed;
        }
    }

    height = computeReplacedHeight() + borderAndPaddingHeight();
    auto availableSpace = containerHeight - height;

    float topLengthValue = 0;
    float bottomLengthValue = 0;
    if(marginTopLength.isAuto() && marginBottomLength.isAuto()) {
        topLengthValue = topLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        auto availableHeight = availableSpace - (topLengthValue + bottomLengthValue);
        marginTop = availableHeight / 2.f;
        marginBottom = availableHeight - marginTop;
    } else if(topLength.isAuto()) {
        marginTop = marginTopLength.calc(containerWidth);
        marginBottom = marginBottomLength.calc(containerWidth);
        bottomLengthValue = bottomLength.calc(containerHeight);

        topLengthValue = availableSpace - (bottomLengthValue + marginTop + marginBottom);
    } else if(bottomLength.isAuto()) {
        marginTop = marginTopLength.calc(containerWidth);
        marginBottom = marginBottomLength.calc(containerWidth);
        topLengthValue = topLength.calc(containerHeight);
    } else if(marginTopLength.isAuto()) {
        marginBottom = marginBottomLength.calc(containerWidth);
        topLengthValue = topLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        marginTop = availableSpace - (topLengthValue + bottomLengthValue + marginBottom);
    } else if(marginBottomLength.isAuto()) {
        marginTop = marginTopLength.calc(containerWidth);
        topLengthValue = topLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        marginBottom = availableSpace - (topLengthValue + bottomLengthValue + marginTop);
    } else {
        marginTop = marginTopLength.calc(containerWidth);
        marginBottom = marginBottomLength.calc(containerWidth);
        topLengthValue = topLength.calc(containerHeight);
    }

    y = topLengthValue + marginTop + container->borderTop();
}

float ReplacedBox::availableReplacedWidth() const
{
    if(style()->width().isPercent())
        return kDefaultReplacedWidth;
    if(isPositioned()) {
        auto container = containingBox();
        auto containerWidth = containingBlockWidthForPositioned(container);

        auto leftLength = style()->left();
        auto rightLength = style()->right();
        computeHorizontalStaticDistance(leftLength, rightLength, container, containerWidth);

        auto marginLeft = style()->marginLeft().calcMin(containerWidth);
        auto marginRight = style()->marginRight().calcMin(containerWidth);
        auto insetWidth = leftLength.calcMin(containerWidth) + rightLength.calcMin(containerWidth);
        return std::max(0.f, containerWidth - insetWidth - marginLeft - marginRight - borderAndPaddingWidth());
    }

    auto containerWidth = containingBlockWidthForContent();
    auto marginLeft = style()->marginLeft().calcMin(containerWidth);
    auto marginRight = style()->marginRight().calcMin(containerWidth);
    return std::max(0.f, containerWidth - marginLeft - marginRight - borderAndPaddingWidth());
}

std::optional<float> ReplacedBox::computeTentativeReplacedWidth() const
{
    auto widthLength = style()->width();
    if(!widthLength.isAuto())
        return computeWidthUsing(widthLength);
    if(isStretchedColumnFlexItem())
        return availableReplacedWidth();
    return std::nullopt;
}

std::optional<float> ReplacedBox::computeTentativeReplacedHeight() const
{
    auto heightLength = style()->height();
    if(!heightLength.isAuto())
        return computeHeightUsing(heightLength);
    if(isStretchedRowFlexItem()) {
        if(auto containerHeight = containingBlockHeightForContent()) {
            return std::max(0.f, containerHeight.value() - marginHeight() - borderAndPaddingHeight());
        }
    }

    return std::nullopt;
}

float ReplacedBox::computeReplacedWidth() const
{
    if(auto width = computeTentativeReplacedWidth())
        return constrainWidth(*width);
    return constrainWidth(computeIntrinsicWidth());
}

float ReplacedBox::computeReplacedHeight() const
{
    if(auto height = computeTentativeReplacedHeight())
        return constrainHeight(*height);
    return constrainHeight(computeIntrinsicHeight());
}

float ReplacedBox::computeIntrinsicWidth() const
{
    if(m_intrinsicRatio > 0.f) {
        float height = 0.f;
        if(hasOverrideHeight())
            height = std::max(0.f, overrideHeight() - borderAndPaddingHeight());
        else if(auto tentativeHeight = computeTentativeReplacedHeight())
            height = tentativeHeight.value();
        else if(m_intrinsicHeight > 0.f)
            height = m_intrinsicHeight;
        else if(m_intrinsicWidth > 0.f)
            height = m_intrinsicWidth / m_intrinsicRatio;
        else
            height = availableReplacedWidth() / m_intrinsicRatio;
        return constrainHeight(height) * m_intrinsicRatio;
    }

    if(m_intrinsicWidth == 0.f)
        return kDefaultReplacedWidth;
    return std::max(0.f, m_intrinsicWidth);
}

float ReplacedBox::computeIntrinsicHeight() const
{
    if(m_intrinsicRatio > 0.f) {
        float width = 0.f;
        if(hasOverrideWidth())
            width = std::max(0.f, overrideWidth() - borderAndPaddingWidth());
        else if(auto tentativeWidth = computeTentativeReplacedWidth())
            width = tentativeWidth.value();
        else if(m_intrinsicWidth > 0.f)
            width = m_intrinsicWidth;
        else if(m_intrinsicHeight > 0.f)
            width = m_intrinsicHeight * m_intrinsicRatio;
        else
            width = availableReplacedWidth();
        return constrainWidth(width) / m_intrinsicRatio;
    }

    if(m_intrinsicHeight == 0.f)
        return kDefaultReplacedHeight;
    return std::max(0.f, m_intrinsicHeight);
}

Size ReplacedBox::computeObjectFitSize(ObjectFit objectFit, const Size& contentSize) const
{
    if(objectFit == ObjectFit::Fill)
        return contentSize;
    auto concreteSize = computeConcreteObjectSize(m_intrinsicWidth, m_intrinsicHeight, m_intrinsicRatio, contentSize);
    if(objectFit == ObjectFit::None) {
        return concreteSize;
    }

    auto xScale = contentSize.w / concreteSize.w;
    auto yScale = contentSize.h / concreteSize.h;
    auto scale = objectFit == ObjectFit::Cover ? std::max(xScale, yScale) : std::min(xScale, yScale);

    Size objectSize(concreteSize.w * scale, concreteSize.h * scale);
    if(objectFit == ObjectFit::ScaleDown && objectSize.w > concreteSize.w)
        return concreteSize;
    return objectSize;
}

Rect ReplacedBox::computeObjectFitRect(const Rect& contentRect) const
{
    auto objectFit = style()->objectFit();
    auto objectPosition = style()->objectPosition();

    Rect objectRect(contentRect.origin(), computeObjectFitSize(objectFit, contentRect.size()));
    const Point positionOffset = {
        objectPosition.x().calcMin(contentRect.w - objectRect.w),
        objectPosition.y().calcMin(contentRect.h - objectRect.h)
    };

    return objectRect.translated(positionOffset);
}

void ReplacedBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    if(hasOverrideWidth()) {
        width = overrideWidth();
        return;
    }

    if(isPositioned()) {
        computePositionedReplacedWidth(x, width, marginLeft, marginRight);
        return;
    }

    auto container = containingBlock();
    auto containerWidth = std::max(0.f, containingBlockWidthForContent(container));
    width = computeReplacedWidth() + borderAndPaddingWidth();
    computeHorizontalMargins(marginLeft, marginRight, width, container, containerWidth);
}

void ReplacedBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    if(hasOverrideHeight()) {
        height = overrideHeight();
        return;
    }

    if(isColumnFlexItem()) {
        height = computeIntrinsicHeight() + borderAndPaddingHeight();
        return;
    }

    if(isPositioned()) {
        computePositionedReplacedHeight(y, height, marginTop, marginBottom);
        return;
    }

    height = computeReplacedHeight() + borderAndPaddingHeight();
    computeVerticalMargins(marginTop, marginBottom);
}

void ReplacedBox::layout(FragmentBuilder* fragmentainer)
{
    updateWidth();
    updateHeight();
    updateOverflowRect();
}

void ReplacedBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(phase != PaintPhase::Contents && phase != PaintPhase::Outlines)
        return;
    if(style()->visibility() != Visibility::Visible)
        return;
    Point adjustedOffset(offset + location());
    if(!info.shouldPaintBox(this, adjustedOffset)) {
        return;
    }

    if(phase == PaintPhase::Outlines) {
        paintOutlines(info, adjustedOffset);
    } else {
        paintDecorations(info, adjustedOffset);
        paintReplaced(info, adjustedOffset);
    }
}

ImageBox::ImageBox(Node* node, const RefPtr<BoxStyle>& style)
    : ReplacedBox(node, style)
{
}

void ImageBox::setImage(RefPtr<Image> image)
{
    image->computeIntrinsicDimensions(m_intrinsicWidth, m_intrinsicHeight, m_intrinsicRatio);
    m_image = std::move(image);
}

void ImageBox::paintReplaced(const PaintInfo& info, const Point& offset)
{
    if(m_image == nullptr)
        return;
    const RectOutsets outsets = {
        borderTop() + paddingTop(),
        borderRight() + paddingRight(),
        borderBottom() + paddingBottom(),
        borderLeft() + paddingLeft()
    };

    Rect borderRect(offset, size());
    Rect contentRect(borderRect - outsets);
    if(contentRect.isEmpty()) {
        return;
    }

    auto objectRect = computeObjectFitRect(contentRect);
    auto clipRect = style()->getBorderRoundedRect(borderRect, true, true) - outsets;

    auto clipping = !contentRect.contains(objectRect) || clipRect.isRounded();
    if(clipping) {
        info->save();
        info->clipRoundedRect(clipRect);
    }

    m_image->setContainerSize(objectRect.size());
    m_image->draw(*info, objectRect, Rect(m_image->size()));
    if(clipping) {
        info->restore();
    }
}

} // namespace plutobook
