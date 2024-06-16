#ifndef PLUTOBOOK_MULTICOLUMNBOX_H
#define PLUTOBOOK_MULTICOLUMNBOX_H

#include "blockbox.h"

#include <list>

namespace plutobook {

class MultiColumnItem {
public:
    MultiColumnItem() = default;

    float x() const { return m_x; }
    float height() const { return m_height; }

    void setX(float x) { m_x = x; }
    void setHeight(float height) { m_height = height; }

private:
    float m_x{0};
    float m_height{0};
};

using MultiColumnItemList = std::pmr::list<MultiColumnItem>;

class MultiColumnRow {
public:
    explicit MultiColumnRow(BoxFrame* box)
        : m_items(box->heap())
        , m_box(box)
    {}

    const MultiColumnItemList& items() const { return m_items; }
    MultiColumnItemList& items() { return m_items; }

    bool hasColumnSpan() const { return m_box->hasColumnSpan(); }
    BoxFrame* box() const { return m_box; }
    float y() const { return m_y; }
    float width() const { return m_width; }

    void setY(float y) { m_y = y; }
    void setWidth(float width) { m_width = width; }

private:
    MultiColumnItemList m_items;
    BoxFrame* m_box;
    float m_y{0};
    float m_width{0};
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
