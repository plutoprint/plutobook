#include "fragmentbuilder.h"
#include "document.h"
#include "boxview.h"
#include "pagebox.h"

#include "plutobook.hpp"

namespace plutobook {

PageBuilder::PageBuilder(const Book* book)
    : m_book(book)
{
    auto document = book->document();
    auto style = document->styleForPage(emptyGlo, 0, PseudoType::FirstPage);
    auto page = PageBox::create(style, emptyGlo, 0);

    page->build();
    page->layout(nullptr);
    page->setPageTop(0);
    page->setPageBottom(page->height());

    document->box()->setCurrentPage(page.get());
    document->pages().push_back(std::move(page));
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
    FragmentBuilder::enterFragment(child, offset);
}

void PageBuilder::leaveFragment(const BoxFrame* child, float offset)
{
    FragmentBuilder::leaveFragment(child, offset);
}

} // namespace plutobook
