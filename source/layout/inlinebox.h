/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_INLINEBOX_H
#define PLUTOBOOK_INLINEBOX_H

#include "box.h"

namespace plutobook {

class FlowLineBox;

using FlowLineBoxList = std::pmr::vector<std::unique_ptr<FlowLineBox>>;

class InlineBox : public BoxModel {
public:
    InlineBox(Node* node, const RefPtr<BoxStyle>& style);
    ~InlineBox() override;

    bool isInlineBox() const final { return true; }
    bool requiresLayer() const override;

    Rect visualOverflowRect() const override;
    Rect borderBoundingBox() const override;
    Rect paintBoundingBox() const override;

    Point relativePositionedInlineOffset(const BoxModel* child) const;

    float innerPaddingBoxWidth() const;
    float innerPaddingBoxHeight() const;

    void addChild(Box* newChild) override;

    const FlowLineBoxList& lines() const { return m_lines; }
    FlowLineBoxList& lines() { return m_lines; }

    InlineBox* continuation() const { return m_continuation; }
    void setContinuation(InlineBox* continuation) { m_continuation = continuation; }

    void paint(const PaintInfo& info, const Point& offset, PaintPhase phase) override;

    const char* name() const override { return "InlineBox"; }

private:
    FlowLineBoxList m_lines;
    InlineBox* m_continuation{nullptr};
};

template<>
struct is_a<InlineBox> {
    static bool check(const Box& box) { return box.isInlineBox(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_INLINEBOX_H
