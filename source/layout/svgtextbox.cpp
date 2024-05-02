#include "svgtextbox.h"

namespace plutobook {

SVGInlineTextBox::SVGInlineTextBox(TextNode* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
    setInline(true);
}

SVGTSpanBox::SVGTSpanBox(SVGTSpanElement* element, const RefPtr<BoxStyle>& style)
    : Box(element, style)
{
    setInline(true);
}

void SVGTSpanBox::build()
{
    m_fill = element()->getPaintServer(style()->fill(), style()->fillOpacity());
    Box::build();
}

SVGTextBox::SVGTextBox(SVGTextElement* element, const RefPtr<BoxStyle>& style)
    : SVGBoxModel(element, style)
    , m_lineLayout(this)
{
}

const Rect& SVGTextBox::fillBoundingBox() const
{
    if(!m_fillBoundingBox.isValid())
        m_fillBoundingBox = m_lineLayout.boundingRect();
    return m_fillBoundingBox;
}

void SVGTextBox::render(const SVGRenderState& state) const
{
    if(style()->visibility() != Visibility::Visible)
        return;
    SVGRenderState newState(this, state, element()->transform());
    SVGBlendInfo blendInfo(newState, m_clipper, m_masker, style());
    blendInfo.beginGroup();
    if(state.mode() == SVGRenderMode::Display) {
        m_fill.applyPaint(newState);
    } else {
        newState->setColor(Color::White);
    }

    m_lineLayout.render(newState);
    blendInfo.endGroup();
}

void SVGTextBox::build()
{
    m_fill = element()->getPaintServer(style()->fill(), style()->fillOpacity());
    m_lineLayout.build();
    SVGBoxModel::build();
}

} // namespace plutobook
