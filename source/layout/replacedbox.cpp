#include "replacedbox.h"
#include "blockbox.h"
#include "imageresource.h"
#include "graphicscontext.h"
#include "pagebuilder.h"
#include "document.h"

namespace plutobook {

ReplacedBox::ReplacedBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
{
    setReplaced(true);
}

void ReplacedBox::computeIntrinsicRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const
{
    intrinsicWidth = intrinsicReplacedWidth();
    intrinsicHeight = intrinsicReplacedHeight();
    if(intrinsicWidth > 0.f && intrinsicHeight > 0.f) {
        intrinsicRatio = intrinsicWidth / intrinsicHeight;
    } else {
        intrinsicRatio = 0.0;
    }
}

void ReplacedBox::computeAspectRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const
{
    computeIntrinsicRatioInformation(intrinsicWidth, intrinsicHeight, intrinsicRatio);
    if(intrinsicRatio && style()->height().isAuto() && style()->width().isAuto()) {
        auto constrainedWidth = constrainReplacedWidth(intrinsicWidth);
        auto constrainedHeight = constrainReplacedHeight(intrinsicHeight);
        intrinsicWidth = constrainedHeight * intrinsicRatio;
        intrinsicHeight = constrainedWidth / intrinsicRatio;
    }
}

float ReplacedBox::computePreferredReplacedWidth() const
{
    auto widthLength = style()->width();
    if(widthLength.isFixed())
        return adjustContentBoxWidth(widthLength.value());

    float intrinsicWidth = 0.f;
    float intrinsicHeight = 0.f;
    double intrinsicRatio = 0.0;
    computeAspectRatioInformation(intrinsicWidth, intrinsicHeight, intrinsicRatio);

    auto heightLength = style()->height();
    if(intrinsicWidth && !heightLength.isFixed())
        return intrinsicWidth;
    if(intrinsicRatio > 0.0) {
        float height = 0.f;
        if(heightLength.isFixed())
            height = adjustContentBoxHeight(heightLength.value());
        else if(intrinsicHeight && !intrinsicWidth) {
            height = intrinsicHeight;
        } else {
            return intrinsicWidth;
        }

        auto minHeightLength = style()->minHeight();
        auto maxHeightLength = style()->maxHeight();
        if(maxHeightLength.isFixed())
            height = std::min(height, adjustContentBoxHeight(maxHeightLength.value()));
        if(minHeightLength.isFixed())
            height = std::max(height, adjustContentBoxHeight(minHeightLength.value()));
        return height * intrinsicRatio;
    }

    if(!intrinsicWidth)
        return intrinsicReplacedWidth();
    return intrinsicWidth;
}

void ReplacedBox::updatePreferredWidths() const
{
    auto widthLength = style()->width();
    if(widthLength.isPercent()) {
        m_maxPreferredWidth = m_minPreferredWidth = intrinsicReplacedWidth();
    } else {
        m_maxPreferredWidth = m_minPreferredWidth = computePreferredReplacedWidth();
    }

    auto maxWidthLength = style()->maxWidth();
    if(widthLength.isPercent() || maxWidthLength.isPercent()) {
        m_minPreferredWidth = 0;
    }

    auto minWidthLength = style()->minWidth();
    if(minWidthLength.isFixed() && minWidthLength.value() > 0) {
        m_minPreferredWidth = std::max(m_minPreferredWidth, adjustContentBoxWidth(minWidthLength.value()));
        m_maxPreferredWidth = std::max(m_maxPreferredWidth, adjustContentBoxWidth(minWidthLength.value()));
    }

    if(maxWidthLength.isFixed()) {
        m_minPreferredWidth = std::min(m_minPreferredWidth, adjustContentBoxWidth(maxWidthLength.value()));
        m_maxPreferredWidth = std::min(m_maxPreferredWidth, adjustContentBoxWidth(maxWidthLength.value()));
    }

    m_minPreferredWidth += borderAndPaddingWidth();
    m_maxPreferredWidth += borderAndPaddingWidth();
}

void ReplacedBox::computePositionedReplacedWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto container = containingBox();
    auto containerWidth = container->availableWidthForPositioned();
    auto containerDirection = container->direction();

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
    auto containerHeight = container->availableHeightForPositioned();

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
    if(!document()->isSVGImageDocument() && (widthLength.isPercent() || widthLength.isIntrinsic())) {
        float containerWidth = 0;
        if(isPositioned())
            containerWidth = containingBox()->availableWidthForPositioned();
        else
            containerWidth = containingBlock()->availableWidth();
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
    if(!document()->isSVGImageDocument() && heightLength.isPercent()) {
        float containerHeight = 0;
        if(isPositioned())
            containerHeight = containingBox()->availableHeightForPositioned();
        else if(auto availableHeight = containingBlock()->availableHeight())
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

float ReplacedBox::computeReplacedWidth() const
{
    if(auto width = computeReplacedWidthUsing(style()->width()))
        return constrainReplacedWidth(*width);

    float intrinsicWidth = 0.f;
    float intrinsicHeight = 0.f;
    double intrinsicRatio = 0.0;
    computeAspectRatioInformation(intrinsicWidth, intrinsicHeight, intrinsicRatio);

    auto height = computeReplacedHeightUsing(style()->height());
    if(intrinsicWidth && !height)
        return constrainReplacedWidth(intrinsicWidth);
    if(intrinsicRatio && height)
        return constrainReplacedWidth(constrainReplacedHeight(*height) * intrinsicRatio);
    if(intrinsicRatio && !intrinsicWidth && intrinsicHeight)
        return constrainReplacedWidth(constrainReplacedHeight(intrinsicHeight) * intrinsicRatio);
    if(intrinsicRatio && !intrinsicWidth && !intrinsicHeight && !height) {
        return availableReplacedWidth();
    }

    if(!intrinsicWidth)
        return constrainReplacedWidth(intrinsicReplacedWidth());
    return constrainReplacedWidth(intrinsicWidth);
}

float ReplacedBox::computeReplacedHeight() const
{
    if(auto height = computeReplacedHeightUsing(style()->height()))
        return constrainReplacedHeight(*height);

    float intrinsicWidth = 0.f;
    float intrinsicHeight = 0.f;
    double intrinsicRatio = 0.0;
    computeAspectRatioInformation(intrinsicWidth, intrinsicHeight, intrinsicRatio);

    auto width = computeReplacedWidthUsing(style()->width());
    if(intrinsicHeight && !width)
        return constrainReplacedHeight(intrinsicHeight);
    if(intrinsicRatio && width)
        return constrainReplacedHeight(constrainReplacedWidth(*width) / intrinsicRatio);
    if(intrinsicRatio && intrinsicWidth && !intrinsicHeight)
        return constrainReplacedHeight(constrainReplacedWidth(intrinsicWidth) / intrinsicRatio);
    if(intrinsicRatio && !intrinsicWidth && !intrinsicHeight && !width) {
        return constrainReplacedHeight(availableReplacedWidth() / intrinsicRatio);
    }

    if(!intrinsicHeight)
        return constrainReplacedHeight(intrinsicReplacedHeight());
    return constrainReplacedHeight(intrinsicHeight);
}

float ReplacedBox::availableReplacedWidth() const
{
    auto containerWidth = containingBlock()->availableWidth();
    auto marginLeft = style()->marginLeft().calcMin(containerWidth);
    auto marginRight = style()->marginRight().calcMin(containerWidth);
    return constrainReplacedWidth(containerWidth - marginLeft - marginRight - borderAndPaddingWidth());
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
    auto containerWidth = std::max(0.f, container->availableWidth());
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

void ReplacedBox::layout()
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
    overflowRect.move(offset + location());
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

void ReplacedBox::paginate(PageBuilder& builder, float top) const
{
    builder.enterBox(this, top + y());
    builder.exitBox(this, top + y());
}

ImageBox::ImageBox(Node* node, const RefPtr<BoxStyle>& style)
    : ReplacedBox(node, style)
{
}

void ImageBox::setImage(RefPtr<Image> image)
{
    m_image = std::move(image);
}

float ImageBox::intrinsicReplacedWidth() const
{
    if(m_image)
        return m_image->width();
    return 0.f;
}

float ImageBox::intrinsicReplacedHeight() const
{
    if(m_image)
        return m_image->height();
    return 0.f;
}

void ImageBox::paintReplaced(const PaintInfo& info, const Point& offset)
{
    if(m_image == nullptr)
        return;
    Rect borderRect(offset, size());
    if(borderRect.isEmpty())
        return;
    auto topWidth = borderTop() + paddingTop();
    auto bottomWidth = borderBottom() + paddingBottom();
    auto leftWidth = borderLeft() + paddingLeft();
    auto rightWidth = borderRight() + paddingRight();
    auto clipRect = style()->getBorderRoundedRect(borderRect, true, true);
    clipRect.shrink(topWidth, bottomWidth, leftWidth, rightWidth);
    if(clipRect.isRounded()) {
        info->save();
        info->clipRoundedRect(clipRect);
    }

    borderRect.shrink(topWidth, bottomWidth, leftWidth, rightWidth);
    m_image->draw(*info, borderRect, Rect(0, 0, m_image->width(), m_image->height()));
    if(clipRect.isRounded()) {
        info->restore();
    }
}

} // namespace plutobook
