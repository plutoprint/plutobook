/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "fragmentbuilder.h"

#include <cmath>

namespace plutobook {

static bool isBreakPropagatingContainer(const Box* box)
{
    return box->isBlockFlowBox() || box->isFlexibleBox()
        || box->isTableBox() || box->isTableSectionBox() || box->isTableRowBox();
}

static bool isInFlowContent(const Box* box)
{
    return !box->isFloatingOrPositioned() && !box->isTableColumnBox();
}

static bool hasInFlowContentBefore(const Box* box)
{
    for(; box && !box->isBoxView(); box = box->parentBox()) {
        for(auto sibling = box->prevSibling(); sibling; sibling = sibling->prevSibling()) {
            if(isInFlowContent(sibling)) {
                return true;
            }
        }

        auto parent = box->parentBox();
        if(parent == nullptr || !isBreakPropagatingContainer(parent)) {
            return true;
        }
    }

    return false;
}

static bool hasInFlowContentAfter(const Box* box)
{
    for(; box && !box->isBoxView(); box = box->parentBox()) {
        for(auto sibling = box->nextSibling(); sibling; sibling = sibling->nextSibling()) {
            if(isInFlowContent(sibling)) {
                return true;
            }
        }

        auto parent = box->parentBox();
        if(parent == nullptr || !isBreakPropagatingContainer(parent)) {
            return true;
        }
    }

    return false;
}

bool FragmentBuilder::needsBreakBefore(const BoxFrame* child) const
{
    return needsBreakBetween(child->style()->breakBefore()) && hasInFlowContentBefore(child);
}

bool FragmentBuilder::needsBreakAfter(const BoxFrame* child) const
{
    return needsBreakBetween(child->style()->breakAfter()) && hasInFlowContentAfter(child);
}

bool FragmentBuilder::needsBreakInside(const BoxFrame* child) const
{
    return child->isReplaced() || needsBreakInside(child->style()->breakInside());
}

float FragmentBuilder::applyFragmentBreakBefore(const BoxFrame* child, float offset)
{
    if(!needsBreakBefore(child))
        return offset;
    auto fragmentHeight = fragmentHeightForOffset(offset);
    addForcedFragmentBreak(offset);
    if(fragmentHeight > 0.f)
        offset += fragmentRemainingHeightForOffset(offset, AssociateWithFormerFragment);
    return offset;
}

float FragmentBuilder::applyFragmentBreakAfter(const BoxFrame* child, float offset)
{
    if(!needsBreakAfter(child))
        return offset;
    auto fragmentHeight = fragmentHeightForOffset(offset);
    addForcedFragmentBreak(offset);
    if(fragmentHeight > 0.f)
        offset += fragmentRemainingHeightForOffset(offset, AssociateWithFormerFragment);
    return offset;
}

float FragmentBuilder::applyFragmentBreakInside(const BoxFrame* child, float offset)
{
    if(!needsBreakInside(child))
        return offset;
    auto childHeight = child->height();
    if(child->isFloating())
        childHeight += child->marginHeight();
    auto fragmentHeight = fragmentHeightForOffset(offset);
    updateMinimumFragmentHeight(offset, childHeight);
    if(fragmentHeight == 0.f)
        return offset;
    auto remainingHeight = fragmentRemainingHeightForOffset(offset, AssociateWithLatterFragment);
    if(remainingHeight < childHeight && remainingHeight < fragmentHeight)
        return offset + remainingHeight;
    return offset;
}

constexpr double kFragmentFixedScale = 1000;

void FragmentBuilder::enterFragment(float offset)
{
    m_fragmentOffset += llround(offset * kFragmentFixedScale);
}

void FragmentBuilder::leaveFragment(float offset)
{
    m_fragmentOffset -= llround(offset * kFragmentFixedScale);
}

float FragmentBuilder::fragmentOffset() const
{
    return m_fragmentOffset / kFragmentFixedScale;
}

bool FragmentBuilder::needsBreakBetween(BreakBetween between) const
{
    if(fragmentType() == FragmentType::Column)
        return between == BreakBetween::Column;
    return between >= BreakBetween::Page;
}

bool FragmentBuilder::needsBreakInside(BreakInside inside) const
{
    if(fragmentType() == FragmentType::Page)
        return inside == BreakInside::Avoid || inside == BreakInside::AvoidPage;
    return inside == BreakInside::Avoid || inside == BreakInside::AvoidColumn;
}

} // namespace plutobook
