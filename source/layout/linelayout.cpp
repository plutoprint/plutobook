/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "linelayout.h"
#include "linebox.h"
#include "textbox.h"
#include "inlinebox.h"
#include "blockbox.h"
#include "document.h"

#include <ranges>

namespace plutobook {

const RefPtr<TextShape>& LineItem::shapeText(const LineItemsData& data) const
{
    assert(m_box && m_endOffset > m_startOffset && isTextItem());
    if(m_textShape == nullptr) {
        auto text = data.text.tempSubStringBetween(m_startOffset, m_endOffset);
        auto direction = m_bidiLevel & 1 ? Direction::Rtl : Direction::Ltr;
        if(m_type == Type::TabulationText) {
            m_textShape = TextShape::createForTabs(text, direction, m_box->style());
        } else {
            m_textShape = TextShape::createForText(text, direction, m_box->isSVGInlineTextBox(), m_box->style());
        }
    }

    return m_textShape;
}

LineItemsBuilder::LineItemsBuilder(LineItemsData& data)
    : m_data(data)
{
}

void LineItemsBuilder::appendText(Box* box, const HeapString& data)
{
    if(box->isWordBreakBox()) {
        appendOpaqueItem(LineItem::Type::SoftBreakOpportunity, box, kZeroWidthSpaceCharacter);
        return;
    }

    auto text = UString::fromUTF8(icu::StringPiece(data.data(), data.size()));
    switch(box->style()->textTransform()) {
    case TextTransform::None:
        appendText(box, text);
        break;
    case TextTransform::Capitalize:
        appendText(box, text.toTitle(nullptr));
        break;
    case TextTransform::Uppercase:
        appendText(box, text.toUpper());
        break;
    case TextTransform::Lowercase:
        appendText(box, text.toLower());
        break;
    }
}

void LineItemsBuilder::appendFloating(Box* box)
{
    appendOpaqueItem(LineItem::Type::Floating, box, kObjectReplacementCharacter);
}

void LineItemsBuilder::appendPositioned(Box* box)
{
    appendOpaqueItem(LineItem::Type::Positioned, box, kObjectReplacementCharacter);
}

void LineItemsBuilder::appendReplaced(Box* box)
{
    restoreTrailingCollapsibleSpaceIfRemoved();
    appendItem(LineItem::Type::Replaced, box, kObjectReplacementCharacter);
}

void LineItemsBuilder::enterInline(Box* box)
{
    auto direction = box->style()->direction();
    switch(box->style()->unicodeBidi()) {
    case UnicodeBidi::Normal:
        break;
    case UnicodeBidi::Embed:
        enterBidi(box, direction, kLeftToRightEmbedCharacter, kRightToLeftEmbedCharacter, kPopDirectionalFormattingCharacter);
        break;
    case UnicodeBidi::BidiOverride:
        enterBidi(box, direction, kLeftToRightOverrideCharacter, kRightToLeftOverrideCharacter, kPopDirectionalFormattingCharacter);
        break;
    case UnicodeBidi::Isolate:
        enterBidi(box, direction, kLeftToRightIsolateCharacter, kRightToLeftIsolateCharacter, kPopDirectionalIsolateCharacter);
        break;
    case UnicodeBidi::IsolateOverride:
        enterBidi(box, kFirstStrongIsolateCharacter, kPopDirectionalIsolateCharacter);
        enterBidi(box, direction, kLeftToRightOverrideCharacter, kRightToLeftOverrideCharacter, kPopDirectionalFormattingCharacter);
        break;
    }

    appendOpaqueItem(LineItem::Type::InlineStart, box);
}

void LineItemsBuilder::exitInline(Box* box)
{
    appendOpaqueItem(LineItem::Type::InlineEnd, box);
    exitBidi(box);
}

void LineItemsBuilder::enterBlock(Box* box)
{
    auto direction = box->style()->direction();
    switch(box->style()->unicodeBidi()) {
    case UnicodeBidi::BidiOverride:
    case UnicodeBidi::IsolateOverride:
        enterBidi(nullptr, direction, kLeftToRightOverrideCharacter, kRightToLeftOverrideCharacter, kPopDirectionalFormattingCharacter);
        break;
    default:
        break;
    }
}

void LineItemsBuilder::exitBlock(Box* box)
{
    exitBidi(nullptr);
    removeTrailingCollapsibleSpaceIfExists();

    thread_local BidiParagraph bidi;
    if(!bidi.setParagraph(m_data.text, box->style()->direction())) {
        m_data.isBidiEnabled = false;
        return;
    }

    size_t itemIndex = 0;
    for(size_t startOffset = 0; startOffset < m_data.text.length(); ++itemIndex) {
        UBiDiLevel bidiLevel;
        auto endOffset = bidi.getLogicalRun(startOffset, &bidiLevel);

        assert(startOffset == m_data.items[itemIndex].startOffset());
        for(; m_data.items[itemIndex].endOffset() < endOffset; ++itemIndex) {
            m_data.items[itemIndex].setBidiLevel(bidiLevel);
        }

        auto& item = m_data.items[itemIndex];
        item.setBidiLevel(bidiLevel);
        if(endOffset == item.endOffset()) {
            while(itemIndex + 1 < m_data.items.size()
                && m_data.items[itemIndex + 1].type() == LineItem::Type::InlineEnd) {
                m_data.items[++itemIndex].setBidiLevel(bidiLevel);
            }
        } else {
            assert(endOffset > item.startOffset());
            assert(endOffset < item.endOffset());
            m_data.items.insert(m_data.items.begin() + itemIndex + 1, item);
            m_data.items[itemIndex].setEndOffset(endOffset);
            m_data.items[itemIndex + 1].setStartOffset(endOffset);
        }

        startOffset = endOffset;
    }

    m_data.isBidiEnabled = true;
}

void LineItemsBuilder::enterBidi(Box* box, UChar enter, UChar exit)
{
    appendOpaqueItem(LineItem::Type::BidiControl, nullptr, enter);
    m_bidiControls.push_back({box, enter, exit});
}

void LineItemsBuilder::enterBidi(Box* box, Direction direction, UChar enterLtr, UChar enterRtl, UChar exit)
{
    enterBidi(box, direction == Direction::Ltr ? enterLtr : enterRtl, exit);
}

void LineItemsBuilder::exitBidi(Box* box)
{
    while(!m_bidiControls.empty()) {
        const auto& bidi = m_bidiControls.back();
        if(box != bidi.box)
            break;
        appendOpaqueItem(LineItem::Type::BidiControl, nullptr, bidi.exit);
        m_bidiControls.pop_back();
    }
}

LineItem& LineItemsBuilder::appendItem(LineItem::Type type, Box* box, uint32_t start, uint32_t end)
{
    m_data.items.emplace_back(type, box, start, end);
    return m_data.items.back();
}

LineItem& LineItemsBuilder::appendItem(LineItem::Type type, Box* box, UChar character)
{
    auto offset = m_data.text.length();
    m_data.text.append(character);
    return appendItem(type, box, offset, offset + 1);
}

LineItem& LineItemsBuilder::appendOpaqueItem(LineItem::Type type, Box* box, UChar character)
{
    auto& item = appendItem(type, box, character);
    item.setCollapseType(LineItem::CollapseType::OpaqueToCollapsing);
    return item;
}

LineItem& LineItemsBuilder::appendOpaqueItem(LineItem::Type type, Box* box)
{
    auto offset = m_data.text.length();
    auto& item = appendItem(type, box, offset, offset);
    item.setCollapseType(LineItem::CollapseType::OpaqueToCollapsing);
    return item;
}

LineItem& LineItemsBuilder::appendTextItem(LineItem::Type type, Box* box, const UString& text)
{
    auto offset = m_data.text.length();
    m_data.text.append(text);
    return appendItem(type, box, offset, offset + text.length());
}

void LineItemsBuilder::removeTrailingCollapsibleSpaceIfExists()
{
    for(int index = m_data.items.size() - 1; index >= 0; --index) {
        const auto& item = m_data.items[index];
        if(item.collapseType() == LineItem::CollapseType::OpaqueToCollapsing)
            continue;
        if(item.collapseType() == LineItem::CollapseType::Collapsible) {
            removeTrailingCollapsibleSpace(index);
        }

        break;
    }
}

void LineItemsBuilder::restoreTrailingCollapsibleSpaceIfRemoved()
{
    for(int index = m_data.items.size() - 1; index >= 0; --index) {
        const auto& item = m_data.items[index];
        if(item.collapseType() == LineItem::CollapseType::OpaqueToCollapsing)
            continue;
        if(item.collapseType() == LineItem::CollapseType::Collapsed) {
            restoreTrailingCollapsibleSpace(index);
        }

        break;
    }
}

void LineItemsBuilder::removeTrailingCollapsibleSpace(int index)
{
    auto& item = m_data.items.at(index);
    assert(item.collapseType() == LineItem::CollapseType::Collapsible);
    if(item.type() == LineItem::Type::HardBreakOpportunity)
        return;
    auto offset = item.endOffset() - 1;
    assert(m_data.text[offset] == kSpaceCharacter);
    m_data.text.remove(offset, 1);
    item.setEndOffset(offset);
    item.setCollapseType(LineItem::CollapseType::Collapsed);
    for(++index; index < m_data.items.size(); ++index) {
        auto& item = m_data.items[index];
        item.setStartOffset(item.startOffset() - 1);
        item.setEndOffset(item.endOffset() - 1);
    }
}

void LineItemsBuilder::restoreTrailingCollapsibleSpace(int index)
{
    auto& item = m_data.items.at(index);
    assert(item.collapseType() == LineItem::CollapseType::Collapsed);
    auto offset = item.endOffset();
    m_data.text.insert(offset, kSpaceCharacter);
    item.setEndOffset(offset + 1);
    item.setCollapseType(LineItem::CollapseType::Collapsible);
    for(++index; index < m_data.items.size(); ++index) {
        auto& item = m_data.items[index];
        item.setStartOffset(item.startOffset() + 1);
        item.setEndOffset(item.endOffset() + 1);
    }
}

bool LineItemsBuilder::shouldInsertBreakOpportunityAfterLeadingPreservedSpaces(Box* box, const UString& text, int start) const
{
    auto style = box->style();
    if(style->collapseWhiteSpace() || !style->autoWrap() || start >= text.length() || text[start] != kSpaceCharacter) {
        return false;
    }

    if(start == 0)
        return m_data.text.isEmpty() || m_data.text[m_data.text.length() - 1] == kNewlineCharacter;
    return text[start - 1] == kNewlineCharacter;
}

int LineItemsBuilder::insertBreakOpportunityAfterLeadingPreservedSpaces(Box* box, const UString& text, int start)
{
    if(shouldInsertBreakOpportunityAfterLeadingPreservedSpaces(box, text, start)) {
        auto end = start;
        do {
            ++end;
        } while(end < text.length() && text[end] == kSpaceCharacter);
        appendTextItem(LineItem::Type::NormalText, box, text.tempSubStringBetween(start, end));
        appendOpaqueItem(LineItem::Type::SoftBreakOpportunity, box, kZeroWidthSpaceCharacter);
        return end;
    }

    return start;
}

void LineItemsBuilder::appendHardBreak(Box* box)
{
    for(const auto& bidi : m_bidiControls | std::views::reverse) {
        appendOpaqueItem(LineItem::Type::BidiControl, box, bidi.exit);
    }

    auto& item = appendItem(LineItem::Type::HardBreakOpportunity, box, kNewlineCharacter);
    item.setCollapseType(LineItem::CollapseType::Collapsible);
    for(const auto& bidi : m_bidiControls) {
        appendOpaqueItem(LineItem::Type::BidiControl, box, bidi.enter);
    }
}

void LineItemsBuilder::appendHardBreakCollapseWhitespace(Box* box)
{
    removeTrailingCollapsibleSpaceIfExists();
    appendHardBreak(box);
}

constexpr bool isCollapsibleSpaceCharacter(UChar cc)
{
    return cc == kSpaceCharacter || cc == kNewlineCharacter || cc == kTabulationCharacter || cc == kCarriageReturnCharacter;
}

inline bool shouldRemoveNewline(const UString& text, int index, UChar cc)
{
    assert(index == text.length() || text[index] == kSpaceCharacter);
    if(index > 0 && text[index - 1] == kZeroWidthSpaceCharacter)
        return true;
    return cc == kZeroWidthSpaceCharacter;
}

void LineItemsBuilder::appendTextCollapseWhitespace(Box* box, const UString& text)
{
    if(box->isLineBreakBox()) {
        appendHardBreakCollapseWhitespace(box);
        return;
    }

    auto collapseType = LineItem::CollapseType::NotCollapsible;
    auto insertSpace = false;
    auto hasNewline = false;
    auto index = 0;

    auto cc = text.charAt(index);
    if(isCollapsibleSpaceCharacter(cc)) {
        if(cc == kNewlineCharacter)
            hasNewline = true;
        for(++index; index < text.length(); ++index) {
            cc = text.charAt(index);
            if(cc == kNewlineCharacter)
                hasNewline = true;
            if(!isCollapsibleSpaceCharacter(cc)) {
                break;
            }
        }

        if(index == text.length())
            collapseType = LineItem::CollapseType::Collapsible;
        for(int itemIndex = m_data.items.size() - 1; itemIndex >= 0; --itemIndex) {
            const auto& item = m_data.items[itemIndex];
            if(item.collapseType() == LineItem::CollapseType::OpaqueToCollapsing)
                continue;
            if(item.collapseType() == LineItem::CollapseType::NotCollapsible) {
                insertSpace = true;
                break;
            }

            assert(item.collapseType() == LineItem::CollapseType::Collapsible);
            if(item.type() == LineItem::Type::NormalText && (hasNewline || item.hasCollapsibleNewline())
                && shouldRemoveNewline(m_data.text, item.endOffset() - 1, text.charAt(index))) {
                removeTrailingCollapsibleSpace(itemIndex);
                hasNewline = false;
            } else {
                auto itemStyle = item.box()->style();
                if(box->style()->autoWrap() && !itemStyle->autoWrap() && !item.isBreakOpportunity()) {
                    appendOpaqueItem(LineItem::Type::SoftBreakOpportunity, box, kZeroWidthSpaceCharacter);
                }
            }

            break;
        }
    } else {
        for(int itemIndex = m_data.items.size() - 1; itemIndex >= 0; --itemIndex) {
            const auto& item = m_data.items[itemIndex];
            if(item.collapseType() == LineItem::CollapseType::OpaqueToCollapsing)
                continue;
            if(item.collapseType() == LineItem::CollapseType::Collapsible && item.hasCollapsibleNewline()
                && shouldRemoveNewline(m_data.text, item.endOffset() - 1, text.charAt(index))) {
                removeTrailingCollapsibleSpace(itemIndex);
            }

            break;
        }
    }

    auto startOffset = m_data.text.length();
    if(hasNewline && shouldRemoveNewline(m_data.text, m_data.text.length(), text.charAt(index)))
        insertSpace = hasNewline = false;
    if(insertSpace)
        m_data.text.append(kSpaceCharacter);
    while(index < text.length()) {
        assert(!isCollapsibleSpaceCharacter(text.charAt(index)));
        auto start = index;
        for(++index; index < text.length(); ++index) {
            cc = text.charAt(index);
            if(isCollapsibleSpaceCharacter(cc)) {
                break;
            }
        }

        m_data.text += text.tempSubStringBetween(start, index);
        if(index == text.length()) {
            collapseType = LineItem::CollapseType::NotCollapsible;
            break;
        }

        assert(isCollapsibleSpaceCharacter(cc));
        hasNewline = cc == kNewlineCharacter;
        for(++index; index < text.length(); ++index) {
            cc = text.charAt(index);
            if(cc == kNewlineCharacter)
                hasNewline = true;
            if(!isCollapsibleSpaceCharacter(cc)) {
                break;
            }
        }

        if(hasNewline && shouldRemoveNewline(m_data.text, m_data.text.length(), text.charAt(index))) {
            collapseType = LineItem::CollapseType::NotCollapsible;
            hasNewline = false;
        } else {
            collapseType = LineItem::CollapseType::Collapsible;
            m_data.text.append(kSpaceCharacter);
        }
    }

    if(startOffset == m_data.text.length()) {
        appendOpaqueItem(LineItem::Type::NormalText, box);
        return;
    }

    auto& item = appendItem(LineItem::Type::NormalText, box, startOffset, m_data.text.length());
    item.setCollapseType(collapseType);
    item.setHasCollapsibleNewline(hasNewline);
}

void LineItemsBuilder::appendTextPreserveWhitespace(Box* box, const UString& text)
{
    if(box->isSVGInlineTextBox()) {
        auto startOffset = m_data.text.length();
        for(int index = 0; index < text.length(); ++index) {
            auto cc = text.charAt(index);
            if(cc == kTabulationCharacter || cc == kNewlineCharacter || cc == kCarriageReturnCharacter)
                cc = kSpaceCharacter;
            m_data.text.append(cc);
        }

        appendItem(LineItem::Type::NormalText, box, startOffset, m_data.text.length());
        return;
    }

    auto start = insertBreakOpportunityAfterLeadingPreservedSpaces(box, text, 0);
    while(start < text.length()) {
        auto cc = text.charAt(start);
        if(cc == kNewlineCharacter) {
            appendHardBreak(box);
            start = insertBreakOpportunityAfterLeadingPreservedSpaces(box, text, start + 1);
        } else if(cc == kCarriageReturnCharacter || cc == kFormFeedCharacter) {
            appendItem(LineItem::Type::SoftBreakOpportunity, box, cc);
            start++;
        } else if(cc == kTabulationCharacter) {
            auto end = start;
            do {
                ++end;
            } while(end < text.length() && text[end] == kTabulationCharacter);
            appendTextItem(LineItem::Type::TabulationText, box, text.tempSubStringBetween(start, end));
            start = end;
        } else {
            auto end = start + 1;
            while(end < text.length()) {
                cc = text.charAt(end);
                if(cc == kNewlineCharacter || cc == kTabulationCharacter
                    || cc == kCarriageReturnCharacter || cc == kFormFeedCharacter
                    || cc == kZeroWidthNonJoinerCharacter) {
                    break;
                }

                ++end;
            }

            appendTextItem(LineItem::Type::NormalText, box, text.tempSubStringBetween(start, end));
            start = end;
        }
    }
}

void LineItemsBuilder::appendTextPreserveNewline(Box* box, const UString& text)
{
    if(box->isSVGInlineTextBox()) {
        appendTextCollapseWhitespace(box, text);
        return;
    }

    int start = 0;
    while(start < text.length()) {
        if(text[start] == kNewlineCharacter) {
            appendHardBreakCollapseWhitespace(box);
            start++;
        } else {
            auto end = text.indexOf(kNewlineCharacter, start + 1);
            if(end == -1)
                end = text.length();
            appendTextCollapseWhitespace(box, text.tempSubStringBetween(start, end));
            start = end;
        }
    }
}

void LineItemsBuilder::appendText(Box* box, const UString& text)
{
    if(text.isEmpty()) {
        appendOpaqueItem(LineItem::Type::NormalText, box);
        return;
    }

    restoreTrailingCollapsibleSpaceIfRemoved();
    if(box->isLeaderBox()) {
        appendTextItem(LineItem::Type::LeaderText, box, text);
        return;
    }

    auto style = box->style();
    if(!style->collapseWhiteSpace()) {
        appendTextPreserveWhitespace(box, text);
    } else if(style->preserveNewline()) {
        appendTextPreserveNewline(box, text);
    } else {
        appendTextCollapseWhitespace(box, text);
    }
}

BidiParagraph::BidiParagraph()
    : m_ubidi(ubidi_open())
{
}

BidiParagraph::~BidiParagraph()
{
    ubidi_close(m_ubidi);
}

bool BidiParagraph::setParagraph(const UString& text, Direction direction)
{
    UErrorCode errorCode = U_ZERO_ERROR;
    UBiDiLevel paragraphLevel = direction == Direction::Ltr ? UBIDI_LTR : UBIDI_RTL;
    ubidi_setPara(m_ubidi, text.getBuffer(), text.length(), paragraphLevel, nullptr, &errorCode);
    assert(U_SUCCESS(errorCode));
    return direction == Direction::Rtl || ubidi_getDirection(m_ubidi) == UBIDI_MIXED;
}

uint32_t BidiParagraph::getLogicalRun(uint32_t start, UBiDiLevel* level) const
{
    int32_t end;
    ubidi_getLogicalRun(m_ubidi, start, &end, level);
    return end;
}

void BidiParagraph::reorderVisual(const std::vector<UBiDiLevel>& levels, std::vector<int32_t>& indices)
{
    assert(levels.size() == indices.size());
    ubidi_reorderVisual(levels.data(), levels.size(), indices.data());
}

LineBreaker::LineBreaker(BlockFlowBox* block, FragmentBuilder* fragmentainer, LineItemsData& data)
    : m_block(block), m_fragmentainer(fragmentainer)
    , m_data(data), m_breakIterator(data.text)
    , m_lineHeight(block->style()->lineHeight())
{
    setCurrentStyle(m_block->style());
}

LineBreaker::~LineBreaker()
{
    if(m_hasUnpositionedFloats)
        m_block->positionNewFloats(m_fragmentainer);
    m_block->setHeight(m_block->height() + m_block->borderAndPaddingBottom());
}

const LineInfo& LineBreaker::nextLine()
{
    m_line.reset(m_currentStyle);
    m_state = LineBreakState::Continue;
    m_skipLeadingWhitespace = true;
    m_hasLeaderText = false;
    m_leadingFloatsEndIndex = m_itemIndex;
    m_currentWidth = 0.f;

    for(; m_leadingFloatsEndIndex < m_data.items.size(); ++m_leadingFloatsEndIndex) {
        const auto& item = m_data.items[m_leadingFloatsEndIndex];
        if(item.type() == LineItem::Type::NormalText && !item.length())
            continue;
        if(item.type() != LineItem::Type::Floating)
            break;
        auto box = to<BoxFrame>(item.box());
        m_block->insertFloatingBox(box);
        m_hasUnpositionedFloats = true;
    }

    if(m_hasUnpositionedFloats) {
        m_block->positionNewFloats(m_fragmentainer);
        m_hasUnpositionedFloats = false;
    }

    m_availableWidth = m_block->availableWidthForLine(m_block->height(), m_lineHeight, m_line.isFirstLine());
    while(m_state != LineBreakState::Done) {
        if(m_state == LineBreakState::Continue && m_autoWrap && !canFitOnLine())
            handleOverflow();
        if(m_itemIndex == m_data.items.size()) {
            m_line.setIsLastLine(true);
            break;
        }

        const auto& item = m_data.items[m_itemIndex];
        if(item.type() == LineItem::Type::NormalText) {
            handleNormalText(item);
            continue;
        }

        if(item.type() == LineItem::Type::TabulationText) {
            handleTabulationText(item);
            continue;
        }

        if(item.type() == LineItem::Type::InlineStart) {
            handleInlineStart(item);
            continue;
        }

        if(item.type() == LineItem::Type::InlineEnd) {
            handleInlineEnd(item);
            continue;
        }

        if(item.type() == LineItem::Type::Floating) {
            handleFloating(item);
            continue;
        }

        if(item.type() == LineItem::Type::BidiControl) {
            handleBidiControl(item);
            continue;
        }

        if(item.type() == LineItem::Type::SoftBreakOpportunity) {
            handleSoftBreak(item);
            continue;
        }

        if(item.type() == LineItem::Type::HardBreakOpportunity) {
            handleHardBreak(item);
            continue;
        }

        if(m_state == LineBreakState::Trailing) {
            assert(!m_line.isLastLine());
            break;
        }

        if(item.type() == LineItem::Type::LeaderText) {
            handleLeaderText(item);
        } else if(item.type() == LineItem::Type::Replaced) {
            handleReplaced(item);
        } else if(item.type() == LineItem::Type::Positioned) {
            handlePositioned(item);
        } else {
            assert(false);
        }
    }

    auto startOffset = m_block->leftOffsetForLine(m_block->height(), m_lineHeight, m_line.isFirstLine());
    if(!m_line.endsWithBreak()) {
        const auto& runs = m_line.runs();
        auto index = runs.size();
        while(index > 0) {
            const auto& run = runs[--index];
            if(run->type() != LineItem::Type::InlineStart) {
                auto nextIndex = index + 1;
                if(nextIndex == runs.size())
                    break;
                const auto& nextRun = runs[nextIndex];
                auto nextTextOffset = nextRun.startOffset;
                auto nextItemIndex = nextRun.itemIndex;
                rewindOverflow(nextIndex);
                m_textOffset = nextTextOffset;
                m_itemIndex = nextItemIndex;
                break;
            }
        }
    }

    auto remainingWidth = m_availableWidth - m_currentWidth;
    if(m_hasLeaderText && remainingWidth > 0.f && !m_line.isEmptyLine()) {
        uint32_t leaderCount = 0;
        for(const auto& run : m_line.runs()) {
            if(run.hasOnlyTrailingSpaces)
                break;
            if(run->type() == LineItem::Type::LeaderText) {
                leaderCount += 1;
            }
        }

        if(leaderCount > 0) {
            for(auto& run : m_line.runs()) {
                if(run.hasOnlyTrailingSpaces)
                    break;
                if(run->type() == LineItem::Type::LeaderText) {
                    auto leaderWidth = remainingWidth / leaderCount;
                    run.width += leaderWidth;
                    remainingWidth -= leaderWidth;
                    leaderCount -= 1;
                }
            }
        }
    }

    auto blockStyle = m_block->style();
    if(blockStyle->textAlign() == TextAlign::Justify && remainingWidth > 0.f && !m_line.isLastLine()) {
        std::vector<uint32_t> expansionOpportunities;
        uint32_t expansionOpportunityCount = 0;
        for(const auto& run : m_line.runs()) {
            if(run.hasOnlyTrailingSpaces)
                break;
            if(run->type() == LineItem::Type::NormalText || run->type() == LineItem::Type::TabulationText) {
                auto expansionOpportunity = run.shape.expansionOpportunityCount();
                expansionOpportunities.push_back(expansionOpportunity);
                expansionOpportunityCount += expansionOpportunity;
            }
        }

        if(expansionOpportunityCount > 0) {
            uint32_t expansionOpportunityIndex = 0;
            for(auto& run : m_line.runs()) {
                if(run.hasOnlyTrailingSpaces)
                    break;
                if(run->type() == LineItem::Type::NormalText || run->type() == LineItem::Type::TabulationText) {
                    if(auto expansionOpportunity = expansionOpportunities.at(expansionOpportunityIndex++)) {
                        auto expansionOpportunityWidth = remainingWidth * expansionOpportunity / expansionOpportunityCount;
                        run.expansion = expansionOpportunityWidth / expansionOpportunity;
                        run.width += expansionOpportunityWidth;
                        remainingWidth -= expansionOpportunityWidth;
                        expansionOpportunityCount -= expansionOpportunity;
                    }
                }
            }
        }
    }

    m_line.setLineOffset(startOffset + m_block->lineOffsetForAlignment(remainingWidth));
    if(m_data.isBidiEnabled && !m_line.isEmptyLine()) {
        UBiDiLevel paragraphLevel = blockStyle->direction() == Direction::Ltr ? 0 : 1;
        auto& logicalRuns = m_line.runs();
        std::vector<UBiDiLevel> levels;
        levels.reserve(logicalRuns.size());
        for(const auto& run : logicalRuns) {
            if(run.hasOnlyTrailingSpaces) {
                levels.push_back(paragraphLevel);
            } else {
                levels.push_back(run->bidiLevel());
            }
        }

        std::vector<int32_t> indices(levels.size());
        BidiParagraph::reorderVisual(levels, indices);

        LineItemRunList visualRuns;
        visualRuns.reserve(indices.size());
        for(auto index : indices)
            visualRuns.push_back(std::move(logicalRuns[index]));
        assert(visualRuns.size() == logicalRuns.size());
        logicalRuns.swap(visualRuns);
    }

    if(!m_line.isEmptyLine())
        m_data.isBlockLevel = false;
    return m_line;
}

LineItemRun& LineBreaker::addItemRun(const LineItem& item, uint32_t startOffset, uint32_t endOffset)
{
    auto& runs = m_line.runs();
    runs.emplace_back(item, m_itemIndex, startOffset, endOffset);
    return runs.back();
}

void LineBreaker::moveToNextOf(const LineItem& item)
{
    m_textOffset = item.endOffset();
    m_itemIndex += 1;
}

void LineBreaker::moveToNextOf(const LineItemRun& run)
{
    m_textOffset = run.endOffset;
    m_itemIndex = run.itemIndex;
    if(m_textOffset == run->endOffset()) {
        m_itemIndex += 1;
    }
}

void LineBreaker::setCurrentStyle(const BoxStyle* currentStyle)
{
    m_autoWrap = currentStyle->autoWrap();
    m_currentStyle = currentStyle;
}

void LineBreaker::handleNormalText(const LineItem& item)
{
    if(item.length()) {
        handleText(item, item.shapeText(m_data));
    } else {
        handleEmptyText(item);
    }
}

void LineBreaker::handleTabulationText(const LineItem& item)
{
    handleText(item, item.shapeText(m_data));
}

void LineBreaker::handleEmptyText(const LineItem& item)
{
    moveToNextOf(item);
}

void LineBreaker::handleLeaderText(const LineItem& item)
{
    const auto& shape = item.shapeText(m_data);
    auto& run = addItemRun(item);
    run.shape = TextShapeView(shape);
    run.width = shape->width();
    m_hasLeaderText = true;
    m_skipLeadingWhitespace = false;
    m_currentWidth += shape->width();
    m_line.setIsEmptyLine(false);
    moveToNextOf(item);
}

void LineBreaker::handleInlineStart(const LineItem& item)
{
    auto& box = to<InlineBox>(*item.box());
    box.updateMarginWidths(m_block);
    box.updatePaddingWidths(m_block);

    auto& run = addItemRun(item);
    run.width += box.marginLeft();
    run.width += box.paddingLeft();
    run.width += box.borderLeft();
    if(run.width && m_line.isEmptyLine()) {
        m_line.setIsEmptyLine(false);
    }

    if(m_state == LineBreakState::Trailing
        && run.width < 0.f && m_currentWidth > m_availableWidth
        && m_currentWidth + run.width <= m_availableWidth) {
        m_state = LineBreakState::Continue;
    }

    auto wasAutoWrap = m_autoWrap;
    setCurrentStyle(box.style());
    moveToNextOf(item);
    m_currentWidth += run.width;

    auto& runs = m_line.runs();
    if(!wasAutoWrap && m_autoWrap && runs.size() >= 2) {
        auto& lastRun = runs[runs.size() - 2];
        lastRun.canBreakAfter = canBreakAfter(lastRun);
    }
}

void LineBreaker::handleInlineEnd(const LineItem& item)
{
    const auto& box = to<InlineBox>(*item.box());
    auto& run = addItemRun(item);
    run.width += box.marginRight();
    run.width += box.paddingRight();
    run.width += box.borderRight();
    if(run.width && m_line.isEmptyLine()) {
        m_line.setIsEmptyLine(false);
    }

    auto wasAutoWrap = m_autoWrap;
    setCurrentStyle(box.parentBox()->style());
    moveToNextOf(item);
    m_currentWidth += run.width;

    auto& runs = m_line.runs();
    if(runs.size() >= 2) {
        auto& lastRun = runs[runs.size() - 2];
        if(wasAutoWrap || lastRun.canBreakAfter) {
            run.canBreakAfter = lastRun.canBreakAfter;
            lastRun.canBreakAfter = false;
            return;
        }
    }

    if(!wasAutoWrap && m_autoWrap) {
        run.canBreakAfter = canBreakAfter(run);
    }
}

void LineBreaker::handleFloating(const LineItem& item)
{
    auto& run = addItemRun(item);
    run.canBreakAfter = m_autoWrap;
    moveToNextOf(item);
    if(m_itemIndex <= m_leadingFloatsEndIndex) {
        return;
    }

    auto box = to<BoxFrame>(item.box());
    assert(!m_block->containsFloat(box));
    if(m_hasUnpositionedFloats) {
        m_block->insertFloatingBox(box);
        return;
    }

    auto floatTop = m_block->height();
    if(m_block->containsFloats()) {
        for(const auto& floatingBox : *m_block->floatingBoxes()) {
            assert(floatingBox.isPlaced());
            floatTop = std::max(floatTop, floatingBox.y());
            if(box->style()->isClearLeft() && floatingBox.type() == Float::Left)
                floatTop = std::max(floatTop, floatingBox.bottom());
            if(box->style()->isClearRight() && floatingBox.type() == Float::Right) {
                floatTop = std::max(floatTop, floatingBox.bottom());
            }
        }
    }

    box->updatePaddingWidths(m_block);
    box->updateVerticalMargins(m_block);

    auto estimatedTop = floatTop + box->marginTop();
    if(m_fragmentainer)
        m_fragmentainer->enterFragment(estimatedTop);
    box->layout(m_fragmentainer);
    if(m_fragmentainer) {
        m_fragmentainer->leaveFragment(estimatedTop);
    }

    auto& floatingBox = m_block->insertFloatingBox(box);
    if(canFitOnLine(box->marginBoxWidth())) {
        m_block->positionFloatingBox(floatingBox, m_fragmentainer, floatTop);
        m_availableWidth = m_block->availableWidthForLine(m_block->height(), m_lineHeight, m_line.isFirstLine());
    } else {
        m_hasUnpositionedFloats = true;
    }
}

void LineBreaker::handlePositioned(const LineItem& item)
{
    auto& run = addItemRun(item);
    run.canBreakAfter = !m_line.isEmptyLine() && canBreakAfter(run);
    moveToNextOf(item);
}

void LineBreaker::handleReplaced(const LineItem& item)
{
    auto& box = to<BoxFrame>(*item.box());
    auto& run = addItemRun(item);
    moveToNextOf(item);

    box.updatePaddingWidths(m_block);
    if(box.isOutsideListMarkerBox()) {
        m_line.setIsEmptyLine(false);
        return;
    }

    box.layout(nullptr);

    run.canBreakAfter = canBreakAfter(run);
    run.width = box.marginBoxWidth();
    m_line.setIsEmptyLine(false);
    m_currentWidth += run.width;
    m_skipLeadingWhitespace = false;
}

void LineBreaker::handleSoftBreak(const LineItem& item)
{
    assert(item.length() == 1);
    auto cc = m_data.text.charAt(item.startOffset());
    if(cc == kZeroWidthSpaceCharacter) {
        auto& run = addItemRun(item);
        run.canBreakAfter = true;
        moveToNextOf(item);
        m_line.setIsEmptyLine(false);
    } else {
        assert(cc == kCarriageReturnCharacter || cc == kFormFeedCharacter);
        handleEmptyText(item);
    }
}

void LineBreaker::handleHardBreak(const LineItem& item)
{
    auto& run = addItemRun(item);
    run.canBreakAfter = true;
    run.hasOnlyTrailingSpaces = true;
    moveToNextOf(item);
    while(m_itemIndex < m_data.items.size()) {
        const auto& item = m_data.items[m_itemIndex];
        if(item.type() == LineItem::Type::NormalText && !item.length()) {
            handleEmptyText(item);
        } if(item.type() == LineItem::Type::InlineEnd) {
            handleInlineEnd(item);
        } else {
            break;
        }
    }

    m_line.setEndsWithBreak(true);
    m_line.setIsEmptyLine(false);
    m_line.setIsLastLine(true);
    m_state = LineBreakState::Done;
}

void LineBreaker::handleBidiControl(const LineItem& item)
{
    assert(item.length() == 1);
    auto cc = m_data.text.charAt(item.startOffset());
    if(cc == kPopDirectionalIsolateCharacter || cc == kPopDirectionalFormattingCharacter) {
        auto& run = addItemRun(item);
        moveToNextOf(item);
        auto& runs = m_line.runs();
        if(runs.size() >= 2) {
            auto& lastRun = runs[runs.size() - 2];
            if(lastRun.canBreakAfter) {
                run.canBreakAfter = lastRun.canBreakAfter;
                lastRun.canBreakAfter = false;
            } else {
                run.canBreakAfter = canBreakAfter(run);
            }
        }
    } else {
        if(m_state == LineBreakState::Trailing && m_line.canBreakAfterLastRun()) {
            assert(!m_line.isLastLine());
            m_state = LineBreakState::Done;
            moveToNextOf(item);
            return;
        }

        const auto& run = addItemRun(item);
        assert(!run.canBreakAfter);
        moveToNextOf(item);
    }
}

void LineBreaker::handleText(const LineItem& item, const RefPtr<TextShape>& shape)
{
    assert(item.type() == LineItem::Type::NormalText || item.type() == LineItem::Type::TabulationText);
    if(m_state == LineBreakState::Trailing) {
        handleTrailingSpaces(item, shape);
        return;
    }

    if(m_skipLeadingWhitespace && item.box()->style()->collapseWhiteSpace()) {
        if(m_data.text[m_textOffset] == kSpaceCharacter) {
            m_textOffset += 1;
            if(m_textOffset == item.endOffset()) {
                handleEmptyText(item);
                return;
            }
        }
    }

    auto& run = addItemRun(item);
    m_line.setIsEmptyLine(false);
    m_skipLeadingWhitespace = false;
    if(!m_autoWrap) {
        assert(run.endOffset == item.endOffset());
        if(run.startOffset == item.startOffset()) {
            run.shape = TextShapeView(shape);
            run.width = shape->width();
        } else {
            assert(run.startOffset > item.startOffset());
            run.shape = TextShapeView(shape, run.startOffset - item.startOffset(), run.endOffset - item.startOffset());
            run.width = run.shape.width();
        }

        assert(!run.mayBreakInside);
        assert(!run.canBreakAfter);
        moveToNextOf(item);
        m_currentWidth += run.width;
        return;
    }

    breakText(run, item, shape, m_availableWidth - m_currentWidth);
    moveToNextOf(run);
    m_currentWidth += run.width;
    if(!canFitOnLine()) {
        handleOverflow();
    } else if(run.endOffset < item.endOffset()) {
        handleTrailingSpaces(item, shape);
    }
}

constexpr float flipRtl(float value, Direction direction)
{
    return direction == Direction::Ltr ? value : -value;
}

void LineBreaker::breakText(LineItemRun& run, const LineItem& item, const RefPtr<TextShape>& shape, float availableWidth)
{
    assert(run.startOffset >= item.startOffset() && run.startOffset < item.endOffset());
    auto startPosition = shape->positionForOffset(run.startOffset - item.startOffset());
    auto endPosition = startPosition + flipRtl(availableWidth, shape->direction());

    auto style = item.box()->style();
    auto breakOffset = item.startOffset() + shape->offsetForPosition(endPosition);
    auto mayBreakInside = true;
    if(style->breakAnywhere()) {
        breakOffset = std::max(breakOffset, run.startOffset + 1);
    } else if(breakOffset < item.endOffset()) {
        auto breakOpportunity = m_breakIterator.previousBreakOpportunity(breakOffset, run.startOffset);
        if(breakOpportunity <= run.startOffset) {
            breakOffset = std::max(breakOffset, run.startOffset + 1);
            breakOpportunity = style->breakWord() ? breakOffset : m_breakIterator.nextBreakOpportunity(breakOffset, item.endOffset());
            mayBreakInside = false;
        }

        breakOffset = std::min(breakOpportunity, item.endOffset());
    }

    assert(breakOffset > run.startOffset);
    run.shape = TextShapeView(shape, run.startOffset - item.startOffset(), breakOffset - item.startOffset());
    run.width = run.shape.width();
    run.endOffset = breakOffset;
    run.mayBreakInside = mayBreakInside;
    if(breakOffset < item.endOffset()) {
        run.canBreakAfter = true;
    } else {
        assert(breakOffset == item.endOffset());
        run.canBreakAfter = m_breakIterator.isBreakable(item.endOffset());
    }
}

void LineBreaker::handleTrailingSpaces(const LineItem& item, const RefPtr<TextShape>& shape)
{
    assert(item.type() == LineItem::Type::NormalText || item.type() == LineItem::Type::TabulationText);
    assert(m_textOffset >= item.startOffset() && m_textOffset < item.endOffset());
    if(!m_autoWrap) {
        m_state = LineBreakState::Done;
        return;
    }

    if(item.box()->style()->collapseWhiteSpace()) {
        if(m_data.text[m_textOffset] != kSpaceCharacter) {
            m_state = LineBreakState::Done;
            return;
        }

        auto& runs = m_line.runs();
        assert(!runs.empty());
        auto& lastRun = runs.back();
        lastRun.canBreakAfter = true;
        m_textOffset += 1;
    } else {
        auto endOffset = m_textOffset;
        while(endOffset < item.endOffset() && isBreakableSpace(m_data.text[endOffset]))
            ++endOffset;
        if(m_textOffset == endOffset) {
            m_state = LineBreakState::Done;
            return;
        }

        auto& run = addItemRun(item, m_textOffset, endOffset);
        run.shape = TextShapeView(shape, m_textOffset - item.startOffset(), endOffset - item.startOffset());
        run.width = run.shape.width();
        run.canBreakAfter = endOffset < m_data.text.length() && !isBreakableSpace(m_data.text[endOffset]);
        run.hasOnlyTrailingSpaces = true;
        m_currentWidth += run.width;
        m_textOffset = endOffset;
    }

    if(m_textOffset < item.endOffset()) {
        m_state = LineBreakState::Done;
        return;
    }

    m_state = LineBreakState::Trailing;
    m_itemIndex += 1;
}

void LineBreaker::rewindOverflow(uint32_t newSize)
{
    auto& runs = m_line.runs();
    assert(newSize > 0 && newSize < runs.size());
    const auto& run = runs[newSize];
    if(run->type() == LineItem::Type::NormalText
        || run->type() == LineItem::Type::InlineEnd) {
        setCurrentStyle(run->box()->style());
    } else {
        auto index = newSize;
        while(true) {
            const auto& run = runs[--index];
            auto box = run->box();
            if(run->type() == LineItem::Type::NormalText
                || run->type() == LineItem::Type::InlineStart) {
                setCurrentStyle(box->style());
                break;
            }

            if(run->type() == LineItem::Type::InlineEnd) {
                setCurrentStyle(box->parentBox()->style());
                break;
            }

            if(index == 0) {
                setCurrentStyle(m_line.lineStyle());
                break;
            }
        }
    }

    while(newSize < runs.size())
        runs.pop_back();
    moveToNextOf(runs.back());
    m_currentWidth = 0.f;
    for(const auto& run : runs) {
        m_currentWidth += run.width;
    }
}

void LineBreaker::handleOverflow()
{
    auto& runs = m_line.runs();
    auto widthToRewind = m_currentWidth - m_availableWidth;
    auto breakBefore = 0u;
    auto index = runs.size();
    while(index > 0) {
        auto& run = runs[--index];
        if(run.canBreakAfter && index < runs.size() - 1) {
            if(widthToRewind <= 0.f) {
                m_state = LineBreakState::Trailing;
                rewindOverflow(index + 1);
                return;
            }

            breakBefore = index + 1;
        }

        widthToRewind -= run.width;
        if(run->type() == LineItem::Type::NormalText && widthToRewind < 0.f && run.mayBreakInside) {
            const auto& shape = run->shapeText(m_data);
            auto itemAvailableWidth = std::min(-widthToRewind, run.width - 1);
            breakText(run, *run, shape, itemAvailableWidth);
            if(run.width <= itemAvailableWidth) {
                assert(run.canBreakAfter && run.endOffset < run->endOffset());
                auto itemEndIndex = index + 1;
                assert(itemEndIndex <= runs.size());
                if(itemEndIndex < runs.size()) {
                    m_state = LineBreakState::Trailing;
                    rewindOverflow(itemEndIndex);
                    return;
                }

                m_currentWidth = m_availableWidth + widthToRewind + run.width;
                m_textOffset = run.endOffset;
                m_itemIndex = run.itemIndex;
                handleTrailingSpaces(*run, shape);
                return;
            }
        }
    }

    if(m_block->containsFloats()) {
        if(m_hasUnpositionedFloats) {
            m_block->positionNewFloats(m_fragmentainer);
            m_hasUnpositionedFloats = false;
        }

        float newLineWidth = m_availableWidth;
        float lastFloatBottom = m_block->height();
        float floatBottom = 0.f;
        while(true) {
            floatBottom = m_block->nextFloatBottom(lastFloatBottom);
            if(floatBottom == 0.f)
                break;
            newLineWidth = m_block->availableWidthForLine(floatBottom, m_lineHeight, m_line.isFirstLine());
            lastFloatBottom = floatBottom;
            if(newLineWidth >= m_currentWidth) {
                break;
            }
        }

        if(newLineWidth > m_availableWidth) {
            m_block->setHeight(lastFloatBottom);
            m_availableWidth = newLineWidth;
            return;
        }
    }

    m_state = LineBreakState::Trailing;
    if(breakBefore > 0) {
        rewindOverflow(breakBefore);
    }
}

LineBuilder::LineBuilder(BlockFlowBox* block, FragmentBuilder* fragmentainer, RootLineBoxList& lines)
    : m_block(block), m_fragmentainer(fragmentainer), m_lines(lines)
{
}

void LineBuilder::buildLine(const LineInfo& info)
{
    if(m_parentLine) {
        m_parentLine = nullptr;
        m_lineIndex += 1;
    }

    for(const auto& run : info.runs()) {
        switch(run->type()) {
        case LineItem::Type::NormalText:
        case LineItem::Type::TabulationText:
        case LineItem::Type::LeaderText:
        case LineItem::Type::SoftBreakOpportunity:
        case LineItem::Type::HardBreakOpportunity:
            handleText(run);
            break;
        case LineItem::Type::InlineStart:
        case LineItem::Type::InlineEnd:
            handleInline(run);
            break;
        case LineItem::Type::Replaced:
        case LineItem::Type::Positioned:
            handleReplaced(run);
            break;
        default:
            break;
        }
    }

    if(!info.isEmptyLine()) {
        for(const auto& run : info.runs()) {
            if(run->type() == LineItem::Type::InlineStart
                || run->type() == LineItem::Type::InlineEnd) {
                const auto& box = to<InlineBox>(*run->box());
                const auto& lines = box.lines();
                if(run->type() == LineItem::Type::InlineStart) {
                    const auto& firstLine = lines.front();
                    if(box.style()->direction() == Direction::Ltr) {
                        firstLine->setHasLeftEdge(true);
                    } else {
                        firstLine->setHasRightEdge(true);
                    }
                } else {
                    const auto& lastLine = lines.back();
                    if(box.style()->direction() == Direction::Ltr) {
                        lastLine->setHasRightEdge(true);
                    } else {
                        lastLine->setHasLeftEdge(true);
                    }
                }
            }
        }
    }

    if(m_lines.empty())
        return;
    const auto& rootLine = m_lines.back();
    if(m_lineIndex != rootLine->lineIndex())
        return;
    rootLine->setIsEmptyLine(info.isEmptyLine());
    rootLine->setIsFirstLine(info.isFirstLine());
    rootLine->alignInHorizontalDirection(info.lineOffset());
    auto blockHeight = rootLine->alignInVerticalDirection(m_fragmentainer, m_block->height());
    if(!rootLine->isEmptyLine()) {
        m_block->setHeight(blockHeight);
    }
}

static bool needsLineBox(const InlineBox* box, size_t lineIndex)
{
    const auto& lines = box->lines();
    if(!lines.empty()) {
        const auto& lastLine = lines.back();
        if(lineIndex == lastLine->lineIndex()) {
            const auto& children = lastLine->parentLine()->children();
            return children.back() != lastLine.get();
        }
    }

    return true;
}

void LineBuilder::addLineBox(LineBox* childLine)
{
    auto parentBox = childLine->box()->parentBox();
    if(m_parentLine && parentBox == m_parentLine->box()) {
        m_parentLine->addChild(childLine);
        return;
    }

    m_parentLine = nullptr;
    while(true) {
        if(m_block == parentBox) {
            if(!m_lines.empty()) {
                const auto& line = m_lines.back();
                if(m_lineIndex == line->lineIndex()) {
                    if(!m_parentLine)
                        m_parentLine = line.get();
                    line->addChild(childLine);
                    break;
                }
            }

            auto line = RootLineBox::create(m_block);
            line->setLineIndex(m_lineIndex);
            if(!m_parentLine)
                m_parentLine = line.get();
            line->addChild(childLine);
            m_lines.push_back(std::move(line));
            break;
        }

        assert(parentBox->isInlineBox());
        auto box = to<InlineBox>(parentBox);
        auto& lines = box->lines();
        if(!needsLineBox(box, m_lineIndex)) {
            const auto& line = lines.back();
            if(!m_parentLine)
                m_parentLine = line.get();
            line->addChild(childLine);
            break;
        }

        auto line = FlowLineBox::create(box);
        line->setLineIndex(m_lineIndex);
        if(!m_parentLine)
            m_parentLine = line.get();
        line->addChild(childLine);
        childLine = line.get();
        lines.push_back(std::move(line));
        parentBox = parentBox->parentBox();
    }
}

void LineBuilder::handleText(const LineItemRun& run)
{
    auto box = to<TextBox>(run->box());
    auto line = TextLineBox::create(box, run.shape, run.width, run.expansion);
    addLineBox(line.get());
    box->lines().push_back(std::move(line));
}

void LineBuilder::handleInline(const LineItemRun& run)
{
    auto box = to<InlineBox>(run->box());
    if(!needsLineBox(box, m_lineIndex))
        return;
    auto line = FlowLineBox::create(box);
    addLineBox(line.get());
    box->lines().push_back(std::move(line));
}

void LineBuilder::handleReplaced(const LineItemRun& run)
{
    auto box = to<BoxFrame>(run->box());
    if(box->isPositioned())
        box->containingBlock()->insertPositonedBox(box);
    if(box->isOutsideListMarkerBox())
        box->layout(nullptr);
    auto line = ReplacedLineBox::create(box);
    addLineBox(line.get());
    box->setLine(std::move(line));
}

std::unique_ptr<LineLayout> LineLayout::create(BlockFlowBox* block)
{
    return std::unique_ptr<LineLayout>(new (block->heap()) LineLayout(block));
}

void LineLayout::updateWidth()
{
    auto blockWidth = m_block->width();
    m_block->updateWidth();
    if(m_block->containsFloats()
        || blockWidth != m_block->width()) {
        m_lines.clear();
    }
}

void LineLayout::updateOverflowRect()
{
    for(const auto& line : m_lines) {
        line->updateOverflowRect(line->lineTop(), line->lineBottom());
        m_block->addOverflowRect(line->visualOverflowRect());
    }
}

void LineLayout::computeIntrinsicWidths(float& minWidth, float& maxWidth) const
{
    LineBreakIterator breakIterator(m_data.text);
    auto currentStyle = m_block->style();
    auto indentLength = currentStyle->textIndent();
    auto indentWidth = indentLength.calcMin(0);
    auto floating = Float::None;

    float inlineMinWidth = 0.f;
    float inlineMaxWidth = 0.f;
    for(const auto& item : m_data.items) {
        if(item.type() == LineItem::Type::NormalText || item.type() == LineItem::Type::TabulationText || item.type() == LineItem::Type::LeaderText) {
            if(item.type() == LineItem::Type::NormalText && !item.length())
                continue;
            if(indentWidth && item.length()) {
                inlineMinWidth += indentWidth;
                inlineMaxWidth += indentWidth;
                indentWidth = 0.f;
            }

            const auto& shape = item.shapeText(m_data);
            if(currentStyle->autoWrap()) {
                if(item.type() == LineItem::Type::LeaderText) {
                    inlineMinWidth += shape->width();
                } else if(item.type() == LineItem::Type::TabulationText) {
                    minWidth = std::max(minWidth, inlineMinWidth);
                    inlineMinWidth = 0.f;
                } else {
                    auto startOffset = item.startOffset();
                    while(startOffset < item.endOffset()) {
                        auto endOffset = breakIterator.nextBreakOpportunity(startOffset, item.endOffset());
                        auto subShape = TextShapeView(shape, startOffset - item.startOffset(), endOffset - item.startOffset());
                        inlineMinWidth += subShape.width();
                        if(endOffset == item.endOffset())
                            break;
                        minWidth = std::max(minWidth, inlineMinWidth);
                        inlineMinWidth = 0.f;
                        startOffset = endOffset + 1;
                    }
                }

                inlineMaxWidth += shape->width();
            } else {
                inlineMinWidth += shape->width();
                inlineMaxWidth += shape->width();
            }
        } else if(item.type() == LineItem::Type::InlineStart || item.type() == LineItem::Type::InlineEnd) {
            auto& child = to<InlineBox>(*item.box());
            if(item.type() == LineItem::Type::InlineStart) {
                child.updateHorizontalMargins(nullptr);
                child.updateHorizontalPaddings(nullptr);
                inlineMinWidth += child.marginLeft() + child.paddingLeft() + child.borderLeft();
                inlineMaxWidth += child.marginLeft() + child.paddingLeft() + child.borderLeft();
                currentStyle = child.style();
            } else {
                inlineMinWidth += child.marginRight() + child.paddingRight() + child.borderRight();
                inlineMaxWidth += child.marginRight() + child.paddingRight() + child.borderRight();
                currentStyle = child.parentBox()->style();
            }
        } else if(item.type() == LineItem::Type::Floating || item.type() == LineItem::Type::Replaced) {
            auto& child = to<BoxFrame>(*item.box());
            if(item.type() == LineItem::Type::Floating) {
                auto childStyle = child.style();
                if((floating == Float::Left && childStyle->isClearLeft())
                    || (floating == Float::Right && childStyle->isClearRight())) {
                    minWidth = std::max(minWidth, inlineMinWidth);
                    maxWidth = std::max(maxWidth, inlineMaxWidth);

                    inlineMinWidth = 0.f;
                    inlineMaxWidth = 0.f;
                }

                floating = childStyle->floating();
            }

            if(currentStyle->autoWrap()) {
                minWidth = std::max(minWidth, inlineMinWidth);
                inlineMinWidth = 0.f;
            }

            child.updateHorizontalMargins(nullptr);
            child.updateHorizontalPaddings(nullptr);

            auto childMinWidth = child.minPreferredWidth() + child.marginWidth();
            auto childMaxWidth = child.maxPreferredWidth() + child.marginWidth();

            if(indentWidth && !child.isFloating()) {
                childMinWidth += indentWidth;
                childMaxWidth += indentWidth;
                indentWidth = 0.f;
            }

            inlineMaxWidth += childMaxWidth;
            if(currentStyle->autoWrap()) {
                minWidth = std::max(minWidth, childMinWidth);
            } else {
                if(child.isFloating()) {
                    minWidth = std::max(minWidth, childMinWidth);
                } else {
                    inlineMinWidth += childMinWidth;
                }
            }
        } else if(item.type() == LineItem::Type::HardBreakOpportunity) {
            minWidth = std::max(minWidth, inlineMinWidth);
            maxWidth = std::max(maxWidth, inlineMaxWidth);

            inlineMinWidth = 0.f;
            inlineMaxWidth = 0.f;
        } else if(item.type() == LineItem::Type::SoftBreakOpportunity) {
            assert(item.length() == 1);
            auto cc = m_data.text.charAt(item.startOffset());
            assert(cc == kZeroWidthSpaceCharacter || cc == kCarriageReturnCharacter || cc == kFormFeedCharacter);
            if(cc == kZeroWidthSpaceCharacter) {
                minWidth = std::max(minWidth, inlineMinWidth);
                inlineMinWidth = 0.f;
            }
        }
    }

    minWidth = std::max(minWidth, inlineMinWidth);
    maxWidth = std::max(maxWidth, inlineMaxWidth);
}

void LineLayout::layout(FragmentBuilder* fragmentainer)
{
    if(!m_lines.empty()) {
        for(const auto& line : m_lines) {
            auto blockHeight = line->alignInVerticalDirection(fragmentainer, m_block->height());
            if(!line->isEmptyLine()) {
                m_block->setHeight(blockHeight);
            }
        }

        m_block->setHeight(m_block->height() + m_block->borderAndPaddingBottom());
        return;
    }

    auto child = m_block->firstChild();
    while(child) {
        if(auto box = to<TextBox>(child)) {
            box->lines().clear();
        } else if(auto box = to<InlineBox>(child)) {
            box->lines().clear();
            if(child->firstChild()) {
                child = child->firstChild();
                continue;
            }
        } else if(auto box = to<BoxFrame>(child)) {
            box->setLine(nullptr);
        } else {
            assert(false);
        }

        while(true) {
            if(child->nextSibling()) {
                child = child->nextSibling();
                break;
            }

            child = child->parentBox();
            if(child == m_block) {
                child = nullptr;
                break;
            }
        }
    }

    LineBreaker breaker(m_block, fragmentainer, m_data);
    LineBuilder builder(m_block, fragmentainer, m_lines);
    while(!breaker.isDone()) {
        builder.buildLine(breaker.nextLine());
    }
}

void LineLayout::build()
{
    LineItemsBuilder builder(m_data);
    builder.enterBlock(m_block);
    auto child = m_block->firstChild();
    while(child) {
        if(auto box = to<TextBox>(child)) {
            builder.appendText(box, box->text());
        } else if(auto box = to<InlineBox>(child)) {
            builder.enterInline(box);
            if(child->firstChild()) {
                child = child->firstChild();
                continue;
            }

            builder.exitInline(box);
        } else if(child->isFloating()) {
            builder.appendFloating(child);
        } else if(child->isPositioned()) {
            builder.appendPositioned(child);
        } else if(child->isReplaced()) {
            builder.appendReplaced(child);
        } else {
            assert(false);
        }

        while(true) {
            if(child->nextSibling()) {
                child = child->nextSibling();
                break;
            }

            child = child->parentBox();
            if(child == m_block) {
                child = nullptr;
                break;
            }

            assert(child->isInlineBox());
            builder.exitInline(child);
        }
    }

    builder.exitBlock(m_block);
}

void LineLayout::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    if(phase == PaintPhase::Contents || phase == PaintPhase::Outlines) {
        for(const auto& line : m_lines) {
            line->paint(info, offset, phase);
        }
    }
}

void LineLayout::serialize(std::ostream& o, int indent) const
{
    for(const auto& line : m_lines) {
        line->serialize(o, indent);
    }
}

LineLayout::LineLayout(BlockFlowBox* block)
    : m_block(block)
    , m_lines(block->heap())
    , m_data(block->heap())
{
}

} // namespace plutobook
