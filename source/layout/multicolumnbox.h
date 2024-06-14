#ifndef PLUTOBOOK_MULTICOLUMNBOX_H
#define PLUTOBOOK_MULTICOLUMNBOX_H

#include "blockbox.h"

namespace plutobook {

class MultiColumnFlowBox final : public BlockFlowBox {
public:
    static MultiColumnFlowBox* create(const BoxStyle* parentStyle);

    bool isMultiColumnFlowBox() const final { return true; }
    BlockFlowBox* columnBlockFlowBox() const;
    uint32_t columnCount() const { return m_columnCount; }

    void updatePreferredWidths() const final;
    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const final;
    void build() final;

    const char* name() const final { return "MultiColumnFlowBox"; }

private:
    MultiColumnFlowBox(const RefPtr<BoxStyle>& style);
    mutable uint32_t m_columnCount{0};
};

inline BlockFlowBox* MultiColumnFlowBox::columnBlockFlowBox() const
{
    return static_cast<BlockFlowBox*>(parentBox());
}

class MultiColumnSetBox final : public BlockBox {
public:
    static MultiColumnSetBox* create(const BoxStyle* parentStyle);

    bool isMultiColumnSetBox() const final { return true; }
    MultiColumnFlowBox* columnFlowBox() const;

    void computePreferredWidths(float& minWidth, float& maxWidth) const final;
    void paginate(PageBuilder& builder, float top) const final;
    void layout() final;

    const char* name() const final { return "MultiColumnSetBox"; }

private:
    MultiColumnSetBox(const RefPtr<BoxStyle>& style);
};

inline MultiColumnFlowBox* MultiColumnSetBox::columnFlowBox() const
{
    return static_cast<BlockFlowBox*>(parentBox())->columnFlowBox();
}

class MultiColumnSpanBox final : public BlockBox {
public:
    static MultiColumnSpanBox* create(BoxFrame* box, const BoxStyle* parentStyle);

    bool isMultiColumnSpanBox() const final { return true; }
    MultiColumnFlowBox* columnFlowBox() const;
    BoxFrame* box() const { return m_box; }

    void computePreferredWidths(float& minWidth, float& maxWidth) const final;
    void paginate(PageBuilder& builder, float top) const final;
    void layout() final;

    const char* name() const final { return "MultiColumnSpanBox"; }

private:
    MultiColumnSpanBox(BoxFrame* box, const RefPtr<BoxStyle>& style);
    BoxFrame* m_box;
};

inline MultiColumnFlowBox* MultiColumnSpanBox::columnFlowBox() const
{
    return static_cast<BlockFlowBox*>(parentBox())->columnFlowBox();
}

} // namespace plutobook

#endif // PLUTOBOOK_MULTICOLUMNBOX_H
