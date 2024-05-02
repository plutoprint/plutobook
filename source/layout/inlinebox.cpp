#include "inlinebox.h"
#include "blockbox.h"
#include "linebox.h"

namespace plutobook {

InlineBox::InlineBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxModel(node, style)
    , m_lines(style->heap())
{
    setInline(true);
}

InlineBox::~InlineBox() = default;

bool InlineBox::requiresLayer() const
{
    return isRelPositioned() || style()->hasOpacity() || style()->hasBlendMode();
}

Rect InlineBox::visualOverflowRect() const
{
    if(m_lines.empty()) {
        return Rect(0, 0, 0, 0);
    }

    float leftSide = 0.f;
    float rightSide = 0.f;

    auto& firstLine = m_lines.front();
    auto& lastLine = m_lines.back();
    for(auto& line : m_lines) {
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
    return Rect(linesBoundingBox().size());
}

Rect InlineBox::linesBoundingBox() const
{
    if(m_lines.empty()) {
        return Rect(0, 0, 0, 0);
    }

    float leftSide = 0.f;
    float rightSide = 0.f;

    auto& firstLine = m_lines.front();
    auto& lastLine = m_lines.back();
    for(auto& line : m_lines) {
        if(line == firstLine || leftSide > line->x())
            leftSide = line->x();
        if(line == firstLine || rightSide < line->x() + line->width()) {
            rightSide = line->x() + line->width();
        }
    }

    auto width = rightSide - leftSide;
    auto height = lastLine->y() + lastLine->height() - firstLine->y();
    return Rect(leftSide, firstLine->y(), width, height);
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
        preBlock->setChildrenInline(container->isChildrenInline());
        container->setChildrenInline(false);

        container->appendChild(preBlock);
        container->appendChild(middleBlock);
        container->appendChild(postBlock);
    }

    middleBlock->addChild(newChild);

    auto clone = new (heap()) InlineBox(nullptr, style());
    auto currentParent = parentBox();
    auto currentClone = clone;
    while(currentParent != preBlock) {
        auto& parent = to<InlineBox>(*currentParent);
        assert(parent.continuation() == nullptr);
        auto parentClone = new (heap()) InlineBox(nullptr, parent.style());
        parentClone->appendChild(currentClone);
        parent.setContinuation(parentClone);

        currentClone = parentClone;
        currentParent = currentParent->parentBox();
    }

    postBlock->appendChild(currentClone);
    setContinuation(clone);
}

void InlineBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(phase == PaintPhase::Contents || phase == PaintPhase::Outlines) {
        for(auto& line : m_lines) {
            line->paint(info, offset, phase);
        }
    }
}

} // namespace plutobook
