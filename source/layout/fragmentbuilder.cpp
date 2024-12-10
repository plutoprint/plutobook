#include "fragmentbuilder.h"
#include "document.h"
#include "boxview.h"
#include "pagebox.h"

#include "plutobook.hpp"

namespace plutobook {

PageBuilder::PageBuilder(const Book* book)
    : m_book(book)
    , m_document(book->document())
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
    FragmentBuilder::enterFragment(child, offset);
    if(m_currentPage && m_currentPage->pageTop() <= fragmentOffset())
        return;
    auto pageStyle = m_document->styleForPage(emptyGlo, 0, PseudoType::FirstPage);
    auto pageSize = pageStyle->getPageSize(m_book->pageSize());
    auto pageBox = PageBox::create(pageStyle, pageSize, emptyGlo, 0, fragmentOffset());

    pageBox->build();
    pageBox->layout(nullptr);

    m_currentPage = pageBox.get();
    m_document->box()->setCurrentPage(pageBox.get());
    m_document->pages().push_back(std::move(pageBox));
}

void PageBuilder::leaveFragment(const BoxFrame* child, float offset)
{
    FragmentBuilder::leaveFragment(child, offset);
}

} // namespace plutobook
