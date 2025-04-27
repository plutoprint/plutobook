#ifndef PLUTOBOOK_FRAGMENTBUILDER_H
#define PLUTOBOOK_FRAGMENTBUILDER_H

namespace plutobook {

class BoxFrame;

enum class FragmentType {
    Column,
    Page
};

enum FragmentBoundaryRule { AssociateWithFormerFragment, AssociateWithLatterFragment };

class FragmentBuilder {
public:
    FragmentBuilder() = default;
    virtual ~FragmentBuilder() = default;

    virtual FragmentType fragmentType() const = 0;

    virtual float fragmentHeightForOffset(float offset) const = 0;
    virtual float fragmentRemainingHeightForOffset(float offset, FragmentBoundaryRule rule) const = 0;

    virtual void addForcedFragmentBreak(float offset) {}
    virtual void setFragmentBreak(float offset, float spaceShortage) {}
    virtual void updateMinimumFragmentHeight(float offset, float minHeight) {}

    float applyFragmentBreakBefore(const BoxFrame* child, float offset);
    float applyFragmentBreakAfter(const BoxFrame* child, float offset);
    float applyFragmentBreakInside(const BoxFrame* child, float offset);

    void enterFragment(float offset) { m_fragmentOffset += offset; }
    void leaveFragment(float offset) { m_fragmentOffset -= offset; }

    float fragmentOffset() const { return m_fragmentOffset; }

private:
    float m_fragmentOffset = 0;
};

} // namespace plutobook

#endif // PLUTOBOOK_FRAGMENTBUILDER_H
