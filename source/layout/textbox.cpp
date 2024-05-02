#include "textbox.h"
#include "linebox.h"
#include "globalstring.h"

namespace plutobook {

TextBox::TextBox(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
    , m_lines(style->heap())
{
    setInline(true);
}

void TextBox::appendText(const std::string_view& text)
{
    m_text = heap()->concatenateString(m_text, text);
}

TextBox::~TextBox() = default;

static const GlobalString newLine("\n");

LineBreakBox::LineBreakBox(Node* node, const RefPtr<BoxStyle>& style)
    : TextBox(node, style)
{
    setText(newLine);
}

WordBreakBox::WordBreakBox(Node* node, const RefPtr<BoxStyle>& style)
    : TextBox(node, style)
{
}

LeaderTextBox::LeaderTextBox(const RefPtr<BoxStyle>& style)
    : TextBox(nullptr, style)
{
}

} // namespace plutobook
