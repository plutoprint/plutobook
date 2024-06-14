#ifndef PLUTOBOOK_BOXVIEW_H
#define PLUTOBOOK_BOXVIEW_H

#include "blockbox.h"

namespace plutobook {

class BoxView final : public BlockFlowBox {
public:
    BoxView(Document* document, const RefPtr<BoxStyle>& style);

    bool isBoxView() const final { return true; }
    bool requiresLayer() const final { return true; }
    BoxStyle* backgroundStyle() const { return m_backgroundStyle; }

    float pageWidth() const { return m_pageWidth; }
    float pageHeight() const { return m_pageHeight; }
    float pageScale() const { return m_pageScale; }

    bool isPrintMedia() const;

    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const final;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const final;
    void layoutContents();
    void layout() final;
    void build() final;

    const char* name() const final { return "BoxView"; }

private:
    BoxStyle* m_backgroundStyle{nullptr};
    float m_pageWidth{0};
    float m_pageHeight{0};
    float m_pageScale{1};
};

template<>
struct is_a<BoxView> {
    static bool check(const Box& box) { return box.isBoxView(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_BOXVIEW_H
