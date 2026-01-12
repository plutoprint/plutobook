/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_TABLEBOX_H
#define PLUTOBOOK_TABLEBOX_H

#include "blockbox.h"

#include <list>

namespace plutobook {

class TableCaptionBox;
class TableSectionBox;
class TableCellBox;
class TableColumnBox;
class TableColumn;
class TableCollapsedBorderEdge;

using TableCaptionBoxList = std::pmr::list<TableCaptionBox*>;
using TableSectionBoxList = std::pmr::list<TableSectionBox*>;
using TableColumnList = std::pmr::vector<TableColumn>;
using TableCollapsedBorderEdgeList = std::pmr::set<TableCollapsedBorderEdge>;

class TableLayoutAlgorithm;

class TableBox final : public BlockBox {
public:
    TableBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableBox() const final { return true; }
    void addChild(Box* newChild) final;

    void updateOverflowRect() final;
    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const final;
    void computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const final;
    void computeBorderWidths(float& borderTop, float& borderBottom, float& borderLeft, float& borderRight) const final;

    std::optional<float> firstLineBaseline() const final;
    std::optional<float> lastLineBaseline() const final;
    std::optional<float> inlineBlockBaseline() const final;

    TableColumnList& columns() { return m_columns; }
    const TableColumnList& columns() const { return m_columns; }

    TableColumnBox* columnAt(size_t index) const;
    size_t columnCount() const;

    const TableSectionBoxList& sections() const { return m_sections; }

    TableSectionBox* headerSection() const;
    TableSectionBox* footerSection() const;

    TableSectionBox* topSection() const;
    TableSectionBox* bottomSection() const;

    TableSectionBox* sectionAbove(const TableSectionBox* sectionBox) const;
    TableSectionBox* sectionBelow(const TableSectionBox* sectionBox) const;

    TableCellBox* cellAbove(const TableCellBox* cellBox) const;
    TableCellBox* cellBelow(const TableCellBox* cellBox) const;
    TableCellBox* cellBefore(const TableCellBox* cellBox) const;
    TableCellBox* cellAfter(const TableCellBox* cellBox) const;

    float borderHorizontalSpacing() const { return m_borderHorizontalSpacing; }
    float borderVerticalSpacing() const { return m_borderVerticalSpacing; }

    float availableHorizontalSpace() const;

    void layoutCaption(TableCaptionBox* caption, FragmentBuilder* fragmentainer);
    void layout(FragmentBuilder* fragmentainer) final;
    void build() final;

    void paintDecorations(const PaintInfo& info, const Point& offset) final;
    void paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase) final;

    const char* name() const final { return "TableBox"; }

private:
    TableColumnList m_columns;
    TableCaptionBoxList m_captions;
    TableSectionBoxList m_sections;
    float m_borderHorizontalSpacing;
    float m_borderVerticalSpacing;
    std::unique_ptr<TableLayoutAlgorithm> m_tableLayout;
    std::unique_ptr<TableCollapsedBorderEdgeList> m_collapsedBorderEdges;
};

template<>
struct is_a<TableBox> {
    static bool check(const Box& box) { return box.isTableBox(); }
};

class TableLayoutAlgorithm : public HeapMember {
public:
    static std::unique_ptr<TableLayoutAlgorithm> create(TableBox* table);

    virtual ~TableLayoutAlgorithm() = default;
    virtual void computeIntrinsicWidths(float& minWidth, float& maxWidth) = 0;
    virtual void build() = 0;
    virtual void layout() = 0;

protected:
    TableLayoutAlgorithm(TableBox* table) : m_table(table) {}
    TableBox* m_table;
};

class FixedTableLayoutAlgorithm final : public TableLayoutAlgorithm {
public:
    static std::unique_ptr<FixedTableLayoutAlgorithm> create(TableBox* table);

    void computeIntrinsicWidths(float& minWidth, float& maxWidth) final;
    void build()  final;
    void layout() final;

private:
    FixedTableLayoutAlgorithm(TableBox* table);
    using LengthList = std::pmr::vector<Length>;
    LengthList m_widths;
};

struct TableColumnWidth {
    float maxFixedWidth = 0.f;
    float maxPercentWidth = 0.f;
    float minWidth = 0.f;
    float maxWidth = 0.f;
    Length width = Length::Auto;
};

using TableCellBoxList = std::pmr::vector<TableCellBox*>;
using TableColumnWidthList = std::pmr::vector<TableColumnWidth>;

class AutoTableLayoutAlgorithm final : public TableLayoutAlgorithm {
public:
    static std::unique_ptr<AutoTableLayoutAlgorithm> create(TableBox* table);

    void computeIntrinsicWidths(float& minWidth, float& maxWidth) final;
    void build()  final;
    void layout() final;

private:
    AutoTableLayoutAlgorithm(TableBox* table);
    TableColumnWidthList m_columnWidths;
    TableCellBoxList m_spanningCells;
};

class TableRowBox;

using TableRowBoxList = std::pmr::vector<TableRowBox*>;

class TableSectionBox final : public BoxFrame {
public:
    TableSectionBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableSectionBox() const final { return true; }
    void addChild(Box* newChild) final;

    void updateOverflowRect() final;

    std::optional<float> firstLineBaseline() const final;
    std::optional<float> lastLineBaseline() const final;

    const TableRowBoxList& rows() const { return m_rows; }
    TableRowBoxList& rows() { return m_rows; }
    TableRowBox* rowAt(size_t index) const { return m_rows.at(index); }
    size_t rowCount() const { return m_rows.size(); }

    TableBox* table() const;
    TableRowBox* firstRow() const;
    TableRowBox* lastRow() const;

    void distributeExcessHeightToRows(float distributableHeight);

    void layoutRows(FragmentBuilder* fragmentainer, float headerHeight, float footerHeight);
    void layout(FragmentBuilder* fragmentainer) final;
    void build() final;

    void paintCollapsedBorders(const PaintInfo& info, const Point& offset, const TableCollapsedBorderEdge& currentEdge) const;
    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) final;

    const char* name() const final { return "TableSectionBox"; }

private:
    TableRowBoxList m_rows;
    TableCellBoxList m_spanningCells;
};

template<>
struct is_a<TableSectionBox> {
    static bool check(const Box& box) { return box.isTableSectionBox(); }
};

inline TableBox* TableSectionBox::table() const
{
    return static_cast<TableBox*>(parentBox());
}

class TableCell {
public:
    TableCell(TableCellBox* box, bool inColSpan, bool inRowSpan)
        : m_box(box), m_inColSpan(inColSpan), m_inRowSpan(inRowSpan)
    {}

    TableCellBox& operator*() const { return *m_box; }
    TableCellBox* operator->() const { return m_box; }
    bool inColOrRowSpan() const { return m_inColSpan || m_inRowSpan; }

    TableCellBox* box() const { return m_box; }
    bool inColSpan() const { return m_inColSpan; }
    bool inRowSpan() const { return m_inRowSpan; }

private:
    TableCellBox* m_box;
    bool m_inColSpan;
    bool m_inRowSpan;
};

using TableCellMap = std::pmr::map<uint32_t, TableCell>;

class TableRowBox final : public BoxFrame {
public:
    TableRowBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableRowBox() const final { return true; }
    void addChild(Box* newChild) final;

    void updateOverflowRect() final;

    TableCellBox* firstCell() const;
    TableCellBox* lastCell() const;

    TableRowBox* prevRow() const;
    TableRowBox* nextRow() const;

    TableSectionBox* section() const;
    TableBox* table() const { return section()->table(); }

    const TableCellMap& cells() const { return m_cells; }
    TableCellMap& cells()  { return m_cells; }
    TableCellBox* cellAt(uint32_t columnIndex) const;

    uint32_t rowIndex() const { return m_rowIndex; }
    void setRowIndex(uint32_t rowIndex) { m_rowIndex = rowIndex; }

    float maxBaseline() const { return m_maxBaseline; }
    void setMaxBaseline(float baseline) { m_maxBaseline = baseline; }

    float maxFixedHeight() const { return m_maxFixedHeight; }
    void setMaxFixedHeight(float height) { m_maxFixedHeight = height; }

    float maxPercentHeight() const { return m_maxPercentHeight; }
    void setMaxPercentHeight(float height) { m_maxPercentHeight = height; }

    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) final;

    const char* name() const final { return "TableRowBox"; }

private:
    TableCellMap m_cells;
    uint32_t m_rowIndex{0};
    float m_maxBaseline{0};
    float m_maxFixedHeight{0};
    float m_maxPercentHeight{0};
};

template<>
struct is_a<TableRowBox> {
    static bool check(const Box& box) { return box.isTableRowBox(); }
};

inline TableRowBox* TableSectionBox::firstRow() const
{
    return static_cast<TableRowBox*>(firstChild());
}

inline TableRowBox* TableSectionBox::lastRow() const
{
    return static_cast<TableRowBox*>(lastChild());
}

inline TableRowBox* TableRowBox::prevRow() const
{
    return static_cast<TableRowBox*>(prevSibling());
}

inline TableRowBox* TableRowBox::nextRow() const
{
    return static_cast<TableRowBox*>(nextSibling());
}

inline TableSectionBox* TableRowBox::section() const
{
    return static_cast<TableSectionBox*>(parentBox());
}

class TableColumn {
public:
    explicit TableColumn(TableColumnBox* box)
        : m_box(box)
    {}

    TableColumnBox* box() const { return m_box; }

    float x() const { return m_x; }
    float width() const { return m_width; }

    void setX(float x) { m_x = x; }
    void setWidth(float width) { m_width = width; }

private:
    TableColumnBox* m_box;
    float m_x{0};
    float m_width{0};
};

inline TableColumnBox* TableBox::columnAt(size_t index) const
{
    return m_columns[index].box();
}

inline size_t TableBox::columnCount() const
{
    return m_columns.size();
}

class TableColumnBox final : public Box {
public:
    TableColumnBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableColumnBox() const final { return true; }

    uint32_t span() const { return m_span; }
    void setSpan(uint32_t span) { m_span = span; }
    TableColumnBox* columnGroup() const;

    const char* name() const final { return "TableColumnBox"; }

private:
    uint32_t m_span{1};
};

template<>
struct is_a<TableColumnBox> {
    static bool check(const Box& box) { return box.isTableColumnBox(); }
};

enum class TableCollapsedBorderSource : uint8_t {
    None,
    Table,
    ColumnGroup,
    Column,
    RowGroup,
    Row,
    Cell
};

class TableCollapsedBorderEdge {
public:
    TableCollapsedBorderEdge() = default;
    TableCollapsedBorderEdge(TableCollapsedBorderSource source, LineStyle style, float width, const Color& color)
        : m_source(source), m_style(style)
        , m_width(style > LineStyle::Hidden ? width : 0.f), m_color(color)
    {}

    TableCollapsedBorderSource source() const { return m_source; }
    LineStyle style() const { return m_style; }
    float width() const { return m_width; }
    const Color& color() const { return m_color; }

    bool exists() const { return m_source != TableCollapsedBorderSource::None; }
    bool isRenderable() const { return m_width > 0 && m_style > LineStyle::Hidden && m_color.alpha() > 0; }
    bool isSameIgnoringColor(const TableCollapsedBorderEdge& edge) const;
    bool isLessThan(const TableCollapsedBorderEdge& edge) const;

private:
    TableCollapsedBorderSource m_source{TableCollapsedBorderSource::None};
    LineStyle m_style{LineStyle::None};
    float m_width{0.f};
    Color m_color;
};

inline bool operator==(const TableCollapsedBorderEdge& a, const TableCollapsedBorderEdge& b)
{
    return a.isSameIgnoringColor(b);
}

inline bool operator<(const TableCollapsedBorderEdge& a, const TableCollapsedBorderEdge& b)
{
    return a.isLessThan(b);
}

class TableCollapsedBorderEdges : public HeapMember {
public:
    static std::unique_ptr<TableCollapsedBorderEdges> create(const TableCellBox* cellBox);

    const TableCollapsedBorderEdge& topEdge() const { return m_topEdge; }
    const TableCollapsedBorderEdge& bottomEdge() const { return m_bottomEdge; }
    const TableCollapsedBorderEdge& leftEdge() const { return m_leftEdge; }
    const TableCollapsedBorderEdge& rightEdge() const { return m_rightEdge; }

    static TableCollapsedBorderEdge chooseEdge(const TableCollapsedBorderEdge& a, const TableCollapsedBorderEdge& b);

    static TableCollapsedBorderEdge getTopEdge(TableCollapsedBorderSource source, const BoxStyle* style);
    static TableCollapsedBorderEdge getBottomEdge(TableCollapsedBorderSource source, const BoxStyle* style);
    static TableCollapsedBorderEdge getLeftEdge(TableCollapsedBorderSource source, const BoxStyle* style);
    static TableCollapsedBorderEdge getRightEdge(TableCollapsedBorderSource source, const BoxStyle* style);

    static TableCollapsedBorderEdge calcTopEdge(const TableCellBox* cellBox);
    static TableCollapsedBorderEdge calcBottomEdge(const TableCellBox* cellBox);
    static TableCollapsedBorderEdge calcLeftEdge(const TableCellBox* cellBox);
    static TableCollapsedBorderEdge calcRightEdge(const TableCellBox* cellBox);

private:
    TableCollapsedBorderEdges(const TableCellBox* cellBox);
    TableCollapsedBorderEdge m_topEdge;
    TableCollapsedBorderEdge m_bottomEdge;
    TableCollapsedBorderEdge m_leftEdge;
    TableCollapsedBorderEdge m_rightEdge;
};

class TableCellBox final : public BlockFlowBox {
public:
    TableCellBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableCellBox() const final { return true; }
    bool avoidsFloats() const final { return true; }

    float computeVerticalAlignShift() const final;

    bool isBaselineAligned() const;
    float cellBaselinePosition() const;
    float heightForRowSizing() const;

    void computeBorderWidths(float& borderTop, float& borderBottom, float& borderLeft, float& borderRight) const;

    const TableCollapsedBorderEdges& collapsedBorderEdges() const;
    EmptyCells emptyCells() const { return style()->emptyCells(); }
    uint32_t colSpan() const { return m_colSpan; }
    uint32_t rowSpan() const { return m_rowSpan; }
    uint32_t columnIndex() const { return m_columnIndex; }
    uint32_t rowIndex() const { return row()->rowIndex(); }

    void setColSpan(uint32_t span) { m_colSpan = span; }
    void setRowSpan(uint32_t span) { m_rowSpan = span; }
    void setColumnIndex(uint32_t columnIndex) { m_columnIndex = columnIndex; }

    TableCellBox* prevCell() const;
    TableCellBox* nextCell() const;

    TableRowBox* row() const;
    TableColumnBox* column() const;
    TableSectionBox* section() const { return row()->section(); }
    TableBox* table() const { return section()->table(); }

    void paintBackgroundBehindCell(const PaintInfo& info, const Point& offset, const BoxStyle* backgroundStyle) const;
    void paintCollapsedBorders(const PaintInfo& info, const Point& offset, const TableCollapsedBorderEdge& currentEdge) const;
    void paintDecorations(const PaintInfo& info, const Point& offset) final;

    const char* name() const final { return "TableCellBox"; }

private:
    mutable std::unique_ptr<TableCollapsedBorderEdges> m_collapsedBorderEdges;
    uint32_t m_colSpan{1};
    uint32_t m_rowSpan{1};
    uint32_t m_columnIndex{0};
};

template<>
struct is_a<TableCellBox> {
    static bool check(const Box& box) { return box.isTableCellBox(); }
};

inline TableCellBox* TableCellBox::prevCell() const
{
    return static_cast<TableCellBox*>(prevSibling());
}

inline TableCellBox* TableCellBox::nextCell() const
{
    return static_cast<TableCellBox*>(nextSibling());
}

inline TableCellBox* TableRowBox::firstCell() const
{
    return static_cast<TableCellBox*>(firstChild());
}

inline TableCellBox* TableRowBox::lastCell() const
{
    return static_cast<TableCellBox*>(lastChild());
}

inline TableRowBox* TableCellBox::row() const
{
    return static_cast<TableRowBox*>(parentBox());
}

inline TableColumnBox* TableCellBox::column() const
{
    return table()->columnAt(m_columnIndex);
}

class TableCaptionBox final : public BlockFlowBox {
public:
    TableCaptionBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isTableCaptionBox() const final { return true; }
    bool avoidsFloats() const final { return true; }
    float containingBlockWidthForContent(const BlockBox* container) const final;
    CaptionSide captionSide() const { return style()->captionSide(); }

    const char* name() const final { return "TableCaptionBox"; }
};

template<>
struct is_a<TableCaptionBox> {
    static bool check(const Box& box) { return box.isTableCaptionBox(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_TABLEBOX_H
