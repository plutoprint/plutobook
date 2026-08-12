/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_GRADIENTIMAGE_H
#define PLUTOBOOK_GRADIENTIMAGE_H

#include "imageresource.h"
#include "graphicscontext.h"
#include "cssproperty.h"

namespace plutobook {

class BoxStyle;

struct GradientColorStop {
    Color color;
    Length position;
    bool hint{false};
};

using GradientColorStopList = std::vector<GradientColorStop>;

class GradientImage final : public Image {
public:
    static RefPtr<GradientImage> create(const BoxStyle& style, const CSSGradientValue& value);

    void draw(GraphicsContext& context, const Rect& dstRect, const Rect& srcRect) final;
    void drawPattern(GraphicsContext& context, const Rect& destRect, const Size& size, const Size& scale, const Point& phase) final;
    void computeIntrinsicDimensions(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) final;

    void setContainerSize(const Size& size) final { m_containerSize = size; }
    Size intrinsicSize() const final { return Size(); }
    Size size() const final { return m_containerSize; }

private:
    GradientImage(CSSGradientType gradientType, bool repeating, GradientColorStopList stops);

    struct ResolvedGradient {
        GradientStops stops;
        float start{0.f};
        float end{1.f};
        SpreadMethod method{SpreadMethod::Pad};
        bool degenerate{false};
        Color color;
    };

    void apply(GraphicsContext& context) const;
    void applyLinearGradient(GraphicsContext& context) const;

    // Resolves the color stop list onto a gradient line of the given length,
    // following the CSS color stop fixup rules and expanding transition hints
    // into sampled stops. The returned offsets are not clamped to [0, 1].
    void buildColorStops(float lineLength, GradientStops& stops) const;

    // Moves the stop offsets onto [0, 1] and reports, as fractions of the
    // gradient line, where those two ends now sit.
    ResolvedGradient resolveGradient(float lineLength) const;

    CSSGradientType m_gradientType;
    bool m_repeating;
    GradientColorStopList m_stops;

    float m_angle{180.f};
    CSSValueID m_directionX{CSSValueID::Unknown};
    CSSValueID m_directionY{CSSValueID::Unknown};

    Size m_containerSize;
};

} // namespace plutobook

#endif // PLUTOBOOK_GRADIENTIMAGE_H
