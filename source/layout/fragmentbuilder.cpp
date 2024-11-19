#include "fragmentbuilder.h"
#include "document.h"
#include "linebox.h"
#include "pagebox.h"
#include "boxview.h"

#include <cmath>

namespace plutobook {

FragmentBuilder::FragmentBuilder(FragmentList& fragments, FragmentationMode mode)
    : m_fragments(fragments), m_mode(mode)
{
}

void FragmentBuilder::handleLineBox(const RootLineBox& line, float top)
{
    m_fragments.emplace_back(line.box(), Fragment::Type::LineBox, top + line.lineTop(), top + line.lineBottom());
}

void FragmentBuilder::handleReplacedBox(const BoxFrame* box, float top)
{
    Fragment fragment(box, Fragment::Type::ReplacedBox, top + box->y(), top + box->y() + box->height());
    applyBreakInside(fragment, box->style());
    handleBreakBefore(box, top);
    m_fragments.push_back(fragment);
}

void FragmentBuilder::enterBox(const BoxFrame* box, float top)
{
    Fragment fragment(box, Fragment::Type::BoxStart, top + box->y(), top + box->y() + box->height());
    applyBreakInside(fragment, box->style());
    handleBreakBefore(box, top);
    m_fragments.push_back(fragment);
}

void FragmentBuilder::exitBox(const BoxFrame* box, float top)
{
    Fragment fragment(box, Fragment::Type::BoxEnd, top + box->y() + box->height(), top + box->y() + box->height());
    m_fragments.push_back(fragment);
    handleBreakAfter(box, top);
}

void FragmentBuilder::applyBreakInside(Fragment& fragment, const BoxStyle* style)
{
    if(style->pageBreakInside() == BreakInside::Auto) {
        fragment.setBreakInside(true);
    }
}

void FragmentBuilder::handleBreakBefore(const BoxFrame* box, float top)
{
    if(box->style()->pageBreakBefore() == BreakBetween::Always) {
        m_fragments.emplace_back(box, Fragment::Type::ForceBreak, top + box->y(), top + box->y());
    }
}

void FragmentBuilder::handleBreakAfter(const BoxFrame* box, float top)
{
    if(box->style()->pageBreakAfter() == BreakBetween::Always) {
        m_fragments.emplace_back(box, Fragment::Type::ForceBreak, top + box->y(), top + box->y());
    }
}

PageBreaker::PageBreaker(Document* document, const FragmentList& fragments)
    : m_document(document), m_fragments(fragments)
{
}

constexpr PseudoType pagePseudoType(uint32_t pageIndex)
{
    if(pageIndex == 0)
        return PseudoType::FirstPage;
    if(pageIndex % 2)
        return PseudoType::LeftPage;
    return PseudoType::RightPage;
}

std::unique_ptr<PageBox> PageBreaker::nextPage()
{
    auto pageStyle = m_document->styleForPage(emptyGlo, m_pageIndex, pagePseudoType(m_pageIndex));
    auto pageBox = PageBox::create(pageStyle, emptyGlo, m_pageIndex);

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

    pageBox->setPageTop(m_pageTop);
    pageBox->setPageBottom(m_pageTop + (pageHeight / pageBox->pageScale()));

    auto pageBottom = pageBox->pageBottom();
    while(m_fragmentIndex < m_fragments.size()) {
        const auto& fragment = m_fragments[m_fragmentIndex];
        if(fragment.type() == Fragment::Type::ForceBreak) {
            pageBottom = fragment.bottom();
            ++m_fragmentIndex;
            break;
        }

        if(fragment.type() == Fragment::Type::BoxEnd) {
            if(fragment.bottom() > pageBox->pageBottom()) {
                pageBottom = pageBox->pageBottom();
                break;
            }

            ++m_fragmentIndex;
            continue;
        }

        if(fragment.top() > pageBox->pageBottom())
            break;
        if(fragment.type() == Fragment::Type::BoxStart) {
            if(fragment.bottom() < pageBox->pageBottom())
                pageBottom = fragment.bottom();
            ++m_fragmentIndex;
            continue;
        }

        if(fragment.canBreakInside()) {
            if(fragment.bottom() > pageBox->pageBottom()) {
                pageBottom = pageBox->pageBottom();
                break;
            }

            pageBottom = fragment.bottom();
            ++m_fragmentIndex;
            continue;
        }

        if(fragment.bottom() > pageBox->pageBottom())
            break;
        pageBottom = fragment.bottom();
        ++m_fragmentIndex;
    }

    m_pageIndex += 1;
    m_pageTop = pageBottom;
    pageBox->setPageBottom(pageBottom);
    return pageBox;
}

bool PageBreaker::isDone() const
{
    return m_fragmentIndex == m_fragments.size();
}

PageBuilder::PageBuilder(Document* document)
    : m_document(document), m_pages(document->pages())
{
    assert(m_pages.empty());
}

void PageBuilder::build()
{
    FragmentList fragments;
    FragmentBuilder builder(fragments, FragmentationMode::Page);
    m_document->box()->fragmentize(builder, 0.f);

    PageBreaker breaker(m_document, fragments);
    while(!breaker.isDone()) {
        m_pages.push_back(breaker.nextPage());
    }
}

} // namespace plutobook
