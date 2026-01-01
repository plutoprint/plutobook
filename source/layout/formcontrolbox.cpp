/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "formcontrolbox.h"
#include "htmldocument.h"
#include "boxlayer.h"

namespace plutobook {

TextInputBox::TextInputBox(HTMLElement* element, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(element, style)
{
    setIsOverflowHidden(true);
}

HTMLElement* TextInputBox::element() const
{
    return static_cast<HTMLElement*>(node());
}

std::optional<float> TextInputBox::inlineBlockBaseline() const
{
    if(m_rows == 1)
        return firstLineBaseline();
    return std::nullopt;
}

void TextInputBox::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    minWidth = maxWidth = m_cols * style()->chFontSize();
}

void TextInputBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    height = m_rows * style()->lineHeight() + borderAndPaddingHeight();
    BlockFlowBox::computeHeight(y, height, marginTop, marginBottom);
}

SelectBox::SelectBox(HTMLSelectElement* element, const RefPtr<BoxStyle>& style)
    : BlockBox(element, style)
    , m_size(element->size())
{
}

HTMLSelectElement* SelectBox::element() const
{
    return static_cast<HTMLSelectElement*>(node());
}

std::optional<float> SelectBox::inlineBlockBaseline() const
{
    if(m_size == 1)
        return firstLineBaseline();
    return std::nullopt;
}

void SelectBox::addChild(Box* newChild)
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

void SelectBox::updateOverflowRect()
{
    BlockBox::updateOverflowRect();
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(!child->isPositioned()) {
            addOverflowRect(child, child->x(), child->y());
        }
    }
}

void SelectBox::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned()) {
            continue;
        }

        child->updateHorizontalMargins(nullptr);
        child->updateHorizontalPaddings(nullptr);

        auto childMinWidth = child->minPreferredWidth() + child->marginWidth();
        auto childMaxWidth = child->maxPreferredWidth() + child->marginWidth();

        minWidth = std::max(minWidth, childMinWidth);
        maxWidth = std::max(maxWidth, childMaxWidth);
    }

    minWidth = std::max(0.f, minWidth);
    maxWidth = std::max(minWidth, maxWidth);
}

void SelectBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    unsigned int index = 0;
    auto child = firstBoxFrame();

    height = borderAndPaddingHeight();
    while(child && index < m_size) {
        if(!child->isPositioned())
            height += child->marginBoxHeight();
        child = child->nextBoxFrame();
        ++index;
    }

    BlockBox::computeHeight(y, height, marginTop, marginBottom);
}

void SelectBox::paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    unsigned int index = 0;
    auto child = firstBoxFrame();
    while(child && index < m_size) {
        if(!child->hasLayer())
            child->paint(info, offset, phase);
        child = child->nextBoxFrame();
        ++index;
    }
}

void SelectBox::layout(FragmentBuilder* fragmentainer)
{
    updateWidth();
    setHeight(borderAndPaddingTop());
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned()) {
            auto childLayer = child->layer();
            childLayer->setStaticTop(height());
            childLayer->setStaticLeft(startOffsetForContent());
            child->containingBlock()->insertPositonedBox(child);
            continue;
        }

        child->updatePaddingWidths(this);
        child->updateVerticalMargins(this);

        auto optionTop = height() + child->marginTop();
        if(fragmentainer)
            fragmentainer->enterFragment(optionTop);
        child->setY(optionTop);
        child->layout(fragmentainer);
        if(fragmentainer) {
            fragmentainer->leaveFragment(optionTop);
        }

        child->setX(borderStart() + paddingStart() + child->marginStart(style()->direction()));
        if(style()->isRightToLeftDirection())
            child->setX(width() - child->x() - child->width());
        setHeight(child->y() + child->height() + child->marginBottom());
    }

    setHeight(height() + borderAndPaddingBottom());
    updateHeight();
    layoutPositionedBoxes();
    updateOverflowRect();
}

} // namespace plutobook
