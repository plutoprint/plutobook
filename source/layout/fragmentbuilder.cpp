#include "fragmentbuilder.h"
#include "box.h"

namespace plutobook {

float FragmentBuilder::applyFragmentBreakBefore(const BoxFrame* child, float offset)
{
    if(child->style()->pageBreakBefore() == BreakBetween::Always)
        offset += fragmentRemainingHeightForOffset(offset, AssociateWithFormerFragment);
    return offset;
}

float FragmentBuilder::applyFragmentBreakAfter(const BoxFrame* child, float offset)
{
    if(child->style()->pageBreakAfter() == BreakBetween::Always)
        offset += fragmentRemainingHeightForOffset(offset, AssociateWithFormerFragment);
    return offset;
}

float FragmentBuilder::applyFragmentBreakInside(const BoxFrame* child, float offset)
{
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
