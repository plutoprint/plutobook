/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "fragmentbuilder.h"
#include "geometry.h"

namespace plutobook {

float FragmentBuilder::applyForcedFragmentBreak(float offset)
{
    auto fragmentHeight = fragmentHeightForOffset(offset);
    addForcedFragmentBreak(offset);
    if(fragmentHeight > 0.f) {
        auto rule = AssociateWithFormerFragment;
        if(isNearlyZero(offset + fragmentOffset()))
            rule = AssociateWithLatterFragment;
        offset += fragmentRemainingHeightForOffset(offset, rule);
    }

    return offset;
}

float FragmentBuilder::applyFragmentBreakBefore(const BoxFrame* child, float offset)
{
    if(!alwaysBreakBefore(child))
        return offset;
    return applyForcedFragmentBreak(offset);
}

float FragmentBuilder::applyFragmentBreakAfter(const BoxFrame* child, float offset)
{
    if(!alwaysBreakAfter(child))
        return offset;
    return applyForcedFragmentBreak(offset);
}

float FragmentBuilder::applyFragmentBreakInside(const BoxFrame* child, float offset)
{
    if(!avoidsBreakInside(child))
        return offset;
    return adjustOffsetInFragmentFlow(offset, child->height(), child->height());
}

float FragmentBuilder::adjustOffsetInFragmentFlow(float offset, float height, float unbreakableHeight)
{
    auto fragmentHeight = fragmentHeightForOffset(offset);
    if(unbreakableHeight > 0.f)
        updateMinimumFragmentHeight(offset, unbreakableHeight);
    if(fragmentHeight <= 0.f)
        return offset;
    auto remainingHeight = fragmentRemainingHeightForOffset(offset, AssociateWithLatterFragment);

    auto isAtFragmentStart = isNearlyEqual(remainingHeight, fragmentHeight);
    if(height <= remainingHeight) {
        if(isAtFragmentStart && !isNearlyZero(offset + fragmentOffset()))
            setFragmentBreak(offset, height);
        return offset;
    }

    setFragmentBreak(offset, height - remainingHeight);
    if(unbreakableHeight > remainingHeight && !isAtFragmentStart)
        return offset + remainingHeight;
    return offset;
}

void FragmentBuilder::enterFragment(float offset)
{
    m_fragmentOffset += offset;
}

void FragmentBuilder::leaveFragment(float offset)
{
    m_fragmentOffset -= offset;
}

float FragmentBuilder::fragmentOffset() const
{
    return m_fragmentOffset;
}

static bool hasInFlowContentBefore(const Box* box)
{
    while(box) {
        const auto* parent = box->parentBox();
        if(parent == nullptr)
            return false;
        if(!box->isFirstInFlowChild())
            return true;
        if(parent->isFloatingOrPositioned() || parent->isMultiColumnFlowBox())
            return false;
        box = parent;
    }

    return false;
}

static bool hasInFlowContentAfter(const Box* box)
{
    while(box) {
        const auto* parent = box->parentBox();
        if(parent == nullptr)
            return false;
        if(!box->isLastInFlowChild())
            return true;
        if(parent->isFloatingOrPositioned() || parent->isMultiColumnFlowBox())
            return false;
        box = parent;
    }

    return false;
}

static bool receivesPropagatedBreaks(const Box* box)
{
    const auto* parent = box->parentBox();
    if(parent == nullptr)
        return true;
    if(!box->isBlockFlowBox() || box->isChildrenInline()
        || box->isFloatingOrPositioned() || box->isMultiColumnFlowBox()) {
        return false;
    }

    return parent->isFlexibleBox() || parent->isTableRowBox()
        || (parent->isBlockFlowBox() && !parent->isChildrenInline());
}

static const Box* firstInFlowChild(const Box* box)
{
    for(auto child = box->firstChild(); child; child = child->nextSibling()) {
        if(child->isFirstInFlowChild()) {
            return child;
        }
    }

    return nullptr;
}

static const Box* lastInFlowChild(const Box* box)
{
    for(auto child = box->lastChild(); child; child = child->prevSibling()) {
        if(child->isLastInFlowChild()) {
            return child;
        }
    }

    return nullptr;
}

static bool propagatesBreakBefore(const Box* box)
{
    if(auto parent = box->parentBox())
        return receivesPropagatedBreaks(parent) && box == firstInFlowChild(parent);
    return false;
}

static bool propagatesBreakAfter(const Box* box)
{
    if(auto parent = box->parentBox())
        return receivesPropagatedBreaks(parent) && box == lastInFlowChild(parent);
    return false;
}

bool FragmentBuilder::hasForcedBreakBefore(const Box* box) const
{
    for(; box; box = firstInFlowChild(box)) {
        if(alwaysBreakBetween(box->style()->breakBefore()))
            return true;
        if(!receivesPropagatedBreaks(box)) {
            break;
        }
    }

    return false;
}

bool FragmentBuilder::hasForcedBreakAfter(const Box* box) const
{
    for(; box; box = lastInFlowChild(box)) {
        if(alwaysBreakBetween(box->style()->breakAfter()))
            return true;
        if(!receivesPropagatedBreaks(box)) {
            break;
        }
    }

    return false;
}

bool FragmentBuilder::alwaysBreakBefore(const BoxFrame* child) const
{
    return !propagatesBreakBefore(child) && hasForcedBreakBefore(child) && hasInFlowContentBefore(child);
}

bool FragmentBuilder::alwaysBreakAfter(const BoxFrame* child) const
{
    return !propagatesBreakAfter(child) && hasForcedBreakAfter(child) && hasInFlowContentAfter(child);
}

bool FragmentBuilder::avoidsBreakInside(const BoxFrame* child) const
{
    return child->isReplaced() || avoidsBreakInside(child->style()->breakInside());
}

bool FragmentBuilder::alwaysBreakBetween(BreakBetween between) const
{
    if(fragmentType() == FragmentType::Column)
        return between == BreakBetween::Column;
    return between >= BreakBetween::Page;
}

bool FragmentBuilder::avoidsBreakInside(BreakInside inside) const
{
    if(fragmentType() == FragmentType::Page)
        return inside == BreakInside::Avoid || inside == BreakInside::AvoidPage;
    return inside == BreakInside::Avoid || inside == BreakInside::AvoidColumn;
}

} // namespace plutobook
