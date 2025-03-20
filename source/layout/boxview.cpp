#include "boxview.h"
#include "boxlayer.h"
#include "pagebox.h"
#include "document.h"

namespace plutobook {

BoxView::BoxView(Document* document, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(document, style)
{
}

Rect BoxView::backgroundRect() const
{
    if(m_currentPage)
        return document()->pageRectAt(m_currentPage->pageIndex());
    return Rect(0, 0, document()->width(), document()->height());
}

float BoxView::availableWidth() const
{
    if(document()->isPrintMedia())
        return document()->pageWidth();
    return document()->viewportWidth();
}

std::optional<float> BoxView::availableHeight() const
{
    if(document()->isPrintMedia())
        return document()->pageHeight();
    return document()->viewportHeight();
}

void BoxView::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
}

void BoxView::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
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
