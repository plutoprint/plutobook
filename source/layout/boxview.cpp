#include "boxview.h"
#include "boxlayer.h"
#include "pagebox.h"
#include "document.h"

#include "plutobook.hpp"

namespace plutobook {

BoxView::BoxView(Document* document, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(document, style)
{
}

bool BoxView::isPrintMedia() const
{
    if(auto book = document()->book())
        return book->mediaType() == MediaType::Print;
    return false;
}

float BoxView::availableWidth() const
{
    if(m_currentPage)
        return m_currentPage->width() - m_currentPage->marginWidth();
    return document()->viewportWidth();
}

std::optional<float> BoxView::availableHeight() const
{
    if(m_currentPage)
        return m_currentPage->height() - m_currentPage->marginHeight();
    return document()->viewportHeight();
}

void BoxView::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    width = document()->viewportWidth();
}

void BoxView::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    height = document()->viewportHeight();
}

void BoxView::layout(FragmentBuilder* fragmentainer)
{
    BlockFlowBox::layout(fragmentainer);
    layer()->layout();
}

void BoxView::build()
{
    auto bodyStyle = document()->bodyStyle();
    if(bodyStyle)
        style()->setDirection(bodyStyle->direction());
    m_backgroundStyle = document()->rootStyle();
    if(m_backgroundStyle && !m_backgroundStyle->hasBackground()
        && bodyStyle && bodyStyle->hasBackground()) {
        m_backgroundStyle = bodyStyle;
    }

    BlockFlowBox::build();
}

} // namespace plutobook
