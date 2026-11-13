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

    if(auto section = topSection())
        borderTop = section->borderTop();
    if(auto section = bottomSection()) {
        borderBottom = section->borderBottom();
    }

    for(auto section : m_sections) {
        borderLeft = std::max(borderLeft, section->borderLeft());
        borderRight = std::max(borderRight, section->borderRight());
    }
}

void TableBox::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    if(!m_columns.empty()) {
        m_tableLayout->computeIntrinsicWidths(minWidth, maxWidth);
        minWidth += m_borderHorizontalSpacing * (m_columns.size() + 1);
        maxWidth += m_borderHorizontalSpacing * (m_columns.size() + 1);
    }

    for(auto caption : m_captions) {
        caption->updateHorizontalPaddings(0.f);
        caption->updateHorizontalMargins(0.f);

        auto [captionMinWidth, captionMaxWidth] = caption->computePreferredWidths();
        minWidth = std::max(minWidth, captionMinWidth + caption->marginWidth());
        maxWidth = std::max(maxWidth, captionMinWidth + caption->marginWidth());
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

std::optional<float> TableBox::repeatedHeaderTop(const TableSectionBox* header, const Rect& pageRect, const Point& offset) const
{
    if(pageRect.y <= offset.y + header->y())
        return std::nullopt;
    for(auto section : m_sections) {
        if(section->isTableHeader() || section->isTableFooter())
            continue;
        auto sectionTop = offset.y + section->y();
        if(sectionTop >= pageRect.bottom())
            break;
        for(auto row : section->rows()) {
            auto rowTop = sectionTop + row->y();
            if(rowTop + row->height() <= pageRect.y)
                continue;
            if(rowTop < pageRect.y || rowTop >= pageRect.bottom())
                return std::nullopt;
            if(row->isContinuation() || row->rowAbove()->section()->isTableHeader())
                return std::nullopt;
            if(isBorderCollapsed())
                return pageRect.y + row->borderTop();
            return pageRect.y;
        }
    }

    return std::nullopt;
}

std::optional<float> TableBox::repeatedFooterTop(const TableSectionBox* footer, const Rect& pageRect, const Point& offset) const
{
    if(pageRect.bottom() >= offset.y + footer->y())
        return std::nullopt;
    for(auto section : m_sections | std::views::reverse) {
        if(section->isTableHeader() || section->isTableFooter())
            continue;
        auto sectionTop = offset.y + section->y();
        if(sectionTop >= pageRect.bottom())
            continue;
        for(auto row : section->rows() | std::views::reverse) {
            if(sectionTop + row->y() >= pageRect.bottom())
                continue;
            auto rowBottom = sectionTop + m_borderVerticalSpacing + row->y() + row->height();
            if(rowBottom <= pageRect.y || rowBottom + footer->height() > pageRect.bottom())
                return std::nullopt;
            auto rowBelow = row->rowBelow();
            if(rowBelow->isContinuation() || rowBelow->section()->isTableFooter())
                return std::nullopt;
            return rowBottom;
        }
    }

    return std::nullopt;
}

float TableBox::unbreakableHeight(const FragmentBuilder* fragmentainer) const
{
    if(fragmentainer->avoidsBreakInside(this))
        return height();
    auto leadingHeight = borderAndPaddingTop();
    for(auto caption : m_captions) {
        if(caption->captionSide() == CaptionSide::Top) {
            leadingHeight += caption->marginBoxHeight();
        }
    }

    if(auto section = topSection()) {
        leadingHeight += m_borderVerticalSpacing;
        if(section->isTableHeader()) {
            if(auto sectionBelow = section->sectionBelow()) {
                leadingHeight += m_borderVerticalSpacing + section->height();
                section = sectionBelow;
            }
        }

        auto unbreakableHeight = m_borderVerticalSpacing + section->unbreakableHeight(fragmentainer);
        if(isPaginatedTable() && !section->isTableFooter()) {
            if(auto footer = footerSection()) {
                unbreakableHeight += footer->height();
            }
        }

        return leadingHeight + unbreakableHeight;
    }

    return leadingHeight + borderAndPaddingBottom();
}

TableColumnBox* TableBox::columnGroupAt(size_t index) const
{
    auto column = columnAt(index);
    if(column == nullptr)
        return nullptr;
    if(column->isColumnGroup())
        return column;
    return column->columnGroup();
}

TableCaptionBox* TableBox::topCaption() const
{
    for(auto caption : m_captions) {
        if(caption->captionSide() == CaptionSide::Top) {
            return caption;
        }
    }

    return nullptr;
}

TableCaptionBox* TableBox::bottomCaption() const
{
    for(auto caption : m_captions | std::views::reverse) {
        if(caption->captionSide() == CaptionSide::Bottom) {
            return caption;
        }
    }

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

float TableBox::availableHorizontalSpace() const
{
    if(!m_columns.empty() && !isBorderCollapsed())
        return contentBoxWidth() - m_borderHorizontalSpacing * (m_columns.size() + 1);
    return contentBoxWidth();
}

void TableBox::layoutCaption(TableCaptionBox* caption, FragmentBuilder* fragmentainer)
{
    caption->updatePaddingWidths(width());
    caption->updateVerticalMargins(width());

    auto captionTop = height() + caption->marginTop();
    if(fragmentainer) {
        captionTop = fragmentainer->applyFragmentBreakBefore(caption, captionTop);
        captionTop = fragmentainer->applyFragmentBreakInside(caption, captionTop);
        fragmentainer->enterFragment(captionTop);
    }

    caption->setY(captionTop);
    caption->layout(fragmentainer);
    caption->setX(caption->marginLeft());
    if(fragmentainer) {
        fragmentainer->leaveFragment(captionTop);
        auto captionHeight = caption->height();
        auto unbreakableHeight = caption->unbreakableHeight(fragmentainer);
        auto newTop = fragmentainer->adjustOffsetInFragmentFlow(captionTop, captionHeight, unbreakableHeight);
        if(!isNearlyEqual(newTop, captionTop)) {
            fragmentainer->enterFragment(newTop);
            caption->setY(newTop);
            caption->layout(fragmentainer);
            fragmentainer->leaveFragment(newTop);
        }
    }

    setHeight(caption->y() + caption->height() + caption->marginBottom());
    if(fragmentainer) {
        setHeight(fragmentainer->applyFragmentBreakAfter(caption, height()));
    }
}

void TableBox::layout(FragmentBuilder* fragmentainer)
{
    updateWidth();
    setHeight(0.f);
    setIsPaginatedTable(isPageFragmentainer(fragmentainer));
    for(auto caption : m_captions) {
        if(caption->captionSide() == CaptionSide::Top) {
            layoutCaption(caption, fragmentainer);
        }
    }

    float tableHeight = 0.f;
    if(!isColumnFlexItem()) {
        if(auto height = computeHeightUsing(style()->height()))
            tableHeight = adjustContentBoxHeight(height.value());
        tableHeight = constrainContentBoxHeight(tableHeight);
    }

    if(hasOverrideHeight()) {
        tableHeight = std::max(tableHeight, overrideHeight() - borderAndPaddingHeight() - height());
    }

    setHeight(height() + borderAndPaddingTop());
    if(m_columns.empty() || m_sections.empty()) {
        setHeight(tableHeight + height());
    } else {
        m_tableLayout->layout();
        auto columnLeft = m_borderHorizontalSpacing;
        for(auto& column : m_columns) {
            column.setX(columnLeft);
            columnLeft += m_borderHorizontalSpacing + column.width();
        }

        if(style()->isRightToLeftDirection()) {
            for(auto& column : m_columns) {
                column.setX(columnLeft - column.width() - column.x());
            }
        }

        size_t sectionCount = 0;
        size_t bodySectionCount = 0;
        float totalSectionHeight = 0.f;
        float totalBodySectionHeight = 0.f;
        for(auto section : m_sections) {
            section->layout(nullptr);
            totalSectionHeight += section->height();
            ++sectionCount;
            if(!section->isTableHeader() && !section->isTableFooter()) {
                totalBodySectionHeight += section->height();
                ++bodySectionCount;
            }
        }

        auto hasBodySections = bodySectionCount > 0;
        auto distributableSectionCount = hasBodySections ? bodySectionCount : sectionCount;
        auto distributableSectionHeight = hasBodySections ? totalBodySectionHeight : totalSectionHeight;
        auto distributableTableHeight = tableHeight - totalSectionHeight - m_borderVerticalSpacing * (sectionCount + 1);
        for(auto section : m_sections) {
            if(distributableTableHeight > 0.f && !(hasBodySections && (section->isTableHeader() || section->isTableFooter()))) {
                if(distributableSectionHeight > 0.f) {
                    section->distributeExcessHeightToRows(distributableTableHeight * section->height() / distributableSectionHeight);
                } else {
                    section->distributeExcessHeightToRows(distributableTableHeight / distributableSectionCount);
                }
            }

            section->updateRowMaxHeights();
        }

        const auto* header = headerSection();
        const auto* footer = footerSection();

        auto sectionTop = m_borderVerticalSpacing + height();
        for(auto section : m_sections) {
            if(fragmentainer) {
                sectionTop = fragmentainer->applyFragmentBreakBefore(section, sectionTop);
                sectionTop = fragmentainer->applyFragmentBreakInside(section, sectionTop);
                fragmentainer->enterFragment(sectionTop);
            }

            float headerHeight = 0;
            float footerHeight = 0;
            if(isPaginatedTable() && !section->isTableHeader() && !section->isTableFooter()) {
                if(header && header != section)
                    headerHeight += header->height();
                if(footer && footer != section) {
                    footerHeight += footer->height();
                }
            }

            section->setY(sectionTop);
            section->setX(borderAndPaddingLeft());
            section->layoutRows(fragmentainer, headerHeight, footerHeight);
            section->updateOverflowRect();
            if(fragmentainer) {
                fragmentainer->leaveFragment(sectionTop);
            }

            sectionTop += m_borderVerticalSpacing + section->height();
            if(fragmentainer) {
                sectionTop = fragmentainer->applyFragmentBreakAfter(section, sectionTop);
            }
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
                for(const auto& cell : row->cells()) {
                    if(!cell.isPrimary())
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

    if(auto caption = topCaption()) {
        caption->setIsFirstInFlowChild(true);
    } else if(auto section = topSection()) {
        section->setIsFirstInFlowChild(true);
    } else if(auto caption = bottomCaption()) {
        caption->setIsFirstInFlowChild(true);
    }

    if(auto caption = bottomCaption()) {
        caption->setIsLastInFlowChild(true);
    } else if(auto section = bottomSection()) {
        section->setIsLastInFlowChild(true);
    } else if(auto caption = topCaption()) {
        caption->setIsLastInFlowChild(true);
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
    if(isPaginatedTable()) {
        if(auto footer = footerSection()) {
            const auto& pageRect = info.rect();
            if(auto footerTop = repeatedFooterTop(footer, pageRect, offset)) {
                Point footerOffset(offset.x, footerTop.value() - footer->y());
                footer->paint(info, footerOffset, phase);
                if(shouldPaintCollapsedBorders) {
                    for(const auto& edge : *m_collapsedBorderEdges) {
                        footer->paintCollapsedBorders(info, footerOffset, edge);
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

    if(isPaginatedTable()) {
        if(auto header = headerSection()) {
            const auto& pageRect = info.rect();
            if(auto headerTop = repeatedHeaderTop(header, pageRect, offset)) {
                Point headerOffset(offset.x, headerTop.value() - header->y());
                header->paint(info, headerOffset, phase);
                if(shouldPaintCollapsedBorders) {
                    for(const auto& edge : *m_collapsedBorderEdges) {
                        header->paintCollapsedBorders(info, headerOffset, edge);
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
        for(const auto& cell : section->firstRow()->cells()) {
            if(cell.isPrimary()) {
                auto cellStyleWidth = cell->style()->width();
                if(cellStyleWidth.isFixed()) {
                    cell->updateHorizontalPaddings(0.f);
                    cellStyleWidth = Length(Length::Type::Fixed, cell->adjustBorderBoxWidth(cellStyleWidth.value()) / cell->colSpan());
                } else if(cellStyleWidth.isPercent()) {
                    cellStyleWidth = Length(Length::Type::Percent, cellStyleWidth.value() / cell->colSpan());
                }

                if(!cellStyleWidth.isZero()) {
                    const auto col = cell->columnIndex();
                    for(size_t index = 0; index < cell->colSpan(); ++index) {
                        if(m_widths[col + index].isAuto()) {
                            m_widths[col + index] = cellStyleWidth;
                        }
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

    const auto innerBorderSpacing = borderSpacing * (cellBox->colSpan() - 1);

    auto [cellMinWidth, cellMaxWidth] = cellBox->computePreferredWidths();
    cellMinWidth = std::max(0.f, cellMinWidth - innerBorderSpacing);
    cellMaxWidth = std::max(0.f, cellMaxWidth - innerBorderSpacing);
    if(cellStyleWidth.isFixed()) {
        cellMaxWidth = std::max(cellMinWidth, cellBox->adjustBorderBoxWidth(cellStyleWidth.value()) - innerBorderSpacing);
    }

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
        columnWidth.maxWidth = columnWidth.maxFixedWidth;
    }

    for(auto section : m_table->sections()) {
        for(auto row : section->rows()) {
            for(const auto& cell : row->cells()) {
                if(!cell.isPrimary())
                    continue;
                cell->updateHorizontalPaddings(0.f);
                if(cell->colSpan() == 1) {
                    auto [cellMinWidth, cellMaxWidth] = cell->computePreferredWidths();
                    auto& columnWidth = m_columnWidths[cell->columnIndex()];
                    columnWidth.minWidth = std::max(columnWidth.minWidth, cellMinWidth);
                    if(columnWidth.maxFixedWidth > 0.f) {
                        columnWidth.maxWidth = std::max(columnWidth.maxWidth, std::max(columnWidth.minWidth, columnWidth.maxFixedWidth));
                    } else {
                        columnWidth.maxWidth = std::max(columnWidth.maxWidth, cellMaxWidth);
                    }
                }
            }
        }
    }

    for(auto cellBox : m_spanningCells) {
        distributeSpanCellToColumns(cellBox, m_columnWidths, m_table->borderHorizontalSpacing());
    }

    constexpr auto kMaxTableWidth = 1000000.f;

    float totalMinWidth = 0.f;
    float totalMaxWidth = 0.f;
    float totalPercent = 0.f;
    float totalNonPercentMaxWidth = 0.f;
    float maxPercentWidth = 0.f;
    for(auto& columnWidth : m_columnWidths) {
        totalMinWidth += columnWidth.minWidth;
        totalMaxWidth += columnWidth.maxWidth;
        if(!columnWidth.width.isPercent()) {
            totalNonPercentMaxWidth += columnWidth.maxWidth;
        } else {
            if(totalPercent + columnWidth.width.value() > 100.f)
                columnWidth.width = Length(Length::Type::Percent, 100.f - totalPercent);
            if(columnWidth.width.value() > 0.f) {
                maxPercentWidth = std::max(maxPercentWidth, columnWidth.maxWidth * 100.f / columnWidth.width.value());
            } else if(columnWidth.maxWidth > 0.f) {
                maxPercentWidth = kMaxTableWidth;
            }

            totalPercent += columnWidth.width.value();
        }
    }

    if(totalNonPercentMaxWidth > 0.f) {
        if(totalPercent < 100.f) {
            totalMaxWidth = std::max(totalMaxWidth, totalNonPercentMaxWidth * 100.f / (100.f - totalPercent));
        } else {
            totalMaxWidth = kMaxTableWidth;
        }
    }

    totalMaxWidth = std::min(kMaxTableWidth, std::max(totalMaxWidth, maxPercentWidth));

    minWidth += totalMinWidth;
    maxWidth += totalMaxWidth;
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
            for(const auto& cell : row->cells()) {
                if(!cell.isPrimary())
                    continue;
                if(cell->colSpan() > 1) {
                    m_spanningCells.push_back(cell.box());
                    continue;
                }

                auto cellStyleWidth = cell->style()->width();
                auto& columnWidth = m_columnWidths[cell->columnIndex()];
                if(cellStyleWidth.isFixed()) {
                    cell->updateHorizontalPaddings(0.f);
                    columnWidth.maxFixedWidth = std::max(columnWidth.maxFixedWidth, cell->adjustBorderBoxWidth(cellStyleWidth.value()));
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
    setIsOverflowHidden(false);
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

void TableSectionBox::computeBorderWidths(float& borderTop, float& borderBottom, float& borderLeft, float& borderRight) const
{
    if(!table()->isBorderCollapsed()) {
        BoxFrame::computeBorderWidths(borderTop, borderBottom, borderLeft, borderRight);
        return;
    }

    borderTop = 0.f;
    borderBottom = 0.f;
    borderLeft = 0.f;
    borderRight = 0.f;

    if(auto row = firstRow())
        borderTop = row->borderTop();
    if(auto row = lastRow()) {
        borderBottom = row->borderBottom();
    }

    for(auto row : m_rows) {
        borderLeft = std::max(borderLeft, row->borderLeft());
        borderRight = std::max(borderRight, row->borderRight());
    }
}

float TableSectionBox::unbreakableHeight(const FragmentBuilder* fragmentainer) const
{
    if(fragmentainer->avoidsBreakInside(this))
        return height();
    if(auto row = firstRow())
        return row->unbreakableHeight(fragmentainer);
    return 0.f;
}

std::optional<float> TableSectionBox::firstLineBaseline() const
{
    if(m_rows.empty())
        return std::nullopt;
    auto firstRowBox = m_rows.front();
    if(auto baseline = firstRowBox->maxBaseline())
        return baseline + firstRowBox->y();
    std::optional<float> baseline;
    for(const auto& cell : firstRowBox->cells()) {
        if(cell.isPrimary() && cell->contentBoxHeight()) {
            auto candidate = firstRowBox->y() + cell->borderAndPaddingTop() + cell->contentBoxHeight();
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
    for(const auto& cell : lastRowBox->cells()) {
        if(cell.isPrimary() && cell->contentBoxHeight()) {
            auto candidate = lastRowBox->y() + cell->borderAndPaddingTop() + cell->contentBoxHeight();
            baseline = std::max(candidate, baseline.value_or(candidate));
        }
    }

    return baseline;
}

TableSectionBox* TableSectionBox::sectionAbove() const
{
    if(isTableHeader())
        return nullptr;
    auto prevSection = isTableFooter() ? table()->lastChild() : prevSibling();
    while(prevSection) {
        auto section = to<TableSectionBox>(prevSection);
        if(section && !section->isTableHeader() && !section->isTableFooter() && section->firstRow())
            return section;
        prevSection = prevSection->prevSibling();
    }

    return table()->headerSection();
}

TableSectionBox* TableSectionBox::sectionBelow() const
{
    if(isTableFooter())
        return nullptr;
    auto nextSection = isTableHeader() ? table()->firstChild() : nextSibling();
    while(nextSection) {
        auto section = to<TableSectionBox>(nextSection);
        if(section && !section->isTableHeader() && !section->isTableFooter() && section->firstRow())
            return section;
        nextSection = nextSection->nextSibling();
    }

    return table()->footerSection();
}

void TableSectionBox::distributeExcessHeightToRows(float distributableHeight)
{
    setHeight(distributableHeight + height());

    float totalHeight = 0;
    float totalAutoHeight = 0;
    for(auto rowBox : m_rows) {
        totalHeight += rowBox->height();
        if(!rowBox->maxFixedHeight() && !rowBox->maxPercentHeight()) {
            totalAutoHeight += rowBox->height();
        }
    }

    for(auto rowBox : m_rows) {
        if(rowBox->maxPercentHeight() && !rowBox->maxFixedHeight()) {
            auto percentHeight = height() * rowBox->maxPercentHeight() / 100.f;
            auto delta = std::min(distributableHeight, std::max(0.f, percentHeight - rowBox->height()));
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
    } else if(totalHeight > 0.f) {
        for(auto rowBox : m_rows) {
            auto delta = distributableHeight * rowBox->height() / totalHeight;
            rowBox->setHeight(delta + rowBox->height());
        }
    } else {
        for(auto rowBox : m_rows) {
            rowBox->setHeight(rowBox->height() + distributableHeight / m_rows.size());
        }
    }
}

void TableSectionBox::updateRowMaxHeights()
{
    auto verticalSpacing = table()->borderVerticalSpacing();
    for(size_t rowIndex = 0; rowIndex < m_rows.size(); ++rowIndex) {
        auto rowBox = m_rows[rowIndex];
        auto maxHeight = rowBox->height();
        for(const auto& cell : rowBox->cells()) {
            if(!cell.isPrimary())
                continue;
            auto height = rowBox->height();
            for(size_t index = 1; index < cell->rowSpan(); ++index)
                height += verticalSpacing + m_rows[rowIndex + index]->height();
            cell->setOverrideHeight(height);
            maxHeight = std::max(maxHeight, height);
        }

        rowBox->setMaxHeight(maxHeight);
    }
}

static const BoxFrame* rowAlwaysBreakBefore(const TableRowBox* rowBox, const FragmentBuilder* fragmentainer)
{
    if(fragmentainer->alwaysBreakBefore(rowBox))
        return rowBox;
    for(auto cellBox = rowBox->firstCell(); cellBox; cellBox = cellBox->nextCell()) {
        if(fragmentainer->alwaysBreakBefore(cellBox)) {
            return cellBox;
        }
    }

    return nullptr;
}

static const BoxFrame* rowAlwaysBreakAfter(const TableRowBox* rowBox, const FragmentBuilder* fragmentainer)
{
    if(fragmentainer->alwaysBreakAfter(rowBox))
        return rowBox;
    for(auto cellBox = rowBox->firstCell(); cellBox; cellBox = cellBox->nextCell()) {
        if(fragmentainer->alwaysBreakAfter(cellBox)) {
            return cellBox;
        }
    }

    return nullptr;
}

void TableSectionBox::layoutRows(FragmentBuilder* fragmentainer, float headerHeight, float footerHeight)
{
    float rowTop = 0;
    auto verticalSpacing = table()->borderVerticalSpacing();
    for(size_t rowIndex = 0; rowIndex < m_rows.size(); ++rowIndex) {
        auto rowBox = m_rows[rowIndex];
        if(fragmentainer) {
            if(auto box = rowAlwaysBreakBefore(rowBox, fragmentainer))
                rowTop = fragmentainer->applyFragmentBreakBefore(box, rowTop);
            auto rowHeight = verticalSpacing + rowBox->maxHeight();
            auto unbreakableHeight = verticalSpacing + rowBox->unbreakableHeight(fragmentainer);
            rowTop = fragmentainer->adjustOffsetInFragmentFlow(rowTop, rowHeight, unbreakableHeight);

            auto fragmentHeight = fragmentainer->fragmentHeightForOffset(rowTop);
            if(fragmentHeight > 0.f) {
                auto remainingHeight = fragmentainer->fragmentRemainingHeightForOffset(rowTop, AssociateWithLatterFragment);
                if(isNearlyEqual(remainingHeight, fragmentHeight)) {
                    if(table()->isBorderCollapsed())
                        rowTop += rowBox->borderTop();
                    if(headerHeight && !rowBox->isContinuation() && !rowBox->rowAbove()->section()->isTableHeader()) {
                        rowTop += headerHeight + verticalSpacing;
                    }
                }
            }

            fragmentainer->enterFragment(rowTop);
        }

        rowBox->setX(0.f);
        rowBox->setY(rowTop);
        float rowHeightIncreaseForFragmentation = 0;
        for(const auto& cell : rowBox->cells()) {
            if(!cell.isPrimary())
                continue;
            cell->setY(0.f);
            cell->layout(fragmentainer);
            if(fragmentainer && cell->height() > cell->overrideHeight()) {
                rowHeightIncreaseForFragmentation = std::max(rowHeightIncreaseForFragmentation, cell->height() - cell->overrideHeight());
                cell->setHeight(cell->overrideHeight());
            }
        }

        if(fragmentainer) {
            fragmentainer->leaveFragment(rowTop);
            if(rowHeightIncreaseForFragmentation > 0) {
                rowBox->setHeight(rowHeightIncreaseForFragmentation + rowBox->height());
                rowBox->setMaxHeight(rowHeightIncreaseForFragmentation + rowBox->maxHeight());
                for(const auto& cell : rowBox->cells()) {
                    if(cell.isPrimary()) {
                        cell->setHeight(rowHeightIncreaseForFragmentation + cell->height());
                        cell->updateOverflowRect();
                    }
                }
            }
        }

        rowBox->updateOverflowRect();
        rowTop += verticalSpacing + rowBox->height();
        if(fragmentainer) {
            if(footerHeight) {
                auto rowBelow = rowBox->rowBelow();
                if(!rowBelow->isContinuation() && !rowBelow->section()->isTableFooter()) {
                    auto fragmentHeight = fragmentainer->fragmentHeightForOffset(rowTop);
                    if(fragmentHeight > 0.f) {
                        auto requiredHeight = footerHeight + verticalSpacing + rowBelow->maxHeight();
                        auto remainingHeight = fragmentainer->fragmentRemainingHeightForOffset(rowTop, AssociateWithLatterFragment);
                        if(requiredHeight > remainingHeight + kEpsilon && requiredHeight <= fragmentHeight + kEpsilon) {
                            rowTop += remainingHeight;
                        }
                    }
                }
            }

            if(auto box = rowAlwaysBreakAfter(rowBox, fragmentainer)) {
                rowTop = fragmentainer->applyFragmentBreakAfter(box, rowTop);
            }
        }
    }

    if(fragmentainer) {
        for(auto cellBox : m_spanningCells) {
            auto firstRow = cellBox->firstRow();
            auto lastRow = cellBox->lastRow();
            auto height = lastRow->y() + lastRow->height() - firstRow->y();
            if(height > cellBox->height()) {
                cellBox->setHeight(height);
                cellBox->updateOverflowRect();
                firstRow->setMaxHeight(std::max(height, firstRow->maxHeight()));
                firstRow->updateOverflowRect();
            }
        }
    }

    setHeight(rowTop - verticalSpacing);
}

static void distributeSpanCellToRows(TableCellBox* cellBox, std::span<TableRowBox*> allRows, float borderSpacing)
{
    auto rows = allRows.subspan(cellBox->rowIndex(), cellBox->rowSpan());
    auto extraHeight = cellBox->heightForRowSizing() - borderSpacing * (rows.size() - 1);
    float totalHeight = 0.f;
    float totalAutoHeight = 0.f;
    for(auto rowBox : rows) {
        extraHeight -= rowBox->height();
        totalHeight += rowBox->height();
        if(!rowBox->maxFixedHeight() && !rowBox->maxPercentHeight()) {
            totalAutoHeight += rowBox->height();
        }
    }

    if(extraHeight <= 0.f)
        return;
    if(totalAutoHeight > 0.f) {
        for(auto rowBox : rows) {
            if(!rowBox->maxFixedHeight() && !rowBox->maxPercentHeight()) {
                rowBox->setHeight(rowBox->height() + extraHeight * rowBox->height() / totalAutoHeight);
            }
        }
    } else if(totalHeight > 0.f) {
        for(auto rowBox : rows) {
            rowBox->setHeight(rowBox->height() + extraHeight * rowBox->height() / totalHeight);
        }
    } else {
        auto lastRow = rows.back();
        lastRow->setHeight(lastRow->height() + extraHeight);
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
        for(const auto& cell : rowBox->cells()) {
            if(!cell.isPrimary())
                continue;
            const auto col = cell->columnIndex();
            auto width = columns[col].width();
            for(size_t index = 1; index < cell->colSpan(); ++index) {
                width += horizontalSpacing + columns[col + index].width();
            }

            if(direction == Direction::Ltr) {
                cell->setX(columns[col].x());
            } else {
                cell->setX(columns[col + cell->colSpan() - 1].x());
            }

            cell->clearOverrideSize();
            cell->setOverrideWidth(width);
            cell->updatePaddingWidths(table()->availableWidth());
            cell->layout(fragmentainer);

            if(cell->rowSpan() == 1)
                cellMaxHeight = std::max(cellMaxHeight, cell->heightForRowSizing());
            if(cell->isBaselineAligned()) {
                if(cell->rowSpan() == 1) {
                    auto ascent = cell->cellBaselinePosition();
                    auto descent = cell->height() - ascent;
                    cellMaxAscent = std::max(cellMaxAscent, ascent);
                    cellMaxDescent = std::max(cellMaxDescent, descent);
                    cellMaxHeight = std::max(cellMaxHeight, cellMaxAscent + cellMaxDescent);
                } else {
                    cellMaxAscent = std::max(cellMaxAscent, cell->cellBaselinePosition());
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

    float sectionHeight = 0;
    for(auto rowBox : m_rows)
        sectionHeight += verticalSpacing + rowBox->height();
    setHeight(sectionHeight - verticalSpacing);
}

void TableSectionBox::build()
{
    auto& columns = table()->columns();

    for(auto rowBox = firstRow(); rowBox; rowBox = rowBox->nextRow()) {
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
        rowBox->setIsFirstInFlowChild(rowIndex == 0);
        rowBox->setIsLastInFlowChild(rowIndex + 1 == rowCount);

        uint32_t columnIndex = 0;
        for(auto cellBox = rowBox->firstCell(); cellBox; cellBox = cellBox->nextCell()) {
            cellBox->setIsFirstInFlowChild(true);
            cellBox->setIsLastInFlowChild(true);

            auto rowSpan = rowCount - rowIndex;
            if(cellBox->rowSpan() > 0) {
                rowSpan = std::min(rowSpan, cellBox->rowSpan());
            }

            const auto& cells = rowBox->cells();
            while(columnIndex < cells.size() && !cells[columnIndex].isNull()) {
                ++columnIndex;
            }

            cellBox->setRowSpan(rowSpan);
            cellBox->setColumnIndex(columnIndex);
            cellBox->setRowIndex(rowIndex);
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

            for(uint32_t rowOffset = 0; rowOffset < cellBox->rowSpan(); ++rowOffset) {
                auto targetRow = m_rows[rowIndex + rowOffset];
                if(rowOffset > 0)
                    targetRow->setIsContinuation(true);
                auto& targetCells = targetRow->cells();

                const auto requiredSize = columnIndex + cellBox->colSpan();
                if(requiredSize > targetCells.size()) {
                    targetCells.resize(requiredSize);
                }

                for(uint32_t colOffset = 0; colOffset < cellBox->colSpan(); ++colOffset) {
                    targetCells[columnIndex + colOffset] = TableCell(cellBox, colOffset, rowOffset);
                }
            }

            columnIndex += cellBox->colSpan();
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
    Point adjustedOffset(offset + location());
    if(!info.shouldPaintBox(this, adjustedOffset))
        return;
    for(auto rowBox : m_rows | std::views::reverse) {
        Point rowOffset(adjustedOffset + rowBox->location());
        if(!info.shouldPaintBox(rowBox, rowOffset))
            continue;
        for(const auto& cell : rowBox->cells()) {
            if(cell.isPrimary()) {
                cell->paintCollapsedBorders(info, rowOffset, currentEdge);
            }
        }
    }
}

void TableSectionBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    Point adjustedOffset(offset + location());
    if(!info.shouldPaintBox(this, adjustedOffset))
        return;
    for(auto rowBox : m_rows) {
        Point rowOffset(adjustedOffset + rowBox->location());
        if(!info.shouldPaintBox(rowBox, rowOffset))
            continue;
        if(phase == PaintPhase::Outlines && !rowBox->hasLayer() && rowBox->style()->visibility() == Visibility::Visible) {
            rowBox->paintOutlines(info, rowOffset);
        }

        for(const auto& cell : rowBox->cells()) {
            if(!cell.isPrimary())
                continue;
            if(phase == PaintPhase::Decorations) {
                if(auto columnBox = table()->columnAt(cell->columnIndex())) {
                    if(auto columnGroupBox = columnBox->columnGroup())
                        cell->paintBackgroundBehindCell(info, rowOffset, columnGroupBox->style());
                    cell->paintBackgroundBehindCell(info, rowOffset, columnBox->style());
                }

                cell->paintBackgroundBehindCell(info, rowOffset, style());
                if(!rowBox->hasLayer()) {
                    cell->paintBackgroundBehindCell(info, rowOffset, rowBox->style());
                }
            }

            if(!cell->hasLayer() && !rowBox->hasLayer()) {
                cell->paint(info, rowOffset, phase);
            }
        }
    }

    if(phase == PaintPhase::Outlines && style()->visibility() == Visibility::Visible) {
        paintOutlines(info, adjustedOffset);
    }
}

TableRowBox::TableRowBox(Node* node, const RefPtr<BoxStyle>& style)
    : BoxFrame(node, style)
    , m_cells(style->heap())
{
    setIsOverflowHidden(false);
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
    for(const auto& cell : m_cells) {
        if(cell.isPrimary()) {
            addOverflowRect(cell.box(), cell->x(), cell->y());
            Rect borderRect(cell->location(), cell->size());
            if(table()->isBorderCollapsed()) {
                const auto& edges = cell->collapsedBorderEdges();
                auto topHalfWidth = edges.topEdge().width() / 2.f;
                auto bottomHalfWidth = edges.bottomEdge().width() / 2.f;
                auto leftHalfWidth = edges.leftEdge().width() / 2.f;
                auto rightHalfWidth = edges.rightEdge().width() / 2.f;
                borderRect.expand(topHalfWidth, rightHalfWidth, bottomHalfWidth, leftHalfWidth);
            }

            addOverflowRect(borderRect);
        }
    }
}

void TableRowBox::computeBorderWidths(float& borderTop, float& borderBottom, float& borderLeft, float& borderRight) const
{
    if(!table()->isBorderCollapsed()) {
        BoxFrame::computeBorderWidths(borderTop, borderBottom, borderLeft, borderRight);
        return;
    }

    const auto totalColumns = table()->columnCount();

    borderTop = 0.f;
    borderBottom = 0.f;
    borderLeft = 0.f;
    borderRight = 0.f;

    const auto isRTL = table()->style()->isRightToLeftDirection();

    for(size_t col = 0; col < m_cells.size(); ++col) {
        const auto& cell = m_cells[col];
        if(cell.isNull())
            continue;
        if(cell.rowOffset() == 0)
            borderTop = std::max(borderTop, cell->borderTop());
        if(cell.rowOffset() == cell->rowSpan() - 1) {
            borderBottom = std::max(borderBottom, cell->borderBottom());
        }

        const auto isLogicalStart = (col == 0);
        const auto isLogicalEnd = (col + 1 == totalColumns);
        if((!isRTL && isLogicalStart) || (isRTL && isLogicalEnd))
            borderLeft = std::max(borderLeft, cell->borderLeft());
        if((!isRTL && isLogicalEnd) || (isRTL && isLogicalStart)) {
            borderRight = std::max(borderRight, cell->borderRight());
        }
    }
}

float TableRowBox::unbreakableHeight(const FragmentBuilder* fragmentainer) const
{
    if(fragmentainer->avoidsBreakInside(this))
        return m_maxHeight;
    float unbreakableHeight = 0.f;
    for(const auto& cell : m_cells) {
        if(cell.isPrimary()) {
            unbreakableHeight = std::max(unbreakableHeight, cell->unbreakableHeight(fragmentainer));
        }
    }

    return unbreakableHeight;
}

TableRowBox* TableRowBox::rowAbove() const
{
    if(auto rowAbove = prevRow())
        return rowAbove;
    if(auto sectionAbove = section()->sectionAbove())
        return sectionAbove->lastRow();
    return nullptr;
}

TableRowBox* TableRowBox::rowBelow() const
{
    if(auto rowBelow = nextRow())
        return rowBelow;
    if(auto sectionBelow = section()->sectionBelow())
        return sectionBelow->firstRow();
    return nullptr;
}

TableCellBox* TableRowBox::cellAt(uint32_t columnIndex) const
{
    if(columnIndex < m_cells.size())
        return m_cells[columnIndex].box();
    return nullptr;
}

void TableRowBox::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    Point adjustedOffset(offset + location());
    if(phase == PaintPhase::Outlines && style()->visibility() == Visibility::Visible) {
        paintOutlines(info, adjustedOffset);
    }

    for(const auto& cell : m_cells) {
        if(!cell.isPrimary())
            continue;
        if(phase == PaintPhase::Decorations)
            cell->paintBackgroundBehindCell(info, adjustedOffset, style());
        if(!cell->hasLayer()) {
            cell->paint(info, adjustedOffset, phase);
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
    if(column && column->isColumnGroup())
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
    const auto borderTopStyle = style->borderTopStyle();
    if(borderTopStyle > LineStyle::Hidden)
        return TableCollapsedBorderEdge(source, borderTopStyle, style->borderTopWidth(), style->borderTopColor());
    return TableCollapsedBorderEdge(source, borderTopStyle);
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::getBottomEdge(TableCollapsedBorderSource source, const BoxStyle* style)
{
    const auto borderBottomStyle = style->borderBottomStyle();
    if(borderBottomStyle > LineStyle::Hidden)
        return TableCollapsedBorderEdge(source, borderBottomStyle, style->borderBottomWidth(), style->borderBottomColor());
    return TableCollapsedBorderEdge(source, borderBottomStyle);
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::getLeftEdge(TableCollapsedBorderSource source, const BoxStyle* style)
{
    const auto borderLeftStyle = style->borderLeftStyle();
    if(borderLeftStyle > LineStyle::Hidden)
        return TableCollapsedBorderEdge(source, borderLeftStyle, style->borderLeftWidth(), style->borderLeftColor());
    return TableCollapsedBorderEdge(source, borderLeftStyle);
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::getRightEdge(TableCollapsedBorderSource source, const BoxStyle* style)
{
    const auto borderRightStyle = style->borderRightStyle();
    if(borderRightStyle > LineStyle::Hidden)
        return TableCollapsedBorderEdge(source, borderRightStyle, style->borderRightWidth(), style->borderRightColor());
    return TableCollapsedBorderEdge(source, borderRightStyle);
}

TableCollapsedBorderEdge TableCollapsedBorderEdges::calcTopEdge(const TableCellBox* cellBox)
{
    auto cellAbove = cellBox->cellAbove();
    auto edge = getTopEdge(TableCollapsedBorderSource::Cell, cellBox->style());
    if(cellAbove) {
        edge = chooseEdge(getBottomEdge(TableCollapsedBorderSource::Cell, cellAbove->style()), edge);
        if(!edge.exists()) {
            return edge;
        }
    }

    auto firstRow = cellBox->firstRow();
    edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::Row, firstRow->style()));
    if(!edge.exists()) {
        return edge;
    }

    if(auto rowAbove = firstRow->rowAbove()) {
        edge = chooseEdge(getBottomEdge(TableCollapsedBorderSource::Row, rowAbove->style()), edge);
        if(!edge.exists()) {
            return edge;
        }
    }

    auto section = cellBox->section();
    if(firstRow == section->firstRow()) {
        edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::RowGroup, section->style()));
        if(!edge.exists()) {
            return edge;
        }

        if(auto sectionAbove = section->sectionAbove()) {
            edge = chooseEdge(getBottomEdge(TableCollapsedBorderSource::RowGroup, sectionAbove->style()), edge);
            if(!edge.exists()) {
                return edge;
            }
        } else {
            auto table = cellBox->table();
            if(auto column = table->columnAt(cellBox->columnIndex()); column && !column->isColumnGroup()) {
                edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::Column, column->style()));
                if(!edge.exists()) {
                    return edge;
                }
            }

            if(auto columnGroup = table->columnGroupAt(cellBox->columnIndex())) {
                edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::ColumnGroup, columnGroup->style()));
                if(!edge.exists()) {
                    return edge;
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
    auto cellBelow = cellBox->cellBelow();
    auto edge = getBottomEdge(TableCollapsedBorderSource::Cell, cellBox->style());
    if(cellBelow) {
        edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::Cell, cellBelow->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    auto lastRow = cellBox->lastRow();
    edge = chooseEdge(edge, getBottomEdge(TableCollapsedBorderSource::Row, lastRow->style()));
    if(!edge.exists()) {
        return edge;
    }

    if(auto rowBelow = lastRow->rowBelow()) {
        edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::Row, rowBelow->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    auto section = cellBox->section();
    if(lastRow == section->lastRow()) {
        edge = chooseEdge(edge, getBottomEdge(TableCollapsedBorderSource::RowGroup, section->style()));
        if(!edge.exists()) {
            return edge;
        }

        if(auto sectionBelow = section->sectionBelow()) {
            edge = chooseEdge(edge, getTopEdge(TableCollapsedBorderSource::RowGroup, sectionBelow->style()));
            if(!edge.exists()) {
                return edge;
            }
        } else {
            auto table = cellBox->table();
            if(auto column = table->columnAt(cellBox->columnIndex()); column && !column->isColumnGroup()) {
                edge = chooseEdge(edge, getBottomEdge(TableCollapsedBorderSource::Column, column->style()));
                if(!edge.exists()) {
                    return edge;
                }
            }

            if(auto columnGroup = table->columnGroupAt(cellBox->columnIndex())) {
                edge = chooseEdge(edge, getBottomEdge(TableCollapsedBorderSource::ColumnGroup, columnGroup->style()));
                if(!edge.exists()) {
                    return edge;
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
    auto cellBefore = direction == Direction::Ltr ? cellBox->cellBefore() : cellBox->cellAfter();
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

    auto columnIndex = direction == Direction::Ltr ? cellBox->columnIndex() : cellBox->columnIndex() + cellBox->colSpan() - 1;
    auto prevColumnIndex = direction == Direction::Ltr ? columnIndex - 1 : columnIndex + 1;
    if(auto column = table->columnAt(columnIndex); column && !column->isColumnGroup()) {
        edge = chooseEdge(edge, getLeftEdge(TableCollapsedBorderSource::Column, column->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    auto columnGroup = table->columnGroupAt(columnIndex);
    auto prevColumnGroup = isStartColumn ? nullptr : table->columnGroupAt(prevColumnIndex);
    if(columnGroup && columnGroup != prevColumnGroup) {
        edge = chooseEdge(edge, getLeftEdge(TableCollapsedBorderSource::ColumnGroup, columnGroup->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    if(!isStartColumn) {
        if(auto column = table->columnAt(prevColumnIndex); column && !column->isColumnGroup()) {
            auto rightEdge = getRightEdge(TableCollapsedBorderSource::Column, column->style());
            edge = direction == Direction::Ltr ? chooseEdge(rightEdge, edge) : chooseEdge(edge, rightEdge);
            if(!edge.exists()) {
                return edge;
            }
        }

        if(prevColumnGroup && prevColumnGroup != columnGroup) {
            auto rightEdge = getRightEdge(TableCollapsedBorderSource::ColumnGroup, prevColumnGroup->style());
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
    auto cellAfter = direction == Direction::Ltr ? cellBox->cellAfter() : cellBox->cellBefore();
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

    auto columnIndex = direction == Direction::Ltr ? cellBox->columnIndex() + cellBox->colSpan() - 1 : cellBox->columnIndex();
    auto nextColumnIndex = direction == Direction::Ltr ? columnIndex + 1 : columnIndex - 1;
    if(auto column = table->columnAt(columnIndex); column && !column->isColumnGroup()) {
        edge = chooseEdge(edge, getRightEdge(TableCollapsedBorderSource::Column, column->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    auto columnGroup = table->columnGroupAt(columnIndex);
    auto nextColumnGroup = isEndColumn ? nullptr : table->columnGroupAt(nextColumnIndex);
    if(columnGroup && columnGroup != nextColumnGroup) {
        edge = chooseEdge(edge, getRightEdge(TableCollapsedBorderSource::ColumnGroup, columnGroup->style()));
        if(!edge.exists()) {
            return edge;
        }
    }

    if(!isEndColumn) {
        if(auto column = table->columnAt(nextColumnIndex); column && !column->isColumnGroup()) {
            auto leftEdge = getLeftEdge(TableCollapsedBorderSource::Column, column->style());
            edge = direction == Direction::Ltr ? chooseEdge(edge, leftEdge) : chooseEdge(leftEdge, edge);
            if(!edge.exists()) {
                return edge;
            }
        }

        if(nextColumnGroup && nextColumnGroup != columnGroup) {
            auto leftEdge = getLeftEdge(TableCollapsedBorderSource::ColumnGroup, nextColumnGroup->style());
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

TableCellBox* TableCellBox::cellAbove() const
{
    if(auto rowBox = firstRow()->rowAbove())
        return rowBox->cellAt(m_columnIndex);
    return nullptr;
}

TableCellBox* TableCellBox::cellBelow() const
{
    if(auto rowBox = lastRow()->rowBelow())
        return rowBox->cellAt(m_columnIndex);
    return nullptr;
}

TableCellBox* TableCellBox::cellBefore() const
{
    if(m_columnIndex == 0)
        return nullptr;
    return row()->cellAt(m_columnIndex - 1);
}

TableCellBox* TableCellBox::cellAfter() const
{
    return row()->cellAt(m_columnIndex + m_colSpan);
}

TableRowBox* TableCellBox::firstRow() const
{
    return section()->rowAt(m_rowIndex);
}

TableRowBox* TableCellBox::lastRow() const
{
    return section()->rowAt(m_rowIndex + m_rowSpan - 1);
}

void TableCellBox::paintBackgroundBehindCell(const PaintInfo& info, const Point& offset, const BoxStyle* backgroundStyle) const
{
    if(style()->visibility() != Visibility::Visible)
        return;
    const auto collapseBorders = table()->isBorderCollapsed();
    if(!collapseBorders && style()->emptyCells() == EmptyCells::Hide && !firstChild())
        return;
    Point adjustedOffset(offset + location());
    Rect borderRect(adjustedOffset, size());
    paintBackgroundStyle(info, borderRect, backgroundStyle);
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
    const auto collapseBorders = table()->isBorderCollapsed();
    if(!collapseBorders && style()->emptyCells() == EmptyCells::Hide && !firstChild())
        return;
    Rect borderRect(offset, size());
    paintBackground(info, borderRect);
    if(!collapseBorders) {
        paintBorder(info, borderRect);
    }
}

TableCaptionBox::TableCaptionBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style)
{
}

} // namespace plutobook
