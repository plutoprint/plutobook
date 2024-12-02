#include "multicolumnbox.h"
#include "borderpainter.h"

#include <cmath>

namespace plutobook {

float MultiColumnContentRun::columnLogicalHeight(float startOffset) const
{
    return (m_breakOffset - startOffset) / (m_assumedImplicitBreaks + 1);
}

MultiColumnRowBox* MultiColumnRowBox::create(MultiColumnFlowBox* columnFlow, const BoxStyle* parentStyle)
{
    auto newStyle = BoxStyle::create(*parentStyle, Display::Block);
    auto newRow = new (newStyle->heap()) MultiColumnRowBox(columnFlow, newStyle);
    newRow->setAnonymous(true);
    return newRow;
}

void MultiColumnRowBox::updateOverflowRect()
{
    BoxFrame::updateOverflowRect();
    addOverflowRect(columnRectAt(numberOfColumns() - 1));
}

MultiColumnRowBox::MultiColumnRowBox(MultiColumnFlowBox* columnFlow, const RefPtr<BoxStyle>& style)
    : BoxFrame(nullptr, style)
    , m_columnFlowBox(columnFlow)
    , m_runs(style->heap())
{
}

void MultiColumnRowBox::computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const
{
    minPreferredWidth = m_columnFlowBox->minPreferredWidth();
    maxPreferredWidth = m_columnFlowBox->maxPreferredWidth();
}

void MultiColumnRowBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    width = m_columnFlowBox->columnBlockFlowBox()->contentBoxWidth();
}

void MultiColumnRowBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    height = m_columnHeight;
}

void MultiColumnRowBox::layout()
{
    updateWidth();
    updateHeight();
    updateOverflowRect();
}

void MultiColumnRowBox::fragmentize(FragmentBuilder& builder, float top) const
{
}

void MultiColumnRowBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(phase != PaintPhase::Decorations || style()->visibility() != Visibility::Visible)
        return;
    auto columnBlock = m_columnFlowBox->columnBlockFlowBox();
    auto columnStyle = columnBlock->style();
    auto columnDirection = columnStyle->direction();

    auto columnRuleStyle = columnStyle->columnRuleStyle();
    auto columnRuleColor = columnStyle->columnRuleColor();
    auto columnRuleWidth = columnStyle->columnRuleWidth();
    if(columnRuleStyle <= LineStyle::Hidden || columnRuleWidth <= 0.f || !columnRuleColor.isVisible())
        return;
    auto columnGap = m_columnFlowBox->columnGap();
    auto columnWidth = m_columnFlowBox->width();
    auto columnCount = numberOfColumns();

    Point adjustedOffset(offset + location());
    auto currentOffset = columnDirection == Direction::Ltr ? 0.f : width();
    auto ruleOffset = columnDirection == Direction::Ltr ? 0.f : width();
    auto boxSide = columnDirection == Direction::Ltr ? BoxSideLeft : BoxSideRight;
    for(uint32_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
        if(columnDirection == Direction::Ltr) {
            ruleOffset += columnWidth + columnGap / 2.f;
            currentOffset += columnWidth + columnGap;
        } else {
            ruleOffset -= columnWidth + columnGap / 2.f;
            currentOffset -= columnWidth + columnGap;
        }

        if(columnIndex < columnCount - 1) {
            Rect ruleRect(adjustedOffset.x + ruleOffset - columnRuleWidth / 2.f, adjustedOffset.y, columnRuleWidth, m_columnHeight);
            BorderPainter::paintBoxSide(*info, boxSide, columnRuleStyle, columnRuleColor, ruleRect);
        }

        ruleOffset = currentOffset;
    }
}

Rect MultiColumnRowBox::columnRectAt(uint32_t columnIndex) const
{
    Rect columnRect(0, 0, m_columnFlowBox->width(), rowHeightAt(columnIndex));
    if(m_columnFlowBox->direction() == Direction::Ltr) {
        columnRect.x += columnIndex * (columnRect.w + m_columnFlowBox->columnGap());
    } else {
        columnRect.x += width() - columnRect.w - columnIndex * (columnRect.w + m_columnFlowBox->columnGap());
    }

    return columnRect;
}

Rect MultiColumnRowBox::rowRectAt(uint32_t columnIndex) const
{
    return Rect(0, rowTopAt(columnIndex), m_columnFlowBox->width(), rowHeightAt(columnIndex));
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
    if(m_minSpaceShortage > 0.f && spaceShortage >= m_minSpaceShortage)
        return;
    assert(spaceShortage > 0.f);
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
    if(m_requiresBalancing && m_runs.size() < m_columnFlowBox->columnCount()) {
        m_runs.emplace_back(endOffset);
    }
}

void MultiColumnRowBox::resetColumnHeight(float columnHeight)
{
    m_runs.clear();
    m_minimumColumnHeight = 0.f;
    m_maxColumnHeight = columnHeight;
    if(!m_isFillBalance && columnHeight > 0.f) {
        m_columnHeight = columnHeight;
        m_requiresBalancing = false;
    } else {
        m_columnHeight = 0.f;
        m_requiresBalancing = true;
    }
}

bool MultiColumnRowBox::recalculateColumnHeight(bool balancing)
{
    auto prevColumnHeight = m_columnHeight;
    if(m_requiresBalancing) {
        if(!balancing)
            distributeImplicitBreaks();
        m_columnHeight = calculateColumnHeight(balancing);
    }

    m_columnHeight = constrainColumnHeight(m_columnHeight);
    if(prevColumnHeight == m_columnHeight)
        return false;
    m_minSpaceShortage = 0.f;
    m_runs.clear();
    return true;
}

float MultiColumnRowBox::constrainColumnHeight(float columnHeight) const
{
    if(m_maxColumnHeight > 0.f && columnHeight > m_maxColumnHeight)
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
    if(columnCount <= m_columnFlowBox->columnCount())
        return m_columnHeight;
    if(m_runs.size() >= m_columnFlowBox->columnCount())
        return m_columnHeight;
    if(m_maxColumnHeight > 0.f && m_columnHeight >= m_maxColumnHeight)
        return m_columnHeight;
    assert(m_minSpaceShortage > 0.f);
    return m_columnHeight + m_minSpaceShortage;
}

float MultiColumnRowBox::rowHeightAt(uint32_t columnIndex) const
{
    auto top = rowTopAt(columnIndex);
    auto bottom = top + m_columnHeight;
    if(bottom > m_rowBottom) {
        assert(columnIndex + 1 == numberOfColumns());
        bottom = m_rowBottom;
        assert(bottom >= top);
    }

    return bottom - top;
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

    while(columnCount < m_columnFlowBox->columnCount()) {
        auto index = findRunWithTallestColumns();
        m_runs[index].assumeAnotherImplicitBreak();
        columnCount++;
    }
}

MultiColumnSpanBox* MultiColumnSpanBox::create(BoxFrame* box, const BoxStyle* parentStyle)
{
    auto newStyle = BoxStyle::create(*parentStyle, Display::Block);
    auto newSpanner = new (newStyle->heap()) MultiColumnSpanBox(box, newStyle);
    newSpanner->setAnonymous(true);
    return newSpanner;
}

void MultiColumnSpanBox::updateOverflowRect()
{
    BoxFrame::updateOverflowRect();
    addOverflowRect(m_box->visualOverflowRect());
}

void MultiColumnSpanBox::computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const
{
    minPreferredWidth = m_box->minPreferredWidth();
    maxPreferredWidth = m_box->maxPreferredWidth();
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
    m_box->layout();

    updateWidth();
    updateHeight();
    updateOverflowRect();
}

void MultiColumnSpanBox::fragmentize(FragmentBuilder& builder, float top) const
{
}

void MultiColumnSpanBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(!m_box->hasLayer()) {
        m_box->paint(info, offset, phase);
    }
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

MultiColumnRowBox* MultiColumnFlowBox::lastRow() const
{
    for(auto box = parentBox()->lastChild(); box; box = box->prevSibling()) {
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

void MultiColumnFlowBox::skipColumnSpanBox(BoxFrame* box, float offset)
{
    auto columnSpanBox = box->columnSpanBox();
    assert(columnSpanBox && box->hasColumnSpanBox());
    auto prevColumnBox = columnSpanBox->prevMultiColumnBox();
    if(prevColumnBox && prevColumnBox->isMultiColumnRowBox()) {
        auto columnRow = to<MultiColumnRowBox>(prevColumnBox);
        if(offset < columnRow->rowTop())
            offset = columnRow->rowTop();
        columnRow->setRowBottom(offset);
    }

    auto nextColumnBox = columnSpanBox->nextMultiColumnBox();
    if(nextColumnBox && nextColumnBox->isMultiColumnRowBox()) {
        auto columnRow = to<MultiColumnRowBox>(nextColumnBox);
        columnRow->setRowTop(offset);
        m_currentRow = columnRow;
    }
}

MultiColumnRowBox* MultiColumnFlowBox::columnRowAtOffset(float offset) const
{
    auto row = m_currentRow;
    while(row->rowTop() > offset) {
        auto prevRow = row->prevRow();
        if(prevRow == nullptr)
            break;
        row = row->prevRow();
    }

    return row;
}

bool MultiColumnFlowBox::layoutColumns(bool balancing)
{
    m_currentRow = firstRow();
    if(m_currentRow)
        m_currentRow->setRowTop(0.f);
    BlockFlowBox::layout();
    if(m_currentRow) {
        assert(m_currentRow == lastRow());
        m_currentRow->setRowBottom(height());
    }

    bool changed = false;
    for(auto row = firstRow(); row; row = row->nextRow())
        changed |= row->recalculateColumnHeight(balancing);
    return changed;
}

void MultiColumnFlowBox::computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const
{
    BlockFlowBox::computePreferredWidths(minPreferredWidth, maxPreferredWidth);

    auto columnBlock = columnBlockFlowBox();
    auto columnStyle = columnBlock->style();
    auto columnCount = columnStyle->columnCount().value_or(1);

    auto totalColumnGap = m_columnGap * (columnCount - 1);
    if(auto columnWidth = columnStyle->columnWidth()) {
        minPreferredWidth = std::min(minPreferredWidth, columnWidth.value());
        maxPreferredWidth = std::max(maxPreferredWidth, columnWidth.value());
    } else {
        minPreferredWidth = minPreferredWidth * columnCount + totalColumnGap;
    }

    maxPreferredWidth = maxPreferredWidth * columnCount + totalColumnGap;
}

void MultiColumnFlowBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    auto columnBlock = columnBlockFlowBox();
    auto columnStyle = columnBlock->style();
    auto columnCount = columnStyle->columnCount();
    auto columnWidth = columnStyle->columnWidth();
    auto availableWidth = columnBlock->contentBoxWidth();

    if(!columnWidth.has_value() && columnCount.has_value()) {
        m_columnCount = columnCount.value();
        width = std::max(0.f, (availableWidth - ((columnCount.value() - 1) * m_columnGap)) / columnCount.value());
    } else if(columnWidth.has_value() && !columnCount.has_value()) {
        m_columnCount = std::max(1.f, std::floor((availableWidth + m_columnGap) / (columnWidth.value() + m_columnGap)));
        width = ((availableWidth + m_columnGap) / m_columnCount) - m_columnGap;
    } else {
        int count = std::floor((availableWidth + m_columnGap) / (columnWidth.value() + m_columnGap));
        m_columnCount = std::max(1, std::min(count, columnCount.value()));
        width = ((availableWidth + m_columnGap) / m_columnCount) - m_columnGap;
    }
}

static bool isValidColumnSpanBox(BoxFrame* box)
{
    return box && box->style()->columnSpan() == ColumnSpan::All && !box->isInline() && !box->isFloatingOrPositioned();
}

void MultiColumnFlowBox::build()
{
    MultiColumnRowBox* currentColumnRow = nullptr;
    auto columnBlock = columnBlockFlowBox();
    auto columnStyle = columnBlock->style();
    auto fillBalance = columnStyle->columnFill() == ColumnFill::Balance;
    m_columnGap = columnStyle->columnGap().value_or(columnStyle->fontSize());

    auto child = firstChild();
    while(child) {
        child->setIsInsideColumnFlow(true);
        if(auto box = to<BoxFrame>(child); isValidColumnSpanBox(box)) {
            auto newSpanner = MultiColumnSpanBox::create(box, columnStyle);
            columnBlock->addChild(newSpanner);
            box->setColumnSpanBox(newSpanner);
            box->setHasColumnSpanBox(true);
            if(currentColumnRow)
                currentColumnRow->setIsFillBalance(true);
            currentColumnRow = nullptr;
        } else if(!child->isFloatingOrPositioned()) {
            if(currentColumnRow == nullptr) {
                auto newRow = MultiColumnRowBox::create(this, columnStyle);
                columnBlock->addChild(newRow);
                newRow->setIsFillBalance(fillBalance);
                currentColumnRow = newRow;
            }

            if(child->firstChild() && !child->isInline() && !child->isFlexibleBox()
                && !child->isTableBox() && !child->style()->hasColumns()) {
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
    auto columnBlock = columnBlockFlowBox();
    auto columnStyle = columnBlock->style();

    float columnHeight = 0.f;
    if(auto height = columnBlock->computeHeightUsing(columnStyle->height()))
        columnHeight = columnBlock->adjustContentBoxHeight(height.value());
    columnHeight = columnBlock->constrainContentBoxHeight(columnHeight);

    for(auto row = firstRow(); row; row = row->nextRow()) {
        row->resetColumnHeight(columnHeight);
    }

    auto changed = layoutColumns(false);
    while(changed) {
        changed = layoutColumns(true);
    }
}

void MultiColumnFlowBox::fragmentize(FragmentBuilder& builder, float top) const
{
}

void MultiColumnFlowBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    BlockFlowBox::paint(info, offset, phase);
}

MultiColumnFlowBox::MultiColumnFlowBox(const RefPtr<BoxStyle>& style)
    : BlockFlowBox(nullptr, style)
{
    setIsInsideColumnFlow(true);
}

} // namespace plutobook
