#ifndef PLUTOBOOK_COLUMNBUILDER_H
#define PLUTOBOOK_COLUMNBUILDER_H

#include <cstdint>

namespace plutobook {

class MultiColumnItem;
class MultiColumnRow;
class MultiColumnFlowBox;
class BoxFrame;

class ColumnBuilder {
public:
    ColumnBuilder(ColumnBuilder* parent, MultiColumnFlowBox* column);

    void enterBox(BoxFrame* box, float y) { m_top += y; }
    void exitBox(BoxFrame* box, float y) { m_top -= y; }
    void advance(float y) { m_uncommittedTop = m_top + y; }
    void commit();

    void columnize();

    void closeColumn();
    void closeRow();

    MultiColumnRow* currentRow() const;
    MultiColumnItem* currentColumn() const;

private:
    ColumnBuilder* m_parent;
    MultiColumnFlowBox* m_column;
    uint32_t m_currentRowIndex{0};
    float m_top{0};
    float m_uncommittedTop{0};
    float m_committedTop{0};
    bool m_isColumnOpen{false};
    bool m_isRowOpen{false};
};

} // namespace plutobook

#endif // PLUTOBOOK_COLUMNBUILDER_H
