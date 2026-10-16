/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "formcontrolbox.h"
#include "fragmentbuilder.h"
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
    height = m_rows * style()->lineHeightValue() + borderAndPaddingHeight();
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

        child->updateHorizontalMargins(0.f);
        child->updateHorizontalPaddings(0.f);

        auto childMinWidth = child->minPreferredWidth() + child->marginWidth();
        auto childMaxWidth = child->maxPreferredWidth() + child->marginWidth();

        minWidth = std::max(minWidth, childMinWidth);
        maxWidth = std::max(maxWidth, childMaxWidth);
    }

    minWidth = std::max(0.f, minWidth);
    maxWidth = std::max(minWidth, maxWidth);
}

void SelectBox::paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(!child->hasLayer()) {
            child->paint(info, offset, phase);
        }
    }
}

void SelectBox::layout(FragmentBuilder* fragmentainer)
{
    updateWidth();
    setHeight(borderAndPaddingHeight());

    unsigned int index = 0;
    auto optionTop = borderAndPaddingTop();
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned()) {
            auto childLayer = child->layer();
            childLayer->setStaticTop(optionTop);
            childLayer->setStaticLeft(startOffsetForContent());
            child->containingBlock()->insertPositonedBox(child);
            continue;
        }

        child->updatePaddingWidths(availableWidth());
        child->updateVerticalMargins(availableWidth());

        optionTop += child->marginTop();
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
        optionTop += child->height() + child->marginBottom();
        if(index < m_size)
            setHeight(height() + child->marginBoxHeight());
        if(child->isSelectItem()) {
            ++index;
        }
    }

    updateHeight();
    layoutPositionedBoxes();
    updateOverflowRect();
}

void SelectBox::build()
{
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        if(child->isPositioned())
            continue;
        auto element = to<HTMLElement>(child->node());
        if(element && (element->tagName() == optionTag || element->tagName() == optgroupTag)) {
            child->setIsSelectItem(true);
        }
    }

    for(auto child = firstChild(); child; child = child->nextSibling()) {
        child->setIsFirstInFlowChild(child->isSelectItem());
        if(child->isFirstInFlowChild()) {
            break;
        }
    }

    for(auto child = lastChild(); child; child = child->prevSibling()) {
        child->setIsLastInFlowChild(child->isSelectItem());
        if(child->isLastInFlowChild()) {
            break;
        }
    }

    BlockBox::build();
}

} // namespace plutobook
