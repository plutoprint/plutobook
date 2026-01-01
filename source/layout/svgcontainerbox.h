/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_SVGCONTAINERBOX_H
#define PLUTOBOOK_SVGCONTAINERBOX_H

#include "svgboxmodel.h"

namespace plutobook {

class SVGContainerBox : public SVGBoxModel {
public:
    SVGContainerBox(SVGElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGContainerBox() const final { return true; }

    Rect fillBoundingBox() const override;
    Rect strokeBoundingBox() const override;
    void layout() override;

    void renderChildren(const SVGRenderState& state) const;

    const char* name() const override { return "SVGContainerBox"; }

private:
    mutable Rect m_fillBoundingBox = Rect::Invalid;
    mutable Rect m_strokeBoundingBox = Rect::Invalid;
};

template<>
struct is_a<SVGContainerBox> {
    static bool check(const Box& box) { return box.isSVGContainerBox(); }
};

class SVGHiddenContainerBox : public SVGContainerBox {
public:
    SVGHiddenContainerBox(SVGElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGHiddenContainerBox() const final { return true; }

    void render(const SVGRenderState& state) const override;

    const char* name() const override { return "SVGHiddenContainerBox"; }
};

template<>
struct is_a<SVGHiddenContainerBox> {
    static bool check(const Box& box) { return box.isSVGHiddenContainerBox(); }
};

class SVGTransformableContainerBox final : public SVGContainerBox {
public:
    SVGTransformableContainerBox(SVGGraphicsElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGTransformableContainerBox() const final { return true; }

    SVGGraphicsElement* element() const;
    Transform localTransform() const final { return m_localTransform; }
    void render(const SVGRenderState& state) const final;
    void layout() final;

    const char* name() const final { return "SVGTransformableContainerBox"; }

private:
    Transform m_localTransform;
};

inline SVGGraphicsElement* SVGTransformableContainerBox::element() const
{
    return static_cast<SVGGraphicsElement*>(node());
}

template<>
struct is_a<SVGTransformableContainerBox> {
    static bool check(const Box& box) { return box.isSVGTransformableContainerBox(); }
};

class SVGViewportContainerBox final : public SVGContainerBox {
public:
    SVGViewportContainerBox(SVGSVGElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGViewportContainerBox() const final { return true; }

    SVGSVGElement* element() const;
    Transform localTransform() const final { return m_localTransform; }
    void render(const SVGRenderState& state) const final;
    void layout() final;

    const char* name() const final { return "SVGViewportContainerBox"; }

private:
    Transform m_localTransform;
};

inline SVGSVGElement* SVGViewportContainerBox::element() const
{
    return static_cast<SVGSVGElement*>(node());
}

template<>
struct is_a<SVGViewportContainerBox> {
    static bool check(const Box& box) { return box.isSVGViewportContainerBox(); }
};

class SVGResourceContainerBox : public SVGHiddenContainerBox {
public:
    SVGResourceContainerBox(SVGElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGResourceContainerBox() const final { return true; }

    const char* name() const override { return "SVGResourceContainerBox"; }
};

template<>
struct is_a<SVGResourceContainerBox> {
    static bool check(const Box& box) { return box.isSVGResourceContainerBox(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_SVGCONTAINERBOX_H
