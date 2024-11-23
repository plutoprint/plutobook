#include "multicolumnbox.h"

#include <cmath>

namespace plutobook {

float MultiColumnContentRun::columnLogicalHeight(float startOffset) const
{
    return (m_breakOffset - startOffset) / (m_assumedImplicitBreaks + 1);
}

MultiColumnRowBox* MultiColumnRowBox::create(MultiColumnFlowBox* column, const BoxStyle* parentStyle)
{
    auto newStyle = BoxStyle::create(*parentStyle, Display::Block);
    auto newRow = new (newStyle->heap()) MultiColumnRowBox(column, newStyle);
    newRow->setAnonymous(true);
    return newRow;
}

MultiColumnRowBox::MultiColumnRowBox(MultiColumnFlowBox* column, const RefPtr<BoxStyle>& style)
    : BoxFrame(nullptr, style)
    , m_column(column), m_runs(style->heap())
{
}

void MultiColumnRowBox::updatePreferredWidths() const
{
    m_minPreferredWidth = m_column->minPreferredWidth();
    m_maxPreferredWidth = m_column->maxPreferredWidth();
}

void MultiColumnRowBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
}

void MultiColumnRowBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
}

void MultiColumnRowBox::layout()
{
    updateWidth();
    updateHeight();
}

MultiColumnRowBox* MultiColumnRowBox::prevRow() const
{
    for(auto box = prevSibling(); box; box = box->prevSibling()) {
        if(auto row = to<MultiColumnRowBox>(box)) {
            return row;
        }
    }

    return nullptr;
}

MultiColumnRowBox* MultiColumnRowBox::nextRow() const
{
    for(auto box = nextSibling(); box; box = box->nextSibling()) {
        if(auto row = to<MultiColumnRowBox>(box)) {
            return row;
        }
    }

    return nullptr;
}

uint32_t MultiColumnRowBox::numberOfColumns() const
{
    if(m_columnHeight == 0.f)
        return 1;
    auto height = rowHeight();
    if(height == 0.f)
        return 1;
    auto count = std::floor(height / m_columnHeight);
    if(count * m_columnHeight < height)
        count++;
    assert(count >= 1);
    return count;
}

float MultiColumnRowBox::columnTopForOffset(float offset) const
{
    return rowTopAt(columnIndexAtOffset(offset, false));
}

void MultiColumnRowBox::recordSpaceShortage(float spaceShortage)
{
    if(spaceShortage >= m_minSpaceShortage)
        return;
    assert(spaceShortage > 0);
    m_minSpaceShortage = spaceShortage;
}

void MultiColumnRowBox::updateMinimumColumnHeight(float height)
{
    if(height > m_minimumColumnHeight) {
        m_minimumColumnHeight = height;
    }
}

void MultiColumnRowBox::addContentRun(float endOffset)
{
    if(!m_runs.empty() && endOffset <= m_runs.back().breakOffset())
        return;
    if(m_runs.size() < m_column->columnCount()) {
        m_runs.emplace_back(endOffset);
    }
}

bool MultiColumnRowBox::recalculateColumnHeight(bool balancing)
{
    if(m_isColumnBalanced && !balancing)
        distributeImplicitBreaks();
    if(m_isColumnBalanced) {
        m_columnHeight = constrainColumnHeight(calculateColumnHeight(balancing));
    } else {
        m_columnHeight = constrainColumnHeight(m_columnHeight);
    }

    return false;
}

float MultiColumnRowBox::constrainColumnHeight(float columnHeight) const
{
    if(columnHeight > m_maxColumnHeight)
        columnHeight = m_maxColumnHeight;
    return columnHeight;
}

float MultiColumnRowBox::calculateColumnHeight(bool balancing) const
{
    if(!balancing) {
        auto index = findRunWithTallestColumns();
        auto startOffset = index == 0 ? m_rowTop : m_runs[index - 1].breakOffset();
        return std::max(m_minimumColumnHeight, m_runs[index].columnLogicalHeight(startOffset));
    }

    auto columnCount = numberOfColumns();
    if(columnCount <= m_column->columnCount())
        return m_columnHeight;
    if(m_runs.size() >= m_column->columnCount())
        return m_columnHeight;
    if(m_columnHeight >= m_maxColumnHeight)
        return m_columnHeight;
    return m_columnHeight + m_minSpaceShortage;
}

uint32_t MultiColumnRowBox::columnIndexAtOffset(float offset, bool clampToExistingColumns) const
{
    if(offset < m_rowTop)
        return 0;
    if(clampToExistingColumns) {
        if(offset >= m_rowBottom) {
            return numberOfColumns() - 1;
        }
    }

    if(m_columnHeight > 0.f)
        return std::floor((offset - m_rowTop) / m_columnHeight);
    return 0;
}

uint32_t MultiColumnRowBox::findRunWithTallestColumns() const
{
    uint32_t indexWithLargestHeight = 0;
    float largestHeight = 0.f;
    auto previousOffset = m_rowTop;
    auto runCount = m_runs.size();
    for(uint32_t i = 0; i < runCount; i++) {
        const auto& run = m_runs[i];
        auto height = run.columnLogicalHeight(previousOffset);
        if(largestHeight < height) {
            largestHeight = height;
            indexWithLargestHeight = i;
        }

        previousOffset = run.breakOffset();
    }

    return indexWithLargestHeight;
}

void MultiColumnRowBox::distributeImplicitBreaks()
{
    addContentRun(m_rowBottom);
    auto columnCount = m_runs.size();

    while(columnCount < m_column->columnCount()) {
        auto index = findRunWithTallestColumns();
        m_runs[index].assumeAnotherImplicitBreak();
        columnCount++;
    }
}

MultiColumnSpanBox* MultiColumnSpanBox::create(BoxFrame* box, const BoxStyle* parentStyle)
{
    auto newStyle = BoxStyle::create(*parentStyle, Display::Block);
    auto newSpanner = new (newStyle->heap()) MultiColumnSpanBox(box,newStyle);
    newSpanner->setAnonymous(true);
    return newSpanner;
}

void MultiColumnSpanBox::updatePreferredWidths() const
{
    m_minPreferredWidth = m_box->minPreferredWidth();
    m_maxPreferredWidth = m_box->maxPreferredWidth();
}

void MultiColumnSpanBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    width = m_box->width();
    marginLeft = m_box->marginLeft();
    marginRight = m_box->marginRight();
}

void MultiColumnSpanBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    height = m_box->height();
    marginTop = m_box->marginTop();
    marginBottom = m_box->marginBottom();
}

void MultiColumnSpanBox::layout()
{
    updateWidth();
    updateHeight();
}

MultiColumnSpanBox::MultiColumnSpanBox(BoxFrame* box, const RefPtr<BoxStyle>& style)
    : BoxFrame(nullptr, style)
    , m_box(box)
{
}

MultiColumnFlowBox* MultiColumnFlowBox::create(const BoxStyle* parentStyle)
{
    auto newStyle = BoxStyle::create(*parentStyle, Display::Block);
    auto newColumn = new (newStyle->heap()) MultiColumnFlowBox(newStyle);
    newColumn->setAnonymous(true);
    return newColumn;
}

MultiColumnRowBox* MultiColumnFlowBox::firstRow() const
{
    for(auto box = nextSibling(); box; box = box->nextSibling()) {
        if(auto row = to<MultiColumnRowBox>(box)) {
            return row;
        }
    }

    return nullptr;
}

float MultiColumnFlowBox::columnHeightForOffset(float offset) const
{
    if(auto row = columnRowAtOffset(offset))
        return row->columnHeight();
    return 0.f;
}

float MultiColumnFlowBox::columnRemainingHeightForOffset(float offset, ColumnBoundaryRule rule) const
{
    if(auto row = columnRowAtOffset(offset)) {
        assert(row->columnHeight() > 0.f);
        auto columnBottom = row->columnTopForOffset(offset) + row->columnHeight();
        auto remainingHeight = columnBottom - offset;
        if(rule == AssociateWithFormerColumn)
            return std::fmod(remainingHeight, row->columnHeight());
        return remainingHeight;
    }

    return 0.f;
}

void MultiColumnFlowBox::addForcedColumnBreak(float offset)
{
    if(auto row = columnRowAtOffset(offset)) {
        row->addContentRun(offset);
    }
}

void MultiColumnFlowBox::setColumnBreak(float offset, float spaceShortage)
{
    if(auto row = columnRowAtOffset(offset)) {
        row->recordSpaceShortage(spaceShortage);
    }
}

void MultiColumnFlowBox::updateMinimumColumnHeight(float offset, float minHeight)
{
    if(auto row = columnRowAtOffset(offset)) {
        row->updateMinimumColumnHeight(minHeight);
    }
}

void MultiColumnFlowBox::skipColumnSpanner(BoxFrame* box, float offset)
{
    assert(box->isColumnSpanAll());
    auto columnSpanner = box->columnSpanBox();
    auto prevColumnBox = columnSpanner->prevMultiColumnBox();
    if(prevColumnBox && prevColumnBox->isMultiColumnRowBox()) {
        auto columnRow = to<MultiColumnRowBox>(prevColumnBox);
        if(offset < columnRow->rowTop())
            offset = columnRow->rowTop();
        columnRow->setRowBottom(offset);
    }

    auto nextColumnBox = columnSpanner->nextMultiColumnBox();
    if(nextColumnBox && nextColumnBox->isMultiColumnRowBox()) {
        auto columnRow = to<MultiColumnRowBox>(prevColumnBox);
        columnRow->setRowTop(offset);
        m_currentRow = columnRow;
    }
}

MultiColumnRowBox* MultiColumnFlowBox::columnRowAtOffset(float offset) const
{
    return m_lastRow;
}

void MultiColumnFlowBox::layoutColumns(bool balancing)
{
    m_currentRow = firstRow();
    if(m_currentRow)
        m_currentRow->setRowTop(0.f);
    BlockFlowBox::layout();
    if(m_lastRow) {
        assert(m_lastRow == m_currentRow);
        m_lastRow->setRowBottom(height());
    }
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
    auto columnBalance = style()->columnFill() == ColumnFill::Balance;
    const MultiColumnSpanBox* currentColumnSpanner = nullptr;
    auto child = firstChild();
    while(child) {
        child->setIsInsideColumn(true);
        if(auto box = to<BoxFrame>(child); isValidColumnSpanBox(box)) {
            auto newSpanner = MultiColumnSpanBox::create(box, style());
            parentBox()->addChild(newSpanner);
            box->setIsColumnSpanAll(true);
            box->setColumnSpanBox(newSpanner);
            currentColumnSpanner = newSpanner;
        } else if(!child->isFloatingOrPositioned()) {
            if(m_lastRow == nullptr || currentColumnSpanner) {
                auto newRow = MultiColumnRowBox::create(this, style());
                parentBox()->addChild(newRow);
                newRow->setIsColumnBalanced(columnBalance || currentColumnSpanner);
                currentColumnSpanner = nullptr;
                m_lastRow = newRow;
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

void MultiColumnFlowBox::layout()
{
    layoutColumns(false);
    layoutColumns(true);
}

MultiColumnFlowBox::MultiColumnFlowBox(const RefPtr<BoxStyle>& style)
    : BlockFlowBox(nullptr, style)
{
}

} // namespace plutobook
