#include "fragmentbuilder.h"

#include "plutobook.hpp"

namespace plutobook {

PageBuilder::PageBuilder(const Book* book)
    : m_book(book)
{
}

float PageBuilder::applyFragmentBreakBefore(const BoxFrame* child, float offset)
{
    return offset;
}

float PageBuilder::applyFragmentBreakAfter(const BoxFrame* child, float offset)
{
    return offset;
}

float PageBuilder::applyFragmentBreakInside(const BoxFrame* child, float offset)
{
    return offset;
}

float PageBuilder::fragmentHeightForOffset(float offset) const
{
    return 0.f;
}

float PageBuilder::fragmentRemainingHeightForOffset(float offset, FragmentBoundaryRule rule) const
{
    return 0.f;
}

void PageBuilder::addForcedFragmentBreak(float offset)
{
}

void PageBuilder::setFragmentBreak(float offset, float spaceShortage)
{
}

void PageBuilder::updateMinimumFragmentHeight(float offset, float minHeight)
{
}

void PageBuilder::enterFragment(const BoxFrame* child, float offset)
{
}

void PageBuilder::leaveFragment(const BoxFrame* child, float offset)
{
}

} // namespace plutobook
