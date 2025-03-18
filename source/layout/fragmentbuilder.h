#ifndef PLUTOBOOK_FRAGMENTBUILDER_H
#define PLUTOBOOK_FRAGMENTBUILDER_H

#include <vector>
#include <memory>

namespace plutobook {

class BoxFrame;

enum FragmentBoundaryRule { AssociateWithFormerFragment, AssociateWithLatterFragment };

class FragmentBuilder {
public:
    FragmentBuilder() = default;
    virtual ~FragmentBuilder() = default;

    virtual float applyFragmentBreakBefore(const BoxFrame* child, float offset) = 0;
    virtual float applyFragmentBreakAfter(const BoxFrame* child, float offset) = 0;
    virtual float applyFragmentBreakInside(const BoxFrame* child, float offset) = 0;

    virtual float fragmentHeightForOffset(float offset) = 0;
    virtual float fragmentRemainingHeightForOffset(float offset, FragmentBoundaryRule rule) = 0;

    virtual void addForcedFragmentBreak(float offset) = 0;
    virtual void setFragmentBreak(float offset, float spaceShortage) = 0;
    virtual void updateMinimumFragmentHeight(float offset, float minHeight) = 0;

    virtual void enterFragment(const BoxFrame* child, float offset) { m_fragmentOffset += offset; }
    virtual void leaveFragment(const BoxFrame* child, float offset) { m_fragmentOffset -= offset; }

    float fragmentOffset() const { return m_fragmentOffset; }

private:
    float m_fragmentOffset = 0;
};

class Book;
class Document;
class PageBox;

using PageBoxList = std::pmr::vector<std::unique_ptr<PageBox>>;

class PageBuilder final : public FragmentBuilder {
public:
    explicit PageBuilder(const Book* book);

    float applyFragmentBreakBefore(const BoxFrame* child, float offset) final;
    float applyFragmentBreakAfter(const BoxFrame* child, float offset) final;
    float applyFragmentBreakInside(const BoxFrame* child, float offset) final;

    float fragmentHeightForOffset(float offset) final;
    float fragmentRemainingHeightForOffset(float offset, FragmentBoundaryRule rule) final;

    void addForcedFragmentBreak(float offset) final;
    void setFragmentBreak(float offset, float spaceShortage) final;
    void updateMinimumFragmentHeight(float offset, float minHeight) final;

    void enterFragment(const BoxFrame* child, float offset) final;
    void leaveFragment(const BoxFrame* child, float offset) final;

private:
    void addPageUntil(const BoxFrame* box, float offset);
    void addPage(const BoxFrame* box);
    const Book* m_book;
    Document* m_document;
    PageBoxList& m_pages;
    PageBox* m_currentPage{nullptr};
};

} // namespace plutobook

#endif // PLUTOBOOK_FRAGMENTBUILDER_H
