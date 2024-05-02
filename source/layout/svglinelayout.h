#ifndef SVGLINELAYOUT_H
#define SVGLINELAYOUT_H

#include "linelayout.h"

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

class SVGTextBox;
class SVGTextPositioningElement;

struct SVGTextPosition {
    SVGTextPositioningElement* element = nullptr;
    uint32_t startOffset = 0;
    uint32_t endOffset = 0;
};

using SVGTextPositionList = std::vector<SVGTextPosition>;

class SVGLineItemsBuilder final : private LineItemsBuilder {
public:
    SVGLineItemsBuilder(LineItemsData& data, SVGCharacterPositions& positions);

    void appendText(Box* box, const HeapString& data);

    void enterInline(Box* box);
    void exitInline(Box* box);

    void enterBlock(Box* box);
    void exitBlock(Box* box);

private:
    void fillCharacterPositions(const SVGTextPosition& position);
    SVGCharacterPositions& m_characterPositions;
    SVGTextPositionList m_textPositions;
    uint32_t m_itemIndex = 0;
};

struct SVGTextFragment {
    explicit SVGTextFragment(const LineItem& item) : item(item) {}
    const LineItem& item;
    TextShapeView shape;
    bool startsNewTextChunk = false;
    float x = 0;
    float y = 0;
    float angle = 0;
    float width = 0;
};

using SVGTextFragmentList = std::pmr::vector<SVGTextFragment>;

class SVGTextFragmentsBuilder {
public:
    SVGTextFragmentsBuilder(SVGTextFragmentList& fragments, const LineItemsData& data, const SVGCharacterPositions& positions);

    void build();

private:
    void handleTextItem(const LineItem& item);
    void handleBidiControl(const LineItem& item);
    SVGTextFragmentList& m_fragments;
    const LineItemsData& m_data;
    const SVGCharacterPositions& m_positions;
    uint32_t m_characterOffset = 0;
    float m_x = 0;
    float m_y = 0;
};

class Rect;
class SVGRenderState;

class SVGLineLayout {
public:
    explicit SVGLineLayout(SVGTextBox* block);

    Rect boundingRect() const;
    void render(const SVGRenderState& state) const;
    void build();

private:
    SVGTextBox* m_block;
    SVGTextFragmentList m_fragments;
    LineItemsData m_data;
};

} // namespace plutobook

#endif // SVGLINELAYOUT_H
