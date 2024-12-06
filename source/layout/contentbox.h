#ifndef PLUTOBOOK_CONTENTBOX_H
#define PLUTOBOOK_CONTENTBOX_H

#include "globalstring.h"
#include "textbox.h"

namespace plutobook {

class ContentBox : public TextBox {
public:
    ContentBox(const RefPtr<BoxStyle>& style);

    bool isContentBox() const final { return true; }

    const char* name() const override { return "ContentBox"; }
};

template<>
struct is_a<ContentBox> {
    static bool check(const Box& box) { return box.isContentBox(); }
};

class LeaderBox final : public ContentBox {
public:
    LeaderBox(const RefPtr<BoxStyle>& style);

    bool isLeaderBox() const final { return true; }

    const char* name() const final { return "LeaderBox"; }
};

template<>
struct is_a<LeaderBox> {
    static bool check(const Box& box) { return box.isLeaderBox(); }
};

class TargetCounterBox final : public ContentBox {
public:
    TargetCounterBox(const RefPtr<BoxStyle>& style);

    bool isTargetCounterBox() const final { return true; }

    const HeapString& fragment() const { return m_fragment; }
    const GlobalString& identifier() const { return m_identifier; }
    const HeapString& seperator() const { return m_seperator; }
    const GlobalString& listStyle() const { return m_listStyle; }

    void setFragment(const HeapString& fragment) { m_fragment = fragment; }
    void setIdentifier(const GlobalString& identifier) { m_identifier = identifier; }
    void setSeperator(const HeapString& seperator) { m_seperator = seperator; }
    void setListStyle(const GlobalString& listStyle) { m_listStyle = listStyle; }

    const char* name() const final { return "TargetCounterBox"; }

private:
    HeapString m_fragment;
    GlobalString m_identifier;
    HeapString m_seperator;
    GlobalString m_listStyle;
};

template<>
struct is_a<TargetCounterBox> {
    static bool check(const Box& box) { return box.isTargetCounterBox(); }
};

class PageCounterBox final : public ContentBox{
public:
    PageCounterBox(const RefPtr<BoxStyle>& style);

    bool isPageCounterBox() const final { return true; }

    const char* name() const final { return "PageCounterBox"; }
};

template<>
struct is_a<PageCounterBox> {
    static bool check(const Box& box) { return box.isPageCounterBox(); }
};

class PagesCounterBox final : public ContentBox{
public:
    PagesCounterBox(const RefPtr<BoxStyle>& style);

    bool isPagesCounterBox() const final { return true; }

    const char* name() const final { return "PagesCounterBox"; }
};

template<>
struct is_a<PagesCounterBox> {
    static bool check(const Box& box) { return box.isPagesCounterBox(); }
};

class Counters;
class Element;
class CSSFunctionValue;

class ContentBoxBuilder {
public:
    ContentBoxBuilder(Counters& counters, Element* element, Box* box);

    void build();

private:
    void addText(const HeapString& text);
    void addLeaderText(const HeapString& text);
    void addLeader(const RefPtr<CSSValue>& value);
    void addTargetCounter(const RefPtr<CSSFunctionValue>& function);
    void addImage(RefPtr<Image> image);
    void addQuote(CSSValueID value);

    Counters& m_counters;
    Element* m_element;
    Box* m_parentBox;
    BoxStyle* m_parentStyle;
    TextBox* m_lastTextBox{nullptr};
};

} // namespace plutobook

#endif // PLUTOBOOK_CONTENTBOX_H
