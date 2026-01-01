/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "linebox.h"
#include "textbox.h"
#include "blockbox.h"
#include "fragmentbuilder.h"
#include "graphicscontext.h"
#include "boxlayer.h"

#include <cmath>

namespace plutobook {

LineBox::~LineBox() = default;

Node* LineBox::node() const
{
    return m_box->node();
}

BoxStyle* LineBox::style() const
{
    return m_box->style();
}

float LineBox::height() const
{
    if(isRootLineBox() || isTextLineBox())
        return style()->fontHeight();
    if(auto box = to<BoxFrame>(m_box))
        return box->height();
    const auto& box = to<BoxModel>(*m_box);
    return style()->fontHeight() + box.borderAndPaddingHeight();
}

Point LineBox::location() const
{
    return Point(m_x, m_y);
}

Size LineBox::size() const
{
    return Size(m_width, height());
}

Rect LineBox::rect() const
{
    return Rect(m_x, m_y, m_width, height());
}

float LineBox::verticalAlignPosition() const
{
    if(isTextLineBox()) {
        if(m_parentLine->isRootLineBox())
            return 0.f;
        return m_parentLine->y();
    }

    auto verticalAlign = style()->verticalAlign();
    if(verticalAlign.type() == VerticalAlignType::Top || verticalAlign.type() == VerticalAlignType::Bottom) {
        return 0.f;
    }

    auto parentBox = m_box->parentBox();
    auto parentStyle = parentBox->style();

    float verticalPosition = 0.f;
    if(parentBox->isInlineBox() && parentStyle->verticalAlignType() != VerticalAlignType::Top
        && parentStyle->verticalAlignType() != VerticalAlignType::Bottom) {
        verticalPosition += m_parentLine->y();
    }

    if(verticalAlign.type() == VerticalAlignType::Baseline)
        return verticalPosition;
    if(verticalAlign.type() == VerticalAlignType::Sub) {
        verticalPosition += parentStyle->fontSize() / 5.f;
    } else if(verticalAlign.type() == VerticalAlignType::Super) {
        verticalPosition -= parentStyle->fontSize() / 3.f;
    } else if(verticalAlign.type() == VerticalAlignType::TextTop) {
        verticalPosition += baselinePosition() - parentStyle->fontAscent();
    } else if(verticalAlign.type() == VerticalAlignType::TextBottom) {
        verticalPosition += parentStyle->fontDescent();
        verticalPosition -= lineHeight() - baselinePosition();
    } else if(verticalAlign.type() == VerticalAlignType::Middle) {
        verticalPosition -= parentStyle->exFontSize() / 2.f;
        verticalPosition -= lineHeight() / 2.f;
        verticalPosition += baselinePosition();
    } else if(verticalAlign.type() == VerticalAlignType::Length) {
        verticalPosition -= verticalAlign.length().calc(style()->lineHeight());
    }

    return verticalPosition;
}

VerticalAlignType LineBox::verticalAlignType() const
{
    if(isTextLineBox() && m_parentLine->isRootLineBox())
        return VerticalAlignType::Baseline;
    return style()->verticalAlignType();
}

LineBox::LineBox(Box* box, float width)
    : m_box(box), m_width(width)
{
}

std::unique_ptr<TextLineBox> TextLineBox::create(TextBox* box, const TextShapeView& shape, float expansion, float width)
{
    return std::unique_ptr<TextLineBox>(new (box->heap()) TextLineBox(box, shape, expansion, width));
}

float TextLineBox::lineHeight() const
{
    return m_parentLine->lineHeight();
}

float TextLineBox::baselinePosition() const
{
    return m_parentLine->baselinePosition();
}

TextBox* TextLineBox::box() const
{
    return static_cast<TextBox*>(m_box);
}

static void paintTextDecoration(GraphicsContext& context, const Point& origin, float width, float thickness, float doubleOffset, int wavyOffsetFactor, TextDecorationStyle style)
{
    float x1 = origin.x;
    float y1 = origin.y;
    float x2 = origin.x + width;
    float y2 = origin.y;

    Path path;
    if(style == TextDecorationStyle::Wavy) {
        assert(y1 == y2);
        float x = x1;
        float y = y1 + doubleOffset * wavyOffsetFactor;

        float distance = 3.f * thickness;
        float step = 2.f * thickness;

        path.moveTo(x, y);
        while(x < x2) {
            path.cubicTo(x + step, y + distance, x + step, y - distance, x + 2.f * step, y);
            x += 2.f * step;
        }
    } else {
        path.moveTo(x1, y1);
        path.lineTo(x2, y2);
        if(style == TextDecorationStyle::Double) {
            path.moveTo(x1, y1 + doubleOffset);
            path.lineTo(x2, y2 + doubleOffset);
        }
    }

    StrokeData strokeData(thickness);
    if(style == TextDecorationStyle::Dashed)
        strokeData.setDashArray({thickness * 3.f});
    else if(style == TextDecorationStyle::Dotted) {
        strokeData.setDashArray({thickness});
    }

    context.strokePath(path, strokeData);
}

static void paintTextDecorations(GraphicsContext& context, const Point& offset, float width, const BoxStyle* style)
{
    auto decorations = style->textDecorationLine();
    if(decorations == TextDecorationLine::None)
        return;
    auto baseline = style->fontAscent();
    auto thickness = style->fontSize() / 16.f;
    auto doubleOffset = thickness + 1.f;
    auto decorationStyle = style->textDecorationStyle();
    context.setColor(style->textDecorationColor());
    if(decorations & TextDecorationLine::Underline) {
        auto gap = std::max(1.f, std::ceil(thickness / 2.f));
        Point origin(offset.x, offset.y + baseline + gap);
        paintTextDecoration(context, origin, width, thickness, doubleOffset, 0, decorationStyle);
    }

    if(decorations & TextDecorationLine::Overline)
        paintTextDecoration(context, offset, width, thickness, -doubleOffset, 1, decorationStyle);
    if(decorations & TextDecorationLine::LineThrough) {
        Point origin(offset.x, offset.y + 2.f * baseline / 3.f);
        paintTextDecoration(context, origin, width, thickness, doubleOffset, 0, decorationStyle);
    }
}

void TextLineBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(m_shapeWidth == 0.f || phase != PaintPhase::Contents || style()->visibility() != Visibility::Visible)
        return;
    Point adjustedOffset(offset + location());
    Point origin(adjustedOffset.x, adjustedOffset.y + style()->fontAscent());
    int repeatCount = std::max(1.f, std::floor(m_width / m_shapeWidth));
    if(repeatCount > 1 && style()->direction() == Direction::Ltr) {
        origin.x += std::max(0.f, m_width - (m_shapeWidth * repeatCount));
    }

    info->setColor(style()->color());
    for(int i = 0; i < repeatCount; ++i) {
        origin.x += m_shape.draw(*info, origin, m_expansion);
    }

    paintTextDecorations(*info, adjustedOffset, m_width, style());
}

void TextLineBox::serialize(std::ostream& o, int indent) const
{
    std::string data;
    m_shape.text().toUTF8String(data);

    Box::serializeStart(o, indent, data.empty(), m_box, this);
    o << data;
    Box::serializeEnd(o, indent, data.empty(), m_box, this);
}

TextLineBox::~TextLineBox() = default;

TextLineBox::TextLineBox(TextBox* box, const TextShapeView& shape, float width, float expansion)
    : LineBox(box, width)
    , m_shape(shape)
    , m_shapeWidth(shape.width(expansion))
    , m_expansion(expansion)
{
}

std::unique_ptr<ReplacedLineBox> ReplacedLineBox::create(BoxFrame* box)
{
    return std::unique_ptr<ReplacedLineBox>(new (box->heap()) ReplacedLineBox(box));
}

float ReplacedLineBox::lineHeight() const
{
    return box()->marginBoxHeight();
}

float ReplacedLineBox::baselinePosition() const
{
    if(auto baseline = box()->inlineBlockBaseline())
        return baseline.value() + box()->marginTop();
    return lineHeight();
}

BoxFrame* ReplacedLineBox::box() const
{
    return static_cast<BoxFrame*>(m_box);
}

void ReplacedLineBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(phase == PaintPhase::Contents) {
        box()->paint(info, offset, PaintPhase::Decorations);
        box()->paint(info, offset, PaintPhase::Floats);
        box()->paint(info, offset, PaintPhase::Contents);
        box()->paint(info, offset, PaintPhase::Outlines);
    }
}

void ReplacedLineBox::serialize(std::ostream& o, int indent) const
{
    Box::serializeStart(o, indent, false, m_box, this);
    m_box->serialize(o, indent + 1);
    Box::serializeEnd(o, indent, false, m_box, this);
}

ReplacedLineBox::ReplacedLineBox(BoxFrame* box)
    : LineBox(box, box->width())
{
}

std::unique_ptr<FlowLineBox> FlowLineBox::create(BoxModel* box)
{
    return std::unique_ptr<FlowLineBox>(new (box->heap()) FlowLineBox(box));
}

float FlowLineBox::lineHeight() const
{
    return box()->style()->lineHeight();
}

float FlowLineBox::baselinePosition() const
{
    return box()->style()->fontAscent() + (lineHeight() - box()->style()->fontHeight()) / 2.f;
}

BoxModel* FlowLineBox::box() const
{
    return static_cast<BoxModel*>(m_box);
}

void FlowLineBox::addChild(LineBox* child)
{
    assert(child->parentLine() == nullptr);
    child->setParentLine(this);
    child->setLineIndex(lineIndex());
    m_children.push_back(child);
}

float FlowLineBox::marginLeft() const
{
    if(m_hasLeftEdge)
        return box()->marginLeft();
    return 0.f;
}

float FlowLineBox::marginRight() const
{
    if(m_hasRightEdge)
        return box()->marginRight();
    return 0.f;
}

float FlowLineBox::paddingLeft() const
{
    if(m_hasLeftEdge)
        return box()->paddingLeft();
    return 0.f;
}

float FlowLineBox::paddingRight() const
{
    if(m_hasRightEdge)
        return box()->paddingRight();
    return 0.f;
}

float FlowLineBox::borderLeft() const
{
    if(m_hasLeftEdge)
        return box()->borderLeft();
    return 0.f;
}

float FlowLineBox::borderRight() const
{
    if(m_hasRightEdge)
        return box()->borderRight();
    return 0.f;
}

void FlowLineBox::adjustMaxAscentAndDescent(float& maxAscent, float& maxDescent, float maxPositionTop, float maxPositionBottom) const
{
    for(auto child : m_children) {
        if(child->box()->isPositioned())
            continue;
        auto verticalAlignType = child->verticalAlignType();
        if(verticalAlignType == VerticalAlignType::Top || verticalAlignType == VerticalAlignType::Bottom) {
            auto lineHeight = child->lineHeight();
            if(verticalAlignType == VerticalAlignType::Top) {
                if(maxAscent + maxDescent < lineHeight) {
                    maxDescent = lineHeight - maxAscent;
                }
            } else {
                if(maxAscent + maxDescent < lineHeight) {
                    maxAscent = lineHeight - maxDescent;
                }
            }

            if(maxAscent + maxDescent >= std::max(maxPositionTop, maxPositionBottom)) {
                break;
            }
        }

        if(auto line = to<FlowLineBox>(child)) {
            line->adjustMaxAscentAndDescent(maxAscent, maxDescent, maxPositionTop, maxPositionBottom);
        }
    }
}

void FlowLineBox::computeMaxAscentAndDescent(float& maxAscent, float& maxDescent, float& maxPositionTop, float& maxPositionBottom)
{
    if(isRootLineBox()) {
        maxAscent = baselinePosition();
        maxDescent = lineHeight() - maxAscent;
    }

    for(auto child : m_children) {
        if(child->box()->isPositioned())
            continue;
        float ascent = 0.f;
        float descent = 0.f;
        if(child->isTextLineBox() && !style()->hasLineHeight()) {
            const auto& line = to<TextLineBox>(*child);
            const auto& shape = line.shape();
            shape.maxAscentAndDescent(ascent, descent);
        }

        if(!ascent && !descent) {
            ascent = child->baselinePosition();
            descent = child->lineHeight() - ascent;
        }

        child->setY(child->verticalAlignPosition());
        auto verticalAlignType = child->verticalAlignType();
        auto height = ascent + descent;
        if(verticalAlignType == VerticalAlignType::Top) {
            if(maxPositionTop < height) {
                maxPositionTop = height;
            }
        } else if(verticalAlignType == VerticalAlignType::Bottom) {
            if(maxPositionBottom < height) {
                maxPositionBottom = height;
            }
        } else {
            ascent -= child->y();
            descent += child->y();
            if(maxAscent < ascent)
                maxAscent = ascent;
            if(maxDescent < descent) {
                maxDescent = descent;
            }
        }

        if(auto line = to<FlowLineBox>(child)) {
            line->computeMaxAscentAndDescent(maxAscent, maxDescent, maxPositionTop, maxPositionBottom);
        }
    }
}

float FlowLineBox::placeInHorizontalDirection(float offsetX, const BlockFlowBox* block)
{
    offsetX += marginLeft();
    setX(offsetX);
    offsetX += paddingLeft() + borderLeft();
    for(auto child : m_children) {
        if(child->box()->isPositioned()) {
            const auto& box = to<BoxFrame>(*child->box());
            if(box.style()->isOriginalDisplayBlockType()) {
                box.layer()->setStaticLeft(block->startOffsetForContent());
            } else if(box.parentBox()->style()->isRightToLeftDirection()) {
                box.layer()->setStaticLeft(-offsetX + block->width());
            } else {
                box.layer()->setStaticLeft(offsetX);
            }

            child->setX(offsetX);
            continue;
        }

        if(auto line = to<FlowLineBox>(child)) {
            offsetX = line->placeInHorizontalDirection(offsetX, block);
            continue;
        }

        if(auto line = to<TextLineBox>(child)) {
            line->setX(offsetX);
            offsetX += line->width();
            continue;
        }

        auto& line = to<ReplacedLineBox>(*child);
        auto& box = to<BoxFrame>(*child->box());
        if(box.isOutsideListMarkerBox()) {
            if(block->style()->direction() == Direction::Ltr) {
                line.setX(-box.width() - box.marginRight());
            } else {
                line.setX(block->width() + box.marginLeft());
            }

            box.setX(line.x());
            continue;
        }

        offsetX += box.marginLeft();
        line.setX(offsetX);
        box.setX(line.x());
        offsetX += line.width();
        offsetX += box.marginRight();
    }

    offsetX += paddingRight() + borderRight();
    setWidth(offsetX - x());
    offsetX += marginRight();
    return offsetX;
}

void FlowLineBox::placeInVerticalDirection(float y, float maxHeight, float maxAscent, RootLineBox* rootLine)
{
    if(rootLine == this)
        rootLine->setY(y + maxAscent - baselinePosition());
    for(auto child : m_children) {
        if(child->box()->isPositioned()) {
            const auto& box = to<BoxFrame>(*child->box());
            if(!rootLine->isEmptyLine() && box.style()->isOriginalDisplayBlockType()) {
                box.layer()->setStaticTop(y + maxHeight);
            } else {
                box.layer()->setStaticTop(y);
            }

            child->setY(y);
            continue;
        }

        if(auto line = to<FlowLineBox>(child))
            line->placeInVerticalDirection(y, maxHeight, maxAscent, rootLine);
        auto verticalAlignType = child->verticalAlignType();
        if(verticalAlignType == VerticalAlignType::Top) {
            child->setY(y);
        } else if(verticalAlignType == VerticalAlignType::Bottom) {
            child->setY(y + maxHeight - child->lineHeight());
        } else {
            auto posAdjust = maxAscent - child->baselinePosition();
            child->setY(posAdjust + y + child->y());
        }

        if(child->isReplacedLineBox()) {
            auto& box = to<BoxFrame>(*child->box());
            child->setY(child->y() + box.marginTop());
            box.setY(child->y());
        } else {
            assert(child->isTextLineBox() || child->isFlowLineBox());
            auto top = child->baselinePosition() - child->style()->fontAscent();
            if(child->isFlowLineBox()) {
                const auto& box = to<BoxModel>(*child->box());
                top -= box.borderTop() + box.paddingTop();
            }

            child->setY(top + child->y());
        }

        rootLine->updateLineTopAndBottom(child);
    }

    if(rootLine == this) {
        rootLine->updateLineTopAndBottom(this);
    }
}

Rect FlowLineBox::visualOverflowRect() const
{
    return Rect(m_overflowLeft, m_overflowTop, m_overflowRight - m_overflowLeft, m_overflowBottom - m_overflowTop);
}

void FlowLineBox::addOverflowRect(const BoxFrame* box, float dx, float dy)
{
    if(box->hasLayer())
        return;
    auto overflowRect = box->visualOverflowRect();
    overflowRect.translate(dx, dy);
    addOverflowRect(overflowRect);
}

void FlowLineBox::addOverflowRect(float top, float bottom, float left, float right)
{
    m_overflowTop = std::min(top, m_overflowTop);
    m_overflowBottom = std::max(bottom, m_overflowBottom);
    m_overflowLeft = std::min(left, m_overflowLeft);
    m_overflowRight = std::max(right, m_overflowRight);
}

void FlowLineBox::addOverflowRect(const Rect& overflowRect)
{
    addOverflowRect(overflowRect.y, overflowRect.bottom(), overflowRect.x, overflowRect.right());
}

void FlowLineBox::updateOverflowRect(float lineTop, float lineBottom)
{
    Rect borderRect(m_x, m_y, m_width, height());
    if(!isRootLineBox()) {
        auto outlineEdge = style()->getOutlineEdge();
        if(outlineEdge.isRenderable()) {
            borderRect.inflate(outlineEdge.width() + style()->outlineOffset());
        }
    }

    m_overflowTop = std::min(lineTop, borderRect.y);
    m_overflowBottom = std::max(lineBottom, borderRect.bottom());
    m_overflowLeft = std::min(m_x, borderRect.x);
    m_overflowRight = std::max(m_x + m_width, borderRect.right());
    for(auto child : m_children) {
        if(child->box()->isPositioned())
            continue;
        if(auto line = to<TextLineBox>(child)) {
            addOverflowRect(line->y(), line->bottom(), line->x(), line->right());
            continue;
        }

        if(auto line = to<ReplacedLineBox>(child)) {
            addOverflowRect(line->box(), line->x(), line->y());
            continue;
        }

        auto& line = to<FlowLineBox>(*child);
        line.updateOverflowRect(lineTop, lineBottom);
        if(!line.box()->hasLayer()) {
            addOverflowRect(line.visualOverflowRect());
        }
    }
}

void FlowLineBox::paintOutlines(const PaintInfo& info, const Point& offset) const
{
    if(style()->visibility() != Visibility::Visible || isRootLineBox())
        return;
    Point adjustedOffset(offset + location());
    Rect borderRect(adjustedOffset, size());
    box()->paintOutline(info, borderRect);
}

void FlowLineBox::paintDecorations(const PaintInfo& info, const Point& offset) const
{
    if(style()->visibility() != Visibility::Visible || isRootLineBox())
        return;
    Point adjustedOffset(offset + location());
    Rect borderRect(adjustedOffset, size());
    box()->paintBackground(info, borderRect, m_hasLeftEdge, m_hasRightEdge);
    box()->paintBorder(info, borderRect, m_hasLeftEdge, m_hasRightEdge);
}

void FlowLineBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    Rect overflowRect(visualOverflowRect());
    overflowRect.translate(offset);
    if(!overflowRect.intersects(info.rect())) {
        return;
    }

    if(phase == PaintPhase::Contents)
        paintDecorations(info, offset);
    for(auto child : m_children) {
        auto box = child->box();
        if(!box->hasLayer()) {
            child->paint(info, offset, phase);
        }
    }

    if(phase == PaintPhase::Outlines) {
        paintOutlines(info, offset);
    }
}

void FlowLineBox::serialize(std::ostream& o, int indent) const
{
    Box::serializeStart(o, indent, m_children.empty(), m_box, this);
    for(auto child : m_children)
        child->serialize(o, indent + 1);
    Box::serializeEnd(o, indent, m_children.empty(), m_box, this);
}

FlowLineBox::FlowLineBox(BoxModel* box)
    : LineBox(box, 0.f)
    , m_children(box->heap())
{
}

std::unique_ptr<RootLineBox> RootLineBox::create(BlockFlowBox* box)
{
    return std::unique_ptr<RootLineBox>(new (box->heap()) RootLineBox(box));
}

BlockFlowBox* RootLineBox::box() const
{
    return static_cast<BlockFlowBox*>(m_box);
}

void RootLineBox::updateLineTopAndBottom(const LineBox* line)
{
    m_lineTop = std::min(m_lineTop, line->y());
    m_lineBottom = std::max(m_lineBottom, line->bottom());
}

float RootLineBox::alignInHorizontalDirection(float startOffset)
{
    return placeInHorizontalDirection(startOffset, box());
}

float RootLineBox::alignInVerticalDirection(FragmentBuilder* fragmentainer, float blockHeight)
{
    float maxAscent = 0.f;
    float maxDescent = 0.f;
    float maxPositionTop = 0.f;
    float maxPositionBottom = 0.f;
    computeMaxAscentAndDescent(maxAscent, maxDescent, maxPositionTop, maxPositionBottom);
    if(maxAscent + maxDescent < std::max(maxPositionTop, maxPositionBottom)) {
        adjustMaxAscentAndDescent(maxAscent, maxDescent, maxPositionTop, maxPositionBottom);
    }

    auto maxHeight = maxAscent + maxDescent;
    if(fragmentainer && maxHeight > 0.f)
        blockHeight += adjustLineBoxInFragmentFlow(fragmentainer, blockHeight, maxHeight);
    m_lineTop = blockHeight;
    m_lineBottom = blockHeight;
    placeInVerticalDirection(blockHeight, maxHeight, maxAscent, this);
    return blockHeight + maxHeight;
}

float RootLineBox::adjustLineBoxInFragmentFlow(FragmentBuilder* fragmentainer, float offset, float lineHeight) const
{
    auto fragmentHeight = fragmentainer->fragmentHeightForOffset(offset);
    fragmentainer->updateMinimumFragmentHeight(offset, lineHeight);
    if(fragmentHeight == 0.f || lineHeight > fragmentHeight)
        return 0.f;
    auto remainingHeight = fragmentainer->fragmentRemainingHeightForOffset(offset, AssociateWithLatterFragment);
    if(remainingHeight < lineHeight) {
        fragmentainer->setFragmentBreak(offset, lineHeight - remainingHeight);
        return remainingHeight;
    }

    if(!isFirstLine() && isNearlyEqual(remainingHeight, fragmentHeight))
        fragmentainer->setFragmentBreak(offset, lineHeight);
    return 0.f;
}

RootLineBox::RootLineBox(BlockFlowBox* box)
    : FlowLineBox(box)
{
}

} // namespace plutobook
