/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_SVGTEXTBOX_H
#define PLUTOBOOK_SVGTEXTBOX_H

#include "svgboxmodel.h"
#include "svgdocument.h"
#include "svglinelayout.h"

namespace plutobook {

class SVGInlineTextBox final : public Box {
public:
    SVGInlineTextBox(TextNode* node, const RefPtr<BoxStyle>& style);

    bool isSVGInlineTextBox() const final { return true; }
    const HeapString& text() const { return node()->data(); }
    TextNode* node() const;

    const char* name() const final { return "SVGInlineTextBox"; }
};

template<>
struct is_a<SVGInlineTextBox> {
    static bool check(const Box& box) { return box.isSVGInlineTextBox(); }
};

inline TextNode* SVGInlineTextBox::node() const
{
    return static_cast<TextNode*>(Box::node());
}

class SVGInlineBox : public Box {
public:
    SVGInlineBox(SVGTextContentElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGInlineBox() const final { return true; }
    SVGTextContentElement* element() const;
    void build() final;

    const SVGPaintServer& fill() const { return m_fill; }
    const SVGPaintServer& stroke() const { return m_stroke; }

    const char* name() const override { return "SVGInlineBox"; }

private:
    SVGPaintServer m_fill;
    SVGPaintServer m_stroke;
};

template<>
struct is_a<SVGInlineBox> {
    static bool check(const Box& box) { return box.isSVGInlineBox(); }
};

inline SVGTextContentElement* SVGInlineBox::element() const
{
    return static_cast<SVGTextContentElement*>(node());
}

class SVGTSpanBox final : public SVGInlineBox {
public:
    SVGTSpanBox(SVGTSpanElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGTSpanBox() const final { return true; }
    SVGTSpanElement* element() const;

    const char* name() const final { return "SVGTSpanBox"; }
};

template<>
struct is_a<SVGTSpanBox> {
    static bool check(const Box& box) { return box.isSVGTSpanBox(); }
};

inline SVGTSpanElement* SVGTSpanBox::element() const
{
    return static_cast<SVGTSpanElement*>(node());
}

class SVGTextBox final : public SVGBoxModel {
public:
    SVGTextBox(SVGTextElement* element, const RefPtr<BoxStyle>& style);

    bool isSVGTextBox() const final { return true; }
    SVGTextElement* element() const;
    Transform localTransform() const final { return element()->transform(); }
    Rect fillBoundingBox() const final;
    Rect strokeBoundingBox() const final;
    void render(const SVGRenderState& state) const final;
    void layout() final;
    void build() final;

    const SVGPaintServer& fill() const { return m_fill; }
    const SVGPaintServer& stroke() const { return m_stroke; }

    const char* name() const final { return "SVGTextBox"; }

private:
    SVGPaintServer m_fill;
    SVGPaintServer m_stroke;
    SVGLineLayout m_lineLayout;
    mutable Rect m_fillBoundingBox = Rect::Invalid;
    mutable Rect m_strokeBoundingBox = Rect::Invalid;
};

template<>
struct is_a<SVGTextBox> {
    static bool check(const Box& box) { return box.isSVGTextBox(); }
};

inline SVGTextElement* SVGTextBox::element() const
{
    return static_cast<SVGTextElement*>(node());
}

} // namespace plutobook

#endif // PLUTOBOOK_SVGTEXTBOX_H
