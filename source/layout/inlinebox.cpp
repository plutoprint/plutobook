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
        return lastLine->right() - firstLine->x() - borderLeft() - borderRight();
    return firstLine->right() - lastLine->x() - borderLeft() - borderRight();
}

float InlineBox::innerPaddingBoxHeight() const
{
    if(m_lines.empty()) {
        return 0.f;
    }

    const auto& firstLine = m_lines.front();
    const auto& lastLine = m_lines.back();
    return lastLine->bottom() - firstLine->y() - borderTop() - borderBottom();
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

    BlockBox* preBlock = nullptr;
    BlockBox* middleBlock = nullptr;
    BlockBox* postBlock = nullptr;

    auto container = containingBlock();
    if(container->isAnonymousBlock()) {
        preBlock = container;
        middleBlock = createAnonymousBlock(style());
        postBlock = createAnonymousBlock(container->style());

        container = container->containingBlock();
        assert(container->lastChild() == preBlock);
        container->appendChild(middleBlock);
        container->appendChild(postBlock);
    } else {
        preBlock = createAnonymousBlock(container->style());
        middleBlock = createAnonymousBlock(style());
        postBlock = createAnonymousBlock(container->style());

        container->moveChildrenTo(preBlock);
        preBlock->setIsChildrenInline(container->isChildrenInline());
        container->setIsChildrenInline(false);

        container->appendChild(preBlock);
        container->appendChild(middleBlock);
        container->appendChild(postBlock);
    }

    middleBlock->addChild(newChild);

    auto clone = new (heap()) InlineBox(nullptr, style());
    auto currentParent = parentBox();
    auto currentClone = clone;
    while(currentParent != preBlock) {
        auto parentClone = new (heap()) InlineBox(nullptr, currentParent->style());
        parentClone->appendChild(currentClone);
        currentClone = parentClone;

        auto parent = to<InlineBox>(currentParent);
        assert(parent && !parent->continuation());
        parent->setContinuation(parentClone);

        currentParent = currentParent->parentBox();
    }

    postBlock->appendChild(currentClone);
    setContinuation(clone);
}

void InlineBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(phase == PaintPhase::Contents || phase == PaintPhase::Outlines) {
        for(const auto& line : m_lines) {
            line->paint(info, offset, phase);
        }
    }
}

} // namespace plutobook
