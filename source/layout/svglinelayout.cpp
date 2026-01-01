/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "svglinelayout.h"
#include "svgtextbox.h"

#include <cmath>

namespace plutobook {

SVGLineItemsBuilder::SVGLineItemsBuilder(LineItemsData& data, SVGTextPositionList& positions)
    : LineItemsBuilder(data)
    , m_textPositions(positions)
{
}

void SVGLineItemsBuilder::appendText(Box* box, const HeapString& data)
{
    LineItemsBuilder::appendText(box, data);
}

void SVGLineItemsBuilder::enterInline(Box* box)
{
    LineItemsBuilder::enterInline(box);
    auto element = to<SVGTSpanBox>(*box).element();
    m_textPositions.emplace_back(element, m_data.text.length(), m_data.text.length());
    m_itemIndex = m_textPositions.size();
}

void SVGLineItemsBuilder::exitInline(Box* box)
{
    auto element = to<SVGTSpanBox>(*box).element();
    auto& position = m_textPositions[--m_itemIndex];
    assert(position.element == element);
    position.endOffset = m_data.text.length();
    LineItemsBuilder::exitInline(box);
}

void SVGLineItemsBuilder::enterBlock(Box* box)
{
    LineItemsBuilder::enterBlock(box);
}

void SVGLineItemsBuilder::exitBlock(Box* box)
{
    LineItemsBuilder::exitBlock(box);
}

SVGTextFragmentsBuilder::SVGTextFragmentsBuilder(SVGTextFragmentList& fragments, const LineItemsData& data, const SVGCharacterPositions& positions)
    : m_fragments(fragments), m_data(data), m_positions(positions)
{
    m_fragments.clear();
}

static bool needsTextAnchorAdjustment(const BoxStyle* style)
{
    if(style == nullptr)
        return false;
    auto direction = style->direction();
    switch(style->textAnchor()) {
    case TextAnchor::Start:
        return direction == Direction::Rtl;
    case TextAnchor::Middle:
        return true;
    case TextAnchor::End:
        return direction == Direction::Ltr;
    default:
        assert(false);
    }

    return false;
}

static float calculateTextAnchorOffset(const BoxStyle* style, float width)
{
    auto direction = style->direction();
    switch(style->textAnchor()) {
    case TextAnchor::Start:
        if(direction == Direction::Ltr)
            return 0.f;
        return -width;
    case TextAnchor::Middle:
        return -width / 2.f;
    case TextAnchor::End:
        if(direction == Direction::Ltr)
            return -width;
        return 0.f;
    default:
        assert(false);
    }

    return 0.f;
}

using SVGTextFragmentIterator = SVGTextFragmentList::iterator;

static float calculateTextChunkLength(SVGTextFragmentIterator begin, SVGTextFragmentIterator end, bool isVerticalText)
{
    float chunkLength = 0;
    const SVGTextFragment* lastFragment = nullptr;
    for(auto it = begin; it != end; ++it) {
        const SVGTextFragment& fragment = *it;
        chunkLength += isVerticalText ? fragment.height : fragment.width;
        if(!lastFragment) {
            lastFragment = &fragment;
            continue;
        }

        if(isVerticalText) {
            chunkLength += fragment.y - (lastFragment->y + lastFragment->height);
        } else {
            chunkLength += fragment.x - (lastFragment->x + lastFragment->width);
        }

        lastFragment = &fragment;
    }

    return chunkLength;
}

static void handleTextChunk(SVGTextFragmentIterator begin, SVGTextFragmentIterator end)
{
    const BoxStyle* style = nullptr;
    for(auto it = begin; it != end; ++it) {
        const SVGTextFragment& fragment = *it;
        if(fragment.item.type() == LineItem::Type::BidiControl)
            continue;
        style = fragment.item.box()->style();
        break;
    }

    if(needsTextAnchorAdjustment(style)) {
        auto isVerticalText = style->isVerticalWritingMode();
        auto chunkLength = calculateTextChunkLength(begin, end, isVerticalText);
        auto chunkOffset = calculateTextAnchorOffset(style, chunkLength);
        for(auto it = begin; it != end; ++it) {
            SVGTextFragment& fragment = *it;
            if(isVerticalText) {
                fragment.y += chunkOffset;
            } else {
                fragment.x += chunkOffset;
            }
        }
    }
}

void SVGTextFragmentsBuilder::layout()
{
    for(const auto& item : m_data.items) {
        if(item.type() == LineItem::Type::InlineStart || item.type() == LineItem::Type::InlineEnd)
            continue;
        if(item.type() == LineItem::Type::NormalText) {
            handleTextItem(item);
        } else if(item.type() == LineItem::Type::BidiControl) {
            handleBidiControl(item);
        } else {
            assert(false);
        }
    }

    if(m_fragments.empty())
        return;
    auto it = m_fragments.begin();
    auto begin = m_fragments.begin();
    auto end = m_fragments.end();
    for(++it; it != end; ++it) {
        const SVGTextFragment& fragment = *it;
        if(!fragment.startsNewTextChunk)
            continue;
        handleTextChunk(begin, it);
        begin = it;
    }

    handleTextChunk(begin, it);
}

static float calculateBaselineShift(const BoxStyle* style)
{
    auto baselineShift = style->baselineShift();
    if(baselineShift.type() == BaselineShiftType::Baseline)
        return 0.f;
    if(baselineShift.type() == BaselineShiftType::Sub)
        return -style->fontHeight() / 2.f;
    if(baselineShift.type() == BaselineShiftType::Super)
        return style->fontHeight() / 2.f;
    return baselineShift.length().calc(style->fontSize());
}

static AlignmentBaseline resolveDominantBaseline(const Box* box)
{
    const auto* style = box->style();
    switch(style->dominantBaseline()) {
    case DominantBaseline::Auto:
        if(style->isVerticalWritingMode())
            return AlignmentBaseline::Central;
        return AlignmentBaseline::Alphabetic;
    case DominantBaseline::UseScript:
    case DominantBaseline::NoChange:
    case DominantBaseline::ResetSize:
        return resolveDominantBaseline(box->parentBox());
    case DominantBaseline::Ideographic:
        return AlignmentBaseline::Ideographic;
    case DominantBaseline::Alphabetic:
        return AlignmentBaseline::Alphabetic;
    case DominantBaseline::Hanging:
        return AlignmentBaseline::Hanging;
    case DominantBaseline::Mathematical:
        return AlignmentBaseline::Mathematical;
    case DominantBaseline::Central:
        return AlignmentBaseline::Central;
    case DominantBaseline::Middle:
        return AlignmentBaseline::Middle;
    case DominantBaseline::TextAfterEdge:
        return AlignmentBaseline::TextAfterEdge;
    case DominantBaseline::TextBeforeEdge:
        return AlignmentBaseline::TextBeforeEdge;
    default:
        assert(false);
    }

    return AlignmentBaseline::Auto;
}

static float calculateBaselineOffset(const Box* box)
{
    const auto* style = box->style();
    auto baseline = style->alignmentBaseline();
    if(baseline == AlignmentBaseline::Auto || baseline == AlignmentBaseline::Baseline) {
        baseline = resolveDominantBaseline(box);
    }

    auto baselineShift = calculateBaselineShift(style);
    auto parent = box->parentBox();
    while(parent && (parent->isSVGTSpanBox() || parent->isSVGTextBox())) {
        baselineShift += calculateBaselineShift(parent->style());
        parent = parent->parentBox();
    }

    switch(baseline) {
    case AlignmentBaseline::BeforeEdge:
    case AlignmentBaseline::TextBeforeEdge:
        baselineShift -= style->fontAscent();
        break;
    case AlignmentBaseline::Middle:
        baselineShift -= style->exFontSize() / 2.f;
        break;
    case AlignmentBaseline::Central:
        baselineShift -= (style->fontAscent() - style->fontDescent()) / 2.f;
        break;
    case AlignmentBaseline::AfterEdge:
    case AlignmentBaseline::TextAfterEdge:
    case AlignmentBaseline::Ideographic:
        baselineShift -= -style->fontDescent();
        break;
    case AlignmentBaseline::Hanging:
        baselineShift -= style->fontAscent() * 8.f / 10.f;
        break;
    case AlignmentBaseline::Mathematical:
        baselineShift -= style->fontAscent() / 2.f;
        break;
    default:
        break;
    }

    return baselineShift;
}

void SVGTextFragmentsBuilder::handleTextItem(const LineItem& item)
{
    if(item.length() == 0)
        return;
    const auto* style = item.box()->style();
    const auto isVerticalText = style->isVerticalWritingMode();
    const auto isUprightText = style->isUprightTextOrientation();
    const auto& shape = item.shapeText(m_data);

    SVGTextFragment fragment(item);
    auto recordTextFragment = [&](auto startOffset, auto endOffset) {
        assert(endOffset > startOffset && (startOffset >= item.startOffset() && startOffset <= item.endOffset()));
        if(shape->direction() == Direction::Ltr) {
            fragment.shape = TextShapeView(shape, startOffset - item.startOffset(), endOffset - item.startOffset());
        } else {
            fragment.shape = TextShapeView(shape, item.endOffset() - endOffset, item.endOffset() - startOffset);
        }

        fragment.width = fragment.shape.width();
        fragment.height = style->fontLineSpacing();
        if(isVerticalText) {
            m_y += isUprightText ? fragment.height : fragment.width;
        } else {
            m_x += fragment.width;
        }

        m_fragments.push_back(fragment);
    };

    auto letterSpacing = style->letterSpacing();
    auto wordSpacing = style->wordSpacing();

    auto baselineOffset = calculateBaselineOffset(item.box());
    auto startOffset = item.startOffset();
    auto textOffset = item.startOffset();
    auto didStartTextFragment = false;
    auto applySpacingToNextCharacter = false;
    auto lastCharacter = 0u;
    auto lastAngle = 0.f;
    while(textOffset < item.endOffset()) {
        SVGCharacterPosition position;
        if(m_positions.contains(m_characterOffset)) {
            position = m_positions.at(m_characterOffset);
        }

        auto currentCharacter = m_data.text.char32At(textOffset);
        auto angle = position.rotate.value_or(0);
        auto dx = position.dx.value_or(0);
        auto dy = position.dy.value_or(0);

        auto shouldStartNewFragment = isVerticalText || applySpacingToNextCharacter || position.x || position.y || dx || dy || angle || angle != lastAngle;
        if(shouldStartNewFragment && didStartTextFragment) {
            recordTextFragment(startOffset, textOffset);
            applySpacingToNextCharacter = false;
            startOffset = textOffset;
        }

        auto startsNewTextChunk = (position.x || position.y) && textOffset == item.startOffset();
        if(startsNewTextChunk || shouldStartNewFragment || !didStartTextFragment) {
            m_x = dx + position.x.value_or(m_x);
            m_y = dy + position.y.value_or(m_y);
            fragment.x = isVerticalText ? m_x + baselineOffset : m_x;
            fragment.y = isVerticalText ? m_y : m_y - baselineOffset;
            fragment.angle = angle;
            if(isVerticalText) {
                if(isUprightText) {
                    fragment.y += style->fontHeight();
                } else {
                    fragment.angle += 90.f;
                }
            }

            fragment.startsNewTextChunk = startsNewTextChunk;
            didStartTextFragment = true;
        }

        auto spacing = letterSpacing;
        if(currentCharacter && lastCharacter && wordSpacing) {
            if(treatAsSpace(currentCharacter) && !treatAsSpace(lastCharacter)) {
                spacing += wordSpacing;
            }
        }

        if(spacing) {
            applySpacingToNextCharacter = true;
            if(isVerticalText) {
                m_y += spacing;
            } else {
                m_x += spacing;
            }
        }

        lastAngle = angle;
        lastCharacter = currentCharacter;
        ++textOffset;
        ++m_characterOffset;
    }

    recordTextFragment(startOffset, textOffset);
}

void SVGTextFragmentsBuilder::handleBidiControl(const LineItem& item)
{
    assert(item.length() == 1);
    if(m_positions.contains(m_characterOffset)) {
        const auto& position = m_positions.at(m_characterOffset);
        m_x = position.x.value_or(m_x) + position.dx.value_or(0);
        m_y = position.y.value_or(m_y) + position.dy.value_or(0);
        if(position.x || position.y) {
            SVGTextFragment fragment(item);
            fragment.startsNewTextChunk = true;
            fragment.x = m_x;
            fragment.y = m_y;
            m_fragments.push_back(fragment);
        }
    }

    m_characterOffset += item.length();
}

SVGLineLayout::SVGLineLayout(SVGTextBox* block)
    : m_block(block)
    , m_fragments(block->heap())
    , m_data(block->heap())
{
}

Rect SVGLineLayout::boundingRect() const
{
    Rect boundingRect = Rect::Invalid;
    for(const auto& fragment : m_fragments) {
        if(fragment.item.type() == LineItem::Type::BidiControl)
            continue;
        assert(fragment.item.type() == LineItem::Type::NormalText);
        auto style = fragment.item.box()->style();
        auto fragmentRect = Rect(fragment.x, fragment.y - style->fontAscent(), fragment.width, style->fontHeight());
        auto fragmentTranform = Transform::makeRotate(fragment.angle, fragment.x, fragment.y);
        boundingRect.unite(fragmentTranform.mapRect(fragmentRect));
    }

    if(!boundingRect.isValid())
        boundingRect = Rect::Empty;
    return boundingRect;
}

static void paintTextDecoration(GraphicsContext& context, const Point& origin, float width, float thickness)
{
    float x1 = origin.x;
    float y1 = origin.y;
    float x2 = origin.x + width;
    float y2 = origin.y;

    Path path;
    path.moveTo(x1, y1);
    path.lineTo(x2, y2);
    context.strokePath(path, StrokeData(thickness));
}

static void paintTextDecorations(GraphicsContext& context, const Point& offset, float width, const BoxStyle* style)
{
    auto decorations = style->textDecorationLine();
    if(decorations == TextDecorationLine::None)
        return;
    auto baseline = style->fontAscent();
    auto thickness = style->fontSize() / 16.f;
    if(decorations & TextDecorationLine::Underline) {
        auto gap = std::max(1.f, std::ceil(thickness / 2.f));
        Point origin(offset.x, offset.y + baseline + gap);
        paintTextDecoration(context, origin, width, thickness);
    }

    if(decorations & TextDecorationLine::Overline)
        paintTextDecoration(context, offset, width, thickness);
    if(decorations & TextDecorationLine::LineThrough) {
        Point origin(offset.x, offset.y + 2.f * baseline / 3.f);
        paintTextDecoration(context, origin, width, thickness);
    }
}

void SVGLineLayout::render(const SVGRenderState& state) const
{
    for(const auto& fragment : m_fragments) {
        if(fragment.item.type() == LineItem::Type::BidiControl)
            continue;
        assert(fragment.item.type() == LineItem::Type::NormalText);
        auto style = fragment.item.box()->style();
        Point offset(fragment.x, fragment.y - style->fontAscent());
        Point origin(fragment.x, fragment.y);

        state->save();
        state->translate(origin.x, origin.y);
        state->rotate(fragment.angle);
        state->translate(-origin.x, -origin.y);

        auto parent = fragment.item.box()->parentBox();
        if(state.mode() == SVGRenderMode::Painting && parent->isSVGTSpanBox()) {
            to<SVGTSpanBox>(*parent).fill().applyPaint(state);
        }

        fragment.shape.draw(*state, origin, 0.f);
        paintTextDecorations(*state, offset, fragment.width, style);
        state->restore();
    }
}

static void fillCharacterPositions(const SVGTextPosition& position, SVGCharacterPositions& characterPositions)
{
    const auto& xList = position.element->x().values();
    const auto& yList = position.element->y().values();
    const auto& dxList = position.element->dx().values();
    const auto& dyList = position.element->dy().values();
    const auto& rotateList = position.element->rotate().values();

    auto xListSize = xList.size();
    auto yListSize = yList.size();
    auto dxListSize = dxList.size();
    auto dyListSize = dyList.size();
    auto rotateListSize = rotateList.size();
    if(!xListSize && !yListSize && !dxListSize && !dyListSize && !rotateListSize) {
        return;
    }

    SVGLengthContext lengthContext(position.element);
    std::optional<float> lastRotation;
    for(auto offset = position.startOffset; offset < position.endOffset; ++offset) {
        auto index = offset - position.startOffset;
        if(index >= xListSize && index >= yListSize && index >= dxListSize && index >= dyListSize && index >= rotateListSize)
            break;
        auto& characterPosition = characterPositions[offset];
        if(index < xListSize)
            characterPosition.x = lengthContext.valueForLength(xList[index]);
        if(index < yListSize)
            characterPosition.y = lengthContext.valueForLength(yList[index]);
        if(index < dxListSize)
            characterPosition.dx = lengthContext.valueForLength(dxList[index]);
        if(index < dyListSize)
            characterPosition.dy = lengthContext.valueForLength(dyList[index]);
        if(index < rotateListSize) {
            characterPosition.rotate = rotateList[index];
            lastRotation = characterPosition.rotate;
        }
    }

    if(lastRotation == std::nullopt)
        return;
    auto offset = position.startOffset + rotateList.size();
    while(offset < position.endOffset) {
        characterPositions[offset++].rotate = lastRotation;
    }
}

void SVGLineLayout::layout()
{
    auto element = to<SVGTextBox>(*m_block).element();
    SVGTextPosition wholePosition(element, 0, m_data.text.length());
    SVGCharacterPositions characterPositions;

    fillCharacterPositions(wholePosition, characterPositions);
    for(const auto& position : m_textPositions) {
        fillCharacterPositions(position, characterPositions);
    }

    SVGTextFragmentsBuilder(m_fragments, m_data, characterPositions).layout();
}

void SVGLineLayout::build()
{
    SVGLineItemsBuilder builder(m_data, m_textPositions);
    builder.enterBlock(m_block);
    auto child = m_block->firstChild();
    while(child) {
        if(auto box = to<SVGInlineTextBox>(child)) {
            builder.appendText(box, box->text());
        } else if(auto box = to<SVGTSpanBox>(child)) {
            builder.enterInline(box);
            if(child->firstChild()) {
                child = child->firstChild();
                continue;
            }

            builder.exitInline(box);
        }

        while(true) {
            if(child->nextSibling()) {
                child = child->nextSibling();
                break;
            }

            child = child->parentBox();
            if(child == m_block) {
                child = nullptr;
                break;
            }

            assert(child->isSVGTSpanBox());
            builder.exitInline(child);
        }
    }

    builder.exitBlock(m_block);
    if(m_data.isBidiEnabled && !m_data.items.empty()) {
        std::vector<UBiDiLevel> levels;
        levels.reserve(m_data.items.size());
        for(const auto& item : m_data.items) {
            levels.push_back(item.bidiLevel());
        }

        std::vector<int32_t> indices(levels.size());
        BidiParagraph::reorderVisual(levels, indices);

        LineItems visualItems(m_block->heap());
        visualItems.reserve(indices.size());
        for(auto index : indices)
            visualItems.push_back(std::move(m_data.items[index]));
        assert(visualItems.size() == m_data.items.size());
        m_data.items.swap(visualItems);
    }
}

} // namespace plutobook
