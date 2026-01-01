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

void ReplacedBox::computeAspectRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const
{
    computeIntrinsicRatioInformation(intrinsicWidth, intrinsicHeight, intrinsicRatio);
    if(intrinsicRatio && intrinsicWidth && intrinsicHeight && style()->height().isAuto() && style()->width().isAuto()) {
        auto constrainedWidth = constrainReplacedWidth(intrinsicWidth);
        auto constrainedHeight = constrainReplacedHeight(intrinsicHeight);
        intrinsicWidth = constrainedHeight * intrinsicRatio;
        intrinsicHeight = constrainedWidth / intrinsicRatio;
    }
}

float ReplacedBox::computePreferredReplacedWidth() const
{
    auto widthLength = style()->width();
    if(widthLength.isFixed()) {
        return adjustContentBoxWidth(widthLength.value());
    }

    float intrinsicWidth = 0.f;
    float intrinsicHeight = 0.f;
    double intrinsicRatio = 0.0;
    computeAspectRatioInformation(intrinsicWidth, intrinsicHeight, intrinsicRatio);

    auto height = computeReplacedHeightUsing(style()->height());
    if(intrinsicWidth && !height)
        return intrinsicWidth;
    if(intrinsicRatio && height)
        return constrainReplacedHeight(height.value()) * intrinsicRatio;
    if(intrinsicRatio && !intrinsicWidth && intrinsicHeight)
        return constrainReplacedHeight(intrinsicHeight) * intrinsicRatio;
    if(intrinsicRatio && !intrinsicWidth && !intrinsicHeight) {
        return 0;
    }

    if(intrinsicWidth > 0.f)
        return intrinsicWidth;
    return intrinsicReplacedWidth();
}

void ReplacedBox::computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const
{
    auto widthLength = style()->width();
    if(widthLength.isPercent()) {
        maxPreferredWidth = minPreferredWidth = intrinsicReplacedWidth();
    } else {
        maxPreferredWidth = minPreferredWidth = computePreferredReplacedWidth();
    }

    auto maxWidthLength = style()->maxWidth();
    if(widthLength.isPercent() || maxWidthLength.isPercent()) {
        minPreferredWidth = 0;
    }

    auto minWidthLength = style()->minWidth();
    if(minWidthLength.isFixed() && minWidthLength.value() > 0) {
        minPreferredWidth = std::max(minPreferredWidth, adjustContentBoxWidth(minWidthLength.value()));
        maxPreferredWidth = std::max(maxPreferredWidth, adjustContentBoxWidth(minWidthLength.value()));
    }

    if(maxWidthLength.isFixed()) {
        minPreferredWidth = std::min(minPreferredWidth, adjustContentBoxWidth(maxWidthLength.value()));
        maxPreferredWidth = std::min(maxPreferredWidth, adjustContentBoxWidth(maxWidthLength.value()));
    }

    minPreferredWidth += borderAndPaddingWidth();
    maxPreferredWidth += borderAndPaddingWidth();
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
        marginTop = marginTopLength.calc(containerHeight);
        marginBottom = marginBottomLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        topLengthValue = availableSpace - (bottomLengthValue + marginTop + marginBottom);
    } else if(bottomLength.isAuto()) {
        marginTop = marginTopLength.calc(containerHeight);
        marginBottom = marginBottomLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
    } else if(marginTopLength.isAuto()) {
        marginBottom = marginBottomLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        marginTop = availableSpace - (topLengthValue + bottomLengthValue + marginBottom);
    } else if(marginBottomLength.isAuto()) {
        marginTop = marginTopLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
        bottomLengthValue = bottomLength.calc(containerHeight);

        marginBottom = availableSpace - (topLengthValue + bottomLengthValue + marginTop);
    } else {
        marginTop = marginTopLength.calc(containerHeight);
        marginBottom = marginBottomLength.calc(containerHeight);
        topLengthValue = topLength.calc(containerHeight);
    }

    y = topLengthValue + marginTop + container->borderTop();
}

std::optional<float> ReplacedBox::computeReplacedWidthUsing(const Length& widthLength) const
{
    if(widthLength.isFixed())
        return adjustContentBoxWidth(widthLength.value());
    if(widthLength.isPercent() || widthLength.isIntrinsic()) {
        float containerWidth = 0;
        if(isPositioned())
            containerWidth = containingBlockWidthForPositioned();
        else
            containerWidth = containingBlockWidthForContent();
        if(widthLength.isPercent())
            return adjustContentBoxWidth(widthLength.calcMin(containerWidth));
        return computeIntrinsicWidthUsing(widthLength, containerWidth) - borderAndPaddingWidth();
    }

    return std::nullopt;
}

std::optional<float> ReplacedBox::computeReplacedHeightUsing(const Length& heightLength) const
{
    if(heightLength.isFixed())
        return adjustContentBoxHeight(heightLength.value());
    if(heightLength.isPercent()) {
        float containerHeight = 0;
        if(isPositioned())
            containerHeight = containingBlockHeightForPositioned();
        else if(auto availableHeight = containingBlockHeightForContent())
            containerHeight = availableHeight.value();
        else
            return std::nullopt;
        return adjustContentBoxHeight(heightLength.calc(containerHeight));
    }

    return std::nullopt;
}

float ReplacedBox::constrainReplacedWidth(float width) const
{
    if(auto maxWidth = computeReplacedWidthUsing(style()->maxWidth()))
        width = std::min(width, *maxWidth);
    if(auto minWidth = computeReplacedWidthUsing(style()->minWidth()))
        width = std::max(width, *minWidth);
    return width;
}

float ReplacedBox::constrainReplacedHeight(float height) const
{
    if(auto maxHeight = computeReplacedHeightUsing(style()->maxHeight()))
        height = std::min(height, *maxHeight);
    if(auto minHeight = computeReplacedHeightUsing(style()->minHeight()))
        height = std::max(height, *minHeight);
    return height;
}

float ReplacedBox::availableReplacedWidth() const
{
    auto containerWidth = containingBlockWidthForContent();
    auto marginLeft = style()->marginLeft().calcMin(containerWidth);
    auto marginRight = style()->marginRight().calcMin(containerWidth);
    return containerWidth - marginLeft - marginRight - borderAndPaddingWidth();
}

float ReplacedBox::computeReplacedWidth() const
{
    if(auto width = computeReplacedWidthUsing(style()->width())) {
        return constrainReplacedWidth(*width);
    }

    float intrinsicWidth = 0.f;
    float intrinsicHeight = 0.f;
    double intrinsicRatio = 0.0;
    computeAspectRatioInformation(intrinsicWidth, intrinsicHeight, intrinsicRatio);

    auto height = computeReplacedHeightUsing(style()->height());
    if(intrinsicWidth && !height)
        return constrainReplacedWidth(intrinsicWidth);
    if(intrinsicRatio && height)
        return constrainReplacedWidth(constrainReplacedHeight(height.value()) * intrinsicRatio);
    if(intrinsicRatio && !intrinsicWidth && intrinsicHeight)
        return constrainReplacedWidth(constrainReplacedHeight(intrinsicHeight) * intrinsicRatio);
    if(intrinsicRatio && !intrinsicWidth && !intrinsicHeight) {
        return constrainReplacedWidth(availableReplacedWidth());
    }

    if(intrinsicWidth > 0.f)
        return constrainReplacedWidth(intrinsicWidth);
    return constrainReplacedWidth(intrinsicReplacedWidth());
}

float ReplacedBox::computeReplacedHeight() const
{
    if(auto height = computeReplacedHeightUsing(style()->height())) {
        return constrainReplacedHeight(*height);
    }

    float intrinsicWidth = 0.f;
    float intrinsicHeight = 0.f;
    double intrinsicRatio = 0.0;
    computeAspectRatioInformation(intrinsicWidth, intrinsicHeight, intrinsicRatio);

    auto width = computeReplacedWidthUsing(style()->width());
    if(intrinsicHeight && !width)
        return constrainReplacedHeight(intrinsicHeight);
    if(intrinsicRatio && width)
        return constrainReplacedHeight(constrainReplacedWidth(width.value()) / intrinsicRatio);
    if(intrinsicRatio && intrinsicWidth && !intrinsicHeight)
        return constrainReplacedHeight(constrainReplacedWidth(intrinsicWidth) / intrinsicRatio);
    if(intrinsicRatio && !intrinsicWidth && !intrinsicHeight) {
        return constrainReplacedHeight(constrainReplacedWidth(availableReplacedWidth()) / intrinsicRatio);
    }

    if(intrinsicHeight > 0.f)
        return constrainReplacedHeight(intrinsicHeight);
    return constrainReplacedHeight(intrinsicReplacedHeight());
}

static Size computeObjectFitSize(ObjectFit objectFit, const Size& intrinsicSize, const Size& contentSize)
{
    if(objectFit == ObjectFit::Fill)
        return contentSize;
    if(objectFit == ObjectFit::None) {
        return intrinsicSize;
    }

    auto xScale = contentSize.w / intrinsicSize.w;
    auto yScale = contentSize.h / intrinsicSize.h;
    auto scale = objectFit == ObjectFit::Cover ? std::max(xScale, yScale) : std::min(xScale, yScale);

    Size objectSize(intrinsicSize.w * scale, intrinsicSize.h * scale);
    if(objectFit == ObjectFit::ScaleDown && objectSize.w > intrinsicSize.w)
        return intrinsicSize;
    return objectSize;
}

Rect ReplacedBox::computeObjectFitRect(const Rect& contentRect) const
{
    if(m_intrinsicSize.isEmpty()) {
        return contentRect;
    }

    auto objectFit = style()->objectFit();
    auto objectPosition = style()->objectPosition();

    Rect objectRect(contentRect.origin(), computeObjectFitSize(objectFit, m_intrinsicSize, contentRect.size()));
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
    if(isInline())
        width = std::max(width, minPreferredWidth());
    computeHorizontalMargins(marginLeft, marginRight, width, container, containerWidth);
}

void ReplacedBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    if(hasOverrideHeight()) {
        height = overrideHeight();
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
    auto overflowRect = visualOverflowRect();
    overflowRect.translate(offset + location());
    if(!overflowRect.intersects(info.rect())) {
        return;
    }

    Point adjustedOffset(offset + location());
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
    setIntrinsicSize(image->intrinsicSize());
    m_image = std::move(image);
}

void ImageBox::computeIntrinsicRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const
{
    if(m_image) {
        m_image->computeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
    }
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
