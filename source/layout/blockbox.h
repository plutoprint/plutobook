/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_BLOCKBOX_H
#define PLUTOBOOK_BLOCKBOX_H

#include "box.h"

#include <set>

namespace plutobook {

using PositionedBoxList = std::pmr::set<BoxFrame*>;

class BlockBox : public BoxFrame {
public:
    BlockBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isBlockBox() const final { return true; }

    virtual void computeIntrinsicWidths(float& minWidth, float& maxWidth) const = 0;
    void computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const override;

    const PositionedBoxList* positionedBoxes() const { return m_positionedBoxes.get(); }
    bool containsPositonedBoxes() const { return m_positionedBoxes && !m_positionedBoxes->empty(); }

    void insertPositonedBox(BoxFrame* box);
    void removePositonedBox(BoxFrame* box);
    void layoutPositionedBoxes();

    float leftOffsetForContent() const { return borderLeft() + paddingLeft(); }
    float rightOffsetForContent() const { return leftOffsetForContent() + contentBoxWidth(); }
    float startOffsetForContent() const { return style()->isLeftToRightDirection() ? leftOffsetForContent() : width() - rightOffsetForContent(); }
    float endOffsetForContent() const { return style()->isRightToLeftDirection() ? leftOffsetForContent() : width() - rightOffsetForContent(); }

    float availableWidthForContent() const { return std::max(0.f, rightOffsetForContent() - leftOffsetForContent()); }

    float availableWidth() const { return contentBoxWidth(); }
    std::optional<float> availableHeight() const;

    bool shrinkToAvoidFloats() const;
    float shrinkWidthToAvoidFloats(float marginLeft, float marginRight, const BlockFlowBox* container) const;

    float computeWidthUsing(const Length& widthLength, const BlockBox* container, float containerWidth) const;
    std::optional<float> computeHeightUsing(const Length& heightLength) const;

    float constrainWidth(float width, const BlockBox* container, float containerWidth) const;
    float constrainBorderBoxHeight(float height) const;
    float constrainContentBoxHeight(float height) const;

    void computePositionedWidthUsing(const Length& widthLength, const BoxModel* container, Direction containerDirection, float containerWidth,
        const Length& leftLength, const Length& rightLength, const Length& marginLeftLength, const Length& marginRightLength,
        float& x, float& width, float& marginLeft, float& marginRight) const;

    void computePositionedWidth(float& x, float& width, float& marginLeft, float& marginRight) const;

    void computePositionedHeightUsing(const Length& heightLength, const BoxModel* container, float containerHeight, float contentHeight,
        const Length& topLength, const Length& bottomLength, const Length& marginTopLength, const Length& marginBottomLength,
        float& y, float& height, float& marginTop, float& marginBottom) const;

    void computePositionedHeight(float& y, float& height, float& marginTop, float& marginBottom) const;

    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const override;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const override;

    std::optional<float> firstLineBaseline() const override;
    std::optional<float> lastLineBaseline() const override;
    std::optional<float> inlineBlockBaseline() const override;

    virtual void paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase);
    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) override;

    const char* name() const override { return "BlockBox"; }

private:
    std::unique_ptr<PositionedBoxList> m_positionedBoxes;
};

template<>
struct is_a<BlockBox> {
    static bool check(const Box& box) { return box.isBlockBox(); }
};

class FloatingBox {
public:
    explicit FloatingBox(BoxFrame* box)
        : m_box(box), m_type(box->style()->floating())
    {}

    BoxFrame* box() const { return m_box; }
    Float type() const { return m_type; }

    void setIsIntruding(bool value) { m_isIntruding = value; }
    void setIsPlaced(bool value) { m_isPlaced = value; }

    bool isIntruding() const { return m_isIntruding; }
    bool isPlaced() const { return m_isPlaced; }

    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setWidth(float width) { m_width = width; }
    void setHeight(float height) { m_height = height; }

    float x() const { return m_x; }
    float y() const { return m_y; }
    float width() const { return m_width; }
    float height() const { return m_height; }

    float right() const { return m_x + m_width; }
    float bottom() const { return m_y + m_height; }

private:
    BoxFrame* m_box;
    Float m_type;

    bool m_isIntruding{false};
    bool m_isPlaced{false};

    float m_x{0};
    float m_y{0};
    float m_width{0};
    float m_height{0};
};

using FloatingBoxList = std::pmr::vector<FloatingBox>;

class MarginInfo;
class LineLayout;
class MultiColumnFlowBox;

class BlockFlowBox : public BlockBox {
public:
    BlockFlowBox(Node* node, const RefPtr<BoxStyle>& style);
    ~BlockFlowBox() override;

    bool isBlockFlowBox() const final { return true; }
    bool avoidsFloats() const override;
    void addChild(Box* newChild) override;

    void updateOverflowRect() override;
    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const override;

    virtual float computeVerticalAlignShift() const { return 0; }

    LineLayout* lineLayout() const { return m_lineLayout.get(); }
    const FloatingBoxList* floatingBoxes() const { return m_floatingBoxes.get(); }
    MultiColumnFlowBox* columnFlowBox() const { return m_columnFlowBox; }

    std::optional<float> firstLineBaseline() const override;
    std::optional<float> lastLineBaseline() const override;
    std::optional<float> inlineBlockBaseline() const override;

    void collectIntrudingFloats();
    void collectOverhangingFloats();

    void addIntrudingFloats(BlockFlowBox* prevBlock, float offsetX, float offsetY);
    void addOverhangingFloats(BlockFlowBox* childBlock);

    void positionFloatingBox(FloatingBox& floatingBox, FragmentBuilder* fragmentainer, float top);
    void positionNewFloats(FragmentBuilder* fragmentainer);

    FloatingBox& insertFloatingBox(BoxFrame* box);

    bool containsFloat(Box* box) const;
    bool containsFloats() const { return m_floatingBoxes && !m_floatingBoxes->empty(); }

    float leftFloatBottom() const;
    float rightFloatBottom() const;
    float floatBottom() const;
    float nextFloatBottom(float y) const;

    float leftOffsetForFloat(float top, float bottom, float offset, float* heightRemaining = nullptr) const;
    float rightOffsetForFloat(float top, float bottom, float offset, float* heightRemaining = nullptr) const;

    float leftOffsetForLine(float y, float height = 0, bool indent = false) const;
    float rightOffsetForLine(float y, float height = 0, bool indent = false) const;

    float lineOffsetForAlignment(float remainingWidth) const;
    float startAlignedOffsetForLine(float y, float height = 0, bool indent = false) const;

    float startOffsetForLine(float y, float height = 0, bool indent = false) const;
    float endOffsetForLine(float y, float height = 0, bool indent = false) const;

    float availableWidthForLine(float y, float height = 0, bool indent = false) const;

    void adjustFloatingBox(FragmentBuilder* fragmentainer, const MarginInfo& marginInfo);
    void adjustPositionedBox(BoxFrame* child, const MarginInfo& marginInfo);

    void handleBottomOfBlock(float top, float bottom, MarginInfo& marginInfo);
    float collapseMargins(BoxFrame* child, FragmentBuilder* fragmentainer, MarginInfo& marginInfo);

    void updateMaxMargins();

    float maxPositiveMarginTop() const { return m_maxPositiveMarginTop; }
    float maxNegativeMarginTop() const { return m_maxNegativeMarginTop; }
    float maxPositiveMarginBottom() const { return m_maxPositiveMarginBottom; }
    float maxNegativeMarginBottom() const { return m_maxNegativeMarginBottom; }

    bool isSelfCollapsingBlock() const override;

    float maxMarginTop(bool positive) const override;
    float maxMarginBottom(bool positive) const override;

    float getClearDelta(BoxFrame* child, float y) const;

    void estimateMarginTop(BoxFrame* child, float& positiveMarginTop, float& negativeMarginTop) const;
    float estimateVerticalPosition(BoxFrame* child, FragmentBuilder* fragmentainer, const MarginInfo& marginInfo) const;

    float determineVerticalPosition(BoxFrame* child, FragmentBuilder* fragmentainer, MarginInfo& marginInfo);
    void determineHorizontalPosition(BoxFrame* child) const;

    float adjustBlockChildInFragmentFlow(BoxFrame* child, FragmentBuilder* fragmentainer, float top);

    void layoutBlockChild(BoxFrame* child, FragmentBuilder* fragmentainer, MarginInfo& marginInfo);
    void layoutBlockChildren(FragmentBuilder* fragmentainer);

    virtual void layoutContents(FragmentBuilder* fragmentainer);
    void layout(FragmentBuilder* fragmentainer) override;

    void build() override;

    void paintFloats(const PaintInfo& info, const Point& offset);
    void paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase) override;
    void serializeChildren(std::ostream& o, int indent) const override;

    const char* name() const override { return "BlockFlowBox"; }

private:
    std::unique_ptr<LineLayout> m_lineLayout;
    std::unique_ptr<FloatingBoxList> m_floatingBoxes;
    MultiColumnFlowBox* m_columnFlowBox{nullptr};

    float m_maxPositiveMarginTop{-1};
    float m_maxNegativeMarginTop{-1};
    float m_maxPositiveMarginBottom{-1};
    float m_maxNegativeMarginBottom{-1};
};

inline float BlockFlowBox::startOffsetForLine(float y, float height, bool indent) const
{
    return style()->isLeftToRightDirection() ? leftOffsetForLine(y, height, indent) : width() - rightOffsetForLine(y, height, indent);
}

inline float BlockFlowBox::endOffsetForLine(float y, float height, bool indent) const
{
    return style()->isRightToLeftDirection() ? leftOffsetForLine(y, height, indent) : width() - rightOffsetForLine(y, height, indent);
}

inline float BlockFlowBox::availableWidthForLine(float y, float height, bool indent) const
{
    return std::max(0.f, rightOffsetForLine(y, height, indent) - leftOffsetForLine(y, height, indent));
}

template<>
struct is_a<BlockFlowBox> {
    static bool check(const Box& box) { return box.isBlockFlowBox(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_BLOCKBOX_H
