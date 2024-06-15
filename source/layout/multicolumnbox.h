#ifndef PLUTOBOOK_MULTICOLUMNBOX_H
#define PLUTOBOOK_MULTICOLUMNBOX_H

#include "blockbox.h"

namespace plutobook {

class MultiColumnRow {
public:
    explicit MultiColumnRow(BoxFrame* box)
        : m_box(box)
    {}

    bool hasColumnSpan() const { return m_box && m_box->hasColumnSpan(); }
    BoxFrame* box() const { return m_box; }
    float y() const { return m_y; }

private:
    BoxFrame* m_box;
    float m_y;
};

using MultiColumnRowList = std::pmr::vector<MultiColumnRow>;

class MultiColumnFlowBox final : public BlockFlowBox {
public:
    static MultiColumnFlowBox* create(const BoxStyle* parentStyle);

    bool isMultiColumnFlowBox() const final { return true; }
    BlockFlowBox* columnBlockFlowBox() const;
    uint32_t columnCount() const { return m_columnCount; }
    const MultiColumnRowList& rows() const { return m_rows; }

    void updatePreferredWidths() const final;
    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const final;
    void build() final;
    void layout() final;

    const char* name() const final { return "MultiColumnFlowBox"; }

private:
    MultiColumnFlowBox(const RefPtr<BoxStyle>& style);
    mutable uint32_t m_columnCount{0};
    MultiColumnRowList m_rows;
};

inline BlockFlowBox* MultiColumnFlowBox::columnBlockFlowBox() const
{
    return static_cast<BlockFlowBox*>(parentBox());
}

template<>
struct is_a<MultiColumnFlowBox> {
    static bool check(const Box& box) { return box.isMultiColumnFlowBox(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_MULTICOLUMNBOX_H
