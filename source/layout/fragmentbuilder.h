#ifndef PLUTOBOOK_FRAGMENTBUILDER_H
#define PLUTOBOOK_FRAGMENTBUILDER_H

#include <cstdint>
#include <vector>
#include <memory>

namespace plutobook {

class BoxFrame;

class Fragment {
public:
    enum class Type : uint8_t {
        LineBox,
        ReplacedBox,
        ForceBreak,
        BoxStart,
        BoxEnd
    };

    Fragment(const BoxFrame* box, Type type, float top, float bottom)
        : m_box(box), m_type(type), m_top(top), m_bottom(bottom)
    {}

    const BoxFrame* box() const { return m_box; }
    Type type() const { return m_type; }
    float top() const { return m_top; }
    float bottom() const { return m_bottom; }

    void setBreakInside(bool breakInside) { m_breakInside = breakInside; }
    bool canBreakInside() const { return m_breakInside; }

private:
    const BoxFrame* m_box;
    Type m_type;
    bool m_breakInside{false};
    float m_top;
    float m_bottom;
};

using FragmentList = std::vector<Fragment>;

enum class FragmentationMode {
    Page,
    Column
};

class RootLineBox;
class BoxStyle;

class FragmentBuilder {
public:
    FragmentBuilder(FragmentList& fragments, FragmentationMode mode);

    void handleLineBox(const RootLineBox& line, float top);
    void handleReplacedBox(const BoxFrame* box, float top);

    void enterBox(const BoxFrame* box, float top);
    void exitBox(const BoxFrame* box, float top);

private:
    void applyBreakInside(Fragment& fragment, const BoxStyle* style);
    void handleBreakBefore(const BoxFrame* box, float top);
    void handleBreakAfter(const BoxFrame* box, float top);
    FragmentList& m_fragments;
    FragmentationMode m_mode;
};

class Document;
class PageBox;

class PageBreaker {
public:
    PageBreaker(Document* document, const FragmentList& fragments);

    std::unique_ptr<PageBox> nextPage();

    bool isDone() const;

private:
    Document* m_document;
    const FragmentList& m_fragments;

    uint32_t m_fragmentIndex{0};
    uint32_t m_pageIndex{0};
    float m_pageTop{0};
};

using PageBoxList = std::pmr::vector<std::unique_ptr<PageBox>>;

class PageBuilder {
public:
    explicit PageBuilder(Document* document);

    void build();

private:
    Document* m_document;
    PageBoxList& m_pages;
};

} // namespace plutobook

#endif // PLUTOBOOK_FRAGMENTBUILDER_H
