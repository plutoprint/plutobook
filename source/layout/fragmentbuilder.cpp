/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "fragmentbuilder.h"
#include "geometry.h"

#include <cmath>

namespace plutobook {

float FragmentBuilder::applyFragmentBreakBefore(const BoxFrame* child, float offset)
{
    if(!alwaysBreakBefore(child))
        return offset;
    auto fragmentHeight = fragmentHeightForOffset(offset);
    addForcedFragmentBreak(offset);
    if(fragmentHeight > 0.f)
        offset += fragmentRemainingHeightForOffset(offset, AssociateWithFormerFragment);
    return offset;
}

float FragmentBuilder::applyFragmentBreakAfter(const BoxFrame* child, float offset)
{
    if(!alwaysBreakAfter(child))
        return offset;
    auto fragmentHeight = fragmentHeightForOffset(offset);
    addForcedFragmentBreak(offset);
    if(fragmentHeight > 0.f)
        offset += fragmentRemainingHeightForOffset(offset, AssociateWithFormerFragment);
    return offset;
}

float FragmentBuilder::applyFragmentBreakInside(const BoxFrame* child, float offset)
{
    if(!avoidsBreakInside(child))
        return offset;
    auto childHeight = child->height();
    if(child->isFloating())
        childHeight += child->marginHeight();
    return adjustOffsetInFragmentFlow(offset, childHeight, childHeight);
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

constexpr double kFragmentFixedScale = 65536;

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

static bool isBreakPropagatingContainer(const Box* box)
{
    return box->isBlockFlowBox() || box->isFlexibleBox()
        || box->isTableBox() || box->isTableSectionBox() || box->isTableRowBox();
}

static bool hasInFlowContentBefore(const Box* box)
{
    for(; box && !box->isBoxView(); box = box->parentBox()) {
        if(box->hasBreakPointBefore())
            return true;
        if(auto parent = box->parentBox()) {
            if(!isBreakPropagatingContainer(parent))
                return true;
            if(parent->isFloatingOrPositioned() || parent->isMultiColumnFlowBox()) {
                return false;
            }
        }
    }

    return false;
}

static bool hasInFlowContentAfter(const Box* box)
{
    for(; box && !box->isBoxView(); box = box->parentBox()) {
        if(box->hasBreakPointAfter())
            return true;
        if(auto parent = box->parentBox()) {
            if(!isBreakPropagatingContainer(parent))
                return true;
            if(parent->isFloatingOrPositioned() || parent->isMultiColumnFlowBox()) {
                return false;
            }
        }
    }

    return false;
}

static bool receivesPropagatedBreaks(const Box* box)
{
    if(box->isBlockFlowBox() && !box->isFloatingOrPositioned() && !box->isChildrenInline()) {
        if(box->isTableCellBox() || box->isMultiColumnFlowBox())
            return false;
        if(auto parent = box->parentBox()) {
            return parent->isFlexibleBox() || (parent->isBlockFlowBox() && !parent->isChildrenInline());
        }
    }

    return box->isBoxView();
}

static const Box* firstInFlowChild(const Box* box)
{
    for(auto child = box->firstChild(); child; child = child->nextSibling()) {
        if(child->isInFlowChild()) {
            return child;
        }
    }

    return nullptr;
}

static const Box* lastInFlowChild(const Box* box)
{
    for(auto child = box->lastChild(); child; child = child->prevSibling()) {
        if(child->isInFlowChild()) {
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
