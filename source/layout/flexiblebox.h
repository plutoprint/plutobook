#ifndef PLUTOBOOK_FLEXIBLEBOX_H
#define PLUTOBOOK_FLEXIBLEBOX_H

#include "blockbox.h"

#include <span>

namespace plutobook {

enum class FlexSign {
    Positive,
    Negative
};

enum class FlexViolation {
    None,
    Min,
    Max
};

class FlexibleBox;

class FlexItem {
public:
    FlexItem(BoxFrame* box, int order, float flexGrow, float flexShrink, AlignItem alignSelf);

    BoxFrame* box() const { return m_box; }
    int order() const { return m_order; }
    float flexGrow() const { return m_flexGrow; }
    float flexShrink() const { return m_flexShrink; }
    float flexFactor(FlexSign sign) const { return sign == FlexSign::Positive ? m_flexGrow : m_flexShrink; }
    AlignItem alignSelf() const { return m_alignSelf; }

    void setViolation(FlexViolation violation) { m_violation = violation; }
    FlexViolation violation() const { return m_violation; }

    bool minViolation() const { return m_violation == FlexViolation::Min; }
    bool maxViolation() const { return m_violation == FlexViolation::Max; }

    float flexBaseSize() const { return m_flexBaseSize; }
    float targetMainSize() const { return m_targetMainSize; }

    void setFlexBaseSize(float value) { m_flexBaseSize = value; }
    void setTargetMainSize(float value) { m_targetMainSize = value; }

    std::optional<float> computeWidthUsing(const Length& widthLength) const;
    std::optional<float> computeHeightUsing(const Length& heightLength) const;

    float constrainWidth(float width) const;
    float constrainHeight(float height) const;

    float constrainMainSize(float size) const;
    float constrainCrossSize(float size) const;
    float computeFlexBaseSize() const;

    FlexibleBox* flexBox() const;
    FlexDirection flexDirection() const;
    Direction direction() const;

    bool isHorizontalFlow() const;
    bool isVerticalFlow() const;

    float flexBaseMarginBoxSize() const;
    float flexBaseBorderBoxSize() const;

    float targetMainMarginBoxSize() const;
    float targetMainBorderBoxSize() const;

    float marginBoxMainSize() const;
    float marginBoxCrossSize() const;
    float marginBoxCrossBaseline() const;

    float borderBoxMainSize() const;
    float borderBoxCrossSize() const;

    float marginStart() const;
    float marginEnd() const;
    float marginBefore() const;
    float marginAfter() const;

private:
    BoxFrame* m_box;
    int m_order;
    float m_flexGrow;
    float m_flexShrink;
    AlignItem m_alignSelf;
    FlexViolation m_violation{FlexViolation::None};
    float m_flexBaseSize{0};
    float m_targetMainSize{0};
};

using FlexItemSpan = std::span<FlexItem>;

class FlexLine {
public:
    explicit FlexLine(const FlexItemSpan& items)
        : m_items(items)
    {}

    const FlexItemSpan& items() const { return m_items; }

    float crossOffset() const { return m_crossOffset; }
    float crossSize() const { return m_crossSize; }
    float crossBaseline() const { return m_crossBaseline; }

    void setCrossOffset(float offset) { m_crossOffset = offset; }
    void setCrossSize(float size) { m_crossSize = size; }
    void setCrossBaseline(float baseline) { m_crossBaseline = baseline; }

private:
    FlexItemSpan m_items;
    float m_crossOffset = 0;
    float m_crossSize = 0;
    float m_crossBaseline = 0;
};

using FlexItemList = std::pmr::vector<FlexItem>;
using FlexLineList = std::pmr::vector<FlexLine>;

class FlexibleBox final : public BlockBox {
public:
    FlexibleBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isFlexibleBox() const final { return true; }
    void addChild(Box* newChild) final;

    void updateOverflowRect() final;
    void computePreferredWidths(float& minWidth, float& maxWidth) const final;

    std::optional<float> firstLineBaseline() const final;
    std::optional<float> lastLineBaseline() const final;
    std::optional<float> inlineBlockBaseline() const final;

    float computeMainSize(float hypotheticalMainSize) const;
    float availableCrossSize() const;

    float borderAndPaddingStart() const;
    float borderAndPaddingEnd() const;
    float borderAndPaddingBefore() const;
    float borderAndPaddingAfter() const;

    FlexDirection flexDirection() const { return m_flexDirection; }
    FlexWrap flexWrap() const { return m_flexWrap; }
    AlignContent justifyContent() const { return m_justifyContent; }
    AlignContent alignContent() const { return m_alignContent; }

    bool isHorizontalFlow() const { return m_flexDirection == FlexDirection::Row || m_flexDirection == FlexDirection::RowReverse; }
    bool isVerticalFlow() const { return m_flexDirection == FlexDirection::Column || m_flexDirection == FlexDirection::ColumnReverse; }
    bool isMultiLine() const { return m_flexWrap == FlexWrap::Wrap || m_flexWrap == FlexWrap::WrapReverse; }

    const FlexItemList& items() { return m_items; }
    const FlexLineList& lines() const { return m_lines; }

    void build() final;
    void layout() final;

    void paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase) final;
    void fragmentize(FragmentBuilder& builder, float top) const final;

    const char* name() const final { return "FlexibleBox"; }

private:
    FlexDirection m_flexDirection;
    FlexWrap m_flexWrap;
    AlignContent m_justifyContent;
    AlignContent m_alignContent;
    FlexItemList m_items;
    FlexLineList m_lines;
};

template<>
struct is_a<FlexibleBox> {
    static bool check(const Box& box) { return box.isFlexibleBox(); }
};

inline FlexibleBox* FlexItem::flexBox() const
{
    return static_cast<FlexibleBox*>(m_box->parentBox());
}

inline FlexDirection FlexItem::flexDirection() const
{
    return flexBox()->flexDirection();
}

inline Direction FlexItem::direction() const
{
    return flexBox()->direction();
}

inline bool FlexItem::isHorizontalFlow() const
{
    return flexBox()->isHorizontalFlow();
}

inline bool FlexItem::isVerticalFlow() const
{
    return flexBox()->isVerticalFlow();
}

} // namespace plutobook

#endif // PLUTOBOOK_FLEXIBLEBOX_H
