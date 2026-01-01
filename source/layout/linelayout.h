/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_LINELAYOUT_H
#define PLUTOBOOK_LINELAYOUT_H

#include "textshape.h"
#include "textbreakiterator.h"

#include <unicode/ubidi.h>

namespace plutobook {

class Box;
class Document;

struct LineItemsData;

class LineItem {
public:
    enum class Type : uint8_t {
        NormalText,
        TabulationText,
        LeaderText,
        SoftBreakOpportunity,
        HardBreakOpportunity,
        BidiControl,
        Floating,
        Positioned,
        Replaced,
        InlineStart,
        InlineEnd
    };

    enum class CollapseType : uint8_t {
        NotCollapsible,
        Collapsible,
        Collapsed,
        OpaqueToCollapsing
    };

    LineItem(Type type, Box* box, uint32_t startOffset, uint32_t endOffset)
        : m_box(box), m_startOffset(startOffset), m_endOffset(endOffset), m_type(type)
    {}

    Type type() const { return m_type; }
    Box* box() const { return m_box; }
    uint32_t startOffset() const { return m_startOffset; }
    uint32_t endOffset() const { return m_endOffset; }
    uint32_t length() const { return m_endOffset - m_startOffset; }

    void setStartOffset(uint32_t offset) { m_startOffset = offset; }
    void setEndOffset(uint32_t offset) { m_endOffset = offset; }

    CollapseType collapseType() const { return m_collapseType; }
    void setCollapseType(CollapseType collapseType) { m_collapseType = collapseType; }

    UBiDiLevel bidiLevel() const { return m_bidiLevel; }
    void setBidiLevel(UBiDiLevel bidiLevel) { m_bidiLevel = bidiLevel; }

    bool hasCollapsibleNewline() const { return m_hasCollapsibleNewline; }
    void setHasCollapsibleNewline(bool hasNewline) { m_hasCollapsibleNewline = hasNewline; }

    bool isTextItem() const { return m_type == Type::NormalText || m_type == Type::TabulationText || m_type == Type::LeaderText; }
    bool isBreakOpportunity() const { return m_type == Type::SoftBreakOpportunity || m_type == Type::HardBreakOpportunity; }

    const RefPtr<TextShape>& shapeText(const LineItemsData& data) const;

private:
    Box* m_box;
    uint32_t m_startOffset;
    uint32_t m_endOffset;
    Type m_type;
    CollapseType m_collapseType{CollapseType::NotCollapsible};
    UBiDiLevel m_bidiLevel{UBIDI_LTR};
    bool m_hasCollapsibleNewline{false};
    mutable RefPtr<TextShape> m_textShape;
};

using LineItems = std::pmr::vector<LineItem>;

struct LineItemsData {
    explicit LineItemsData(Heap* heap)
        : items(heap)
    {}

    LineItems items;
    UString text;
    bool isBidiEnabled{false};
    bool isBlockLevel{true};
};

class TextBox;
class InlineBox;
class BlockFlowBox;

class LineItemsBuilder {
public:
    explicit LineItemsBuilder(LineItemsData& data);

    void appendText(Box* box, const HeapString& data);
    void appendFloating(Box* box);
    void appendPositioned(Box* box);
    void appendReplaced(Box* box);

    void enterInline(Box* box);
    void exitInline(Box* box);

    void enterBlock(Box* box);
    void exitBlock(Box* box);

protected:
    void enterBidi(Box* box, UChar enter, UChar exit);
    void enterBidi(Box* box, Direction direction, UChar enterLtr, UChar enterRtl, UChar exit);
    void exitBidi(Box* box);

    LineItem& appendItem(LineItem::Type type, Box* box, uint32_t start, uint32_t end);
    LineItem& appendItem(LineItem::Type type, Box* box, UChar character);
    LineItem& appendOpaqueItem(LineItem::Type type, Box* box, UChar character);
    LineItem& appendOpaqueItem(LineItem::Type type, Box* box);
    LineItem& appendTextItem(LineItem::Type type, Box* box, const UString& text);

    void removeTrailingCollapsibleSpaceIfExists();
    void restoreTrailingCollapsibleSpaceIfRemoved();

    void removeTrailingCollapsibleSpace(int index);
    void restoreTrailingCollapsibleSpace(int index);

    bool shouldInsertBreakOpportunityAfterLeadingPreservedSpaces(Box* box, const UString& text, int start) const;
    int insertBreakOpportunityAfterLeadingPreservedSpaces(Box* box, const UString& text, int start);

    void appendHardBreak(Box* box);
    void appendHardBreakCollapseWhitespace(Box* box);

    void appendTextCollapseWhitespace(Box* box, const UString& text);
    void appendTextPreserveWhitespace(Box* box, const UString& text);
    void appendTextPreserveNewline(Box* box, const UString& text);
    void appendText(Box* box, const UString& text);

    struct BidiControl {
        Box* box;
        UChar enter;
        UChar exit;
    };

    LineItemsData& m_data;
    std::vector<BidiControl> m_bidiControls;
};

struct LineItemRun {
    LineItemRun(const LineItem& item, uint32_t index, uint32_t start, uint32_t end)
        : item(&item), itemIndex(index), startOffset(start), endOffset(end)
    {}

    const LineItem& operator*() const { return *item; }
    const LineItem* operator->() const { return item; }

    const LineItem* item;
    uint32_t itemIndex;
    uint32_t startOffset;
    uint32_t endOffset;
    bool canBreakAfter{false};
    bool mayBreakInside{false};
    bool hasOnlyTrailingSpaces{false};
    float expansion{0.f};
    float width{0.f};
    TextShapeView shape;
};

using LineItemRunList = std::vector<LineItemRun>;

class LineInfo {
public:
    LineInfo() = default;

    const LineItemRunList& runs() const { return m_runs; }
    LineItemRunList& runs() { return m_runs; }

    bool canBreakAfterLastRun() const { return !m_runs.empty() && m_runs.back().canBreakAfter; }

    bool endsWithBreak() const { return m_endsWithBreak; }
    void setEndsWithBreak(bool value) { m_endsWithBreak = value; }

    bool isEmptyLine() const { return m_isEmptyLine; }
    bool isFirstLine() const { return m_isFirstLine; }
    bool isLastLine() const { return m_isLastLine; }

    void setIsEmptyLine(bool value) { m_isEmptyLine = value; }
    void setIsFirstLine(bool value) { m_isFirstLine = value; }
    void setIsLastLine(bool value) { m_isLastLine = value; }

    float lineOffset() const { return m_lineOffset; }
    void setLineOffset(float offset) { m_lineOffset = offset; }

    void setLineStyle(const BoxStyle* style) { m_lineStyle = style; }
    const BoxStyle* lineStyle() const { return m_lineStyle; }

    void reset(const BoxStyle* style);

private:
    LineItemRunList m_runs;
    bool m_endsWithBreak{false};
    bool m_isEmptyLine{true};
    bool m_isFirstLine{true};
    bool m_isLastLine{false};
    float m_lineOffset{0.f};
    const BoxStyle* m_lineStyle{nullptr};
};

inline void LineInfo::reset(const BoxStyle* style)
{
    m_runs.clear();
    if(!m_isEmptyLine) {
        m_isEmptyLine = true;
        m_isFirstLine = false;
    }

    m_endsWithBreak = false;
    m_isLastLine = false;
    m_lineOffset = 0.f;
    m_lineStyle = style;
}

class BidiParagraph {
public:
    BidiParagraph();
    ~BidiParagraph();

    bool setParagraph(const UString& text, Direction direction);
    uint32_t getLogicalRun(uint32_t start, UBiDiLevel* level) const;

    static void reorderVisual(const std::vector<UBiDiLevel>& levels, std::vector<int32_t>& indices);

private:
    UBiDi* m_ubidi;
};

class FragmentBuilder;

class LineBreaker {
public:
    LineBreaker(BlockFlowBox* block, FragmentBuilder* fragmentainer, LineItemsData& data);
    ~LineBreaker();

    const LineInfo& nextLine();

    bool isDone() const { return m_itemIndex == m_data.items.size(); }

private:
    enum class LineBreakState {
        Continue,
        Trailing,
        Done
    };

    LineItemRun& addItemRun(const LineItem& item, uint32_t startOffset, uint32_t endOffset);
    LineItemRun& addItemRun(const LineItem& item) { return addItemRun(item, m_textOffset, item.endOffset()); }

    void moveToNextOf(const LineItem& item);
    void moveToNextOf(const LineItemRun& run);

    void setCurrentStyle(const BoxStyle* currentStyle);

    void handleNormalText(const LineItem& item);
    void handleTabulationText(const LineItem& item);
    void handleEmptyText(const LineItem& item);
    void handleLeaderText(const LineItem& item);
    void handleInlineStart(const LineItem& item);
    void handleInlineEnd(const LineItem& item);
    void handleFloating(const LineItem& item);
    void handlePositioned(const LineItem& item);
    void handleReplaced(const LineItem& item);
    void handleSoftBreak(const LineItem& item);
    void handleHardBreak(const LineItem& item);
    void handleBidiControl(const LineItem& item);

    void handleText(const LineItem& item, const RefPtr<TextShape>& shape);
    void breakText(LineItemRun& run, const LineItem& item, const RefPtr<TextShape>& shape, float availableWidth);
    void handleTrailingSpaces(const LineItem& item, const RefPtr<TextShape>& shape);

    void rewindOverflow(uint32_t newSize);
    void handleOverflow();

    bool canFitOnLine() const { return m_currentWidth <= m_availableWidth; }
    bool canFitOnLine(float extra) const { return extra + m_currentWidth <= m_availableWidth; }
    bool canBreakAfter(const LineItemRun& run) const { return m_autoWrap && m_breakIterator.isBreakable(run.endOffset); }

    static bool isBreakableSpace(UChar cc) { return cc == kSpaceCharacter || cc == kTabulationCharacter; }

    LineInfo m_line;
    BlockFlowBox* m_block;
    FragmentBuilder* m_fragmentainer;
    LineItemsData& m_data;
    LineBreakIterator m_breakIterator;
    float m_lineHeight;

    const BoxStyle* m_currentStyle{nullptr};
    LineBreakState m_state{LineBreakState::Continue};
    uint32_t m_itemIndex{0};
    uint32_t m_textOffset{0};
    uint32_t m_leadingFloatsEndIndex{0};
    float m_availableWidth{0};
    float m_currentWidth{0};
    bool m_autoWrap{false};
    bool m_skipLeadingWhitespace{true};
    bool m_hasUnpositionedFloats{false};
    bool m_hasLeaderText{false};
};

class LineBox;
class FlowLineBox;
class RootLineBox;

using RootLineBoxList = std::pmr::vector<std::unique_ptr<RootLineBox>>;

class LineBuilder {
public:
    LineBuilder(BlockFlowBox* block, FragmentBuilder* fragmentainer, RootLineBoxList& lines);

    void buildLine(const LineInfo& info);

private:
    void addLineBox(LineBox* childLine);
    void handleText(const LineItemRun& run);
    void handleInline(const LineItemRun& run);
    void handleReplaced(const LineItemRun& run);

    BlockFlowBox* m_block;
    FragmentBuilder* m_fragmentainer;
    RootLineBoxList& m_lines;
    FlowLineBox* m_parentLine{nullptr};
    uint32_t m_lineIndex{0};
};

class FragmentBuilder;
class PaintInfo;
class Point;
class Rect;

enum class PaintPhase;

class LineLayout : public HeapMember {
public:
    static std::unique_ptr<LineLayout> create(BlockFlowBox* block);

    BlockFlowBox* block() const { return m_block; }
    const RootLineBoxList& lines() const { return m_lines; }
    const LineItemsData& data() const { return m_data; }

    bool isBlockLevel() const { return m_data.isBlockLevel; }

    void updateWidth();
    void updateOverflowRect();
    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const;
    void layout(FragmentBuilder* fragmentainer);
    void build();

    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase);
    void serialize(std::ostream& o, int indent) const;

private:
    LineLayout(BlockFlowBox* block);
    BlockFlowBox* m_block;
    RootLineBoxList m_lines;
    LineItemsData m_data;
};

} // namespace plutobook

#endif // PLUTOBOOK_LINELAYOUT_H
