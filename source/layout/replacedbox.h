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

class ReplacedBox : public BoxFrame {
public:
    ReplacedBox(Node* node, const RefPtr<BoxStyle>& style);

    bool isReplacedBox() const final { return true; }

    virtual void computeIntrinsicRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const = 0;
    void computeAspectRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const;

    virtual float computePreferredReplacedWidth() const;
    void computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const override;

    void computePositionedReplacedWidth(float& x, float& width, float& marginLeft, float& marginRight) const;
    void computePositionedReplacedHeight(float& y, float& height, float& marginTop, float& marginBottom) const;

    std::optional<float> computeReplacedWidthUsing(const Length& widthLength) const;
    std::optional<float> computeReplacedHeightUsing(const Length& heightLength) const;

    float constrainReplacedWidth(float width) const;
    float constrainReplacedHeight(float height) const;

    float availableReplacedWidth() const;

    virtual float computeReplacedWidth() const;
    virtual float computeReplacedHeight() const;

    Rect computeObjectFitRect(const Rect& contentRect) const;

    void computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const override;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const override;
    void layout(FragmentBuilder* fragmentainer) override;

    virtual void paintReplaced(const PaintInfo& info, const Point& offset) = 0;
    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) override;

    float intrinsicReplacedWidth() const { return m_intrinsicSize.w; }
    float intrinsicReplacedHeight() const { return m_intrinsicSize.h; }

    void setIntrinsicSize(const Size& intrinsicSize) { m_intrinsicSize = intrinsicSize; }
    Size intrinsicSize() const { return m_intrinsicSize; }

    const char* name() const override { return "ReplacedBox"; }

private:
    Size m_intrinsicSize;
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

    void computeIntrinsicRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const final;
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
