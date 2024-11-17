#ifndef PLUTOBOOK_FRAGMENTBUILDER_H
#define PLUTOBOOK_FRAGMENTBUILDER_H

#include "document.h"

namespace plutobook {

class BoxFrame;

class FragmentBuilder {
public:
    FragmentBuilder() = default;
    virtual ~FragmentBuilder() = default;

    virtual void enterBox(const BoxFrame* box, float top) = 0;
    virtual void exitBox(const BoxFrame* box, float top) = 0;

    virtual void addFragmentUntil(const BoxFrame* box, float top) = 0;
    virtual void setFragmentBreakAt(float top) = 0;

    virtual bool canFitOnFragment(float top) const = 0;
};

class PageBuilder final : public FragmentBuilder {
public:
    explicit PageBuilder(Document* document);

    void build();

    void enterBox(const BoxFrame* box, float top) final;
    void exitBox(const BoxFrame* box, float top) final;

    void addFragmentUntil(const BoxFrame* box, float top) final;
    void setFragmentBreakAt(float top) final;

    bool canFitOnFragment(float top) const final;

private:
    void newPage(const BoxFrame* box, float top);
    Document* m_document;
    PageBoxList& m_pages;
    PageBox* m_currentPage{nullptr};
};

} // namespace plutobook

#endif // PLUTOBOOK_FRAGMENTBUILDER_H
