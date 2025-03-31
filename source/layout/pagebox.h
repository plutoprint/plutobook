#ifndef PLUTOBOOK_PAGEBOX_H
#define PLUTOBOOK_PAGEBOX_H

#include "globalstring.h"
#include "blockbox.h"

#include "plutobook.hpp"

namespace plutobook {

class PageMarginBox;

class PageBox final : public BlockBox {
public:
    static std::unique_ptr<PageBox> create(const RefPtr<BoxStyle>& style, const PageSize& pageSize, const GlobalString& pageName, uint32_t pageIndex);

    bool isPageBox() const final { return true; }
    bool requiresLayer() const final { return true; }

    const PageSize& pageSize() const { return m_pageSize; }
    const GlobalString& pageName() const { return m_pageName; }
    uint32_t pageIndex() const { return m_pageIndex; }

    PageMarginBox* firstMarginBox() const;
    PageMarginBox* lastMarginBox() const;

    void updateOverflowRect() final;
    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const final;
    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const final;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const final;
    void layout(FragmentBuilder* fragmentainer) final;

    void layoutCornerPageMargin(PageMarginBox* cornerBox, const Rect& cornerRect);
    void layoutEdgePageMargin(PageMarginBox* edgeBox, const Rect& edgeRect, float mainAxisSize);
    void layoutEdgePageMargins(PageMarginBox* edgeStartBox, PageMarginBox* edgeCenterBox, PageMarginBox* edgeEndBox, const Rect& edgeRect);

    void paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase) final;

    const char* name() const final { return "PageBox"; }

private:
    PageBox(const RefPtr<BoxStyle>& style, const PageSize& pageSize, const GlobalString& pageName, uint32_t pageIndex);
    PageSize m_pageSize;
    GlobalString m_pageName;
    uint32_t m_pageIndex;
};

template<>
struct is_a<PageBox> {
    static bool check(const Box& box) { return box.isPageBox(); }
};

enum class PageMarginType : uint8_t;

class PageMarginBox final : public BlockFlowBox {
public:
    PageMarginBox(const RefPtr<BoxStyle>& style, PageMarginType marginType);

    bool isPageMarginBox() const final { return true; }
    bool requiresLayer() const final { return true; }
    PageMarginType marginType() const { return m_marginType; }
    PageBox* pageBox() const;

    bool isHorizontalFlow() const;
    bool isVerticalFlow() const;

    PageMarginBox* nextMarginBox() const;
    PageMarginBox* prevMarginBox() const;

    void updatePaddingWidths() const final;
    bool updateIntrinsicPaddings(float availableHeight);

    void updatePaddings(const Size& availableSize);
    void updateMargins(const Size& availableSize);
    void updateAutoMargins(const Size& availableSize);
    void layoutContents(const Size& availableSize);

    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const final;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const final;

    const char* name() const final { return "PageMarginBox"; }

private:
    PageMarginType m_marginType;
};

inline PageBox* PageMarginBox::pageBox() const
{
    return static_cast<PageBox*>(parentBox());
}

inline PageMarginBox* PageBox::firstMarginBox() const
{
    return static_cast<PageMarginBox*>(firstChild());
}

inline PageMarginBox* PageBox::lastMarginBox() const
{
    return static_cast<PageMarginBox*>(lastChild());
}

inline PageMarginBox* PageMarginBox::nextMarginBox() const
{
    return static_cast<PageMarginBox*>(nextSibling());
}

inline PageMarginBox* PageMarginBox::prevMarginBox() const
{
    return static_cast<PageMarginBox*>(prevSibling());
}

template<>
struct is_a<PageMarginBox> {
    static bool check(const Box& box) { return box.isPageMarginBox(); }
};

class Counters;

class PageBoxBuilder {
public:
    PageBoxBuilder(Document* document, const PageSize& pageSize, float pageWidth, float pageHeight, float marginTop, float marginRight, float marginBottom, float marginLeft);

    void build();

private:
    void buildPageMargin(const Counters& counters, PageBox* pageBox, PageMarginType marginType);
    void buildPageMargins(const Counters& counters, PageBox* pageBox);

    Document* m_document;
    PageSize m_pageSize;
    float m_pageWidth;
    float m_pageHeight;
    float m_marginTop;
    float m_marginRight;
    float m_marginBottom;
    float m_marginLeft;
};

} // namespace plutobook

#endif // PLUTOBOOK_PAGEBOX_H
