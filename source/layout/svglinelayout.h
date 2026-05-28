/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SVGLINELAYOUT_H
#define SVGLINELAYOUT_H

#include "linelayout.h"
#include "geometry.h"

#include <optional>
#include <map>

namespace plutobook {

struct SVGCharacterPosition {
    std::optional<float> x;
    std::optional<float> y;
    std::optional<float> dx;
    std::optional<float> dy;
    std::optional<float> rotate;
};

using SVGCharacterPositions = std::map<uint32_t, SVGCharacterPosition>;

class SVGTextPositioningElement;

struct SVGTextPosition {
    SVGTextPosition(SVGTextPositioningElement* element, uint32_t startOffset, uint32_t endOffset)
        : element(element), startOffset(startOffset), endOffset(endOffset)
    {}

    SVGTextPositioningElement* element;
    uint32_t startOffset;
    uint32_t endOffset;
};

using SVGTextPositionList = std::vector<SVGTextPosition>;

class SVGLineItemsBuilder final : private LineItemsBuilder {
public:
    SVGLineItemsBuilder(LineItemsData& data, SVGTextPositionList& positions);

    void appendText(Box* box, const HeapString& data);

    void enterInline(Box* box);
    void exitInline(Box* box);

    void enterBlock(Box* box);
    void exitBlock(Box* box);

private:
    SVGTextPositionList& m_textPositions;
    uint32_t m_itemIndex = 0;
};

class SVGTextContentElement;

struct SVGTextFragment {
    explicit SVGTextFragment(const SVGTextContentElement* element) : element(element) {}

    const SVGTextContentElement* element;
    TextShapeView shape;
    Transform lengthAdjustTransform;

    bool startsNewTextChunk = false;
    bool inTextPath = false;

    float x = 0;
    float y = 0;
    float width = 0;
    float height = 0;
    float angle = 0;

    float xShift = 0;
    float yShift = 0;
};

using SVGTextFragmentList = std::pmr::vector<SVGTextFragment>;

class SVGTextFragmentsBuilder {
public:
    SVGTextFragmentsBuilder(SVGTextFragmentList& fragments, const LineItemsData& data, const SVGCharacterPositions& positions);

    void layout();

private:
    void handleInlineStart(const LineItem& item);
    void handleInlineEnd(const LineItem& item);
    void handleTextItem(const LineItem& item);

    SVGTextFragmentList& m_fragments;
    const LineItemsData& m_data;
    const SVGCharacterPositions& m_positions;

    Path m_textPath;
    float m_textPathLength = 0;
    float m_textPathStartOffset = 0;
    float m_textPathCurrentOffset = 0;

    uint32_t m_characterOffset = 0;
    float m_dx = 0;
    float m_dy = 0;
    float m_x = 0;
    float m_y = 0;
};

class SVGTextBox;
class SVGRenderState;

class SVGLineLayout {
public:
    explicit SVGLineLayout(SVGTextBox* block);

    Rect boundingRect(bool includeStroke) const;
    void render(const SVGRenderState& state) const;
    void layout();
    void build();

private:
    SVGTextBox* m_block;
    SVGTextPositionList m_textPositions;
    SVGTextFragmentList m_fragments;
    LineItemsData m_data;
};

} // namespace plutobook

#endif // SVGLINELAYOUT_H
