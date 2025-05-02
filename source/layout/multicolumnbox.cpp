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
    newRow->setIsAnonymous(true);
    return newRow;
}

void MultiColumnRowBox::updateOverflowRect()
{
    BoxFrame::updateOverflowRect();
    auto columnRect = columnRectAt(numberOfColumns() - 1);
    auto overflowRect = m_columnFlowBox->visualOverflowRect().moved(columnRect.origin());
    addOverflowRect(columnRect.y, columnRect.bottom(), overflowRect.x, overflowRect.right());
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

void MultiColumnRowBox::layout(FragmentBuilder* fragmentainer)
{
    updateWidth();
    updateHeight();
    updateOverflowRect();
}

void MultiColumnRowBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
}

void MultiColumnRowBox::paintColumnRules(GraphicsContext& context, const Point& offset)
{
    if(style()->visibility() != Visibility::Visible)
        return;
    auto columnBlock = m_columnFlowBox->columnBlockFlowBox();
    auto columnStyle = columnBlock->style();
    auto columnDirection = columnStyle->direction();

    auto columnRuleWidth = columnStyle->columnRuleWidth();
    auto columnRuleStyle = columnStyle->columnRuleStyle();
    auto columnRuleColor = columnStyle->columnRuleColor();
    if(columnRuleWidth <= 0.f || columnRuleStyle <= LineStyle::Hidden || !columnRuleColor.isVisible())
        return;
    auto columnGap = m_columnFlowBox->columnGap();
    auto columnWidth = m_columnFlowBox->width();
    auto columnCount = numberOfColumns();

    Point adjustedOffset(offset + location());
    auto columnOffset = columnDirection == Direction::Ltr ? 0.f : width();
    auto ruleOffset = columnDirection == Direction::Ltr ? 0.f : width();
    auto boxSide = columnDirection == Direction::Ltr ? BoxSideLeft : BoxSideRight;
    for(uint32_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
        if(columnDirection == Direction::Ltr) {
            ruleOffset += columnWidth + columnGap / 2.f;
            columnOffset += columnWidth + columnGap;
        } else {
            ruleOffset -= columnWidth + columnGap / 2.f;
            columnOffset -= columnWidth + columnGap;
        }

        if(columnIndex < columnCount - 1) {
            Rect ruleRect(adjustedOffset.x + ruleOffset - columnRuleWidth / 2.f, adjustedOffset.y, columnRuleWidth, m_columnHeight);
            BorderPainter::paintBoxSide(context, boxSide, columnRuleStyle, columnRuleColor, ruleRect);
        }

        ruleOffset = columnOffset;
    }
}

Rect MultiColumnRowBox::columnRectAt(uint32_t columnIndex) const
{
    Rect columnRect(0, 0, m_columnFlowBox->width(), rowHeightAt(columnIndex));
    if(m_columnFlowBox->style()->direction() == Direction::Ltr) {
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
    if(spaceShortage <= 0.f)
        return;
    if(m_minSpaceShortage > 0.f) {
        m_minSpaceShortage = std::min(spaceShortage, m_minSpaceShortage);
    } else {
        m_minSpaceShortage = spaceShortage;
    }
}

void MultiColumnRowBox::updateMinimumColumnHeight(float height)
{
    m_minimumColumnHeight = std::max(height, m_minimumColumnHeight);
}

void MultiColumnRowBox::addContentRun(float endOffset)
{
    if(!m_runs.empty() && endOffset <= m_runs.back().breakOffset())
        return;
    if(m_requiresBalancing && m_runs.size() < m_columnFlowBox->columnCount()) {
        m_runs.emplace_back(endOffset);
    }
}

void MultiColumnRowBox::resetColumnHeight(float availableColumnHeight)
{
    m_runs.clear();
    m_minimumColumnHeight = 0.f;
    m_maxColumnHeight = availableColumnHeight;
    if(!m_isFillBalance && availableColumnHeight > 0.f) {
        m_columnHeight = availableColumnHeight;
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
    if(isNearlyEqual(prevColumnHeight, m_columnHeight))
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
    newSpanner->setIsAnonymous(true);
    return newSpanner;
}

void MultiColumnSpanBox::computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const
{
    minPreferredWidth = 0;
    maxPreferredWidth = 0;
}

void MultiColumnSpanBox::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
}

void MultiColumnSpanBox::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
}

void MultiColumnSpanBox::layout(FragmentBuilder* fragmentainer)
{
}

void MultiColumnSpanBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
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
    newColumn->setIsAnonymous(true);
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

float MultiColumnFlowBox::fragmentHeightForOffset(float offset) const
{
    offset += fragmentOffset();
    if(auto row = columnRowAtOffset(offset))
        return row->columnHeight();
    return 0.f;
}

float MultiColumnFlowBox::fragmentRemainingHeightForOffset(float offset, FragmentBoundaryRule rule) const
{
    offset += fragmentOffset();
    if(auto row = columnRowAtOffset(offset)) {
        assert(row->columnHeight() > 0.f);
        auto columnBottom = row->columnTopForOffset(offset) + row->columnHeight();
        auto remainingHeight = columnBottom - offset;
        if(rule == AssociateWithFormerFragment)
            return std::fmod(remainingHeight, row->columnHeight());
        return remainingHeight;
    }

    return 0.f;
}

void MultiColumnFlowBox::addForcedFragmentBreak(float offset)
{
    offset += fragmentOffset();
    if(auto row = columnRowAtOffset(offset)) {
        row->addContentRun(offset);
    }
}

void MultiColumnFlowBox::setFragmentBreak(float offset, float spaceShortage)
{
    offset += fragmentOffset();
    if(auto row = columnRowAtOffset(offset)) {
        row->recordSpaceShortage(spaceShortage);
    }
}

void MultiColumnFlowBox::updateMinimumFragmentHeight(float offset, float minHeight)
{
    offset += fragmentOffset();
    if(auto row = columnRowAtOffset(offset)) {
        row->updateMinimumColumnHeight(minHeight);
    }
}

void MultiColumnFlowBox::skipColumnSpanBox(MultiColumnSpanBox* spanner, float offset)
{
    offset += fragmentOffset();
    if(auto columnRow = spanner->prevColumnRowBox()) {
        if(offset < columnRow->rowTop())
            offset = columnRow->rowTop();
        columnRow->setRowBottom(offset);
    }

    if(auto columnRow = spanner->nextColumnRowBox()) {
        columnRow->setRowTop(offset);
        m_currentRow = columnRow;
    }
}

MultiColumnRowBox* MultiColumnFlowBox::columnRowAtOffset(float offset) const
{
    assert(m_currentRow && offset >= fragmentOffset());
    auto row = m_currentRow;
    while(row->rowTop() > offset) {
        auto prevRow = row->prevRow();
        if(prevRow == nullptr)
            break;
        row = prevRow;
    }

    return row;
}

bool MultiColumnFlowBox::layoutColumns(bool balancing)
{
    m_currentRow = firstRow();
    if(m_currentRow)
        m_currentRow->setRowTop(0.f);
    assert(fragmentOffset() == 0.f);
    BlockFlowBox::layout(this);
    assert(fragmentOffset() == 0.f);
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

void MultiColumnFlowBox::layout(FragmentBuilder* fragmentainer)
{
    auto columnBlock = columnBlockFlowBox();
    auto columnStyle = columnBlock->style();

    float columnHeight = 0.f;
    if(auto height = columnBlock->computeHeightUsing(columnStyle->height()))
        columnHeight = columnBlock->adjustBorderBoxHeight(height.value());
    columnHeight = columnBlock->constrainBorderBoxHeight(columnHeight);

    auto availableColumnHeight = std::max(0.f, columnHeight - columnBlock->borderAndPaddingHeight());
    for(auto row = firstRow(); row; row = row->nextRow()) {
        row->resetColumnHeight(availableColumnHeight);
    }

    auto changed = layoutColumns(false);
    while(changed) {
        changed = layoutColumns(true);
    }
}

static bool isValidColumnSpanBox(BoxFrame* box)
{
    return box && !box->isInline() && !box->isFloatingOrPositioned() && box->style()->columnSpan() == ColumnSpan::All;
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
        if(auto box = to<BoxFrame>(child); isValidColumnSpanBox(box)) {
            auto container = to<BlockFlowBox>(child->parentBox());
            assert(container && !container->isChildrenInline());
            auto newSpanner = MultiColumnSpanBox::create(box, container->style());
            container->insertChild(newSpanner, child->nextSibling());
            container->removeChild(child);
            columnBlock->addChild(child);
            if(currentColumnRow)
                currentColumnRow->setIsFillBalance(true);
            currentColumnRow = nullptr;
            child = newSpanner;
        } else if(!child->isFloatingOrPositioned()) {
            if(currentColumnRow == nullptr) {
                auto newRow = MultiColumnRowBox::create(this, columnStyle);
                columnBlock->addChild(newRow);
                newRow->setIsFillBalance(fillBalance);
                currentColumnRow = newRow;
            }

            if(child->firstChild() && child->isBlockFlowBox() && !child->isChildrenInline()
                && !child->style()->hasColumns()) {
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

} // namespace plutobook
