/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FORMCONTROLBOX_H
#define FORMCONTROLBOX_H

#include "blockbox.h"

namespace plutobook {

class HTMLElement;

class TextInputBox final : public BlockFlowBox {
public:
    TextInputBox(HTMLElement* element, const RefPtr<BoxStyle>& style);

    bool isTextInputBox() const final { return true; }

    HTMLElement* element() const;
    std::optional<float> inlineBlockBaseline() const final;
    uint32_t rows() const { return m_rows; }
    uint32_t cols() const { return m_cols; }

    void setRows(uint32_t rows) { m_rows = rows; }
    void setCols(uint32_t cols) { m_cols = cols; }

    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const final;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const final;

    const char* name() const final { return "TextInputBox"; }

private:
    uint32_t m_rows{1};
    uint32_t m_cols{1};
};

template<>
struct is_a<TextInputBox> {
    static bool check(const Box& box) { return box.isTextInputBox(); }
};

class HTMLSelectElement;

class SelectBox final : public BlockBox {
public:
    SelectBox(HTMLSelectElement* element, const RefPtr<BoxStyle>& style);

    bool isSelectBox() const final { return true; }

    HTMLSelectElement* element() const;
    std::optional<float> inlineBlockBaseline() const final;
    uint32_t size() const { return m_size; }

    void addChild(Box* newChild) final;
    void updateOverflowRect() final;
    void computeIntrinsicWidths(float& minWidth, float& maxWidth) const final;
    void computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const final;
    void paintContents(const PaintInfo& info, const Point& offset, PaintPhase phase) final;
    void layout(FragmentBuilder* fragmentainer) final;

    const char* name() const final { return "SelectBox"; }

private:
    const uint32_t m_size;
};

template<>
struct is_a<SelectBox> {
    static bool check(const Box& box) { return box.isSelectBox(); }
};

} // namespace plutobook

#endif // FORMCONTROLBOX_H
