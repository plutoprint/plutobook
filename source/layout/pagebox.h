#ifndef PLUTOBOOK_PAGEBOX_H
#define PLUTOBOOK_PAGEBOX_H

#include "globalstring.h"
#include "blockbox.h"

#include "plutobook.hpp"

namespace plutobook {

class PageBox final : public BlockBox {
public:
    static std::unique_ptr<PageBox> create(const RefPtr<BoxStyle>& style, const PageSize& pageSize, const GlobalString& pageName, uint32_t pageIndex);

    bool isPageBox() const final { return true; }
    bool requiresLayer() const final { return false; }

    const PageSize& pageSize() const { return m_pageSize; }
    const GlobalString& pageName() const { return m_pageName; }
    uint32_t pageIndex() const { return m_pageIndex; }

    void updateOverflowRect() final;
    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const final;
    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const final;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const final;
    void layout(FragmentBuilder* fragmentainer) final;
    void build() final;

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
    bool requiresLayer() const final { return false; }
    PageMarginType marginType() const { return m_marginType; }
    PageBox* pageBox() const;

    const char* name() const final { return "PageMarginBox"; }

private:
    PageMarginType m_marginType;
};

inline PageBox* PageMarginBox::pageBox() const
{
    return static_cast<PageBox*>(parentBox());
}

template<>
struct is_a<PageMarginBox> {
    static bool check(const Box& box) { return box.isPageMarginBox(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_PAGEBOX_H
