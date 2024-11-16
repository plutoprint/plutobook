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
    explicit ColumnBuilder(MultiColumnFlowBox* column);

    void enterBox(const BoxFrame* box, float y) { m_top += y; }
    void exitBox(const BoxFrame* box, float y) { m_top -= y; }

private:
    MultiColumnFlowBox* m_column;
    uint32_t m_currentRowIndex{0};
    float m_top{0};
};

} // namespace plutobook

#endif // PLUTOBOOK_COLUMNBUILDER_H
