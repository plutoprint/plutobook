/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_BOX_H
#define PLUTOBOOK_BOX_H

#include "boxstyle.h"
#include "geometry.h"

#include <memory>

namespace plutobook {

enum class PaintPhase {
    Decorations,
    Floats,
    Contents,
    Outlines
};

class GraphicsContext;

class PaintInfo {
public:
    PaintInfo(GraphicsContext& context, const Rect& rect)
        : m_context(context), m_rect(rect)
    {}

    GraphicsContext& operator*() const { return m_context; }
    GraphicsContext* operator->() const { return &m_context; }

    GraphicsContext& context() const { return m_context; }
    const Rect& rect() const { return m_rect; }

private:
    GraphicsContext& m_context;
    Rect m_rect;
};

class Node;
class BoxLayer;
class BoxView;
class BoxModel;
class BlockBox;
class BlockFlowBox;
class LineBox;

class Box : public HeapMember {
public:
    Box(Node* node, const RefPtr<BoxStyle>& style);

    virtual ~Box();
    virtual bool avoidsFloats() const { return true; }
    virtual void addChild(Box* newChild);

    Node* node() const { return m_node; }
    BoxStyle* style() const { return m_style.get(); }
    Box* parentBox() const { return m_parentBox; }
    Box* nextSibling() const { return m_nextSibling; }
    Box* prevSibling() const { return m_prevSibling; }
    Box* firstChild() const { return m_firstChild; }
    Box* lastChild() const { return m_lastChild; }

    void setParentBox(Box* box) { m_parentBox = box; }
    void setPrevSibling(Box* box) { m_prevSibling = box; }
    void setNextSibling(Box* box) { m_nextSibling = box; }

    void insertChild(Box* newChild, Box* nextChild);
    void appendChild(Box* newChild);
    void removeChild(Box* child);

    void moveChildrenTo(Box* newParent);

    static Box* create(Node* node, const RefPtr<BoxStyle>& style);
    static Box* createAnonymous(Display display, const BoxStyle* parentStyle);
    static BlockFlowBox* createAnonymousBlock(const BoxStyle* parentStyle);

    bool canContainFixedPositionedBoxes() const;
    bool canContainAbsolutePositionedBoxes() const;

    BlockBox* containingBlock() const;
    BoxModel* containingBox() const;
    BoxLayer* enclosingLayer() const;
    BoxView* view() const;

    bool isBodyBox() const;
    bool isRootBox() const;
    bool isListMarkerBox() const { return isInsideListMarkerBox() || isOutsideListMarkerBox(); }
    bool isFlexItem() const;

    virtual bool isBoxModel() const { return false; }
    virtual bool isBoxFrame() const { return false; }
    virtual bool isBoxView() const { return false; }
    virtual bool isTextBox() const { return false; }
    virtual bool isLineBreakBox() const { return false; }
    virtual bool isWordBreakBox() const { return false; }
    virtual bool isContentBox() const { return false; }
    virtual bool isLeaderBox() const { return false; }
    virtual bool isTargetCounterBox() const { return false; }
    virtual bool isInlineBox() const { return false; }
    virtual bool isBlockBox() const { return false; }
    virtual bool isBlockFlowBox() const { return false; }
    virtual bool isFlexibleBox() const { return false; }
    virtual bool isReplacedBox() const { return false; }
    virtual bool isImageBox() const { return false; }
    virtual bool isListItemBox() const { return false; }
    virtual bool isInsideListMarkerBox() const { return false; }
    virtual bool isOutsideListMarkerBox() const { return false; }
    virtual bool isMultiColumnRowBox() const { return false; }
    virtual bool isMultiColumnSpanBox() const { return false; }
    virtual bool isMultiColumnFlowBox() const { return false; }
    virtual bool isPageBox() const { return false; }
    virtual bool isPageMarginBox() const { return false; }
    virtual bool isTableBox() const { return false; }
    virtual bool isTableCellBox() const { return false; }
    virtual bool isTableColumnBox() const { return false; }
    virtual bool isTableRowBox() const { return false; }
    virtual bool isTableCaptionBox() const { return false; }
    virtual bool isTableSectionBox() const { return false; }
    virtual bool isTextInputBox() const { return false; }
    virtual bool isSelectBox() const { return false; }
    virtual bool isSVGInlineTextBox() const { return false; }
    virtual bool isSVGTSpanBox() const { return false; }
    virtual bool isSVGTextBox() const { return false; }
    virtual bool isSVGBoxModel() const { return false; }
    virtual bool isSVGRootBox() const { return false; }
    virtual bool isSVGImageBox() const { return false; }
    virtual bool isSVGGeometryBox() const { return false; }
    virtual bool isSVGPathBox() const { return false; }
    virtual bool isSVGShapeBox() const { return false; }
    virtual bool isSVGContainerBox() const { return false; }
    virtual bool isSVGHiddenContainerBox() const { return false; }
    virtual bool isSVGTransformableContainerBox() const { return false; }
    virtual bool isSVGViewportContainerBox() const { return false; }
    virtual bool isSVGResourceContainerBox() const { return false; }
    virtual bool isSVGResourceMarkerBox() const { return false; }
    virtual bool isSVGResourceClipperBox() const { return false; }
    virtual bool isSVGResourceMaskerBox() const { return false; }
    virtual bool isSVGResourcePaintServerBox() const { return false; }
    virtual bool isSVGResourcePatternBox() const { return false; }
    virtual bool isSVGGradientStopBox() const { return false; }
    virtual bool isSVGResourceGradientBox() const { return false; }
    virtual bool isSVGResourceLinearGradientBox() const { return false; }
    virtual bool isSVGResourceRadialGradientBox() const { return false; }

    bool isRelativePositioned() const { return m_style->position() == Position::Relative; }
    bool isFixedPositioned() const { return m_style->position() == Position::Fixed; }

    bool isAnonymous() const { return m_isAnonymous; }
    bool isAnonymousBlock() const { return m_isAnonymousBlock; }
    bool isChildrenInline() const { return m_isChildrenInline; }
    bool isInline() const { return m_isInline; }
    bool isFloating() const { return m_isFloating; }
    bool isPositioned() const { return m_isPositioned; }
    bool isFloatingOrPositioned() const { return m_isFloating || m_isPositioned; }
    bool isReplaced() const { return m_isReplaced; }
    bool isRunning() const { return m_isRunning; }
    bool isOverflowHidden() const { return m_isOverflowHidden; }
    bool isBackgroundStolen() const { return m_isBackgroundStolen; }
    bool isBorderCollapsed() const { return m_isBorderCollapsed; }
    bool isTableHeader() const { return m_isTableHeader; }
    bool isTableFooter() const { return m_isTableFooter; }
    bool isColumnSpanner() const { return m_isColumnSpanner; }

    void setIsAnonymous(bool value) { m_isAnonymous = value; }
    void setIsAnonymousBlock(bool value) { m_isAnonymousBlock = value; }
    void setIsChildrenInline(bool value) { m_isChildrenInline = value; }
    void setIsInline(bool value) { m_isInline = value; }
    void setIsFloating(bool value) { m_isFloating = value; }
    void setIsPositioned(bool value) { m_isPositioned = value; }
    void setIsReplaced(bool value) { m_isReplaced = value; }
    void setIsRunning(bool value) { m_isRunning = value; }
    void setIsOverflowHidden(bool value) { m_isOverflowHidden = value; }
    void setIsBackgroundStolen(bool value) { m_isBackgroundStolen = value; }
    void setIsBorderCollapsed(bool value) { m_isBorderCollapsed = value; }
    void setIsTableHeader(bool value) { m_isTableHeader = value; }
    void setIsTableFooter(bool value) { m_isTableFooter = value; }
    void setIsColumnSpanner(bool value) { m_isColumnSpanner = value; }

    bool hasColumnFlowBox() const { return m_hasColumnFlowBox; }
    bool hasTransform() const { return m_hasTransform; }
    bool hasLayer() const { return m_hasLayer; }

    void setHasColumnFlowBox(bool value) { m_hasColumnFlowBox = value; }
    void setHasTransform(bool value) { m_hasTransform = value; }
    void setHasLayer(bool value) { m_hasLayer = value; }

    Heap* heap() const { return m_style->heap(); }
    Document* document() const { return m_style->document(); }

    void paintAnnotation(GraphicsContext& context, const Rect& rect) const;

    virtual Rect fillBoundingBox() const { return Rect::Invalid; }
    virtual Rect strokeBoundingBox() const { return Rect::Invalid; }
    virtual Rect paintBoundingBox() const { return Rect::Invalid; }
    virtual Transform localTransform() const { return Transform::Identity; }

    virtual void build();

    static void serializeStart(std::ostream& o, int indent, bool selfClosing, const Box* box, const LineBox* line);
    static void serializeEnd(std::ostream& o, int indent, bool selfClosing, const Box* box, const LineBox* line);

    void serialize(std::ostream& o, int indent) const;
    virtual void serializeChildren(std::ostream& o, int indent) const;

    virtual const char* name() const { return "Box"; }

private:
    Node* m_node;
    RefPtr<BoxStyle> m_style;
    Box* m_parentBox{nullptr};
    Box* m_nextSibling{nullptr};
    Box* m_prevSibling{nullptr};
    Box* m_firstChild{nullptr};
    Box* m_lastChild{nullptr};
    bool m_isAnonymous : 1 {false};
    bool m_isAnonymousBlock : 1 {false};
    bool m_isChildrenInline : 1 {false};
    bool m_isInline : 1 {false};
    bool m_isFloating : 1 {false};
    bool m_isPositioned : 1 {false};
    bool m_isReplaced : 1 {false};
    bool m_isRunning : 1 {false};
    bool m_isOverflowHidden : 1 {false};
    bool m_isBackgroundStolen : 1 {false};
    bool m_isColumnSpanner : 1 {false};
    bool m_isBorderCollapsed : 1 {false};
    bool m_isTableHeader : 1 {false};
    bool m_isTableFooter : 1 {false};
    bool m_hasColumnFlowBox : 1 {false};
    bool m_hasTransform : 1 {false};
    bool m_hasLayer : 1 {false};
};

class BoxModel : public Box {
public:
    BoxModel(Node* node, const RefPtr<BoxStyle>& style);
    ~BoxModel() override;

    bool isBoxModel() const final { return true; }
    void addChild(Box* newChild) override;

    void paintBackgroundStyle(const PaintInfo& info, const Rect& borderRect, const BoxStyle* backgroundStyle, bool includeLeftEdge = true, bool includeRightEdge = true) const;
    void paintBackground(const PaintInfo& info, const Rect& borderRect, bool includeLeftEdge = true, bool includeRightEdge = true) const;

    virtual void paintRootBackground(const PaintInfo& info) const {}

    void paintBorder(const PaintInfo& info, const Rect& borderRect, bool includeLeftEdge = true, bool includeRightEdge = true) const;
    void paintOutline(const PaintInfo& info, const Rect& borderRect) const;

    virtual void paint(const PaintInfo& info, const Point& offset, PaintPhase phase);

    virtual Rect visualOverflowRect() const = 0;
    virtual Rect borderBoundingBox() const = 0;
    virtual bool requiresLayer() const = 0;

    BoxLayer* layer() const { return m_layer.get(); }

    void paintLayer(GraphicsContext& context, const Rect& rect);
    void updateLayerPosition();

    float relativePositionOffsetX() const;
    float relativePositionOffsetY() const;
    Point relativePositionOffset() const;

    float containingBlockWidthForPositioned(const BoxModel* container) const;
    float containingBlockWidthForPositioned() const { return containingBlockWidthForPositioned(containingBox()); }

    float containingBlockHeightForPositioned(const BoxModel* container) const;
    float containingBlockHeightForPositioned() const { return containingBlockHeightForPositioned(containingBox()); }

    virtual float containingBlockWidthForContent(const BlockBox* container) const;
    float containingBlockWidthForContent() const { return containingBlockWidthForContent(containingBlock()); }

    std::optional<float> containingBlockHeightForContent(const BlockBox* container) const;
    std::optional<float> containingBlockHeightForContent() const { return containingBlockHeightForContent(containingBlock()); }

    float marginTop() const { return m_marginTop; }
    float marginBottom() const { return m_marginBottom; }
    float marginLeft() const { return m_marginLeft; }
    float marginRight() const { return m_marginRight; }

    float marginWidth() const { return m_marginLeft + m_marginRight; }
    float marginHeight() const { return m_marginTop + m_marginBottom; }

    void setMarginTop(float value) { m_marginTop = value; }
    void setMarginBottom(float value) { m_marginBottom = value; }
    void setMarginLeft(float value) { m_marginLeft = value; }
    void setMarginRight(float value) { m_marginRight = value; }

    void updateVerticalMargins(const BlockBox* container);
    void updateHorizontalMargins(const BlockBox* container);
    void updateMarginWidths(const BlockBox* container);

    float paddingTop() const { return m_paddingTop; }
    float paddingBottom() const { return m_paddingBottom; }
    float paddingLeft() const { return m_paddingLeft; }
    float paddingRight() const { return m_paddingRight; }

    float paddingWidth() const { return paddingLeft() + paddingRight(); }
    float paddingHeight() const { return paddingTop() + paddingBottom(); }

    void setPaddingTop(float value) { m_paddingTop = value; }
    void setPaddingBottom(float value) { m_paddingBottom = value; }
    void setPaddingLeft(float value) { m_paddingLeft = value; }
    void setPaddingRight(float value) { m_paddingRight = value; }

    void updateVerticalPaddings(const BlockBox* container);
    void updateHorizontalPaddings(const BlockBox* container);
    void updatePaddingWidths(const BlockBox* container);

    virtual void computeBorderWidths(float& borderTop, float& borderBottom, float& borderLeft, float& borderRight) const;

    float borderTop() const;
    float borderBottom() const;
    float borderLeft() const;
    float borderRight() const;

    float borderWidth() const { return borderLeft() + borderRight(); }
    float borderHeight() const { return borderTop() + borderBottom(); }

    float borderAndPaddingTop() const { return borderTop() + paddingTop(); }
    float borderAndPaddingBottom() const { return borderBottom() + paddingBottom(); }
    float borderAndPaddingLeft() const { return borderLeft() + paddingLeft(); }
    float borderAndPaddingRight() const { return borderRight() + paddingRight(); }

    float borderAndPaddingWidth() const { return borderWidth() + paddingWidth(); }
    float borderAndPaddingHeight() const { return borderHeight() + paddingHeight(); }

    float marginStart(Direction direction) const { return direction == Direction::Ltr ? m_marginLeft : m_marginRight; }
    float marginEnd(Direction direction) const { return direction == Direction::Ltr ? m_marginRight : m_marginLeft; }

    float borderStart(Direction direction) const { return direction == Direction::Ltr ? borderLeft() : borderRight(); }
    float borderEnd(Direction direction) const { return direction == Direction::Ltr ? borderRight() : borderLeft(); }

    float paddingStart(Direction direction) const { return direction == Direction::Ltr ? paddingLeft() : paddingRight(); }
    float paddingEnd(Direction direction) const { return direction == Direction::Ltr ? paddingRight() : paddingLeft(); }

    float marginStart() const { return marginStart(style()->direction()); }
    float marginEnd() const { return marginEnd(style()->direction()); }

    float borderStart() const { return borderStart(style()->direction()); }
    float borderEnd() const { return borderEnd(style()->direction()); }

    float paddingStart() const { return paddingStart(style()->direction()); }
    float paddingEnd() const { return paddingEnd(style()->direction()); }

    void build() override;

    const char* name() const override { return "BoxModel"; }

private:
    std::unique_ptr<BoxLayer> m_layer;

    float m_marginTop{0};
    float m_marginBottom{0};
    float m_marginLeft{0};
    float m_marginRight{0};

    float m_paddingTop{0};
    float m_paddingBottom{0};
    float m_paddingLeft{0};
    float m_paddingRight{0};

    mutable float m_borderTop{-1};
    mutable float m_borderBottom{-1};
    mutable float m_borderLeft{-1};
    mutable float m_borderRight{-1};

    friend class BoxFrame;
};

template<>
struct is_a<BoxModel> {
    static bool check(const Box& box) { return box.isBoxModel(); }
};

class ReplacedLineBox;
class FragmentBuilder;

class BoxFrame : public BoxModel {
public:
    BoxFrame(Node* node, const RefPtr<BoxStyle>& style);
    ~BoxFrame() override;

    bool isBoxFrame() const final { return true; }
    bool requiresLayer() const override;

    BoxFrame* parentBoxFrame() const;
    BoxFrame* nextBoxFrame() const;
    BoxFrame* prevBoxFrame() const;
    BoxFrame* firstBoxFrame() const;
    BoxFrame* lastBoxFrame() const;

    ReplacedLineBox* line() const { return m_line.get(); }
    void setLine(std::unique_ptr<ReplacedLineBox> line);

    float x() const { return m_x; }
    float y() const { return m_y; }
    float width() const { return m_width; }
    float height() const { return m_height; }

    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setWidth(float width) { m_width = width; }
    void setHeight(float height) { m_height = height; }

    void setLocation(float x, float y) { m_x = x; m_y = y; }
    void setSize(float width, float height) { m_width = width; m_height = height; }

    Point location() const { return Point(m_x, m_y); }
    Size size() const { return Size(m_width, m_height); }

    float borderBoxWidth() const { return m_width; }
    float borderBoxHeight() const { return m_height; }

    float paddingBoxWidth() const { return borderBoxWidth() - borderLeft() - borderRight(); }
    float paddingBoxHeight() const { return borderBoxHeight() - borderTop() - borderBottom(); }

    float contentBoxWidth() const { return paddingBoxWidth() - paddingLeft() - paddingRight(); }
    float contentBoxHeight() const { return paddingBoxHeight() - paddingTop() - paddingBottom(); }

    float marginBoxWidth() const { return borderBoxWidth() + marginLeft() + marginRight(); }
    float marginBoxHeight() const { return borderBoxHeight() + marginTop() + marginBottom(); }

    Size borderBoxSize() const { return Size(borderBoxWidth(), borderBoxHeight()); }
    Size paddingBoxSize() const { return Size(paddingBoxWidth(), paddingBoxHeight()); }
    Size contentBoxSize() const { return Size(contentBoxWidth(), contentBoxHeight()); }
    Size marginBoxSize() const { return Size(marginBoxWidth(), marginBoxHeight()); }

    Rect borderBoxRect() const { return Rect(0, 0, borderBoxWidth(), borderBoxHeight()); }
    Rect paddingBoxRect() const { return Rect(borderLeft(), borderTop(), paddingBoxWidth(), paddingBoxHeight()); }
    Rect contentBoxRect() const { return Rect(borderLeft() + paddingLeft(), borderTop() + paddingTop(), contentBoxWidth(), contentBoxHeight()); }
    Rect marginBoxRect() const { return Rect(-marginLeft(), -marginRight(), marginBoxWidth(), marginBoxHeight()); }

    Rect visualOverflowRect() const override;
    Rect borderBoundingBox() const override;
    Rect paintBoundingBox() const override;

    float overrideWidth() const { return m_overrideWidth; }
    float overrideHeight() const { return m_overrideHeight; }

    void setOverrideWidth(float width) { m_overrideWidth = width; }
    void setOverrideHeight(float height) { m_overrideHeight = height; }

    bool hasOverrideWidth() const { return m_overrideWidth >= 0; }
    bool hasOverrideHeight() const { return m_overrideHeight >= 0; }

    void setOverrideSize(float width, float height) { m_overrideWidth = width; m_overrideHeight = height; }
    void clearOverrideSize() { setOverrideSize(-1, -1); }

    virtual void computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const;

    float minPreferredWidth() const;
    float maxPreferredWidth() const;

    float adjustBorderBoxWidth(float width) const;
    float adjustBorderBoxHeight(float height) const;
    float adjustContentBoxWidth(float width) const;
    float adjustContentBoxHeight(float height) const;

    void computeHorizontalStaticDistance(Length& leftLength, Length& rightLength, const BoxModel* container, float containerWidth) const;
    void computeVerticalStaticDistance(Length& topLength, Length& bottomLength, const BoxModel* container) const;

    void computeHorizontalMargins(float& marginLeft, float& marginRight, float childWidth, const BlockBox* container, float containerWidth) const;
    void computeVerticalMargins(float& marginTop, float& marginBottom) const;

    float computeIntrinsicWidthUsing(const Length& widthLength, float containerWidth) const;

    virtual void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const;
    virtual void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const;

    void updateWidth();
    void updateHeight();

    virtual bool isSelfCollapsingBlock() const { return false; }

    virtual float maxMarginTop(bool positive) const;
    virtual float maxMarginBottom(bool positive) const;

    float collapsedMarginTop() const;
    float collapsedMarginBottom() const;

    virtual std::optional<float> firstLineBaseline() const { return std::nullopt; }
    virtual std::optional<float> lastLineBaseline() const { return std::nullopt; }
    virtual std::optional<float> inlineBlockBaseline() const { return std::nullopt; }

    float overflowTop() const { return m_overflowTop; }
    float overflowBottom() const { return m_overflowBottom; }
    float overflowLeft() const { return m_overflowLeft; }
    float overflowRight() const { return m_overflowRight; }

    virtual void updateOverflowRect();

    void addOverflowRect(const BoxFrame* child, float dx, float dy);
    void addOverflowRect(float top, float bottom, float left, float right);
    void addOverflowRect(const Rect& overflowRect);

    virtual void paintOutlines(const PaintInfo& info, const Point& offset);
    virtual void paintDecorations(const PaintInfo& info, const Point& offset);
    virtual void layout(FragmentBuilder* fragmentainer);

    const char* name() const override { return "BoxFrame"; }

private:
    std::unique_ptr<ReplacedLineBox> m_line;

    float m_x{0};
    float m_y{0};
    float m_width{0};
    float m_height{0};

    float m_overrideWidth{-1};
    float m_overrideHeight{-1};

    float m_overflowTop{0};
    float m_overflowBottom{0};
    float m_overflowLeft{0};
    float m_overflowRight{0};

    mutable float m_minPreferredWidth{-1};
    mutable float m_maxPreferredWidth{-1};
};

template<>
struct is_a<BoxFrame> {
    static bool check(const Box& box) { return box.isBoxFrame(); }
};

inline BoxFrame* BoxFrame::parentBoxFrame() const
{
    return static_cast<BoxFrame*>(parentBox());
}

inline BoxFrame* BoxFrame::nextBoxFrame() const
{
    return static_cast<BoxFrame*>(nextSibling());
}

inline BoxFrame* BoxFrame::prevBoxFrame() const
{
    return static_cast<BoxFrame*>(prevSibling());
}

inline BoxFrame* BoxFrame::firstBoxFrame() const
{
    return static_cast<BoxFrame*>(firstChild());
}

inline BoxFrame* BoxFrame::lastBoxFrame() const
{
    return static_cast<BoxFrame*>(lastChild());
}

inline Rect BoxFrame::visualOverflowRect() const
{
    if(!isOverflowHidden())
        return Rect(m_overflowLeft, m_overflowTop, m_overflowRight - m_overflowLeft, m_overflowBottom - m_overflowTop);
    return borderBoxRect();
}

inline Rect BoxFrame::borderBoundingBox() const
{
    return Rect(m_x, m_y, m_width, m_height);
}

inline Rect BoxFrame::paintBoundingBox() const
{
    return borderBoundingBox();
}

} // namespace plutobook

#endif // PLUTOBOOK_BOX_H
