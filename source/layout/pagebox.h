#ifndef PLUTOBOOK_PAGEBOX_H
#define PLUTOBOOK_PAGEBOX_H

#include "globalstring.h"
#include "blockbox.h"
#include "plutobook.hpp"

namespace plutobook {

class PageBox final : public BlockBox {
public:
    static std::unique_ptr<PageBox> create(const RefPtr<BoxStyle>& style, const GlobalString& pageName, uint32_t pageIndex);

    bool isPageBox() const final { return true; }
    bool requiresLayer() const final { return false; }
    const GlobalString& pageName() const { return m_pageName; }
    uint32_t pageIndex() const { return m_pageIndex; }
    const PageSize& pageSize() const;

    float pageTop() const { return m_pageTop; }
    float pageBottom() const { return m_pageBottom; }

    void setPageTop(float pageTop) { m_pageTop = pageTop; }
    void setPageBottom(float pageBottom) { m_pageBottom = pageBottom; }

    void updateOverflowRect() final;
    void computePreferredWidths(float& minWidth, float& maxWidth) const final;
    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const final;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const final;
    void layout() final;
    void build() final;

    const char* name() const final { return "PageBox"; }

private:
    PageBox(const RefPtr<BoxStyle>& style, const GlobalString& pageName, uint32_t pageIndex);
    GlobalString m_pageName;
    uint32_t m_pageIndex;
    float m_pageTop{0};
    float m_pageBottom{0};
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
    bool requiresLayer() const final { return false; }
    PageMarginType marginType() const { return m_marginType; }

    const char* name() const final { return "PageMarginBox"; }

private:
    PageMarginType m_marginType;
};

template<>
struct is_a<PageMarginBox> {
    static bool check(const Box& box) { return box.isPageMarginBox(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_PAGEBOX_H
