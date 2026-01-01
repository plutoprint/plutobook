/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_LINEBOX_H
#define PLUTOBOOK_LINEBOX_H

#include "textshape.h"

namespace plutobook {

class Box;
class BoxModel;
class BoxFrame;
class BlockFlowBox;
class FlowLineBox;
class RootLineBox;

class Node;
class PaintInfo;
class Point;
class Size;
class Rect;

enum class PaintPhase;
enum class VerticalAlignType : uint8_t;

class LineBox : public HeapMember {
public:
    virtual ~LineBox();
    virtual bool isTextLineBox() const { return false; }
    virtual bool isReplacedLineBox() const { return false; }
    virtual bool isFlowLineBox() const { return false; }
    virtual bool isRootLineBox() const { return false; }

    virtual float lineHeight() const = 0;
    virtual float baselinePosition() const = 0;

    Box* box() const { return m_box; }
    Node* node() const;
    BoxStyle* style() const;

    FlowLineBox* parentLine() const { return m_parentLine; }
    void setParentLine(FlowLineBox* parentLine) { m_parentLine = parentLine; }

    uint32_t lineIndex() const { return m_lineIndex; }
    void setLineIndex(uint32_t lineIndex) { m_lineIndex = lineIndex; }

    float x() const { return m_x; }
    float y() const { return m_y; }
    float width() const { return m_width; }
    float height() const;

    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setWidth(float width) { m_width = width; }

    float right() const { return m_x + m_width; }
    float bottom() const { return m_y + height(); }

    Point location() const;
    Size size() const;
    Rect rect() const;

    float verticalAlignPosition() const;
    VerticalAlignType verticalAlignType() const;

    virtual void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) = 0;
    virtual void serialize(std::ostream& o, int indent) const = 0;

    virtual const char* name() const { return "LineBox"; }

protected:
    LineBox(Box* box, float width);
    Box* m_box;
    FlowLineBox* m_parentLine{nullptr};
    uint32_t m_lineIndex{0};
    float m_x{0};
    float m_y{0};
    float m_width;
};

class TextBox;

class TextLineBox final : public LineBox {
public:
    static std::unique_ptr<TextLineBox> create(TextBox* box, const TextShapeView& shape, float width, float expansion);

    bool isTextLineBox() const final { return true; }

    float lineHeight() const final;
    float baselinePosition() const final;

    TextBox* box() const;
    const TextShapeView& shape() const { return m_shape; }
    float shapeWidth() const { return m_shapeWidth; }
    float expansion() const { return m_expansion; }

    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) final;
    void serialize(std::ostream& o, int indent) const final;

    ~TextLineBox() final;

    const char* name() const final { return "TextLineBox"; }

private:
    TextLineBox(TextBox* box, const TextShapeView& shape, float width, float expansion);
    TextShapeView m_shape;
    float m_shapeWidth;
    float m_expansion;
};

template<>
struct is_a<TextLineBox> {
    static bool check(const LineBox& line) { return line.isTextLineBox(); }
};

class ReplacedLineBox final : public LineBox {
public:
    static std::unique_ptr<ReplacedLineBox> create(BoxFrame* box);

    bool isReplacedLineBox() const final { return true; }
    float lineHeight() const final;
    float baselinePosition() const final;
    BoxFrame* box() const;

    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) final;
    void serialize(std::ostream& o, int indent) const final;

    const char* name() const final { return "ReplacedLineBox"; }

private:
    ReplacedLineBox(BoxFrame* box);
};

template<>
struct is_a<ReplacedLineBox> {
    static bool check(const LineBox& line) { return line.isReplacedLineBox(); }
};

using LineBoxList = std::pmr::vector<LineBox*>;

class FlowLineBox : public LineBox {
public:
    static std::unique_ptr<FlowLineBox> create(BoxModel* box);

    bool isFlowLineBox() const final { return true; }

    float lineHeight() const override;
    float baselinePosition() const override;

    BoxModel* box() const;
    const LineBoxList& children() const { return m_children; }
    bool hasLeftEdge() const { return m_hasLeftEdge; }
    bool hasRightEdge() const { return m_hasRightEdge; }
    bool isEmptyLine() const { return m_isEmptyLine; }
    bool isFirstLine() const { return m_isFirstLine; }

    void setHasLeftEdge(bool value) { m_hasLeftEdge = value; }
    void setHasRightEdge(bool value) { m_hasRightEdge = value; }
    void setIsEmptyLine(bool value) { m_isEmptyLine = value; }
    void setIsFirstLine(bool value) { m_isFirstLine = value; }

    void addChild(LineBox* child);

    float marginLeft() const;
    float marginRight() const;

    float paddingLeft() const;
    float paddingRight() const;

    float borderLeft() const;
    float borderRight() const;

    void computeMaxAscentAndDescent(float& maxAscent, float& maxDescent, float& maxPositionTop, float& maxPositionBottom);
    void adjustMaxAscentAndDescent(float& maxAscent, float& maxDescent, float maxPositionTop, float maxPositionBottom) const;

    float placeInHorizontalDirection(float offsetX, const BlockFlowBox* block);
    void placeInVerticalDirection(float y, float maxHeight, float maxAscent, RootLineBox* rootLine);

    float overflowTop() const { return m_overflowTop; }
    float overflowBottom() const { return m_overflowBottom; }
    float overflowLeft() const { return m_overflowLeft; }
    float overflowRight() const { return m_overflowRight; }

    Rect visualOverflowRect() const;

    void addOverflowRect(const BoxFrame* child, float dx, float dy);
    void addOverflowRect(float top, float bottom, float left, float right);
    void addOverflowRect(const Rect& overflowRect);

    void updateOverflowRect(float lineTop, float lineBottom);

    void paintOutlines(const PaintInfo& info, const Point& offset) const;
    void paintDecorations(const PaintInfo& info, const Point& offset) const;
    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) override;
    void serialize(std::ostream& o, int indent) const override;

    const char* name() const override { return "FlowLineBox"; }

protected:
    FlowLineBox(BoxModel* box);
    LineBoxList m_children;
    bool m_hasLeftEdge{false};
    bool m_hasRightEdge{false};
    bool m_isEmptyLine{false};
    bool m_isFirstLine{false};

    float m_overflowTop{0};
    float m_overflowBottom{0};
    float m_overflowLeft{0};
    float m_overflowRight{0};
};

template<>
struct is_a<FlowLineBox> {
    static bool check(const LineBox& line) { return line.isFlowLineBox(); }
};

class FragmentBuilder;

class RootLineBox final : public FlowLineBox {
public:
    static std::unique_ptr<RootLineBox> create(BlockFlowBox* box);

    bool isRootLineBox() const final { return true; }

    BlockFlowBox* box() const;
    float lineTop() const { return m_lineTop; }
    float lineBottom() const { return m_lineBottom; }
    void updateLineTopAndBottom(const LineBox* line);

    float alignInHorizontalDirection(float startOffset);
    float alignInVerticalDirection(FragmentBuilder* fragmentainer, float blockHeight);
    float adjustLineBoxInFragmentFlow(FragmentBuilder* fragmentainer, float offset, float lineHeight) const;

    const char* name() const final { return "RootLineBox"; }

private:
    RootLineBox(BlockFlowBox* box);
    float m_lineTop{0};
    float m_lineBottom{0};
};

template<>
struct is_a<RootLineBox> {
    static bool check(const LineBox& line) { return line.isRootLineBox(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_LINEBOX_H
