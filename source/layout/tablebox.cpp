/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "tablebox.h"
#include "borderpainter.h"
#include "fragmentbuilder.h"
#include "boxview.h"

#include <span>
#include <ranges>

namespace plutobook {

TableBox::TableBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockBox(node, style)
    , m_columns(style->heap())
    , m_captions(style->heap())
    , m_sections(style->heap())
    , m_borderHorizontalSpacing(0.f)
    , m_borderVerticalSpacing(0.f)
{
    switch(style->borderCollapse()) {
    case BorderCollapse::Separate:
        m_borderHorizontalSpacing = style->borderHorizontalSpacing();
        m_borderVerticalSpacing = style->borderVerticalSpacing();
        setIsBorderCollapsed(false);
        break;
    case BorderCollapse::Collapse:
        setIsBorderCollapsed(true);
        break;
    }
}

void TableBox::addChild(Box* newChild)
{
    if(newChild->isTableCaptionBox() || newChild->isTableColumnBox()
        || newChild->isTableSectionBox()) {
        appendChild(newChild);
        return;
    }

    auto lastSection = lastChild();
    if(lastSection && lastSection->isAnonymous() && lastSection->isTableSectionBox()) {
        lastSection->addChild(newChild);
        return;
    }

    auto newSection = createAnonymous(Display::TableRowGroup, style());
    appendChild(newSection);
    newSection->addChild(newChild);
}

void TableBox::updateOverflowRect()
{
    BlockBox::updateOverflowRect();
    for(auto caption : m_captions)
        addOverflowRect(caption, caption->x(), caption->y());
    for(auto section : m_sections) {
        addOverflowRect(section, section->x(), section->y());
    }
}

void TableBox::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    if(!m_columns.empty()) {
        m_tableLayout->computeIntrinsicWidths(minWidth, maxWidth);
        minWidth += borderHorizontalSpacing() * (m_columns.size() + 1);
        maxWidth += borderHorizontalSpacing() * (m_columns.size() + 1);
    }

    for(auto caption : m_captions) {
        caption->updateHorizontalPaddings(nullptr);

        minWidth = std::max(minWidth, caption->minPreferredWidth());
        maxWidth = std::max(maxWidth, caption->minPreferredWidth());
    }
}

void TableBox::computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const
{
    minPreferredWidth = 0;
    maxPreferredWidth = 0;
    computeIntrinsicWidths(minPreferredWidth, maxPreferredWidth);

    auto widthLength = style()->width();
    auto minWidthLength = style()->minWidth();
    auto maxWidthLength = style()->maxWidth();
    if(widthLength.isFixed() && widthLength.value() > 0) {
        maxPreferredWidth = std::max(minPreferredWidth, adjustContentBoxWidth(widthLength.value()));
        if(maxWidthLength.isFixed()) {
            maxPreferredWidth = std::min(maxPreferredWidth, adjustContentBoxWidth(maxWidthLength.value()));
            maxPreferredWidth = std::max(minPreferredWidth, maxPreferredWidth);
        }

        minPreferredWidth = maxPreferredWidth;
    }

    if(minWidthLength.isFixed() && minWidthLength.value() > 0) {
        minPreferredWidth = std::max(minPreferredWidth, adjustContentBoxWidth(minWidthLength.value()));
        maxPreferredWidth = std::max(maxPreferredWidth, adjustContentBoxWidth(minWidthLength.value()));
    }

    if(maxWidthLength.isFixed()) {
        maxPreferredWidth = std::min(maxPreferredWidth, adjustContentBoxWidth(maxWidthLength.value()));
        maxPreferredWidth = std::max(minPreferredWidth, maxPreferredWidth);
    }

    minPreferredWidth += borderAndPaddingWidth();
    maxPreferredWidth += borderAndPaddingWidth();
}

void TableBox::computeBorderWidths(float& borderTop, float& borderBottom, float& borderLeft, float& borderRight) const
{
    if(!isBorderCollapsed()) {
        BlockBox::computeBorderWidths(borderTop, borderBottom, borderLeft, borderRight);
        return;
    }

    borderTop = 0.f;
    borderBottom = 0.f;
    borderLeft = 0.f;
    borderRight = 0.f;
    if(auto section = topSection()) {
        auto row = section->firstRow();
        for(const auto& [col, cell] : row->cells()) {
            borderTop = std::max(borderTop, cell->borderTop());
        }
    }

    if(auto section = bottomSection()) {
        auto row = section->lastRow();
        for(const auto& [col, cell] : row->cells()) {
            borderBottom = std::max(borderBottom, cell->borderBottom());
        }
    }

    if(m_columns.empty())
        return;
    size_t startColumnIndex = 0;
    size_t endColumnIndex = m_columns.size() - 1;
    if(style()->isRightToLeftDirection()) {
        std::swap(startColumnIndex, endColumnIndex);
    }

    for(auto section : m_sections) {
        for(auto row : section->rows()) {
            if(auto cell = row->cellAt(startColumnIndex))
                borderLeft = std::max(borderLeft, cell->borderLeft());
            if(auto cell = row->cellAt(endColumnIndex)) {
                borderRight = std::max(borderRight, cell->borderRight());
            }
        }
    }
}

std::optional<float> TableBox::firstLineBaseline() const
{
    if(auto section = topSection()) {
        if(auto baseline = section->firstLineBaseline()) {
            return baseline.value() + section->y();
        }
    }

    return std::nullopt;
}

std::optional<float> TableBox::lastLineBaseline() const
{
    if(auto section = bottomSection()) {
        if(auto baseline = section->lastLineBaseline()) {
            return baseline.value() + section->y();
        }
    }

    return std::nullopt;
}

std::optional<float> TableBox::inlineBlockBaseline() const
{
    return firstLineBaseline();
}

TableSectionBox* TableBox::headerSection() const
{
    auto section = topSection();
    if(section && section->isTableHeader())
        return section;
    return nullptr;
}

TableSectionBox* TableBox::footerSection() const
{
    auto section = bottomSection();
    if(section && section->isTableFooter())
        return section;
    return nullptr;
}

TableSectionBox* TableBox::topSection() const
{
    if(m_sections.empty())
        return nullptr;
    return m_sections.front();
}

TableSectionBox* TableBox::bottomSection() const
{
    if(m_sections.empty())
        return nullptr;
    return m_sections.back();
}

TableSectionBox* TableBox::sectionAbove(const TableSectionBox* sectionBox) const
{
    if(sectionBox->isTableHeader())
        return nullptr;
    auto prevSection = sectionBox->isTableFooter() ? lastChild() : sectionBox->prevSibling();
    while(prevSection) {
        auto section = to<TableSectionBox>(prevSection);
        if(section && !section->isTableHeader() && !section->isTableFooter() && section->firstRow())
            return section;
        prevSection = prevSection->prevSibling();
    }

    return headerSection();
}

TableSectionBox* TableBox::sectionBelow(const TableSectionBox* sectionBox) const
{
    if(sectionBox->isTableFooter())
        return nullptr;
    auto nextSection = sectionBox->isTableHeader() ? firstChild() : sectionBox->nextSibling();
    while(nextSection) {
        auto section = to<TableSectionBox>(nextSection);
        if(section && !section->isTableHeader() && !section->isTableFooter() && section->firstRow())
            return section;
        nextSection = nextSection->nextSibling();
    }

    return footerSection();
}

TableCellBox* TableBox::cellAbove(const TableCellBox* cellBox) const
{
    const TableRowBox* rowBox = nullptr;
    if(auto rowIndex = cellBox->rowIndex()) {
        rowBox = cellBox->section()->rowAt(rowIndex - 1);
    } else if(auto section = sectionAbove(cellBox->section())) {
        rowBox = section->lastRow();
    }

    if(rowBox == nullptr)
        return nullptr;
    return rowBox->cellAt(cellBox->columnIndex());
}

TableCellBox* TableBox::cellBelow(const TableCellBox* cellBox) const
{
    const TableRowBox* rowBox = nullptr;
    auto rowIndex = cellBox->rowIndex() + cellBox->rowSpan() - 1;
    if(rowIndex < cellBox->section()->rowCount() - 1) {
        rowBox = cellBox->section()->rowAt(rowIndex + 1);
    } else if(auto section = sectionBelow(cellBox->section())) {
        rowBox = section->firstRow();
    }

    if(rowBox == nullptr)
        return nullptr;
    return rowBox->cellAt(cellBox->columnIndex());
}

TableCellBox* TableBox::cellBefore(const TableCellBox* cellBox) const
{
    auto columnIndex = cellBox->columnIndex();
    if(columnIndex == 0)
        return nullptr;
    return cellBox->row()->cellAt(columnIndex - 1);
}

TableCellBox* TableBox::cellAfter(const TableCellBox* cellBox) const
{
    auto columnIndex = cellBox->columnIndex() + cellBox->colSpan();
    if(columnIndex == m_columns.size())
        return nullptr;
    return cellBox->row()->cellAt(columnIndex);
}

float TableBox::availableHorizontalSpace() const
{
    if(!m_columns.empty() && !isBorderCollapsed())
        return contentBoxWidth() - borderHorizontalSpacing() * (m_columns.size() + 1);
    return contentBoxWidth();
}

void TableBox::layoutCaption(TableCaptionBox* caption, FragmentBuilder* fragmentainer)
{
    caption->updatePaddingWidths(this);
    caption->updateVerticalMargins(this);

    auto captionTop = height() + caption->marginTop();
    if(fragmentainer) {
        fragmentainer->enterFragment(captionTop);
    }

    caption->setY(captionTop);
    caption->layout(fragmentainer);
    caption->setX(caption->marginLeft());
    if(fragmentainer) {
        fragmentainer->leaveFragment(captionTop);
    }

    setHeight(captionTop + caption->height() + caption->marginBottom());
}

void TableBox::layout(FragmentBuilder* fragmentainer)
{
    updateWidth();
    setHeight(0.f);
    for(auto caption : m_captions) {
        if(caption->captionSide() == CaptionSide::Top) {
            layoutCaption(caption, fragmentainer);
        }
    }

    float tableHeight = 0.f;
    if(auto height = computeHeightUsing(style()->height()))
        tableHeight = adjustContentBoxHeight(height.value());
    tableHeight = constrainContentBoxHeight(tableHeight);
    if(hasOverrideHeight()) {
        tableHeight = std::max(tableHeight, overrideHeight() - borderAndPaddingHeight() - height());
    }

    setHeight(height() + borderAndPaddingTop());
    if(m_columns.empty()) {
        setHeight(tableHeight + height());
    } else {
        m_tableLayout->layout();
        auto columnLeft = borderHorizontalSpacing();
        for(auto& column : m_columns) {
            column.setX(columnLeft);
            columnLeft += column.width() + borderHorizontalSpacing();
        }

        if(style()->isRightToLeftDirection()) {
            for(auto& column : m_columns) {
                column.setX(columnLeft - column.width() - column.x());
            }
        }

        auto totalSectionHeight = borderVerticalSpacing();
        for(auto section : m_sections) {
            section->layout(nullptr);
            totalSectionHeight += section->height() + borderVerticalSpacing();
        }

        auto distributableTableHeight = tableHeight - totalSectionHeight;
        if(distributableTableHeight > 0.f) {
            for(auto section : m_sections) {
                section->distributeExcessHeightToRows(distributableTableHeight / m_sections.size());
            }
        }

        const auto* header = headerSection();
        const auto* footer = footerSection();

        auto sectionTop = height() + borderVerticalSpacing();
        for(auto section : m_sections) {
            if(fragmentainer) {
                fragmentainer->enterFragment(sectionTop);
            }

            float headerHeight = 0;
            float footerHeight = 0;
            if(header && header != section)
                headerHeight += borderVerticalSpacing() + header->height();
            if(footer && footer != section) {
                footerHeight += borderVerticalSpacing() + footer->height();
            }

            section->setY(sectionTop);
            section->setX(borderAndPaddingLeft());
            section->layoutRows(fragmentainer, headerHeight, footerHeight);
            section->updateOverflowRect();
            if(fragmentainer) {
                fragmentainer->leaveFragment(sectionTop);
            }

            sectionTop += section->height() + borderVerticalSpacing();
        }

        setHeight(sectionTop);
    }

    setHeight(height() + borderAndPaddingBottom());
    for(auto caption : m_captions) {
        if(caption->captionSide() == CaptionSide::Bottom) {
            layoutCaption(caption, fragmentainer);
        }
    }

    updateHeight();
    layoutPositionedBoxes();
    updateOverflowRect();
}

void TableBox::build()
{
    TableSectionBox* headerSection = nullptr;
    TableSectionBox* footerSection = nullptr;
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        if(auto section = to<TableSectionBox>(child)) {
            if(section->firstRow()) {
                switch(section->style()->display()) {
                case Display::TableHeaderGroup:
                    if(!headerSection)
                        headerSection = section;
                    else
                        m_sections.push_back(section);
                    break;
                case Display::TableFooterGroup:
                    if(!footerSection)
                        footerSection = section;
                    else
                        m_sections.push_back(section);
                    break;
                case Display::TableRowGroup:
                    m_sections.push_back(section);
                    break;
                default:
                    assert(false);
                }
            }
        } else if(auto column = to<TableColumnBox>(child)) {
            auto addColumn = [this](TableColumnBox* column) {
                auto columnSpanCount = column->span();
                while(columnSpanCount-- > 0) {
                    m_columns.emplace_back(column);
                }
            };

            if(column->style()->display() == Display::TableColumn) {
                addColumn(column);
            } else {
                if(auto child = column->firstChild()) {
                    do {
                        if(auto column = to<TableColumnBox>(child))
                            addColumn(column);
                        child = child->nextSibling();
                    } while(child);
                } else {
                    addColumn(column);
                }
            }
        } else if(auto caption = to<TableCaptionBox>(child)) {
            m_captions.push_back(caption);
        }
    }

    if(headerSection) {
        headerSection->setIsTableHeader(true);
        m_sections.push_front(headerSection);
    }

    if(footerSection) {
        footerSection->setIsTableFooter(true);
        m_sections.push_back(footerSection);
    }

    BlockBox::build();
    if(!m_columns.empty()) {
        m_tableLayout = TableLayoutAlgorithm::create(this);
        m_tableLayout->build();
    }

    if(isBorderCollapsed()) {
        for(auto section : m_sections) {
            for(auto row : section->rows()) {
                for(const auto& [col, cell] : row->cells()) {
                    if(cell.inColOrRowSpan())
                        continue;
                    const auto& edges = cell->collapsedBorderEdges();
                    if(m_collapsedBorderEdges == nullptr)
                        m_collapsedBorderEdges = std::make_unique<TableCollapsedBorderEdgeList>(heap());
                    m_collapsedBorderEdges->insert(edges.topEdge());
                    m_collapsedBorderEdges->insert(edges.bottomEdge());
                    m_collapsedBorderEdges->insert(edges.leftEdge());
                    m_collapsedBorderEdges->insert(edges.rightEdge());
                }
            }
        }
    }
}

void TableBox::paintDecorations(const PaintInfo& info, const Point& offset)
{
    Rect borderRect(offset, size());
    for(auto caption : m_captions) {
        borderRect.h -= caption->marginBoxHeight();
        if(caption->captionSide() == CaptionSide::Top) {
            borderRect.y += caption->marginBoxHeight();
        }
    }

    paintBackground(info, borderRect);
    if(!isBorderCollapsed()) {
        paintBorder(info, borderRect);
    }
}

void TableBox::paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    for(auto caption : m_captions) {
        if(!caption->hasLayer()) {
            caption->paint(info, offset, phase);
        }
    }

    for(auto section : m_sections) {
        if(!section->hasLayer()) {
            section->paint(info, offset, phase);
        }
    }

    auto shouldPaintCollapsedBorders = phase == PaintPhase::Decorations && m_collapsedBorderEdges && isBorderCollapsed();
    if(view()->currentPage()) {
        if(auto header = headerSection()) {
            const auto& rect = info.rect();
            if(rect.y > offset.y + header->y()) {
                Point headerOffset(offset.x, rect.y - header->y());
                if(isBorderCollapsed())
                    headerOffset.y += borderTop();
                header->paint(info, headerOffset, phase);
                if(shouldPaintCollapsedBorders) {
                    for(const auto& edge : *m_collapsedBorderEdges) {
                        header->paintCollapsedBorders(info, headerOffset, edge);
                    }
                }
            }
        }
    }

    if(shouldPaintCollapsedBorders) {
        for(const auto& edge : *m_collapsedBorderEdges) {
            for(auto section : m_sections | std::views::reverse) {
                section->paintCollapsedBorders(info, offset, edge);
            }
        }
    }

    if(view()->currentPage()) {
        if(auto footer = footerSection()) {
            const auto& rect = info.rect();
            if(rect.bottom() < offset.y + footer->y()) {
                float sectionBottom = 0.f;
                for(auto section : m_sections) {
                    auto sectionTop = offset.y + section->y();
                    if(sectionTop < rect.bottom()) {
                        for(auto row : section->rows() | std::views::reverse) {
                            auto rowBottom = sectionTop + row->y() + row->height();
                            if(rowBottom < rect.bottom()) {
                                sectionBottom = rowBottom;
                                break;
                            }
                        }
                    }
                }

                Point footerOffset(offset.x, sectionBottom - footer->y());
                footer->paint(info, footerOffset, phase);
                if(shouldPaintCollapsedBorders) {
                    for(const auto& edge : *m_collapsedBorderEdges) {
                        footer->paintCollapsedBorders(info, footerOffset, edge);
                    }
                }
            }
        }
    }
}

std::unique_ptr<TableLayoutAlgorithm> TableLayoutAlgorithm::create(TableBox* table)
{
    const auto* tableStyle = table->style();
    if(tableStyle->tableLayout() == TableLayout::Auto || tableStyle->width().isAuto())
        return AutoTableLayoutAlgorithm::create(table);
    return FixedTableLayoutAlgorithm::create(table);
}

std::unique_ptr<FixedTableLayoutAlgorithm> FixedTableLayoutAlgorithm::create(TableBox* table)
{
    return std::unique_ptr<FixedTableLayoutAlgorithm>(new (table->heap()) FixedTableLayoutAlgorithm(table));
}

void FixedTableLayoutAlgorithm::computeIntrinsicWidths(float& minWidth, float& maxWidth)
{
    for(const auto& width : m_widths) {
        if(width.isFixed()) {
            minWidth += width.value();
            maxWidth += width.value();
        }
    }
}

void FixedTableLayoutAlgorithm::build()
{
    const auto& columns = m_table->columns();
    m_widths.reserve(columns.size());
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        auto columnBox = columns[columnIndex].box();
        if(columnBox == nullptr) {
            m_widths.push_back(Length::Auto);
            continue;
        }

        auto columnStyleWidth = columnBox->style()->width();
        if(columnStyleWidth.isZero()) {
            m_widths.push_back(Length::Auto);
        } else {
            m_widths.push_back(columnStyleWidth);
        }
    }

    if(auto section = m_table->topSection()) {
        auto row = section->firstRow();
        for(const auto& [col, cell] : row->cells()) {
            if(!cell.inColOrRowSpan() && m_widths[col].isAuto()) {
                auto cellBox = cell.box();
                auto cellStyleWidth = cellBox->style()->width();
                if(cellStyleWidth.isFixed()) {
                    cellBox->updateHorizontalPaddings(nullptr);
                    cellStyleWidth = Length(Length::Type::Fixed, cellBox->adjustBorderBoxWidth(cellStyleWidth.value()) / cellBox->colSpan());
                } else if(cellStyleWidth.isPercent()) {
                    cellStyleWidth = Length(Length::Type::Percent, cellStyleWidth.value() / cellBox->colSpan());
                }

                if(!cellStyleWidth.isZero()) {
                    for(size_t index = 0; index < cellBox->colSpan(); ++index) {
                        m_widths[col + index] = cellStyleWidth;
                    }
                }
            }
        }
    }
}

void FixedTableLayoutAlgorithm::layout()
{
    auto availableWidth = m_table->availableHorizontalSpace();
    float totalFixedWidth = 0;
    float totalPercentWidth = 0;
    float totalPercent = 0;
    size_t autoColumnCount = 0;
    auto& columns = m_table->columns();
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        auto& column = columns[columnIndex];
        const auto& width = m_widths[columnIndex];
        if(width.isFixed()) {
            column.setWidth(width.value());
            totalFixedWidth += column.width();
        } else if(width.isPercent()) {
            column.setWidth(width.calc(availableWidth));
            totalPercentWidth += column.width();
            totalPercent += width.value();
        } else if(width.isAuto()) {
            column.setWidth(0);
            autoColumnCount++;
        }
    }

    auto totalWidth = totalFixedWidth + totalPercentWidth;
    if(autoColumnCount == 0 || totalWidth > availableWidth) {
        if(totalFixedWidth > 0 && totalWidth < availableWidth) {
            auto availableFixedWidth = availableWidth - totalPercentWidth;
            auto totalFixed = totalFixedWidth;
            totalFixedWidth = 0;
            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                auto& column = columns[columnIndex];
                const auto& width = m_widths[columnIndex];
                if(width.isFixed()) {
                    column.setWidth(width.value() * availableFixedWidth / totalFixed);
                    totalFixedWidth += column.width();
                }
            }
        }

        if(totalPercentWidth > 0 && totalFixedWidth >= availableWidth) {
            totalPercentWidth = 0;
            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                auto& column = columns[columnIndex];
                const auto& width = m_widths[columnIndex];
                if(width.isPercent()) {
                    column.setWidth(0);
                }
            }
        }

        if(totalPercentWidth > 0) {
            auto availablePercentWidth = availableWidth - totalFixedWidth;
            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                auto& column = columns[columnIndex];
                const auto& width = m_widths[columnIndex];
                if(width.isPercent()) {
                    column.setWidth(width.value() * availablePercentWidth / totalPercent);
                }
            }
        }
    } else {
        auto remainingWidth = availableWidth - totalFixedWidth - totalPercentWidth;
        for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
            auto& column = columns[columnIndex];
            const auto& width = m_widths[columnIndex];
            if(width.isAuto()) {
                column.setWidth(remainingWidth / autoColumnCount);
                remainingWidth -= column.width();
                autoColumnCount--;
            }
        }
    }
}

FixedTableLayoutAlgorithm::FixedTableLayoutAlgorithm(TableBox* table)
    : TableLayoutAlgorithm(table)
    , m_widths(table->heap())
{
}

std::unique_ptr<AutoTableLayoutAlgorithm> AutoTableLayoutAlgorithm::create(TableBox* table)
{
    return std::unique_ptr<AutoTableLayoutAlgorithm>(new (table->heap()) AutoTableLayoutAlgorithm(table));
}

static std::vector<float> distributeWidthToColumns(float availableWidth, std::span<TableColumnWidth> columns, bool constrained)
{
    size_t percentColumnCount = 0;
    size_t fixedColumnCount = 0;
    size_t autoColumnCount = 0;

    float totalPercent = 0.f;
    float totalFixedMaxWidth = 0.f;
    float totalAutoMaxWidth = 0.f;

    enum { kMinGuess, kPercentageGuess, kSpecifiedGuess, kMaxGuess, kGuessCount };
    float guessWidths[kGuessCount] = {0.f, 0.f, 0.f, 0.f};
    float guessWidthIncreases[kGuessCount] = {0.f, 0.f, 0.f, 0.f};
    for(const auto& column : columns) {
        if(column.width.isPercent()) {
            auto percentWidth = std::max(column.minWidth, column.width.calc(availableWidth));
            guessWidths[kMinGuess] += column.minWidth;
            guessWidths[kPercentageGuess] += percentWidth;
            guessWidths[kSpecifiedGuess] += percentWidth;
            guessWidths[kMaxGuess] += percentWidth;
            guessWidthIncreases[kPercentageGuess] += percentWidth - column.minWidth;
            totalPercent += column.width.value();
            percentColumnCount++;
        } else if(column.width.isFixed()) {
            guessWidths[kMinGuess] += column.minWidth;
            guessWidths[kPercentageGuess] += column.minWidth;
            guessWidths[kSpecifiedGuess] += column.maxWidth;
            guessWidths[kMaxGuess] += column.maxWidth;
            guessWidthIncreases[kSpecifiedGuess] += column.maxWidth - column.minWidth;
            totalFixedMaxWidth += column.maxWidth;
            fixedColumnCount++;
        } else {
            guessWidths[kMinGuess] += column.minWidth;
            guessWidths[kPercentageGuess] += column.minWidth;
            guessWidths[kSpecifiedGuess] += column.minWidth;
            guessWidths[kMaxGuess] += column.maxWidth;
            guessWidthIncreases[kMaxGuess] += column.maxWidth - column.minWidth;
            totalAutoMaxWidth += column.maxWidth;
            autoColumnCount++;
        }
    }

    availableWidth = std::max(availableWidth, guessWidths[kMinGuess]);

    auto startingGuess = kGuessCount;
    if(guessWidths[kMinGuess] >= availableWidth) { startingGuess = kMinGuess; }
    else if(guessWidths[kPercentageGuess] >= availableWidth) { startingGuess = kPercentageGuess; }
    else if(guessWidths[kSpecifiedGuess] >= availableWidth) { startingGuess = kSpecifiedGuess; }
    else if(guessWidths[kMaxGuess] >= availableWidth) { startingGuess = kMaxGuess; }

    std::vector<float> widths(columns.size());
    if(startingGuess == kMinGuess) {
        for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
            widths[columnIndex] = columns[columnIndex].minWidth;
        }
    } else if(startingGuess == kPercentageGuess) {
        auto percentWidthIncrease = guessWidthIncreases[kPercentageGuess];
        auto distributableWidth = availableWidth - guessWidths[kMinGuess];
        for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
            const auto& column = columns[columnIndex];
            if(column.width.isPercent()) {
                auto percentWidth = std::max(column.minWidth, column.width.calc(availableWidth));
                auto columnWidthIncrease = percentWidth - column.minWidth;
                float delta = 0.f;
                if(percentWidthIncrease > 0.f) {
                    delta = distributableWidth * columnWidthIncrease / percentWidthIncrease;
                } else {
                    delta = distributableWidth / percentColumnCount;
                }

                widths[columnIndex] = column.minWidth + delta;
            } else {
                widths[columnIndex] = column.minWidth;
            }
        }
    } else if(startingGuess == kSpecifiedGuess) {
        auto fixedWidthIncrease = guessWidthIncreases[kSpecifiedGuess];
        auto distributableWidth = availableWidth - guessWidths[kPercentageGuess];
        for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
            const auto& column = columns[columnIndex];
            if(column.width.isPercent()) {
                widths[columnIndex] = std::max(column.minWidth, column.width.calc(availableWidth));
            } else if(column.width.isFixed()) {
                auto columnWidthIncrease = column.maxWidth - column.minWidth;
                float delta = 0.f;
                if(fixedWidthIncrease > 0.f) {
                    delta = distributableWidth * columnWidthIncrease / fixedWidthIncrease;
                } else {
                    delta = distributableWidth / fixedColumnCount;
                }

                widths[columnIndex] = column.minWidth + delta;
            } else {
                widths[columnIndex] = column.minWidth;
            }
        }
    } else if(startingGuess == kMaxGuess) {
        auto autoWidthIncrease = guessWidthIncreases[kMaxGuess];
        auto distributableWidth = availableWidth - guessWidths[kSpecifiedGuess];
        for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
            const auto& column = columns[columnIndex];
            if(column.width.isPercent()) {
                widths[columnIndex] = std::max(column.minWidth, column.width.calc(availableWidth));
            } else if(column.width.isFixed()) {
                widths[columnIndex] = column.maxWidth;
            } else {
                auto columnWidthIncrease = column.maxWidth - column.minWidth;
                float delta = 0.f;
                if(autoWidthIncrease > 0.f) {
                    delta = distributableWidth * columnWidthIncrease / autoWidthIncrease;
                } else {
                    delta = distributableWidth / autoColumnCount;
                }

                widths[columnIndex] = column.minWidth + delta;
            }
        }
    } else {
        auto distributableWidth = availableWidth - guessWidths[kMaxGuess];
        if(autoColumnCount > 0) {
            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                const auto& column = columns[columnIndex];
                if(column.width.isPercent()) {
                    widths[columnIndex] = std::max(column.minWidth, column.width.calc(availableWidth));
                } else if(column.width.isFixed()) {
                    widths[columnIndex] = column.maxWidth;
                } else {
                    float delta = 0.f;
                    if(totalAutoMaxWidth > 0.f) {
                        delta = distributableWidth * column.maxWidth / totalAutoMaxWidth;
                    } else {
                        delta = distributableWidth / autoColumnCount;
                    }

                    widths[columnIndex] = column.maxWidth + delta;
                }
            }
        } else if(fixedColumnCount > 0 && constrained) {
            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                const auto& column = columns[columnIndex];
                if(column.width.isPercent()) {
                    widths[columnIndex] = std::max(column.minWidth, column.width.calc(availableWidth));
                } else if(column.width.isFixed()) {
                    float delta = 0.f;
                    if(totalFixedMaxWidth > 0.f) {
                        delta = distributableWidth * column.maxWidth / totalFixedMaxWidth;
                    } else {
                        delta = distributableWidth / fixedColumnCount;
                    }

                    widths[columnIndex] = column.maxWidth + delta;
                }
            }
        } else if(percentColumnCount > 0) {
            for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                const auto& column = columns[columnIndex];
                if(column.width.isPercent()) {
                    auto percentWidth = std::max(column.minWidth, column.width.calc(availableWidth));
                    float delta = 0.f;
                    if(totalPercent > 0.f) {
                        delta = distributableWidth * column.width.value() / totalPercent;
                    } else {
                        delta = distributableWidth / percentColumnCount;
                    }

                    widths[columnIndex] = percentWidth + delta;
                }
            }
        }
    }

    return widths;
}

static void distributeSpanCellToColumns(const TableCellBox* cellBox, std::span<TableColumnWidth> allColumns, float borderSpacing)
{
    auto columns = allColumns.subspan(cellBox->columnIndex(), cellBox->colSpan());
    auto cellStyleWidth = cellBox->style()->width();
    if(cellStyleWidth.isPercent()) {
        float totalPercent = 0.f;
        float totalNonPercentMaxWidth = 0.f;
        size_t nonPercentColumnCount = 0;
        for(const auto& column : columns) {
            if(column.width.isPercent()) {
                totalPercent += column.width.value();
            } else {
                totalNonPercentMaxWidth += column.maxWidth;
                nonPercentColumnCount++;
            }
        }

        auto surplusPercent = cellStyleWidth.value() - totalPercent;
        if(surplusPercent > 0.f && nonPercentColumnCount > 0) {
            for(auto& column : columns) {
                if(column.width.isPercent())
                    continue;
                float delta = 0.f;
                if(totalNonPercentMaxWidth > 0.f) {
                    delta = surplusPercent * column.maxWidth / totalNonPercentMaxWidth;
                } else {
                    delta = surplusPercent / nonPercentColumnCount;
                }

                column.width = Length(Length::Type::Percent, delta);
            }
        }
    }

    auto cellMinWidth = std::max(0.f, cellBox->minPreferredWidth() - borderSpacing * (cellBox->colSpan() - 1));
    auto cellMaxWidth = std::max(0.f, cellBox->maxPreferredWidth() - borderSpacing * (cellBox->colSpan() - 1));

    auto minWidths = distributeWidthToColumns(cellMinWidth, columns, true);
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        columns[columnIndex].minWidth = std::max(columns[columnIndex].minWidth, minWidths[columnIndex]);
    }

    auto maxWidths = distributeWidthToColumns(cellMaxWidth, columns, cellStyleWidth.isFixed());
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        columns[columnIndex].maxWidth = std::max(columns[columnIndex].maxWidth, maxWidths[columnIndex]);
    }
}

void AutoTableLayoutAlgorithm::computeIntrinsicWidths(float& minWidth, float& maxWidth)
{
    for(auto& columnWidth : m_columnWidths) {
        columnWidth.width = Length::Auto;
        if(columnWidth.maxFixedWidth > 0.f)
            columnWidth.width = Length(Length::Type::Fixed, columnWidth.maxFixedWidth);
        if(columnWidth.maxPercentWidth > 0.f)
            columnWidth.width = Length(Length::Type::Percent, columnWidth.maxPercentWidth);
        columnWidth.minWidth = 0.f;
        columnWidth.maxWidth = 0.f;
    }

    for(auto section : m_table->sections()) {
        for(auto row : section->rows()) {
            for(const auto& [col, cell] : row->cells()) {
                auto cellBox = cell.box();
                if(cell.inColOrRowSpan())
                    continue;
                cellBox->updateHorizontalPaddings(nullptr);
                if(cellBox->colSpan() == 1) {
                    auto& columnWidth = m_columnWidths[col];
                    columnWidth.minWidth = std::max(columnWidth.minWidth, cellBox->minPreferredWidth());
                    if(columnWidth.maxFixedWidth > 0.f) {
                        columnWidth.maxWidth = std::max(columnWidth.maxWidth, std::max(columnWidth.minWidth, columnWidth.maxFixedWidth));
                    } else {
                        columnWidth.maxWidth = std::max(columnWidth.maxWidth, cellBox->maxPreferredWidth());
                    }
                }
            }
        }
    }

    for(auto cellBox : m_spanningCells) {
        distributeSpanCellToColumns(cellBox, m_columnWidths, m_table->borderHorizontalSpacing());
    }

    float totalPercent = 0;
    for(auto& columnWidth : m_columnWidths) {
        if(columnWidth.width.isPercent()) {
            if(totalPercent + columnWidth.width.value() > 100.f)
                columnWidth.width = Length(Length::Type::Percent, 100.f - totalPercent);
            totalPercent += columnWidth.width.value();
        }
    }

    for(const auto& columnWidth : m_columnWidths) {
        minWidth += columnWidth.minWidth;
        maxWidth += columnWidth.maxWidth;
    }
}

void AutoTableLayoutAlgorithm::build()
{
    const auto& columns = m_table->columns();
    m_columnWidths.resize(columns.size());
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        auto& columnWidth = m_columnWidths[columnIndex];
        columnWidth.maxFixedWidth = 0.f;
        columnWidth.maxPercentWidth = 0.f;
        if(auto columnBox = columns[columnIndex].box()) {
            auto columnStyleWidth = columnBox->style()->width();
            if(columnStyleWidth.isFixed()) {
                columnWidth.maxFixedWidth = columnStyleWidth.value();
            } else if(columnStyleWidth.isPercent()) {
                columnWidth.maxPercentWidth = columnStyleWidth.value();
            }
        }
    }

    for(auto section : m_table->sections()) {
        for(auto row : section->rows()) {
            for(const auto& [col, cell] : row->cells()) {
                if(cell.inColOrRowSpan())
                    continue;
                auto cellBox = cell.box();
                if(cellBox->colSpan() > 1) {
                    m_spanningCells.push_back(cellBox);
                    continue;
                }

                auto cellStyleWidth = cellBox->style()->width();
                auto& columnWidth = m_columnWidths[col];
                if(cellStyleWidth.isFixed()) {
                    columnWidth.maxFixedWidth = std::max(columnWidth.maxFixedWidth, cellBox->adjustBorderBoxWidth(cellStyleWidth.value()));
                } else if(cellStyleWidth.isPercent()) {
                    columnWidth.maxPercentWidth = std::max(columnWidth.maxPercentWidth, cellStyleWidth.value());
                }
            }
        }
    }

    auto compare_func = [](const auto& a, const auto& b) { return a->colSpan() < b->colSpan(); };
    std::sort(m_spanningCells.begin(), m_spanningCells.end(), compare_func);
}

void AutoTableLayoutAlgorithm::layout()
{
    auto& columns = m_table->columns();
    auto widths = distributeWidthToColumns(m_table->availableHorizontalSpace(), m_columnWidths, true);
    for(size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
        columns[columnIndex].setWidth(widths[columnIndex]);
    }
}

AutoTableLayoutAlgorithm::AutoTableLayoutAlgorithm(TableBox* table)
    : TableLayoutAlgorithm(table)
    , m_columnWidths(table->heap())
    , m_spanningCells(table->heap())
{
}

TableSectionBox::TableSectionBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
    , m_rows(style->heap())
    , m_spanningCells(style->heap())
{
}

void TableSectionBox::addChild(Box* newChild)
{
    if(newChild->isTableRowBox()) {
        appendChild(newChild);
        return;
    }

    auto lastRow = lastChild();
    if(lastRow && lastRow->isAnonymous() && lastRow->isTableRowBox()) {
        lastRow->addChild(newChild);
        return;
    }

    auto newRow = createAnonymous(Display::TableRow, style());
    appendChild(newRow);
    newRow->addChild(newChild);
}

void TableSectionBox::updateOverflowRect()
{
    BoxFrame::updateOverflowRect();
    for(auto rowBox : m_rows) {
        addOverflowRect(rowBox, rowBox->x(), rowBox->y());
    }
}

std::optional<float> TableSectionBox::firstLineBaseline() const
{
    if(m_rows.empty())
        return std::nullopt;
    auto firstRowBox = m_rows.front();
    if(auto baseline = firstRowBox->maxBaseline())
        return baseline + firstRowBox->y();
    std::optional<float> baseline;
    for(const auto& [col, cell] : firstRowBox->cells()) {
        auto cellBox = cell.box();
        if(!cell.inColOrRowSpan() && cellBox->contentBoxHeight()) {
            auto candidate = firstRowBox->y() + cellBox->borderAndPaddingTop() + cellBox->contentBoxHeight();
            baseline = std::max(candidate, baseline.value_or(candidate));
        }
    }

    return baseline;
}

std::optional<float> TableSectionBox::lastLineBaseline() const
{
    if(m_rows.empty())
        return std::nullopt;
    auto lastRowBox = m_rows.back();
    if(auto baseline = lastRowBox->maxBaseline())
        return baseline + lastRowBox->y();
    std::optional<float> baseline;
    for(const auto& [col, cell] : lastRowBox->cells()) {
        auto cellBox = cell.box();
        if(!cell.inColOrRowSpan() && cellBox->contentBoxHeight()) {
            auto candidate = lastRowBox->y() + cellBox->borderAndPaddingTop() + cellBox->contentBoxHeight();
            baseline = std::max(candidate, baseline.value_or(candidate));
        }
    }

    return baseline;
}

void TableSectionBox::distributeExcessHeightToRows(float distributableHeight)
{
    float totalHeight = 0;
    float totalAutoHeight = 0;
    for(auto rowBox : m_rows) {
        totalHeight += rowBox->height();
        if(!rowBox->maxFixedHeight() && !rowBox->maxPercentHeight()) {
            totalAutoHeight += rowBox->height();
        }
    }

    auto availableHeight = distributableHeight + height();
    for(auto rowBox : m_rows) {
        if(rowBox->maxPercentHeight() && !rowBox->maxFixedHeight()) {
            auto height = availableHeight * rowBox->maxPercentHeight() / 100.f;
            auto delta = std::min(distributableHeight, std::max(0.f, height - rowBox->height()));
            distributableHeight -= delta;
            totalHeight += delta;
            rowBox->setHeight(delta + rowBox->height());
        }
    }

    if(distributableHeight <= 0.f)
        return;
    if(totalAutoHeight > 0.f) {
        for(auto rowBox : m_rows) {
            if(!rowBox->maxFixedHeight() && !rowBox->maxPercentHeight()) {
                auto delta = distributableHeight * rowBox->height() / totalAutoHeight;
                rowBox->setHeight(delta + rowBox->height());
            }
        }
    } else {
        for(auto rowBox : m_rows) {
            auto delta = distributableHeight * rowBox->height() / totalHeight;
            rowBox->setHeight(delta + rowBox->height());
        }
    }
}

void TableSectionBox::layoutRows(FragmentBuilder* fragmentainer, float headerHeight, float footerHeight)
{
    float rowTop = 0;
    auto verticalSpacing = table()->borderVerticalSpacing();
    for(size_t rowIndex = 0; rowIndex < m_rows.size(); ++rowIndex) {
        auto rowBox = m_rows[rowIndex];
        if(fragmentainer) {
            auto fragmentHeight = fragmentainer->fragmentHeightForOffset(rowTop);
            if(fragmentHeight > 0.f) {
                auto maxRowHeight = rowBox->height();
                for(const auto& [col, cell] : rowBox->cells()) {
                    auto cellBox = cell.box();
                    if(cell.inColOrRowSpan())
                        continue;
                    auto rowHeight = -verticalSpacing;
                    for(size_t index = 0; index < cellBox->rowSpan(); ++index) {
                        auto row = m_rows[rowIndex + index];
                        rowHeight += verticalSpacing + row->height();
                    }

                    maxRowHeight = std::max(rowHeight, maxRowHeight);
                }

                auto remainingHeight = fragmentainer->fragmentRemainingHeightForOffset(rowTop, AssociateWithLatterFragment);
                if(maxRowHeight >= remainingHeight - footerHeight - verticalSpacing && maxRowHeight < fragmentHeight) {
                    rowTop += remainingHeight + headerHeight;
                    if(table()->isBorderCollapsed()) {
                        if(headerHeight) {
                            rowTop += table()->borderTop();
                        } else {
                            float borderTop = 0.f;
                            for(const auto& [col, cell] : rowBox->cells())
                                borderTop = std::max(borderTop, cell->borderTop());
                            rowTop += borderTop;
                        }
                    }
                }
            }

            fragmentainer->enterFragment(rowTop);
        }

        rowBox->setX(0.f);
        rowBox->setY(rowTop);
        float rowHeightIncreaseForFragmentation = 0;
        for(const auto& [col, cell] : rowBox->cells()) {
            auto cellBox = cell.box();
            if(cell.inColOrRowSpan())
                continue;
            auto rowHeight = -verticalSpacing;
            for(size_t index = 0; index < cellBox->rowSpan(); ++index) {
                auto row = m_rows[rowIndex + index];
                rowHeight += verticalSpacing + row->height();
            }

            cellBox->setY(0.f);
            cellBox->setOverrideHeight(rowHeight);
            cellBox->layout(fragmentainer);
            if(fragmentainer && cellBox->height() > rowHeight) {
                rowHeightIncreaseForFragmentation = std::max(rowHeightIncreaseForFragmentation, cellBox->height() - rowHeight);
                cellBox->setHeight(rowHeight);
            }
        }

        if(fragmentainer) {
            fragmentainer->leaveFragment(rowTop);
            if(rowHeightIncreaseForFragmentation > 0) {
                rowBox->setHeight(rowHeightIncreaseForFragmentation + rowBox->height());
                for(const auto& [col, cell] : rowBox->cells()) {
                    auto cellBox = cell.box();
                    if(!cell.inColSpan()) {
                        cellBox->setHeight(rowHeightIncreaseForFragmentation + cellBox->height());
                        cellBox->updateOverflowRect();
                    }
                }
            }
        }

        rowBox->updateOverflowRect();
        rowTop += verticalSpacing + rowBox->height();
    }

    setHeight(rowTop - verticalSpacing);
}

static void distributeSpanCellToRows(TableCellBox* cellBox, std::span<TableRowBox*> allRows, float borderSpacing)
{
    auto cellMinHeight = cellBox->heightForRowSizing();
    auto rows = allRows.subspan(cellBox->rowIndex(), cellBox->rowSpan());
    for(auto rowBox : rows)
        cellMinHeight -= rowBox->height();
    cellMinHeight -= borderSpacing * (rows.size() - 1);
    if(cellMinHeight > 0.f) {
        auto lastRow = rows.back();
        lastRow->setHeight(cellMinHeight + lastRow->height());
    }
}

void TableSectionBox::layout(FragmentBuilder* fragmentainer)
{
    setWidth(table()->contentBoxWidth());
    const auto& columns = table()->columns();
    auto horizontalSpacing = table()->borderHorizontalSpacing();
    auto direction = table()->style()->direction();
    for(auto rowBox : m_rows) {
        float cellMaxAscent = 0.f;
        float cellMaxDescent = 0.f;
        float cellMaxHeight = rowBox->maxFixedHeight();
        for(const auto& [col, cell] : rowBox->cells()) {
            auto cellBox = cell.box();
            if(cell.inColOrRowSpan())
                continue;
            auto width = -horizontalSpacing;
            for(size_t index = 0; index < cellBox->colSpan(); ++index) {
                const auto& column = columns[col + index];
                width += horizontalSpacing + column.width();
            }

            if(direction == Direction::Ltr) {
                cellBox->setX(columns[col].x());
            } else {
                cellBox->setX(columns[col + cellBox->colSpan() - 1].x());
            }

            cellBox->clearOverrideSize();
            cellBox->setOverrideWidth(width);
            cellBox->updatePaddingWidths(table());
            cellBox->layout(fragmentainer);

            if(cellBox->rowSpan() == 1)
                cellMaxHeight = std::max(cellMaxHeight, cellBox->heightForRowSizing());
            if(cellBox->isBaselineAligned()) {
                if(cellBox->rowSpan() == 1) {
                    auto ascent = cellBox->cellBaselinePosition();
                    auto descent = cellBox->height() - ascent;
                    cellMaxAscent = std::max(cellMaxAscent, ascent);
                    cellMaxDescent = std::max(cellMaxDescent, descent);
                    cellMaxHeight = std::max(cellMaxHeight, cellMaxAscent + cellMaxDescent);
                } else {
                    cellMaxAscent = std::max(cellMaxAscent, cellBox->cellBaselinePosition());
                    cellMaxHeight = std::max(cellMaxHeight, cellMaxAscent);
                }
            }
        }

        rowBox->setWidth(width());
        rowBox->setHeight(cellMaxHeight);
        rowBox->setMaxBaseline(cellMaxAscent);
    }

    auto verticalSpacing = table()->borderVerticalSpacing();
    for(auto cellBox : m_spanningCells) {
        distributeSpanCellToRows(cellBox, m_rows, verticalSpacing);
    }

    auto sectionHeight = -verticalSpacing;
    for(auto rowBox : m_rows) {
        sectionHeight += verticalSpacing + rowBox->height();
    }

    setHeight(sectionHeight);
}

void TableSectionBox::build()
{
    for(auto rowBox = firstRow(); rowBox; rowBox = rowBox->nextRow()) {
        rowBox->setRowIndex(m_rows.size());
        auto rowStyleHeight = rowBox->style()->height();
        if(rowStyleHeight.isFixed()) {
            rowBox->setMaxFixedHeight(rowStyleHeight.value());
        } else if(rowStyleHeight.isPercent()) {
            rowBox->setMaxPercentHeight(rowStyleHeight.value());
        } else {
            rowBox->setMaxFixedHeight(0.f);
            rowBox->setMaxPercentHeight(0.f);
        }

        m_rows.push_back(rowBox);
    }

    const uint32_t rowCount = m_rows.size();
    for(uint32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
        auto rowBox = m_rows[rowIndex];
        uint32_t columnIndex = 0;
        for(auto cellBox = rowBox->firstCell(); cellBox; cellBox = cellBox->nextCell()) {
            auto rowSpan = rowCount - rowIndex;
            if(cellBox->rowSpan() > 0) {
                rowSpan = std::min(rowSpan, cellBox->rowSpan());
            }

            const auto& cells = rowBox->cells();
            while(true) {
                if(!cells.contains(columnIndex))
                    break;
                ++columnIndex;
            }

            cellBox->setRowSpan(rowSpan);
            cellBox->setColumnIndex(columnIndex);
            if(cellBox->rowSpan() > 1) {
                m_spanningCells.push_back(cellBox);
            } else {
                auto cellStyleHeight = cellBox->style()->height();
                if(cellStyleHeight.isFixed()) {
                    rowBox->setMaxFixedHeight(std::max(rowBox->maxFixedHeight(), cellStyleHeight.value()));
                } else if(cellStyleHeight.isPercent()) {
                    rowBox->setMaxPercentHeight(std::max(rowBox->maxPercentHeight(), cellStyleHeight.value()));
                }
            }

            for(uint32_t row = 0; row < cellBox->rowSpan(); ++row) {
                auto& cells = m_rows[row + rowIndex]->cells();
                for(uint32_t col = 0; col < cellBox->colSpan(); ++col) {
                    cells.emplace(col + columnIndex, TableCell(cellBox, col > 0, row > 0));
                }
            }

            columnIndex += cellBox->colSpan();
            auto& columns = table()->columns();
            while(columnIndex > columns.size()) {
                columns.emplace_back(nullptr);
            }
        }
    }

    float totalPercent = 0;
    for(auto rowBox : m_rows) {
        if(rowBox->maxPercentHeight() && !rowBox->maxFixedHeight()) {
            rowBox->setMaxPercentHeight(std::min(100.0f - totalPercent, rowBox->maxPercentHeight()));
            totalPercent += rowBox->maxPercentHeight();
        }
    }

    auto compare_func = [](const auto& a, const auto& b) { return a->rowSpan() < b->rowSpan(); };
    std::sort(m_spanningCells.begin(), m_spanningCells.end(), compare_func);
    BoxFrame::build();
}

void TableSectionBox::paintCollapsedBorders(const PaintInfo& info, const Point& offset, const TableCollapsedBorderEdge& currentEdge) const
{
    for(auto row : m_rows | std::views::reverse) {
        Point adjustedOffset(offset + location() + row->location());
        for(const auto& [col, cell] : row->cells()) {
            auto cellBox = cell.box();
            if(!cell.inColOrRowSpan()) {
                cellBox->paintCollapsedBorders(info, adjustedOffset, currentEdge);
            }
        }
    }
}

void TableSectionBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    for(auto rowBox : m_rows) {
        if(phase == PaintPhase::Outlines && !rowBox->hasLayer() && rowBox->style()->visibility() == Visibility::Visible) {
            rowBox->paintOutlines(info, offset + location() + rowBox->location());
        }

        for(const auto& [col, cell] : rowBox->cells()) {
            auto cellBox = cell.box();
            if(cell.inColOrRowSpan() || (cellBox->emptyCells() == EmptyCells::Hide && !cellBox->firstChild()))
                continue;
            Point adjustedOffset(offset + location() + rowBox->location());
            if(phase == PaintPhase::Decorations) {
                if(auto columnBox = table()->columnAt(col)) {
                    if(auto columnGroupBox = columnBox->columnGroup())
                        cellBox->paintBackgroundBehindCell(info, adjustedOffset, columnGroupBox->style());
                    cellBox->paintBackgroundBehindCell(info, adjustedOffset, columnBox->style());
                }

                cellBox->paintBackgroundBehindCell(info, adjustedOffset, style());
                if(!rowBox->hasLayer()) {
                    cellBox->paintBackgroundBehindCell(info, adjustedOffset, rowBox->style());
                }
            }

            if(!cellBox->hasLayer() && !rowBox->hasLayer()) {
                cellBox->paint(info, adjustedOffset, phase);
            }
        }
    }

    if(phase == PaintPhase::Outlines && style()->visibility() == Visibility::Visible) {
        paintOutlines(info, offset + location());
    }
}

TableRowBox::TableRowBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
    , m_cells(style->heap())
{
}

void TableRowBox::addChild(Box* newChild)
{
    if(newChild->isTableCellBox()) {
        appendChild(newChild);
        return;
    }

    auto lastCell = lastChild();
    if(lastCell && lastCell->isAnonymous() && lastCell->isTableCellBox()) {
        lastCell->addChild(newChild);
        return;
    }

    auto newCell = createAnonymous(Display::TableCell, style());
    appendChild(newCell);
    newCell->addChild(newChild);
}

void TableRowBox::updateOverflowRect()
{
    BoxFrame::updateOverflowRect();
    for(const auto& [col, cell] : m_cells) {
        auto cellBox = cell.box();
        if(!cell.inColOrRowSpan()) {
            addOverflowRect(cellBox, cellBox->x(), cellBox->y());
        }
    }
}

TableCellBox* TableRowBox::cellAt(uint32_t columnIndex) const
{
    auto it = m_cells.find(columnIndex);
    if(it == m_cells.end())
        return nullptr;
    return it->second.box();
}

void TableRowBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(phase == PaintPhase::Outlines && style()->visibility() == Visibility::Visible) {
        paintOutlines(info, offset + location());
    }

    for(const auto& [col, cell] : m_cells) {
        auto cellBox = cell.box();
        if(cell.inColOrRowSpan() || (cellBox->emptyCells() == EmptyCells::Hide && !cellBox->firstChild()))
            continue;
        Point adjustedOffset(offset + location());
        if(phase == PaintPhase::Decorations)
            cellBox->paintBackgroundBehindCell(info, adjustedOffset, style());
        if(!cellBox->hasLayer()) {
            cellBox->paint(info, adjustedOffset, phase);
        }
    }
}

TableColumnBox::TableColumnBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
}

TableColumnBox* TableColumnBox::columnGroup() const
{
    auto column = to<TableColumnBox>(parentBox());
    if(column && column->style()->display() == Display::TableColumnGroup)
        return column;
    return nullptr;
}

bool TableCollapsedBorderEdge::isSameIgnoringColor(const TableCollapsedBorderEdge& edge) const
{
    return m_source == edge.source() && m_style == edge.style() && m_width == edge.width();
}

bool TableCollapsedBorderEdge::isLessThan(const TableCollapsedBorderEdge& edge) const
{
    if(!edge.exists())
        return false;
    if(!exists())
        return true;
    if(style() == LineStyle::Hidden)
        return false;
    if(edge.style() == LineStyle::Hidden)
        return true;
    if(edge.style() == LineStyle::None)
        return false;
    if(style() == LineStyle::None)
        return true;
    if(width() != edge.width())
        return width() < edge.width();
    if(style() != edge.style())
        return style() < edge.style();
    return source() < edge.source();
}

std::unique_ptr<TableCollapsedBorderEdges> TableCollapsedBorderEdges::create(const TableCellBox* cellBox)
{
    return std::unique_ptr<TableCollapsedBorderEdges>(new (cellBox->heap()) TableCollapsedBorderEdges(cellBox));
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::chooseEdge(const TableCollapsedBorderEdge& a, const TableCollapsedBorderEdge& b)
{
    const auto& edge = a < b ? b : a;
    if(edge.style() == LineStyle::Hidden)
        return TableCollapsedBorderEdge();
    return edge;
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::getTopEdge(TableCollapsedBorderSource source, const BoxStyle* style)
{
    return TableCollapsedBorderEdge(source, style->borderTopStyle(), style->borderTopWidth(), style->borderTopColor());
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::getBottomEdge(TableCollapsedBorderSource source, const BoxStyle* style)
{
    return TableCollapsedBorderEdge(source, style->borderBottomStyle(), style->borderBottomWidth(), style->borderBottomColor());
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::getLeftEdge(TableCollapsedBorderSource source, const BoxStyle* style)
{
    return TableCollapsedBorderEdge(source, style->borderLeftStyle(), style->borderLeftWidth(), style->borderLeftColor());
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::getRightEdge(TableCollapsedBorderSource source, const BoxStyle* style)
{
    return TableCollapsedBorderEdge(source, style->borderRightStyle(), style->borderRightWidth(), style->borderRightColor());
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::calcTopEdge(const TableCellBox* cellBox)
{
    auto table = cellBox->table();
    auto cellAbove = table->cellAbove(cellBox);
    auto edge = getTopEdge(TableCollapsedBorderSource::Cell, cellBox->style());
    if(cellAbove) {
        edge = chooseEdge(getBottomEdge(TableCollapsedBorderSource::Cell, cellAbove->style()), edge);
        if(!edge.exists()) {
            return edge;
        }
    }

    edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::Row, cellBox->row()->style()));
    if(!edge.exists()) {
        return edge;
    }

    if(cellAbove) {
        edge = chooseEdge(getBottomEdge(TableCollapsedBorderSource::Row, cellAbove->row()->style()), edge);
        if(!edge.exists()) {
            return edge;
        }
    }

    if(auto section = cellBox->section(); cellBox->rowIndex() == 0) {
        edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::RowGroup, section->style()));
        if(!edge.exists()) {
            return edge;
        }

        if(auto sectionAbove = table->sectionAbove(section)) {
            edge = chooseEdge(getBottomEdge(TableCollapsedBorderSource::RowGroup, sectionAbove->style()), edge);
            if(!edge.exists()) {
                return edge;
            }
        } else {
            if(auto column = cellBox->column()) {
                edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::Column, column->style()));
                if(!edge.exists()) {
                    return edge;
                }

                if(auto columnGroup = column->columnGroup()) {
                    edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::ColumnGroup, columnGroup->style()));
                    if(!edge.exists()) {
                        return edge;
                    }
                }
            }

            edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::Table, table->style()));
            if(!edge.exists()) {
                return edge;
            }
        }
    }

    return edge;
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::calcBottomEdge(const TableCellBox* cellBox)
{
    auto table = cellBox->table();
    auto cellBelow = table->cellBelow(cellBox);
    auto edge = getBottomEdge(TableCollapsedBorderSource::Cell, cellBox->style());
    if(cellBelow) {
        edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::Cell, cellBelow->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    edge = chooseEdge(edge, getBottomEdge(TableCollapsedBorderSource::Row, cellBox->row()->style()));
    if(!edge.exists()) {
        return edge;
    }

    if(cellBelow) {
        edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::Row, cellBelow->row()->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    if(auto section = cellBox->section(); cellBox->rowIndex() + cellBox->rowSpan() == section->rowCount()) {
        edge = chooseEdge(edge, getBottomEdge(TableCollapsedBorderSource::RowGroup, section->style()));
        if(!edge.exists()) {
            return edge;
        }

        if(auto sectionBelow = table->sectionBelow(section)) {
            edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::RowGroup, sectionBelow->style()));
            if(!edge.exists()) {
                return edge;
            }
        } else {
            if(auto column = cellBox->column()) {
                edge = chooseEdge(edge, getBottomEdge(TableCollapsedBorderSource::Column, column->style()));
                if(!edge.exists()) {
                    return edge;
                }

                if(auto columnGroup = column->columnGroup()) {
                    edge = chooseEdge(edge, getBottomEdge(TableCollapsedBorderSource::ColumnGroup, columnGroup->style()));
                    if(!edge.exists()) {
                        return edge;
                    }
                }
            }

            edge = chooseEdge(edge, getBottomEdge(TableCollapsedBorderSource::Table, table->style()));
            if(!edge.exists()) {
                return edge;
            }
        }
    }

    return edge;
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::calcLeftEdge(const TableCellBox* cellBox)
{
    auto table = cellBox->table();
    auto direction = table->style()->direction();
    auto cellBefore = direction == Direction::Ltr ? table->cellBefore(cellBox) : table->cellAfter(cellBox);
    auto edge = getLeftEdge(TableCollapsedBorderSource::Cell, cellBox->style());
    if(cellBefore) {
        auto rightEdge = getRightEdge(TableCollapsedBorderSource::Cell, cellBefore->style());
        edge = direction == Direction::Ltr ? chooseEdge(rightEdge, edge) : chooseEdge(edge, rightEdge);
        if(!edge.exists()) {
            return edge;
        }
    }

    bool isStartColumn;
    if(direction == Direction::Ltr) {
        isStartColumn = cellBox->columnIndex() == 0;
    } else {
        isStartColumn = cellBox->columnIndex() + cellBox->colSpan() == table->columnCount();
    }

    if(isStartColumn) {
        edge = chooseEdge(edge, getLeftEdge(TableCollapsedBorderSource::Row, cellBox->row()->style()));
        if(!edge.exists()) {
            return edge;
        }

        edge = chooseEdge(edge, getLeftEdge(TableCollapsedBorderSource::RowGroup, cellBox->section()->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    if(auto column = table->columnAt(direction == Direction::Ltr ? cellBox->columnIndex() : cellBox->columnIndex() + cellBox->colSpan() - 1)) {
        edge = chooseEdge(edge, getLeftEdge(TableCollapsedBorderSource::Column, column->style()));
        if(!edge.exists()) {
            return edge;
        }

        if(auto columnGroup = column->columnGroup(); columnGroup && (direction == Direction::Ltr ? !column->prevSibling() : !column->nextSibling())) {
            edge = chooseEdge(edge, getLeftEdge(TableCollapsedBorderSource::ColumnGroup, columnGroup->style()));
            if(!edge.exists()) {
                return edge;
            }
        }
    }

    if(!isStartColumn) {
        if(auto column = table->columnAt(direction == Direction::Ltr ? cellBox->columnIndex() - 1 : cellBox->columnIndex() + cellBox->colSpan())) {
            auto rightEdge = getRightEdge(TableCollapsedBorderSource::Column, column->style());
            edge = direction == Direction::Ltr ? chooseEdge(rightEdge, edge) : chooseEdge(edge, rightEdge);
            if(!edge.exists()) {
                return edge;
            }
        }
    } else {
        edge = chooseEdge(edge, getLeftEdge(TableCollapsedBorderSource::Table, table->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    return edge;
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::calcRightEdge(const TableCellBox* cellBox)
{
    auto table = cellBox->table();
    auto direction = table->style()->direction();
    auto cellAfter = direction == Direction::Ltr ? table->cellAfter(cellBox) : table->cellBefore(cellBox);
    auto edge = getRightEdge(TableCollapsedBorderSource::Cell, cellBox->style());
    if(cellAfter) {
        auto leftEdge = getLeftEdge(TableCollapsedBorderSource::Cell, cellAfter->style());
        edge = direction == Direction::Ltr ? chooseEdge(edge, leftEdge) : chooseEdge(leftEdge, edge);
        if(!edge.exists()) {
            return edge;
        }
    }

    bool isEndColumn;
    if(direction == Direction::Ltr) {
        isEndColumn = cellBox->columnIndex() + cellBox->colSpan() == table->columnCount();
    } else {
        isEndColumn = cellBox->columnIndex() == 0;
    }

    if(isEndColumn) {
        edge = chooseEdge(edge, getRightEdge(TableCollapsedBorderSource::Row, cellBox->row()->style()));
        if(!edge.exists()) {
            return edge;
        }

        edge = chooseEdge(edge, getRightEdge(TableCollapsedBorderSource::RowGroup, cellBox->section()->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    if(auto column = table->columnAt(direction == Direction::Ltr ? cellBox->columnIndex() + cellBox->colSpan() - 1 : cellBox->columnIndex())) {
        edge = chooseEdge(edge, getRightEdge(TableCollapsedBorderSource::Column, column->style()));
        if(!edge.exists()) {
            return edge;
        }

        if(auto columnGroup = column->columnGroup(); columnGroup && (direction == Direction::Ltr ? !column->nextSibling() : !column->prevSibling())) {
            edge = chooseEdge(edge, getRightEdge(TableCollapsedBorderSource::ColumnGroup, columnGroup->style()));
            if(!edge.exists()) {
                return edge;
            }
        }
    }

    if(!isEndColumn) {
        if(auto column = table->columnAt(direction == Direction::Ltr ? cellBox->columnIndex() + cellBox->colSpan() : cellBox->columnIndex() - 1)) {
            auto leftEdge = getLeftEdge(TableCollapsedBorderSource::Column, column->style());
            edge = direction == Direction::Ltr ? chooseEdge(edge, leftEdge) : chooseEdge(leftEdge, edge);
            if(!edge.exists()) {
                return edge;
            }
        }
    } else {
        edge = chooseEdge(edge, getRightEdge(TableCollapsedBorderSource::Table, table->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    return edge;
}

TableCollapsedBorderEdges::TableCollapsedBorderEdges(const TableCellBox* cellBox)
    : m_topEdge(calcTopEdge(cellBox))
    , m_bottomEdge(calcBottomEdge(cellBox))
    , m_leftEdge(calcLeftEdge(cellBox))
    , m_rightEdge(calcRightEdge(cellBox))
{
}

TableCellBox::TableCellBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style)
{
}

bool TableCellBox::isBaselineAligned() const
{
    switch(style()->verticalAlignType()) {
    case VerticalAlignType::Baseline:
    case VerticalAlignType::TextBottom:
    case VerticalAlignType::TextTop:
    case VerticalAlignType::Super:
    case VerticalAlignType::Sub:
    case VerticalAlignType::Length:
        return true;
    default:
        return false;
    }
}

float TableCellBox::cellBaselinePosition() const
{
    if(auto baseline = firstLineBaseline())
        return baseline.value();
    return paddingTop() + borderTop() + contentBoxHeight();
}

float TableCellBox::heightForRowSizing() const
{
    auto cellStyleHeight = style()->height();
    if(cellStyleHeight.isFixed())
        return std::max(height(), adjustBorderBoxHeight(cellStyleHeight.value()));
    return height();
}

float TableCellBox::computeVerticalAlignShift() const
{
    auto rowHeight = overrideHeight();
    if(rowHeight < height())
        return 0.f;
    switch(style()->verticalAlignType()) {
    case VerticalAlignType::Sub:
    case VerticalAlignType::Super:
    case VerticalAlignType::TextTop:
    case VerticalAlignType::TextBottom:
    case VerticalAlignType::Length:
    case VerticalAlignType::Baseline:
        return std::max(0.f, row()->maxBaseline() - cellBaselinePosition());
    case VerticalAlignType::Middle:
        return (rowHeight - height()) / 2.f;
    case VerticalAlignType::Bottom:
        return rowHeight - height();
    default:
        return 0.f;
    }
}

void TableCellBox::computeBorderWidths(float& borderTop, float& borderBottom, float& borderLeft, float& borderRight) const
{
    if(!table()->isBorderCollapsed()) {
        BlockBox::computeBorderWidths(borderTop, borderBottom, borderLeft, borderRight);
        return;
    }

    const auto& edges = collapsedBorderEdges();
    borderTop = edges.topEdge().width() / 2.f;
    borderBottom = edges.bottomEdge().width() / 2.f;
    borderLeft = edges.leftEdge().width() / 2.f;
    borderRight = edges.rightEdge().width() / 2.f;
}

const TableCollapsedBorderEdges& TableCellBox::collapsedBorderEdges() const
{
    assert(table()->isBorderCollapsed());
    if(m_collapsedBorderEdges == nullptr)
        m_collapsedBorderEdges = TableCollapsedBorderEdges::create(this);
    return *m_collapsedBorderEdges;
}

void TableCellBox::paintBackgroundBehindCell(const PaintInfo& info, const Point& offset, const BoxStyle* backgroundStyle) const
{
    if(style()->visibility() == Visibility::Visible) {
        Point adjustedOffset(offset + location());
        Rect borderRect(adjustedOffset, size());
        paintBackgroundStyle(info, borderRect, backgroundStyle);
    }
}

void TableCellBox::paintCollapsedBorders(const PaintInfo& info, const Point& offset, const TableCollapsedBorderEdge& currentEdge) const
{
    const auto& edges = collapsedBorderEdges();
    const auto& topEdge = edges.topEdge();
    const auto& bottomEdge = edges.bottomEdge();
    const auto& leftEdge = edges.leftEdge();
    const auto& rightEdge = edges.rightEdge();

    auto topHalfWidth = topEdge.width() / 2.f;
    auto bottomHalfWidth = bottomEdge.width() / 2.f;
    auto leftHalfWidth = leftEdge.width() / 2.f;
    auto rightHalfWidth = rightEdge.width() / 2.f;

    Point adjustedOffset(offset + location());
    Rect borderRect(adjustedOffset, size());
    borderRect.expand(topHalfWidth, rightHalfWidth, bottomHalfWidth, leftHalfWidth);
    if(!borderRect.intersects(info.rect())) {
        return;
    }

    struct Border {
        const TableCollapsedBorderEdge* edge = nullptr;
        BoxSide side;
        float x1;
        float y1;
        float x2;
        float y2;

        Rect edgeRect() const { return Rect(x1, y1, x2 - x1, y2 - y1); }
        LineStyle style() const {
            if(edge == nullptr)
                return LineStyle::Hidden;
            auto style = edge->style();
            if(style == LineStyle::Outset)
                return LineStyle::Groove;
            if(style == LineStyle::Inset)
                return LineStyle::Ridge;
            return style;
        }
    };

    Border borders[4];
    if(topEdge.isRenderable()) {
        borders[0].edge = &topEdge;
        borders[0].side = BoxSideTop;
        borders[0].x1 = borderRect.x;
        borders[0].y1 = borderRect.y;
        borders[0].x2 = borderRect.right();
        borders[0].y2 = borderRect.y + topEdge.width();
    }

    if(bottomEdge.isRenderable()) {
        borders[1].edge = &bottomEdge;
        borders[1].side = BoxSideBottom;
        borders[1].x1 = borderRect.x;
        borders[1].y1 = borderRect.bottom() - bottomEdge.width();
        borders[1].x2 = borderRect.right();
        borders[1].y2 = borderRect.bottom();
    }

    if(leftEdge.isRenderable()) {
        borders[2].edge = &leftEdge;
        borders[2].side = BoxSideLeft;
        borders[2].x1 = borderRect.x;
        borders[2].y1 = borderRect.y;
        borders[2].x2 = borderRect.x + leftEdge.width();
        borders[2].y2 = borderRect.bottom();
    }

    if(rightEdge.isRenderable()) {
        borders[3].edge = &rightEdge;
        borders[3].side = BoxSideRight;
        borders[3].x1 = borderRect.right() - rightEdge.width();
        borders[3].y1 = borderRect.y;
        borders[3].x2 = borderRect.right();
        borders[3].y2 = borderRect.bottom();
    }

    for(const Border& border : borders) {
        if(border.edge && border.edge->isSameIgnoringColor(currentEdge)) {
            BorderPainter::paintBoxSide(*info, border.side, border.style(), border.edge->color(), border.edgeRect());
        }
    }
}

void TableCellBox::paintDecorations(const PaintInfo& info, const Point& offset)
{
    Rect borderRect(offset, size());
    paintBackground(info, borderRect);
    if(!table()->isBorderCollapsed()) {
        paintBorder(info, borderRect);
    }
}

TableCaptionBox::TableCaptionBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style)
{
}

float TableCaptionBox::containingBlockWidthForContent(const BlockBox* container) const
{
    if(container)
        return container->width();
    return 0.f;
}

} // namespace plutobook
