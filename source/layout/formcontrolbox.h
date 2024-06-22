#ifndef FORMCONTROLBOX_H
#define FORMCONTROLBOX_H

#include "blockbox.h"

namespace plutobook {

class HTMLSelectElement;

class SelectBox : public BlockBox {
public:
    SelectBox(HTMLSelectElement* element, const RefPtr<BoxStyle>& style);

    HTMLSelectElement* element() const;
    std::optional<float> inlineBlockBaseline() const final;
    uint32_t size() const { return m_size; }

    void addChild(Box* newChild) final;
    void updateOverflowRect() final;
    void computePreferredWidths(float& minWidth, float& maxWidth) const final;

    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const final;
    void paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase) final;
    void paginate(PageBuilder& builder, float top) const final;
    void layout() final;

private:
    uint32_t m_size;
};

} // namespace plutobook

#endif // FORMCONTROLBOX_H
