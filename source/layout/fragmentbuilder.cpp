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
    if(height > remainingHeight) {
        setFragmentBreak(offset, height - remainingHeight);
        if(unbreakableHeight > remainingHeight && remainingHeight < fragmentHeight)
            return offset + remainingHeight;
    } else if(isNearlyEqual(fragmentHeight, remainingHeight) && !isNearlyZero(offset + fragmentOffset())) {
        setFragmentBreak(offset, height);
    }

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

static bool isInFlowContent(const Box* box)
{
    return !box->isFloatingOrPositioned() && !box->isTableColumnBox();
}

static bool isListMarkerWrapper(const Box* box)
{
    if(!box->isAnonymousBlock())
        return false;
    if(auto child = box->firstChild())
        return child == box->lastChild() && child->isOutsideListMarkerBox();
    return false;
}

static bool hasInFlowContentBefore(const Box* box)
{
    for(; box && !box->isBoxView(); box = box->parentBox()) {
        for(auto sibling = box->prevSibling(); sibling; sibling = sibling->prevSibling()) {
            if(isInFlowContent(sibling) && !isListMarkerWrapper(sibling)) {
                return true;
            }
        }

        auto parent = box->parentBox();
        if(parent == nullptr || !isBreakPropagatingContainer(parent)) {
            return true;
        }

        if(parent->isMultiColumnFlowBox() || !isInFlowContent(parent)) {
            return false;
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

        if(parent->isMultiColumnFlowBox() || !isInFlowContent(parent)) {
            return false;
        }
    }

    return false;
}

static bool receivesPropagatedBreaks(const Box* box)
{
    if(box->isBoxView())
        return true;
    if(!box->isBlockFlowBox() || box->isChildrenInline() || box->isMultiColumnFlowBox() || box->isTableCellBox())
        return false;
    if(!isInFlowContent(box))
        return false;
    auto parent = box->parentBox();
    if(parent == nullptr)
        return false;
    return (parent->isBlockFlowBox() && !parent->isChildrenInline()) || parent->isFlexibleBox();
}

static const Box* firstInFlowChild(const Box* box)
{
    for(auto child = box->firstChild(); child; child = child->nextSibling()) {
        if(isInFlowContent(child) && !isListMarkerWrapper(child)) {
            return child;
        }
    }

    return nullptr;
}

static const Box* lastInFlowChild(const Box* box)
{
    for(auto child = box->lastChild(); child; child = child->prevSibling()) {
        if(isInFlowContent(child)) {
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
