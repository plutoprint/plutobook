#ifndef PLUTOBOOK_FRAGMENTBUILDER_H
#define PLUTOBOOK_FRAGMENTBUILDER_H

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

    virtual float fragmentHeightForOffset(float offset) const = 0;
    virtual float fragmentRemainingHeightForOffset(float offset, FragmentBoundaryRule rule) const = 0;

    virtual void addForcedFragmentBreak(float offset) = 0;
    virtual void setFragmentBreak(float offset, float spaceShortage) = 0;
    virtual void updateMinimumFragmentHeight(float offset, float minHeight) = 0;

    virtual void enterFragment(const BoxFrame* child, float offset) = 0;
    virtual void leaveFragment(const BoxFrame* child, float offset) = 0;
};

class Book;

class PageBuilder final : public FragmentBuilder {
public:
    explicit PageBuilder(const Book* book);

    float applyFragmentBreakBefore(const BoxFrame* child, float offset) final;
    float applyFragmentBreakAfter(const BoxFrame* child, float offset) final;
    float applyFragmentBreakInside(const BoxFrame* child, float offset) final;

    float fragmentHeightForOffset(float offset) const final;
    float fragmentRemainingHeightForOffset(float offset, FragmentBoundaryRule rule) const final;

    void addForcedFragmentBreak(float offset) final;
    void setFragmentBreak(float offset, float spaceShortage) final;
    void updateMinimumFragmentHeight(float offset, float minHeight) final;

    void enterFragment(const BoxFrame* child, float offset) final;
    void leaveFragment(const BoxFrame* child, float offset) final;

private:
    const Book* m_book;
};

} // namespace plutobook

#endif // PLUTOBOOK_FRAGMENTBUILDER_H
