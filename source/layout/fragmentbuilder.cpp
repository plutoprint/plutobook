#include "fragmentbuilder.h"
#include "document.h"
#include "boxview.h"
#include "pagebox.h"
#include "plutobook.hpp"

#include <cmath>

namespace plutobook {

PageBuilder::PageBuilder(const Book* book)
    : m_book(book)
    , m_document(book->document())
    , m_pages(m_document->pages())
{
}

float PageBuilder::applyFragmentBreakBefore(const BoxFrame* child, float offset)
{
    addPageUntil(child, offset);
    if(child->style()->pageBreakBefore() == BreakBetween::Always)
        offset += fragmentRemainingHeightForOffset(offset, AssociateWithFormerFragment);
    return offset;
}

float PageBuilder::applyFragmentBreakAfter(const BoxFrame* child, float offset)
{
    addPageUntil(child, offset);
    if(child->style()->pageBreakAfter() == BreakBetween::Always)
        offset += fragmentRemainingHeightForOffset(offset, AssociateWithFormerFragment);
    return offset;
}

float PageBuilder::applyFragmentBreakInside(const BoxFrame* child, float offset)
{
    return offset;
}

float PageBuilder::fragmentHeightForOffset(float offset)
{
    if(m_currentPage)
        return m_currentPage->height();
    return 0.f;
}

float PageBuilder::fragmentRemainingHeightForOffset(float offset, FragmentBoundaryRule rule)
{
    if(m_currentPage == nullptr)
        return 0.f;
    auto remainingHeight = m_currentPage->pageBottom() - (offset + fragmentOffset());
    if(rule == AssociateWithFormerFragment)
        remainingHeight = std::fmod(remainingHeight, m_currentPage->height());
    return std::max(0.f, remainingHeight);
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

void PageBuilder::addPageUntil(const BoxFrame* box, float offset)
{
    offset += fragmentOffset();
    while(m_currentPage == nullptr || offset > m_currentPage->pageBottom()) {
        addPage(box);
    }
}

void PageBuilder::addPage(const BoxFrame* box)
{
    auto pageStyle = m_document->styleForPage(emptyGlo, 0, PseudoType::FirstPage);
    auto pageSize = pageStyle->getPageSize(m_book->pageSize());
    auto pageBox = PageBox::create(pageStyle, pageSize, emptyGlo, m_pages.size());

    pageBox->build();
    pageBox->layout(nullptr);

    if(m_currentPage)
        pageBox->setPageTop(m_currentPage->pageBottom());
    pageBox->setPageBottom(pageBox->pageTop() + pageBox->pageHeight());

    m_currentPage = pageBox.get();
    m_document->box()->setCurrentPage(pageBox.get());
    m_pages.push_back(std::move(pageBox));
}

} // namespace plutobook
