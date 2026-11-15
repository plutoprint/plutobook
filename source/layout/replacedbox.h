/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_REPLACEDBOX_H
#define PLUTOBOOK_REPLACEDBOX_H

#include "box.h"

namespace plutobook {

constexpr float kDefaultReplacedWidth = 300.f;
constexpr float kDefaultReplacedHeight = 150.f;

class ReplacedBox : public BoxFrame {
public:
    ReplacedBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isReplacedBox() const final { return true; }

    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const override;
    std::pair<float, float> computePreferredWidths() const override;

    void computePositionedReplacedWidth(float& x, float& width, float& marginLeft, float& marginRight) const;
    void computePositionedReplacedHeight(float& y, float& height, float& marginTop, float& marginBottom) const;

    std::optional<float> computeReplacedWidthUsing(const Length& widthLength) const;
    std::optional<float> computeReplacedHeightUsing(const Length& heightLength) const;

    float constrainReplacedWidth(float width) const;
    float constrainReplacedHeight(float height) const;

    float availableReplacedWidth() const;

    std::optional<float> computeMainReplacedWidth() const;
    std::optional<float> computeMainReplacedHeight() const;

    virtual float computeReplacedWidth() const;
    virtual float computeReplacedHeight() const;

    float computeIntrinsicWidth() const;
    float computeIntrinsicHeight() const;

    Size computeObjectFitSize(ObjectFit objectFit, const Size& contentSize) const;
    Rect computeObjectFitRect(const Rect& contentRect) const;

    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const override;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const override;
    void layout(FragmentBuilder* fragmentainer) override;

    virtual void paintReplaced(const PaintInfo& info, const Point& offset) = 0;
    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) override;

    const char* name() const override { return "ReplacedBox"; }

protected:
    float m_intrinsicWidth = -1.f;
    float m_intrinsicHeight = -1.f;
    float m_intrinsicRatio = -1.f;
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
