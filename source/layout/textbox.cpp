/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "textbox.h"
#include "linebox.h"
#include "globalstring.h"
#include "blockbox.h"
#include "linelayout.h"
#include "localedata.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unicode/ubrk.h>

namespace plutobook {

TextBox::TextBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
    , m_lines(style->heap())
{
    setIsInline(true);
}

void TextBox::appendText(std::string_view text)
{
    m_text = heap()->concatenateString(m_text, text);
}

TextBox::~TextBox() = default;

WordBoxList TextBox::words() const
{
    WordBoxList result;
    const auto* block = to<BlockFlowBox>(containingBlock());
    if(m_lines.empty() || !block || !block->lineLayout() || style()->visibility() != Visibility::Visible)
        return result;

    // Use the same transformed/collapsed text as layout. Segmentation must span
    // line breaks and bidi items, rather than treating each visual line as text.
    const auto& data = block->lineLayout()->data();
    uint32_t first = std::numeric_limits<uint32_t>::max();
    uint32_t last = 0;
    for(const auto& item : data.items) {
        if(item.box() == this && item.isTextItem()) {
            first = std::min(first, item.startOffset());
            last = std::max(last, item.endOffset());
        }
    }
    if(first >= last)
        return result;
    const auto text = data.text.tempSubStringBetween(first, last);
    UErrorCode error = U_ZERO_ERROR;
    std::unique_ptr<UBreakIterator, decltype(&ubrk_close)> iterator(
        ubrk_open(UBRK_WORD, style()->locale()->lang(), text.getBuffer(), text.length(), &error), ubrk_close);
    if(U_FAILURE(error) || !iterator)
        return result;
    auto start = ubrk_first(iterator.get());
    for(auto end = ubrk_next(iterator.get()); end != UBRK_DONE; start = end, end = ubrk_next(iterator.get())) {
        if(ubrk_getRuleStatus(iterator.get()) < UBRK_WORD_NONE_LIMIT)
            continue;
        WordBox word;
        text.tempSubStringBetween(start, end).toUTF8String(word.word);
        word.startOffset = start;
        word.endOffset = end;
        result.push_back(std::move(word));
    }

    for(const auto& line : m_lines) {
        if(line->shapeWidth() <= 0.f)
            continue;
        const auto& view = line->shape();
        const auto& shape = view.shape();
        const auto repeatCount = static_cast<int>(std::max(1.f, std::floor(line->width() / line->shapeWidth())));
        float repeatStart = 0;
        if(repeatCount > 1 && style()->isLeftToRightDirection())
            repeatStart = std::max(0.f, line->width() - line->shapeWidth() * repeatCount);
        for(int repeat = 0; repeat < repeatCount; ++repeat) {
            std::vector<Rect> wordRects(result.size(), Rect(0, 0, 0, 0));
            float position = repeatStart + repeat * line->shapeWidth();
            for(const auto& run : shape->runs()) {
                // Cluster ends must be obtained in logical order, including for
                // RTL runs. Never split a ligature or combining-mark cluster.
                std::vector<uint32_t> boundaries;
                const auto& glyphs = run->glyphs();
                for(size_t i = 0; i < glyphs.size(); ++i)
                    boundaries.push_back(run->offset() + glyphs[i].characterIndex);
                boundaries.push_back(run->offset() + run->length());
                std::sort(boundaries.begin(), boundaries.end());
                boundaries.erase(std::unique(boundaries.begin(), boundaries.end()), boundaries.end());
                for(size_t i = 0; i < glyphs.size();) {
                    const auto cluster = run->offset() + glyphs[i].characterIndex;
                    float advance = 0;
                    do {
                        advance += glyphs[i].advance;
                        if(line->expansion() && treatAsSpace(shape->text().charAt(cluster)))
                            advance += line->expansion();
                        ++i;
                    } while(i < glyphs.size() && run->offset() + glyphs[i].characterIndex == cluster);
                    // Match TextShapeView::draw's cluster selection exactly.
                    if(cluster < view.startOffset() || cluster >= view.endOffset())
                        continue;
                    const auto clusterEnd = *std::upper_bound(boundaries.begin(), boundaries.end(), cluster);
                    const auto sourceStart = line->textStartOffset() + cluster - view.startOffset() - first;
                    const auto sourceEnd = line->textStartOffset() + std::min(clusterEnd, view.endOffset()) - view.startOffset() - first;
                    auto word = std::lower_bound(result.begin(), result.end(), sourceStart,
                        [](const WordBox& item, uint32_t offset) { return item.endOffset <= offset; });
                    while(word != result.end() && word->startOffset < sourceEnd) {
                        Rect rect(line->x() + std::min(position, position + advance), line->y(), std::abs(advance), line->height());
                        auto& bounds = wordRects[word - result.begin()];
                        if(bounds.isEmpty())
                            bounds = rect;
                        else
                            bounds.unite(rect);
                        ++word;
                    }
                    position += advance;
                }
            }
            for(size_t i = 0; i < result.size(); ++i) {
                if(wordRects[i].isEmpty())
                    continue;
                auto rect = block->lineTransform().mapRect(wordRects[i]);
                if(result[i].fragments.empty())
                    result[i].bounds = rect;
                else
                    result[i].bounds.unite(rect);
                result[i].fragments.push_back(rect);
            }
        }
    }
    // Suppressed/non-rendered text has no selectable rectangle.
    std::erase_if(result, [](const WordBox& word) { return word.fragments.empty(); });
    return result;
}

LineBreakBox::LineBreakBox(Node* node, const RefPtr<BoxStyle>& style)
    : TextBox(node, style)
{
    setText(newLineGlo);
}

WordBreakBox::WordBreakBox(Node* node, const RefPtr<BoxStyle>& style)
    : TextBox(node, style)
{
}

} // namespace plutobook
