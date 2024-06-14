#include "boxview.h"
#include "boxlayer.h"
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

void BoxView::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    width = pageWidth();
}

void BoxView::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    height = pageHeight();
}

void BoxView::layoutContents()
{
    BlockFlowBox::layout();
    layer()->layout();
}

void BoxView::layout()
{
    auto pageScale = isPrintMedia() ? document()->book()->pageScale() : 1.f;
    m_pageWidth = document()->viewportWidth();
    m_pageHeight = document()->viewportHeight();
    m_pageScale = 1.f;
    if(pageScale > 0.f && isPrintMedia()) {
        m_pageScale = pageScale / 100.f;
        m_pageWidth /= m_pageScale;
        m_pageHeight /= m_pageScale;
    }

    layoutContents();
    if(pageScale <= 0.f && isPrintMedia() && m_pageWidth < document()->width()) {
        m_pageScale = m_pageWidth / document()->width();
        m_pageWidth /= m_pageScale;
        m_pageHeight /= m_pageScale;
        layoutContents();
    }
}

void BoxView::build()
{
    m_backgroundStyle = document()->rootStyle();
    if(m_backgroundStyle && !m_backgroundStyle->hasBackground()) {
        if(auto bodyElement = document()->bodyElement()) {
            auto bodyStyle = bodyElement->style();
            if(bodyStyle && bodyStyle->hasBackground()) {
                m_backgroundStyle = bodyStyle;
            }
        }
    }

    BlockFlowBox::build();
}

} // namespace plutobook
