#ifndef PLUTOBOOK_TEXTBOX_H
#define PLUTOBOOK_TEXTBOX_H

#include "box.h"

namespace plutobook {

class TextLineBox;

using TextLineBoxList = std::pmr::vector<std::unique_ptr<TextLineBox>>;

class TextBox : public Box {
public:
    TextBox(Node* node, const RefPtr<BoxStyle>& style);
    ~TextBox() override;

    bool isTextBox() const final { return true; }

    const HeapString& text() const { return m_text; }
    void setText(const HeapString& text) { m_text = text; }
    void appendText(const std::string_view& text);

    const TextLineBoxList& lines() const { return m_lines; }
    TextLineBoxList& lines() { return m_lines; }

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
