/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "inlinebox.h"
#include "blockbox.h"
#include "linebox.h"

namespace plutobook {

InlineBox::InlineBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxModel(node, style)
    , m_lines(style->heap())
{
    setIsInline(true);
}

InlineBox::~InlineBox() = default;

bool InlineBox::requiresLayer() const
{
    return isRelativePositioned() || style()->hasOpacity() || style()->hasBlendMode();
}

Rect InlineBox::visualOverflowRect() const
{
    if(m_lines.empty()) {
        return Rect(0, 0, 0, 0);
    }

    float leftSide = 0.f;
    float rightSide = 0.f;

    const auto& firstLine = m_lines.front();
    const auto& lastLine = m_lines.back();
    for(const auto& line : m_lines) {
        if(line == firstLine || leftSide > line->overflowLeft())
            leftSide = line->overflowLeft();
        if(line == firstLine || rightSide < line->overflowRight()) {
            rightSide = line->overflowRight();
        }
    }

    auto width = rightSide - leftSide;
    auto height = lastLine->overflowBottom() - firstLine->overflowTop();
    return Rect(leftSide, firstLine->overflowTop(), width, height);
}

Rect InlineBox::borderBoundingBox() const
{
    return Rect(paintBoundingBox().size());
}

Rect InlineBox::paintBoundingBox() const
{
    if(m_lines.empty()) {
        return Rect(0, 0, 0, 0);
    }

    float leftSide = 0.f;
    float rightSide = 0.f;

    const auto& firstLine = m_lines.front();
    const auto& lastLine = m_lines.back();
    for(const auto& line : m_lines) {
        if(line == firstLine || leftSide > line->x())
            leftSide = line->x();
        if(line == firstLine || rightSide < line->right()) {
            rightSide = line->right();
        }
    }

    auto width = rightSide - leftSide;
    auto height = lastLine->bottom() - firstLine->y();
    return Rect(leftSide, firstLine->y(), width, height);
}

Point InlineBox::relativePositionedInlineOffset(const BoxModel* child) const
{
    if(m_lines.empty()) {
        return Point(0, 0);
    }

    const auto& firstLine = m_lines.front();
    const auto* childStyle = child->style();

    Point offset;
    if(!childStyle->left().isAuto() || !childStyle->right().isAuto())
        offset.x = firstLine->x();
    if(!childStyle->top().isAuto() || !childStyle->bottom().isAuto()) {
        offset.y = firstLine->y();
    }

    return offset;
}

float InlineBox::innerPaddingBoxWidth() const
{
    if(m_lines.empty()) {
        return 0.f;
    }

    const auto& firstLine = m_lines.front();
    const auto& lastLine = m_lines.back();
    if(style()->isLeftToRightDirection())
        return std::max(0.f, lastLine->right() - firstLine->x() - borderLeft() - borderRight());
    return std::max(0.f, firstLine->right() - lastLine->x() - borderLeft() - borderRight());
}

float InlineBox::innerPaddingBoxHeight() const
{
    if(m_lines.empty()) {
        return 0.f;
    }

    const auto& firstLine = m_lines.front();
    const auto& lastLine = m_lines.back();
    return std::max(0.f, lastLine->bottom() - firstLine->y() - borderTop() - borderBottom());
}

void InlineBox::addChild(Box* newChild)
{
    if(m_continuation) {
        m_continuation->addChild(newChild);
        return;
    }

    if(newChild->isInline() || newChild->isFloatingOrPositioned()) {
        BoxModel::addChild(newChild);
        return;
    }

    auto container = containingBlock();
    if(isContinuation() && !firstChild()) {
        auto prevBlock = container->prevSibling();
        assert(prevBlock && prevBlock->isContinuation());
        prevBlock->addChild(newChild);
        return;
    }

    auto preBlock = container;
    if(container->isAnonymousBlock()) {
        container = container->containingBlock();
        assert(container->lastChild() == preBlock);
    } else {
        preBlock = createAnonymousBlock(container->style());
        container->moveChildrenTo(preBlock);
        preBlock->setIsChildrenInline(container->isChildrenInline());
        container->setIsChildrenInline(false);
        container->appendChild(preBlock);
    }

    auto isRelative = isRelativePositioned();

    auto continuation = createContinuation();
    for(auto parent = parentBox(); parent != preBlock; parent = parent->parentBox()) {
        auto parentContinuation = to<InlineBox>(*parent).createContinuation();
        parentContinuation->appendChild(continuation);
        continuation = parentContinuation;
        isRelative |= parent->isRelativePositioned();
    }

    auto middleBlock = createAnonymousBlock(style());
    middleBlock->setIsContinuation(true);
    if(isRelative) {
        middleBlock->style()->setPosition(Position::Relative);
    }

    container->appendChild(middleBlock);
    middleBlock->addChild(newChild);

    auto postBlock = createAnonymousBlock(container->style());
    container->appendChild(postBlock);
    postBlock->appendChild(continuation);
}

void InlineBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(phase == PaintPhase::Contents || phase == PaintPhase::Outlines) {
        for(const auto& line : m_lines) {
            line->paint(info, offset, phase);
        }
    }
}

InlineBox* InlineBox::createContinuation()
{
    assert(m_continuation == nullptr);
    m_continuation = new (heap()) InlineBox(nullptr, style());
    m_continuation->setIsContinuation(true);
    return m_continuation;
}

} // namespace plutobook
