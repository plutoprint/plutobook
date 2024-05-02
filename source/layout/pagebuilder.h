#ifndef PLUTOBOOK_PAGEBUILDER_H
#define PLUTOBOOK_PAGEBUILDER_H

#include "document.h"

namespace plutobook {

class BoxFrame;

class PageBuilder {
public:
    explicit PageBuilder(Document* document);

    void build();

    void enterBox(const BoxFrame* box, float top);
    void exitBox(const BoxFrame* box, float top);
    void addPageUntil(const BoxFrame* box, float top);
    void setPageBreakAt(float top);
    bool canFitOnPage(float top) const;

private:
    void newPage(const BoxFrame* box, float top);
    Document* m_document;
    PageBoxList& m_pages;
    PageBox* m_currentPage{nullptr};
};

} // namespace plutobook

#endif // PLUTOBOOK_PAGEBUILDER_H
