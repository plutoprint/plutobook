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
    float rightOffsetForContent() const { return leftOffsetForContent() + availableWidth(); }
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

    enum ColumnBoundaryRule { AssociateWithFormerColumn, AssociateWithLatterColumn };

    float columnHeightForOffset(float offset) const;
    float columnRemainingHeightForOffset(float offset, ColumnBoundaryRule rule) const;
    float nextColumnTop(float offset, ColumnBoundaryRule rule) const;

    void setColumnBreak(float offset, float spaceShortage);
    void updateMinimumColumnHeight(float offset, float minHeight);

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

    LineLayout* lineLayout() const { return m_lineLayout.get(); }
    const FloatingBoxList* floatingBoxes() const { return m_floatingBoxes.get(); }
    MultiColumnFlowBox* columnFlowBox() const { return m_columnFlowBox; }

    std::optional<float> firstLineBaseline() const override;
    std::optional<float> lastLineBaseline() const override;
    std::optional<float> inlineBlockBaseline() const override;

    void insertFloatingBox(BoxFrame* box);
    void removeFloatingBox(BoxFrame* box);

    void collectIntrudingFloats();
    void collectOverhangingFloats();
    void addIntrudingFloats(BlockFlowBox* prevBlock, float offsetX, float offsetY);
    void addOverhangingFloats(BlockFlowBox* childBlock);
    void positionNewFloats();
    bool containsFloat(Box* box) const;
    bool containsFloats() const { return m_floatingBoxes && !m_floatingBoxes->empty(); }

    float leftFloatBottom() const;
    float rightFloatBottom() const;
    float floatBottom() const;
    float nextFloatBottom(float y) const;

    float leftOffsetForFloat(float y, float offset, bool indent, float* heightRemaining = nullptr) const;
    float rightOffsetForFloat(float y, float offset, bool indent, float* heightRemaining = nullptr) const;

    float lineOffsetForAlignment(float remainingWidth) const;
    float startAlignedOffsetForLine(float y, bool indent) const;

    float leftOffsetForLine(float y, bool indent) const { return leftOffsetForFloat(y, leftOffsetForContent(), indent); }
    float rightOffsetForLine(float y, bool indent) const { return rightOffsetForFloat(y, rightOffsetForContent(), indent); }
    float startOffsetForLine(float y, bool indent) const { return style()->isLeftToRightDirection() ? leftOffsetForLine(y, indent) : width() - rightOffsetForLine(y, indent); }
    float endOffsetForLine(float y, bool indent) const { return style()->isRightToLeftDirection() ? leftOffsetForLine(y, indent) : width() - rightOffsetForLine(y, indent); }
    float availableWidthForLine(float y, bool indent) const { return std::max(0.f, rightOffsetForLine(y, indent) - leftOffsetForLine(y, indent)); }

    void adjustFloatingBox(const MarginInfo& marginInfo);
    void adjustPositionedBox(BoxFrame* child, const MarginInfo& marginInfo);

    void handleBottomOfBlock(float top, float bottom, MarginInfo& marginInfo);
    float collapseMargins(BoxFrame* child, MarginInfo& marginInfo);

    void updateMaxMargins();

    float maxPositiveMarginTop() const { return m_maxPositiveMarginTop; }
    float maxNegativeMarginTop() const { return m_maxNegativeMarginTop; }
    float maxPositiveMarginBottom() const { return m_maxPositiveMarginBottom; }
    float maxNegativeMarginBottom() const { return m_maxNegativeMarginBottom; }

    bool isSelfCollapsingBlock() const override;
    float maxMarginTop(bool positive) const override;
    float maxMarginBottom(bool positive) const override;

    float getClearDelta(BoxFrame* child, float y) const;
    void clearFloats(Clear clear);

    void determineHorizontalPosition(BoxFrame* child) const;

    void layoutBlockChild(BoxFrame* child, MarginInfo& marginInfo);
    void layoutBlockChildren();

    void layout() override;
    void build() override;

    void paintFloats(const PaintInfo& info, const Point& offset);
    void paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase) override;
    void fragmentize(FragmentBuilder& builder, float top) const override;

    const char* name() const override { return "BlockFlowBox"; }

private:
    std::unique_ptr<LineLayout> m_lineLayout;
    std::unique_ptr<FloatingBoxList> m_floatingBoxes;
    MultiColumnFlowBox* m_columnFlowBox{nullptr};

    float m_maxPositiveMarginTop{0};
    float m_maxNegativeMarginTop{0};
    float m_maxPositiveMarginBottom{0};
    float m_maxNegativeMarginBottom{0};
};

template<>
struct is_a<BlockFlowBox> {
    static bool check(const Box& box) { return box.isBlockFlowBox(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_BLOCKBOX_H
