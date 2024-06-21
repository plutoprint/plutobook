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
    width = document()->viewportWidth();
}

void BoxView::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    height = document()->viewportHeight();
}

void BoxView::layout()
{
    BlockFlowBox::layout();
    layer()->layout();
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
