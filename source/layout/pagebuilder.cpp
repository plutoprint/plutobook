#include "pagebuilder.h"
#include "pagebox.h"
#include "boxview.h"

#include <cmath>

namespace plutobook {

PageBuilder::PageBuilder(Document* document)
    : m_document(document)
    , m_pages(document->pages())
{
    assert(m_pages.empty());
}

void PageBuilder::build()
{
    if(!m_document->width() || !m_document->height())
        return;
    auto box = m_document->box();
    auto child = box->firstBoxFrame();
    while(child) {
        child->paginate(*this, 0.f);
        child = child->nextBoxFrame();
    }

    addPageUntil(box, m_document->height());
    setPageBreakAt(m_document->height());
}

void PageBuilder::enterBox(const BoxFrame* box, float top)
{
    addPageUntil(box, top);
    if(box->style()->pageBreakBefore() == BreakBetween::Always) {
        setPageBreakAt(top);
    } else if(box->style()->pageBreakInside() == BreakInside::Avoid
        && !canFitOnPage(top + box->height())) {
        setPageBreakAt(top);
    }
}

void PageBuilder::exitBox(const BoxFrame* box, float top)
{
    addPageUntil(box, top + box->height());
    if(box->style()->pageBreakAfter() == BreakBetween::Always) {
        setPageBreakAt(top + box->height());
    }
}

constexpr PseudoType pagePseudoType(size_t pageIndex)
{
    if(pageIndex == 0)
        return PseudoType::FirstPage;
    if(pageIndex % 2)
        return PseudoType::LeftPage;
    return PseudoType::RightPage;
}

void PageBuilder::addPageUntil(const BoxFrame* box, float top)
{
    while(!canFitOnPage(top)) {
        newPage(box, !m_currentPage ? 0.f : m_currentPage->pageBottom());
    }
}

void PageBuilder::setPageBreakAt(float top)
{
    if(m_currentPage && top > m_currentPage->pageTop()) {
        m_currentPage->setPageBottom(top);
    }
}

bool PageBuilder::canFitOnPage(float top) const
{
    return m_currentPage && top <= m_currentPage->pageBottom();
}

void PageBuilder::newPage(const BoxFrame* box, float top)
{
    auto pageStyle = m_document->styleForPage(emptyGlo, m_pages.size(), pagePseudoType(m_pages.size()));
    auto pageBox = PageBox::create(pageStyle, emptyGlo, m_pages.size());
    pageBox->build();
    pageBox->layout();

    auto pageWidth = std::max(1.f, pageBox->width() - pageBox->marginWidth());
    auto pageHeight = std::max(1.f, pageBox->height() - pageBox->marginHeight());
    if(auto pageScale = pageStyle->pageScale()) {
        pageBox->setPageScale(pageScale.value());
    } else if(pageWidth < m_document->width()) {
        pageBox->setPageScale(pageWidth / m_document->width());
    } else {
        pageBox->setPageScale(1.f);
    }

    pageBox->setPageTop(top);
    pageBox->setPageBottom(top + (pageHeight / pageBox->pageScale()));

    m_currentPage = pageBox.get();
    m_pages.push_back(std::move(pageBox));
}

} // namespace plutobook
