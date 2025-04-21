#include "fragmentbuilder.h"
#include "box.h"

namespace plutobook {

constexpr bool needsFragmentBreakBetween(BreakBetween between, FragmentType type)
{
    if(type == FragmentType::Column)
        return between == BreakBetween::Column;
    return between >= BreakBetween::Page;
}

constexpr bool needsFragmentBreakInside(BreakInside inside, FragmentType type)
{
    if(type == FragmentType::Page)
        return inside == BreakInside::Avoid || inside == BreakInside::AvoidPage;
    return inside == BreakInside::Avoid || inside == BreakInside::AvoidColumn;
}

float FragmentBuilder::applyFragmentBreakBefore(const BoxFrame* child, float offset)
{
    if(!needsFragmentBreakBetween(child->style()->breakBefore(), fragmentType()))
        return offset;
    auto fragmentHeight = fragmentHeightForOffset(offset);
    addForcedFragmentBreak(offset);
    if(fragmentHeight > 0.f)
        offset += fragmentRemainingHeightForOffset(offset, AssociateWithFormerFragment);
    return offset;
}

float FragmentBuilder::applyFragmentBreakAfter(const BoxFrame* child, float offset)
{
    if(!needsFragmentBreakBetween(child->style()->breakAfter(), fragmentType()))
        return offset;
    auto fragmentHeight = fragmentHeightForOffset(offset);
    addForcedFragmentBreak(offset);
    if(fragmentHeight > 0.f)
        offset += fragmentRemainingHeightForOffset(offset, AssociateWithFormerFragment);
    return offset;
}

float FragmentBuilder::applyFragmentBreakInside(const BoxFrame* child, float offset)
{
    if(!child->isReplaced() && !needsFragmentBreakInside(child->style()->breakInside(), fragmentType()))
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

void FragmentBuilder::addForcedFragmentBreak(float offset)
{
}

void FragmentBuilder::setFragmentBreak(float offset, float spaceShortage)
{
}

void FragmentBuilder::updateMinimumFragmentHeight(float offset, float minHeight)
{
}

} // namespace plutobook
