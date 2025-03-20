#ifndef PLUTOBOOK_MULTICOLUMNBOX_H
#define PLUTOBOOK_MULTICOLUMNBOX_H

#include "blockbox.h"
#include "fragmentbuilder.h"

namespace plutobook {

class MultiColumnContentRun {
public:
    explicit MultiColumnContentRun(float breakOffset)
        : m_breakOffset(breakOffset)
    {}

    float breakOffset() const { return m_breakOffset; }
    uint32_t assumedImplicitBreaks() const { return m_assumedImplicitBreaks; }
    void assumeAnotherImplicitBreak() { ++m_assumedImplicitBreaks; }

    float columnLogicalHeight(float startOffset) const;

private:
    float m_breakOffset;
    uint32_t m_assumedImplicitBreaks{0};
};

using MultiColumnContentRunList = std::pmr::vector<MultiColumnContentRun>;

class MultiColumnFlowBox;

class MultiColumnRowBox final : public BoxFrame {
public:
    static MultiColumnRowBox* create(MultiColumnFlowBox* columnFlow, const BoxStyle* parentStyle);

    bool isMultiColumnRowBox() const final { return true; }
    bool requiresLayer() const final { return false; }

    void updateOverflowRect() final;

    void computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const final;
    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const final;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const final;
    void layout(FragmentBuilder* fragmentainer) final;

    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) final;
    void paintColumnRules(GraphicsContext& context, const Point& offset);

    Rect columnRectAt(uint32_t columnIndex) const;
    Rect rowRectAt(uint32_t columnIndex) const;

    MultiColumnRowBox* prevRow() const;
    MultiColumnRowBox* nextRow() const;

    bool isFillBalance() const { return m_isFillBalance; }
    void setIsFillBalance(bool value) { m_isFillBalance = value; }

    bool requiresBalancing() const { return m_requiresBalancing; }

    MultiColumnFlowBox* columnFlowBox() const { return m_columnFlowBox; }
    float rowTop() const { return m_rowTop; }
    float rowBottom() const { return m_rowBottom; }

    void setRowTop(float top) { m_rowTop = top; }
    void setRowBottom(float bottom) { m_rowBottom = bottom; }

    float rowHeight() const { return m_rowBottom - m_rowTop; }
    float columnHeight() const { return m_columnHeight; }

    uint32_t numberOfColumns() const;
    float columnTopForOffset(float offset) const;

    void recordSpaceShortage(float spaceShortage);
    void updateMinimumColumnHeight(float height);
    void addContentRun(float endOffset);

    void resetColumnHeight(float columnHeight);
    bool recalculateColumnHeight(bool balancing);

private:
    MultiColumnRowBox(MultiColumnFlowBox* columnFlow, const RefPtr<BoxStyle>& style);

    float constrainColumnHeight(float columnHeight) const;
    float calculateColumnHeight(bool balancing) const;

    float rowTopAt(uint32_t columnIndex) const { return m_rowTop + columnIndex * m_columnHeight; }
    float rowHeightAt(uint32_t columnIndex) const;

    uint32_t columnIndexAtOffset(float offset, bool clampToExistingColumns) const;
    uint32_t findRunWithTallestColumns() const;

    void distributeImplicitBreaks();

    MultiColumnFlowBox* m_columnFlowBox;
    MultiColumnContentRunList m_runs;
    bool m_isFillBalance{true};
    bool m_requiresBalancing{true};
    float m_rowTop{0};
    float m_rowBottom{0};
    float m_columnHeight{0};
    float m_maxColumnHeight{0};
    float m_minSpaceShortage{0};
    float m_minimumColumnHeight{0};
};

template<>
struct is_a<MultiColumnRowBox> {
    static bool check(const Box& box) { return box.isMultiColumnRowBox(); }
};

class MultiColumnSpanBox final : public BoxFrame {
public:
    static MultiColumnSpanBox* create(BoxFrame* box, const BoxStyle* parentStyle);

    bool isMultiColumnSpanBox() const final { return true; }
    bool requiresLayer() const final { return false; }

    BoxFrame* box() const { return m_box; }
    MultiColumnFlowBox* columnFlowBox() const;

    void updateOverflowRect() final;

    void computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const final;
    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const final;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const final;
    void layout(FragmentBuilder* fragmentainer) final;

    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) final;

private:
    MultiColumnSpanBox(BoxFrame* box, const RefPtr<BoxStyle>& style);
    BoxFrame* m_box;
};

inline MultiColumnFlowBox* MultiColumnSpanBox::columnFlowBox() const
{
    return to<BlockFlowBox>(parentBox())->columnFlowBox();
}

template<>
struct is_a<MultiColumnSpanBox> {
    static bool check(const Box& box) { return box.isMultiColumnSpanBox(); }
};

class MultiColumnFlowBox final : public BlockFlowBox, public FragmentBuilder {
public:
    static MultiColumnFlowBox* create(const BoxStyle* parentStyle);

    bool isMultiColumnFlowBox() const final { return true; }
    bool requiresLayer() const final { return true; }
    bool avoidsFloats() const final { return true; }

    MultiColumnRowBox* firstRow() const;
    MultiColumnRowBox* lastRow() const;

    BoxFrame* firstMultiColumnBox() const;
    BoxFrame* lastMultiColumnBox() const;

    FragmentType fragmentType() const final { return FragmentType::Column; }

    float fragmentHeightForOffset(float offset) const final;
    float fragmentRemainingHeightForOffset(float offset, FragmentBoundaryRule rule) const final;

    void addForcedFragmentBreak(float offset) final;
    void setFragmentBreak(float offset, float spaceShortage) final;
    void updateMinimumFragmentHeight(float offset, float minHeight) final;

    void skipColumnSpanBox(BoxFrame* box, float offset);

    MultiColumnRowBox* columnRowAtOffset(float offset) const;
    BlockFlowBox* columnBlockFlowBox() const;

    uint32_t columnCount() const { return m_columnCount; }
    float columnGap() const { return m_columnGap; }

    bool layoutColumns(bool balancing);

    void computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const final;
    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const final;
    void layout(FragmentBuilder* fragmentainer) final;
    void build() final;

    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) final;

    const char* name() const final { return "MultiColumnFlowBox"; }

private:
    MultiColumnFlowBox(const RefPtr<BoxStyle>& style);
    MultiColumnRowBox* m_currentRow{nullptr};
    mutable uint32_t m_columnCount{0};
    float m_columnGap{0};
};

inline BoxFrame* MultiColumnFlowBox::firstMultiColumnBox() const
{
    return nextBoxFrame();
}

inline BoxFrame* MultiColumnFlowBox::lastMultiColumnBox() const
{
    auto box = parentBoxFrame()->lastBoxFrame();
    if(box && !box->isMultiColumnFlowBox())
        return box;
    return nullptr;
}

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
