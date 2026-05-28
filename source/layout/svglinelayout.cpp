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
    if(box->isSVGTSpanBox()) {
        auto element = to<SVGTSpanBox>(*box).element();
        m_textPositions.emplace_back(element, m_data.text.length(), m_data.text.length());
        m_itemIndex = m_textPositions.size();
    }
}

void SVGLineItemsBuilder::exitInline(Box* box)
{
    if(box->isSVGTSpanBox()) {
        auto element = to<SVGTSpanBox>(*box).element();
        auto& position = m_textPositions[--m_itemIndex];
        assert(position.element == element);
        position.endOffset = m_data.text.length();
    }

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
    const SVGTextFragment* textFragment = nullptr;
    for(; begin != end; ++begin) {
        const SVGTextFragment& fragment = *begin;
        if(fragment.element && !fragment.inTextPath) {
            textFragment = &fragment;
            break;
        }
    }

    if(textFragment == nullptr)
        return;
    const auto* style = textFragment->element->style();
    const auto isVerticalText = style->isVerticalWritingMode();
    if(textFragment->element->hasAttribute(textLengthAttr)) {
        SVGLengthContext lengthContext(textFragment->element);
        auto textLength = lengthContext.valueForLength(textFragment->element->textLength());
        auto chunkLength = calculateTextChunkLength(begin, end, isVerticalText);
        if(textLength > 0.f && chunkLength > 0.f) {
            size_t numCharacters = 0;
            for(auto it = begin; it != end; ++it) {
                const SVGTextFragment& fragment = *it;
                numCharacters += fragment.shape.length();
            }

            if(textFragment->element->lengthAdjust() == SVGLengthAdjustSpacingAndGlyphs) {
                auto textLengthScale = textLength / chunkLength;
                auto lengthAdjustTransform = Transform::makeTranslate(textFragment->x, textFragment->y);
                if(isVerticalText) {
                    lengthAdjustTransform.scale(1.f, textLengthScale);
                } else {
                    lengthAdjustTransform.scale(textLengthScale, 1.f);
                }

                lengthAdjustTransform.translate(-textFragment->x, -textFragment->y);
                for(auto it = begin; it != end; ++it) {
                    SVGTextFragment& fragment = *it;
                    fragment.lengthAdjustTransform = lengthAdjustTransform;
                }
            } else if(numCharacters > 1) {
                assert(textFragment->element->lengthAdjust() == SVGLengthAdjustSpacing);
                size_t characterOffset = 0;
                auto textLengthShift = (textLength - chunkLength) / (numCharacters - 1);
                for(auto it = begin; it != end; ++it) {
                    SVGTextFragment& fragment = *it;
                    if(isVerticalText) {
                        fragment.y += textLengthShift * characterOffset;
                    } else {
                        fragment.x += textLengthShift * characterOffset;
                    }

                    characterOffset += fragment.shape.length();
                }
            }
        }
    }

    if(needsTextAnchorAdjustment(style)) {
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
        if(item.type() == LineItem::Type::InlineStart) {
            handleInlineStart(item);
        } else if(item.type() == LineItem::Type::InlineEnd) {
            handleInlineEnd(item);
        } else if(item.type() == LineItem::Type::NormalText) {
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

void SVGTextFragmentsBuilder::handleInlineStart(const LineItem& item)
{
    auto box = to<SVGTextPathBox>(item.box());
    if(box == nullptr)
        return;
    m_textPath = box->textPath();
    m_textPathLength = m_textPath.length();

    const auto& startOffset = box->element()->startOffset();
    if(startOffset.type() == SVGLengthType::Percentage) {
        m_textPathStartOffset = m_textPathLength * (startOffset.value() / 100.f);
    } else {
        m_textPathStartOffset = startOffset.value();
    }

    const auto* style = box->style();
    if(needsTextAnchorAdjustment(style)) {
        float chunkLength = 0;
        const auto* it = &item + 1;
        while(box != it->box()) {
            if(it->type() == LineItem::Type::NormalText && it->length()) {
                const auto& shape = it->shapeText(m_data);
                chunkLength += shape->width();
            }

            ++it;
        }

        m_textPathStartOffset += calculateTextAnchorOffset(style, chunkLength);
    }

    m_textPathCurrentOffset = m_textPathStartOffset;
}

void SVGTextFragmentsBuilder::handleInlineEnd(const LineItem& item)
{
    auto box = to<SVGTextPathBox>(item.box());
    if(box == nullptr)
        return;
    m_textPath.clear();
    m_textPathLength = 0.f;
    m_textPathStartOffset = 0.f;
    m_textPathCurrentOffset = 0.f;
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

static AlignmentBaseline resolveDominantBaseline(const BoxStyle* style)
{
    switch(style->dominantBaseline()) {
    case DominantBaseline::Auto:
        if(style->isVerticalWritingMode())
            return AlignmentBaseline::Central;
        return AlignmentBaseline::Alphabetic;
    case DominantBaseline::UseScript:
    case DominantBaseline::NoChange:
    case DominantBaseline::ResetSize:
        return resolveDominantBaseline(style->parentStyle());
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
    const auto* parent = box->parentBox();

    auto baselineShift = calculateBaselineShift(style);
    while(parent->isSVGInlineBox() || parent->isSVGTextBox()) {
        baselineShift += calculateBaselineShift(parent->style());
        parent = parent->parentBox();
    }

    auto baseline = style->alignmentBaseline();
    if(baseline == AlignmentBaseline::Auto || baseline == AlignmentBaseline::Baseline) {
        baseline = resolveDominantBaseline(style);
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

static const SVGTextContentElement* elementForTextItem(const LineItem& item)
{
    const auto* box = item.box();
    assert(box && box->isSVGInlineTextBox());
    const auto* parent = box->parentBox();
    assert(parent->isSVGInlineBox() || parent->isSVGTextBox());
    return static_cast<SVGTextContentElement*>(parent->node());
}

void SVGTextFragmentsBuilder::handleTextItem(const LineItem& item)
{
    if(item.length() == 0)
        return;
    const auto& shape = item.shapeText(m_data);
    const auto* element = elementForTextItem(item);
    const auto* style = element->style();

    const auto lineHeight = style->fontLineHeight();
    const auto isVerticalText = style->isVerticalWritingMode();
    const auto isUprightText = style->isUprightTextOrientation();
    const auto isTextOnPath = !m_textPath.isEmpty();

    auto recordTextFragment = [&](SVGTextFragment& fragment, uint32_t startOffset, uint32_t endOffset) {
        assert(endOffset > startOffset && (startOffset >= item.startOffset() && startOffset <= item.endOffset()));
        if(shape->direction() == Direction::Ltr) {
            fragment.shape = TextShapeView(shape, startOffset - item.startOffset(), endOffset - item.startOffset());
        } else {
            fragment.shape = TextShapeView(shape, item.endOffset() - endOffset, item.endOffset() - startOffset);
        }

        fragment.inTextPath = isTextOnPath;
        fragment.width = fragment.shape.width();
        fragment.height = lineHeight;
        if(isVerticalText) {
            auto advance = isUprightText ? fragment.height : fragment.width;
            if(isTextOnPath)
                m_textPathCurrentOffset += advance;
            m_y += advance;
        } else {
            if(isTextOnPath)
                m_textPathCurrentOffset += fragment.width;
            m_x += fragment.width;
        }

        m_fragments.push_back(fragment);
    };

    auto needsTextLengthSpacing = element->lengthAdjust() == SVGLengthAdjustSpacing && element->hasAttribute(textLengthAttr);
    auto letterSpacing = style->letterSpacing();
    auto wordSpacing = style->wordSpacing();

    auto baselineOffset = calculateBaselineOffset(element->box());
    auto startOffset = item.startOffset();
    auto textOffset = item.startOffset();
    auto didStartTextFragment = false;
    auto applySpacingToNextCharacter = false;
    auto lastCharacter = 0u;
    auto lastAngle = 0.f;

    SVGTextFragment fragment(element);
    while(textOffset < item.endOffset()) {
        SVGCharacterPosition position;
        if(m_positions.contains(m_characterOffset)) {
            position = m_positions.at(m_characterOffset);
        }

        auto currentCharacter = m_data.text.char32At(textOffset);
        auto angle = position.rotate.value_or(0);
        auto dx = position.dx.value_or(0);
        auto dy = position.dy.value_or(0);
        if(!isTextOnPath) {
            m_dx = dx;
            m_dy = dy;
        } else {
            if(isVerticalText) {
                m_dx += dx;
                m_dy = dy;
            } else {
                m_dx = dx;
                m_dy += dy;
            }
        }

        auto shouldStartNewFragment = needsTextLengthSpacing || isTextOnPath || isVerticalText || applySpacingToNextCharacter
            || position.x || position.y || dx || dy || angle || angle != lastAngle;
        if(shouldStartNewFragment && didStartTextFragment) {
            recordTextFragment(fragment, startOffset, textOffset);
            applySpacingToNextCharacter = false;
            startOffset = textOffset;
        }

        auto startsNewTextChunk = (position.x || position.y) && textOffset == item.startOffset();
        if(startsNewTextChunk || shouldStartNewFragment || !didStartTextFragment) {
            fragment.xShift = 0;
            fragment.yShift = 0;
            fragment.angle = angle;

            if(isTextOnPath) {
                if(isVerticalText) {
                    fragment.xShift += m_dx + baselineOffset;
                    if(position.y.has_value())
                        m_textPathCurrentOffset = m_textPathStartOffset + position.y.value();
                    m_textPathCurrentOffset += m_dy;
                    m_dy = 0;
                } else {
                    fragment.yShift += m_dy - baselineOffset;
                    if(position.x.has_value())
                        m_textPathCurrentOffset = m_textPathStartOffset + position.x.value();
                    m_textPathCurrentOffset += m_dx;
                    m_dx = 0;
                }

                if(m_textPathCurrentOffset > m_textPathLength)
                    return;
                auto pointAndAngle = m_textPath.pointAndNormalAtLength(m_textPathCurrentOffset);
                fragment.x = m_x = pointAndAngle.first.x;
                fragment.y = m_y = pointAndAngle.first.y;
                fragment.angle += pointAndAngle.second;
            } else {
                fragment.x = m_x = m_dx + position.x.value_or(m_x);
                fragment.y = m_y = m_dy + position.y.value_or(m_y);
                if(isVerticalText) {
                    fragment.x += baselineOffset;
                } else {
                    fragment.y -= baselineOffset;
                }
            }

            if(isVerticalText) {
                if(isUprightText) {
                    fragment.y += style->fontHeight();
                } else {
                    fragment.angle += isTextOnPath ? -90.f : 90.f;
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
            m_textPathCurrentOffset += spacing;
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

    recordTextFragment(fragment, startOffset, textOffset);
}

void SVGTextFragmentsBuilder::handleBidiControl(const LineItem& item)
{
    assert(item.length() == 1);
    if(m_positions.contains(m_characterOffset)) {
        const auto& position = m_positions.at(m_characterOffset);
        m_x = position.x.value_or(m_x) + position.dx.value_or(0);
        m_y = position.y.value_or(m_y) + position.dy.value_or(0);
        if(position.x || position.y) {
            SVGTextFragment fragment(nullptr);
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

Rect SVGLineLayout::boundingRect(bool includeStroke) const
{
    Rect boundingRect = Rect::Invalid;
    for(const auto& fragment : m_fragments) {
        if(fragment.element == nullptr)
            continue;
        auto style = fragment.element->style();
        auto origin = Point(fragment.x + fragment.xShift, fragment.y + fragment.yShift);
        auto fragmentRect = Rect(origin.x, origin.y - style->fontAscent(), fragment.width, style->fontHeight());
        auto fragmentTranform = Transform::makeRotate(fragment.angle, fragment.x, fragment.y) * fragment.lengthAdjustTransform;
        if(includeStroke && style->hasStroke()) {
            SVGLengthContext lengthContext(fragment.element);
            auto strokeWidth = lengthContext.valueForLength(style->strokeWidth());
            fragmentRect.inflate(strokeWidth / 2.f);
        }

        boundingRect.unite(fragmentTranform.mapRect(fragmentRect));
    }

    if(!boundingRect.isValid())
        boundingRect = Rect::Empty;
    return boundingRect;
}

static void paintSVGTextDecoration(GraphicsContext& context, const Point& offset, float width, float thickness)
{
    context.fillRect(Rect(offset, Size(width, thickness)));
}

static void paintSVGTextDecorations(GraphicsContext& context, const Point& origin, float width, const BoxStyle* style)
{
    auto decorations = style->textDecorationLine();
    if(decorations == TextDecorationLine::None)
        return;
    auto baseline = style->fontAscent();
    auto thickness = style->fontSize() / 16.f;
    if(decorations & TextDecorationLine::Underline) {
        auto gap = std::max(1.f, std::ceil(thickness / 2.f));
        Point offset(origin.x, origin.y + gap);
        paintSVGTextDecoration(context, offset, width, thickness);
    }

    if(decorations & TextDecorationLine::Overline) {
        Point offset(origin.x, origin.y - baseline);
        paintSVGTextDecoration(context, offset, width, thickness);
    }

    if(decorations & TextDecorationLine::LineThrough) {
        Point offset(origin.x, origin.y - baseline / 3.f);
        paintSVGTextDecoration(context, offset, width, thickness);
    }
}

static void paintTextFragment(const SVGRenderState& state, const SVGTextFragment& fragment, const SVGPaintServer& fill, const SVGPaintServer& stroke)
{
    const auto* style = fragment.element->style();
    if(style->visibility() != Visibility::Visible)
        return;
    auto origin = Point(fragment.x + fragment.xShift, fragment.y + fragment.yShift);
    auto transform = Transform::makeRotate(fragment.angle, fragment.x, fragment.y) * fragment.lengthAdjustTransform;

    state->save();
    state->addTransform(transform);

    if(state.mode() == SVGRenderMode::Clipping) {
        state->setColor(Color::White);
        fragment.shape.draw(*state, origin, 0.f, false);
    } else {
        if(fill.applyPaint(state)) {
            fragment.shape.draw(*state, origin, 0.f, false);
            paintSVGTextDecorations(*state, origin, fragment.width, style);
        }

        if(stroke.applyPaint(state)) {
            auto strokeData = fragment.element->getStrokeData(style);
            strokeData.apply(state.context());
            fragment.shape.draw(*state, origin, 0.f, true);
        }
    }

    state->restore();
}

void SVGLineLayout::render(const SVGRenderState& state) const
{
    for(const auto& fragment : m_fragments) {
        if(fragment.element == nullptr)
            continue;
        if(auto box = to<SVGInlineBox>(fragment.element->box())) {
            paintTextFragment(state, fragment, box->fill(), box->stroke());
        } else {
            paintTextFragment(state, fragment, m_block->fill(), m_block->stroke());
        }
    }
}

static void fillCharacterPositions(const SVGTextPosition& position, SVGCharacterPositions& characterPositions)
{
    const auto& xList = position.element->x().values();
    const auto& yList = position.element->y().values();
    const auto& dxList = position.element->dx().values();
    const auto& dyList = position.element->dy().values();
    const auto& rotateList = position.element->rotate().values();

    const auto xListSize = xList.size();
    const auto yListSize = yList.size();
    const auto dxListSize = dxList.size();
    const auto dyListSize = dyList.size();
    const auto rotateListSize = rotateList.size();
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
        } else if(auto box = to<SVGInlineBox>(child)) {
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

            assert(child->isSVGInlineBox());
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
