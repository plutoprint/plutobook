#include "multicolumnbox.h"

#include <cmath>

namespace plutobook {

MultiColumnFlowBox* MultiColumnFlowBox::create(const BoxStyle* parentStyle)
{
    auto newStyle = BoxStyle::create(*parentStyle, Display::Block);
    auto newColumn = new (newStyle->heap()) MultiColumnFlowBox(newStyle);
    newColumn->setAnonymous(true);
    return newColumn;
}

void MultiColumnFlowBox::updatePreferredWidths() const
{
    BlockFlowBox::updatePreferredWidths();

    auto columnBlock = columnBlockFlowBox();
    auto columnStyle = columnBlock->style();
    auto columnCount = columnStyle->columnCount().value_or(1);
    auto totalColumnGap = (columnCount - 1) * columnStyle->columnGap().value_or(columnStyle->fontSize());
    if(auto columnWidth = columnStyle->columnWidth()) {
        m_minPreferredWidth = std::min(m_minPreferredWidth, columnWidth.value());
        m_maxPreferredWidth = std::max(m_maxPreferredWidth, columnWidth.value());
    } else {
        m_minPreferredWidth = m_minPreferredWidth * columnCount + totalColumnGap;
    }

    m_maxPreferredWidth = m_maxPreferredWidth * columnCount + totalColumnGap;
}

void MultiColumnFlowBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto columnBlock = columnBlockFlowBox();
    auto columnStyle = columnBlock->style();
    auto columnCount = columnStyle->columnCount();
    auto columnWidth = columnStyle->columnWidth();
    auto columnGap = columnStyle->columnGap().value_or(columnStyle->fontSize());
    auto availableWidth = columnBlock->contentBoxWidth();
    if(!columnWidth.has_value() && columnCount.has_value()) {
        m_columnCount = columnCount.value();
        width = std::max(0.f, (availableWidth - ((columnCount.value() - 1) * columnGap)) / columnCount.value());
    } else if(columnWidth.has_value() && !columnCount.has_value()) {
        m_columnCount = std::max(1.f, std::floor((availableWidth + columnGap) / (columnWidth.value() + columnGap)));
        width = ((availableWidth + columnGap) / m_columnCount) - columnGap;
    } else {
        int count = std::floor((availableWidth + columnGap) / (columnWidth.value() + columnGap));
        m_columnCount = std::max(1, std::min(count, columnCount.value()));
        width = ((availableWidth + columnGap) / m_columnCount) - columnGap;
    }
}

static bool isValidColumnSpanBox(BoxFrame* box)
{
    return box->style()->columnSpan() == ColumnSpan::All && !box->isInline() && !box->isFloatingOrPositioned();
}

void MultiColumnFlowBox::build()
{
    MultiColumnSetBox* lastColumnSet = nullptr;
    auto child = firstChild();
    while(child) {
        if(auto box = to<BoxFrame>(child); box && isValidColumnSpanBox(box)) {
            auto columnSpanBox = MultiColumnSpanBox::create(box, columnBlockFlowBox()->style());
            columnBlockFlowBox()->addChild(columnSpanBox);
            lastColumnSet = nullptr;
        } else if(!child->isPositioned()) {
            if(lastColumnSet == nullptr) {
                auto columnSetBox = MultiColumnSetBox::create(columnBlockFlowBox()->style());
                columnBlockFlowBox()->addChild(columnSetBox);
                lastColumnSet = columnSetBox;
            }

            if(child->firstChild()) {
                child = child->firstChild();
                continue;
            }
        }

        while(true) {
            if(child->nextSibling()) {
                child = child->nextSibling();
                break;
            }

            child = child->parentBox();
            if(child == this) {
                child = nullptr;
                break;
            }
        }
    }

    BlockFlowBox::build();
}

MultiColumnFlowBox::MultiColumnFlowBox(const RefPtr<BoxStyle>& style)
    : BlockFlowBox(nullptr, style)
{
}

MultiColumnSetBox* MultiColumnSetBox::create(const BoxStyle* parentStyle)
{
    auto newStyle = BoxStyle::create(*parentStyle, Display::Block);
    auto newColumn = new (newStyle->heap()) MultiColumnSetBox(newStyle);
    newColumn->setAnonymous(true);
    return newColumn;
}

void MultiColumnSetBox::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = columnFlowBox()->minPreferredWidth();
    maxWidth = columnFlowBox()->maxPreferredWidth();
}

void MultiColumnSetBox::paginate(PageBuilder& builder, float top) const
{
}

void MultiColumnSetBox::layout()
{
    updateWidth();
    updateHeight();
}

MultiColumnSetBox::MultiColumnSetBox(const RefPtr<BoxStyle>& style)
    : BlockBox(nullptr, style)
{
}

MultiColumnSpanBox* MultiColumnSpanBox::create(BoxFrame* box, const BoxStyle* parentStyle)
{
    auto newStyle = BoxStyle::create(*parentStyle, Display::Block);
    auto newColumn = new (newStyle->heap()) MultiColumnSpanBox(box, newStyle);
    newColumn->setAnonymous(true);
    return newColumn;
}

void MultiColumnSpanBox::computePreferredWidths(float& minWidth, float& maxWidth) const
{
    minWidth = m_box->minPreferredWidth();
    maxWidth = m_box->maxPreferredWidth();
}

void MultiColumnSpanBox::paginate(PageBuilder& builder, float top) const
{
}

void MultiColumnSpanBox::layout()
{
    updateWidth();
    updateHeight();
}

MultiColumnSpanBox::MultiColumnSpanBox(BoxFrame* box, const RefPtr<BoxStyle>& style)
    : BlockBox(nullptr, style)
{
    box->setColumnSpanBox(this);
    box->setHasColumnSpanBox(true);
}

} // namespace plutobook
