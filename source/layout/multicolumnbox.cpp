#include "multicolumnbox.h"
#include "linelayout.h"

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
    auto columnGap = columnStyle->columnGap().value_or(columnStyle->fontSize());

    auto totalColumnGap = (columnCount - 1) * columnGap;
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
    return box && box->style()->columnSpan() == ColumnSpan::All && !box->isInline() && !box->isFloatingOrPositioned();
}

void MultiColumnFlowBox::build()
{
    auto child = firstChild();
    while(child) {
        auto box = to<BoxFrame>(child);
        if(isValidColumnSpanBox(box)) {
            box->setHasColumnSpan(true);
            m_rows.emplace_back(box);
        } else if(!child->isFloatingOrPositioned()) {
            if(m_rows.empty() || m_rows.back().hasColumnSpan())
                m_rows.emplace_back(this);
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

void MultiColumnFlowBox::layout()
{
    BlockFlowBox::layout();
}

MultiColumnFlowBox::MultiColumnFlowBox(const RefPtr<BoxStyle>& style)
    : BlockFlowBox(nullptr, style)
    , m_rows(style->heap())
{
}

} // namespace plutobook
