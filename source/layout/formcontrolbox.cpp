#include "formcontrolbox.h"
#include "htmldocument.h"
#include "pagebuilder.h"
#include "boxlayer.h"

namespace plutobook {

TextInputBox::TextInputBox(HTMLElement* element, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(element, style)
{
}

HTMLElement* TextInputBox::element() const
{
    return static_cast<HTMLElement*>(node());
}

std::optional<float> TextInputBox::inlineBlockBaseline() const
{
    if(m_rows == 1)
        return firstLineBaseline();
    return std::nullopt;
}

void TextInputBox::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = maxWidth = m_cols * style()->chFontSize();
}

void TextInputBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    height = m_rows * style()->lineHeight() + borderAndPaddingHeight();
    BlockFlowBox::computeHeight(y, height, marginTop, marginBottom);
}

SelectBox::SelectBox(HTMLSelectElement* element, const RefPtr<BoxStyle>& style)
    : BlockBox(element, style)
    , m_size(element->size())
{
}

HTMLSelectElement* SelectBox::element() const
{
    return static_cast<HTMLSelectElement*>(node());
}

std::optional<float> SelectBox::inlineBlockBaseline() const
{
    if(m_size == 1)
        return firstLineBaseline();
    return std::nullopt;
}

void SelectBox::addChild(Box* newChild)
{
    if(newChild->isPositioned() || !newChild->isInline()) {
        BlockBox::addChild(newChild);
        return;
    }

    auto lastBlock = lastChild();
    if(lastBlock && lastBlock->isAnonymousBlock()) {
        lastBlock->addChild(newChild);
        return;
    }

    auto newBlock = createAnonymousBlock(style());
    appendChild(newBlock);
    newBlock->addChild(newChild);
}

void SelectBox::updateOverflowRect()
{
    BlockBox::updateOverflowRect();
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(!child->isPositioned()) {
            addOverflowRect(child, child->x(), child->y());
        }
    }
}

void SelectBox::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned())
            continue;
        auto childStyle = child->style();
        auto marginLeftLength = childStyle->marginLeft();
        auto marginRightLength = childStyle->marginRight();

        float marginWidth = 0;
        if(marginLeftLength.isFixed())
            marginWidth += marginLeftLength.value();
        if(marginRightLength.isFixed()) {
            marginWidth += marginRightLength.value();
        }

        auto childMinWidth = child->minPreferredWidth();
        auto childMaxWidth = child->maxPreferredWidth();

        childMinWidth += marginWidth;
        childMaxWidth += marginWidth;

        minWidth = std::max(minWidth, childMinWidth);
        maxWidth = std::max(maxWidth, childMaxWidth);
    }

    minWidth = std::max(0.f, minWidth);
    maxWidth = std::max(minWidth, maxWidth);
}

void SelectBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    unsigned itemIndex = 0;
    height = borderAndPaddingHeight();
    for(auto child = firstBoxFrame(); child && itemIndex < m_size; child = child->nextBoxFrame()) {
        if(!child->isPositioned()) {
            height += child->height();
            ++itemIndex;
        }
    }

    BlockBox::computeHeight(y, height, marginTop, marginBottom);
}

void SelectBox::paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    unsigned itemIndex = 0;
    for(auto child = firstBoxFrame(); child && itemIndex < m_size; child = child->nextBoxFrame()) {
        if(!child->hasLayer()) {
            child->paint(info, offset, phase);
            ++itemIndex;
        }
    }
}

void SelectBox::paginate(PageBuilder& builder, float top) const
{
    builder.enterBox(this, top + y());
    builder.exitBox(this, top + y());
}

void SelectBox::layout()
{
    updateWidth();
    setHeight(borderAndPaddingTop());
    for(auto child = firstBoxFrame(); child; child = child->nextBoxFrame()) {
        if(child->isPositioned()) {
            auto childLayer = child->layer();
            childLayer->setStaticTop(height());
            childLayer->setStaticLeft(startOffsetForContent());
            child->containingBlock()->insertPositonedBox(child);
            continue;
        }

        child->layout();
        child->setY(height() + child->marginTop());
        child->setX(borderStart() + paddingStart() + child->marginLeft());
        if(style()->isRightToLeftDirection())
            child->setX(width() - child->x() - child->width());
        setHeight(child->y() + child->height() + child->marginBottom());
    }

    setHeight(height() + borderAndPaddingBottom());
    updateHeight();
    layoutPositionedBoxes();
    updateOverflowRect();
}

} // namespace plutobook
