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
    TargetCounterBox(const RefPtr<BoxStyle>& style, const HeapString& fragment, const GlobalString& identifier, const HeapString& seperator, const GlobalString& listStyle);

    bool isTargetCounterBox() const final { return true; }

    void build() final;

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

class CSSCounterValue;
class CSSFunctionValue;
class CSSAttrValue;

class ContentBoxBuilder {
public:
    ContentBoxBuilder(Counters& counters, Element* element, Box* box);

    void build();

private:
    void addText(const HeapString& text);
    void addLeaderText(const HeapString& text);
    void addLeader(const CSSValue& value);
    void addElement(const CSSValue& value);
    void addCounter(const CSSCounterValue& counter);
    void addTargetCounter(const CSSFunctionValue& function);
    void addQuote(CSSValueID value);
    void addQrCode(const CSSFunctionValue& function);
    void addImage(RefPtr<Image> image);

    const HeapString& resolveAttr(const CSSAttrValue& attr) const;

    Counters& m_counters;
    Element* m_element;
    Box* m_parentBox;
    BoxStyle* m_parentStyle;
    TextBox* m_lastTextBox{nullptr};
};

} // namespace plutobook

#endif // PLUTOBOOK_CONTENTBOX_H
