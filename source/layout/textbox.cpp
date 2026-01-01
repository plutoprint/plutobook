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

namespace plutobook {

TextBox::TextBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
    , m_lines(style->heap())
{
    setIsInline(true);
}

void TextBox::appendText(const std::string_view& text)
{
    m_text = heap()->concatenateString(m_text, text);
}

TextBox::~TextBox() = default;

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
