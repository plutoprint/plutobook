#ifndef PLUTOBOOK_REPLACEDBOX_H
#define PLUTOBOOK_REPLACEDBOX_H

#include "box.h"

namespace plutobook {

class ReplacedBox : public BoxFrame {
public:
    ReplacedBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isReplacedBox() const final { return true; }

    virtual void computeIntrinsicRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const;
    void computeAspectRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const;

    float computePreferredReplacedWidth() const;
    void computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const override;

    void computePositionedReplacedWidth(float& x, float& width, float& marginLeft, float& marginRight) const;
    void computePositionedReplacedHeight(float& y, float& height, float& marginTop, float& marginBottom) const;

    std::optional<float> computeReplacedWidthUsing(const Length& widthLength) const;
    std::optional<float> computeReplacedHeightUsing(const Length& heightLength) const;

    float constrainReplacedWidth(float width) const;
    float constrainReplacedHeight(float height) const;

    float computeReplacedWidth() const;
    float computeReplacedHeight() const;
    float availableReplacedWidth() const;

    virtual float intrinsicReplacedWidth() const = 0;
    virtual float intrinsicReplacedHeight() const = 0;

    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const override;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const override;
    void layout() override;

    virtual void paintReplaced(const PaintInfo& info, const Point& offset) = 0;
    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) override;
    void fragmentize(FragmentBuilder& builder, float top) const override;

    const char* name() const override { return "ReplacedBox"; }
};

template<>
struct is_a<ReplacedBox> {
    static bool check(const Box& box) { return box.isReplacedBox(); }
};

class Image;

class ImageBox final : public ReplacedBox {
public:
    ImageBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isImageBox() const final { return true; }

    const RefPtr<Image>& image() const { return m_image; }
    void setImage(RefPtr<Image> image);

    float intrinsicReplacedWidth() const final;
    float intrinsicReplacedHeight() const final;

    void paintReplaced(const PaintInfo& info, const Point& offset) final;

    const char* name() const final { return "ImageBox"; }

private:
    RefPtr<Image> m_image;
};

template<>
struct is_a<ImageBox> {
    static bool check(const Box& box) { return box.isImageBox(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_REPLACEDBOX_H
