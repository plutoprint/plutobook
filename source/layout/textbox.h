/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_TEXTBOX_H
#define PLUTOBOOK_TEXTBOX_H

#include "box.h"
#include "plutobook.h"

#include <string>
#include <vector>

namespace plutobook {

class TextLineBox;

using TextLineBoxList = std::pmr::vector<std::unique_ptr<TextLineBox>>;

// Selection/advance bounds, not glyph ink bounds. Coordinates are physical CSS
// pixels relative to the containing block, before CSS transforms and clipping.
// Offsets address this TextBox's processed layout text in UTF-16 code units.
struct WordBox {
    std::string word;
    uint32_t startOffset{0};
    uint32_t endOffset{0};
    Rect bounds{0, 0, 0, 0};
    std::vector<Rect> fragments;
};

using WordBoxList = std::vector<WordBox>;
using WordIterator = WordBoxList::const_iterator;

class TextBox : public Box {
public:
    TextBox(Node* node, const RefPtr<BoxStyle>& style);
    ~TextBox() override;

    bool isTextBox() const final { return true; }

    const HeapString& text() const { return m_text; }
    void setText(const HeapString& text) { m_text = text; }
    void appendText(std::string_view text);

    const TextLineBoxList& lines() const { return m_lines; }
    TextLineBoxList& lines() { return m_lines; }

    // Call after layout. An owning snapshot; does not lay out, paint, or reshape.
    // ICU word segmentation excludes standalone punctuation and whitespace.
    // Words are local to this TextBox: adjacent DOM text nodes are not joined.
    PLUTOBOOK_API WordBoxList words() const;

    const char* name() const override { return "TextBox"; }

private:
    HeapString m_text;
    TextLineBoxList m_lines;
};

template<>
struct is_a<TextBox> {
    static bool check(const Box& box) { return box.isTextBox(); }
};

class LineBreakBox final : public TextBox {
public:
    LineBreakBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isLineBreakBox() const final { return true; }

    const char* name() const final { return "LineBreakBox"; }
};

template<>
struct is_a<LineBreakBox> {
    static bool check(const Box& box) { return box.isLineBreakBox(); }
};

class WordBreakBox final : public TextBox {
public:
    WordBreakBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isWordBreakBox() const final { return true; }

    const char* name() const final { return "WordBreakBox"; }
};

template<>
struct is_a<WordBreakBox> {
    static bool check(const Box& box) { return box.isWordBreakBox(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_TEXTBOX_H
