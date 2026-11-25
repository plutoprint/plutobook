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
    }
}

void SVGLineItemsBuilder::exitInline(Box* box)
{
    if(box->isSVGTSpanBox()) {
        auto element = to<SVGTSpanBox>(*box).element();
        auto itemIndex = m_textPositions.size();
        do {
            auto& position = m_textPositions[--itemIndex];
            if(position.element == element) {
                position.endOffset = m_data.text.length();
                break;
            }
        } while(itemIndex);
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

static float calculateTextFragmentAdvance(const SVGTextFragment& fragment, bool isVerticalText)
{
    if(isVerticalText && fragment.element->style()->isUprightTextOrientation())
        return fragment.height;
    return fragment.width;
}

using SVGTextFragmentIterator = SVGTextFragmentList::iterator;

static float calculateTextChunkLength(SVGTextFragmentIterator begin, SVGTextFragmentIterator end, bool isVerticalText)
{
    float chunkLength = 0;
    const SVGTextFragment* lastFragment = nullptr;
    for(auto it = begin; it != end; ++it) {
        const SVGTextFragment& fragment = *it;
        if(fragment.inTextPath)
            continue;
        chunkLength += calculateTextFragmentAdvance(fragment, isVerticalText);
        if(!lastFragment) {
            lastFragment = &fragment;
            continue;
        }

        auto advance = calculateTextFragmentAdvance(*lastFragment, isVerticalText);
        if(isVerticalText) {
            chunkLength += fragment.y - (lastFragment->y + advance);
        } else {
            chunkLength += fragment.x - (lastFragment->x + advance);
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
        if(!fragment.inTextPath) {
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
                if(!fragment.inTextPath) {
                    numCharacters += fragment.shape.length();
                }
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
                    if(!fragment.inTextPath) {
                        fragment.lengthAdjustTransform = lengthAdjustTransform;
                    }
                }
            } else if(numCharacters > 1) {
                assert(textFragment->element->lengthAdjust() == SVGLengthAdjustSpacing);
                size_t characterOffset = 0;
                auto textLengthShift = (textLength - chunkLength) / (numCharacters - 1);
                for(auto it = begin; it != end; ++it) {
                    SVGTextFragment& fragment = *it;
                    if(fragment.inTextPath)
                        continue;
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
            if(fragment.inTextPath)
                continue;
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
        SVGLengthContext lengthContext(box->element());
        m_textPathStartOffset = lengthContext.valueForLength(startOffset);
    }

    float chunkLength = 0;
    uint32_t numCharacters = 0;
    const auto* it = &item;
    for(++it; box != it->box(); ++it) {
        if(it->type() == LineItem::Type::NormalText && it->length()) {
            const auto& shape = it->shapeText(m_data);
            chunkLength += shape->width();
            numCharacters += it->length();
        }
    }

    const auto* element = box->element();
    if(element->hasAttribute(textLengthAttr)) {
        SVGLengthContext lengthContext(element);
        auto textLength = lengthContext.valueForLength(element->textLength());
        if(textLength > 0.f && chunkLength > 0.f) {
            if(element->lengthAdjust() == SVGLengthAdjustSpacingAndGlyphs) {
                m_textPathScaling = textLength / chunkLength;
                chunkLength = textLength;
            } else if(numCharacters > 1) {
                m_textPathSpacing = (textLength - chunkLength) / (numCharacters - 1);
                chunkLength = textLength;
            }
        }
    }

    const auto* style = box->style();
    if(needsTextAnchorAdjustment(style)) {
        m_textPathStartOffset += calculateTextAnchorOffset(style, chunkLength);
    }

    m_textPathCurrentOffset = m_textPathStartOffset;
    m_isTextOnPath = true;
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
    m_textPathScaling = 1.f;
    m_textPathSpacing = 0.f;
    m_isTextOnPath = false;
}

static float calculateBaselineShift(const BoxStyle* style)
{
    auto baselineShift = style->baselineShift();
    if(baselineShift.type() == BaselineShift::Type::Baseline)
        return 0.f;
    if(baselineShift.type() == BaselineShift::Type::Sub)
        return -style->fontHeight() / 2.f;
    if(baselineShift.type() == BaselineShift::Type::Super)
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

static bool isTextLengthSpacingInEffect(const Box* box)
{
    while(box->isSVGInlineBox() || box->isSVGTextBox()) {
        const auto* element = static_cast<const SVGTextContentElement*>(box->node());
        if(element->lengthAdjust() == SVGLengthAdjustSpacing && element->hasAttribute(textLengthAttr))
            return true;
        box = box->parentBox();
    }

    return false;
}

void SVGTextFragmentsBuilder::handleTextItem(const LineItem& item)
{
    if(item.length() == 0)
        return;
    const auto* element = elementForTextItem(item);
    const auto* style = element->style();

    m_isVerticalText = style->isVerticalWritingMode();
    m_isUprightText = style->isUprightTextOrientation();

    auto needsTextLengthSpacing = isTextLengthSpacingInEffect(element->box());
    auto letterSpacing = style->letterSpacing();
    auto wordSpacing = style->wordSpacing();

    auto baselineOffset = calculateBaselineOffset(element->box());
    auto startOffset = item.startOffset();
    auto textOffset = item.startOffset();
    auto didStartTextFragment = false;
    auto applySpacingToNextCharacter = false;
    auto lastAngle = 0.f;

    SVGTextFragment fragment(element);
    while(textOffset < item.endOffset()) {
        auto position = characterPositionAt(m_characterOffset);
        auto currentCharacter = m_data.text.char32At(textOffset);
        auto angle = position.rotate.value_or(0);
        auto dx = position.dx.value_or(0);
        auto dy = position.dy.value_or(0);
        updateCharacterShift(dx, dy);

        auto shouldStartNewFragment = needsTextLengthSpacing || m_isTextOnPath || m_isVerticalText || applySpacingToNextCharacter
            || position.x || position.y || dx || dy || angle || angle != lastAngle;
        if(shouldStartNewFragment && didStartTextFragment) {
            recordTextFragment(fragment, item, startOffset, textOffset);
            applySpacingToNextCharacter = false;
            startOffset = textOffset;
        }

        if(shouldStartNewFragment || !didStartTextFragment) {
            startTextFragment(fragment, position, baselineOffset);
            didStartTextFragment = true;
        }

        auto spacing = letterSpacing;
        if(wordSpacing && treatAsSpace(currentCharacter)) {
            spacing += wordSpacing;
        }

        if(spacing) {
            applyCharacterSpacing(spacing);
            applySpacingToNextCharacter = true;
        }

        lastAngle = angle;
        ++textOffset;
        ++m_characterOffset;
    }

    recordTextFragment(fragment, item, startOffset, textOffset);
}

SVGCharacterPosition SVGTextFragmentsBuilder::characterPositionAt(uint32_t characterOffset) const
{
    auto it = m_positions.find(characterOffset);
    if(it == m_positions.end())
        return SVGCharacterPosition();
    return it->second;
}

void SVGTextFragmentsBuilder::updateCharacterShift(float dx, float dy)
{
    if(!m_isTextOnPath) {
        m_dx = dx;
        m_dy = dy;
    } else if(m_isVerticalText) {
        m_dx += dx;
        m_dy = dy;
    } else {
        m_dx = dx;
        m_dy += dy;
    }
}

void SVGTextFragmentsBuilder::startTextFragment(SVGTextFragment& fragment, const SVGCharacterPosition& position, float baselineOffset)
{
    fragment.xShift = 0;
    fragment.yShift = 0;
    fragment.angle = position.rotate.value_or(0);
    if(m_isTextOnPath) {
        placeTextFragmentOnPath(fragment, position, baselineOffset);
        if(m_isVerticalText && m_isUprightText) {
            fragment.yShift += fragment.element->style()->fontHeight();
            fragment.angle -= 90.f;
        }
    } else {
        placeTextFragment(fragment, position, baselineOffset);
        if(m_isVerticalText) {
            if(m_isUprightText) {
                fragment.yShift += fragment.element->style()->fontHeight();
            } else {
                fragment.angle += 90.f;
            }
        }
    }

    fragment.inTextPath = m_isTextOnPath;
    fragment.startsNewTextChunk = position.x || position.y;
}

void SVGTextFragmentsBuilder::placeTextFragmentOnPath(SVGTextFragment& fragment, const SVGCharacterPosition& position, float baselineOffset)
{
    if(m_isVerticalText) {
        if(m_isUprightText) {
            fragment.xShift += m_dx + baselineOffset;
        } else {
            fragment.yShift -= m_dx + baselineOffset;
        }

        if(position.y.has_value())
            m_textPathCurrentOffset = m_textPathStartOffset + position.y.value();
        m_textPathCurrentOffset += m_dy;
    } else {
        fragment.yShift += m_dy - baselineOffset;
        if(position.x.has_value())
            m_textPathCurrentOffset = m_textPathStartOffset + position.x.value();
        m_textPathCurrentOffset += m_dx;
    }

    m_textPathFragmentOffset = m_textPathCurrentOffset;
    if(m_textPathCurrentOffset > m_textPathLength)
        return;
    auto [point, angle] = m_textPath.pointAndNormalAtLength(m_textPathCurrentOffset);
    fragment.x = m_x = point.x;
    fragment.y = m_y = point.y;
    fragment.angle += angle;
}

void SVGTextFragmentsBuilder::placeTextFragment(SVGTextFragment& fragment, const SVGCharacterPosition& position, float baselineOffset)
{
    fragment.x = m_x = m_dx + position.x.value_or(m_x);
    fragment.y = m_y = m_dy + position.y.value_or(m_y);
    if(m_isVerticalText) {
        fragment.x += baselineOffset;
    } else {
        fragment.y -= baselineOffset;
    }
}

void SVGTextFragmentsBuilder::recordTextFragment(SVGTextFragment& fragment, const LineItem& item, uint32_t startOffset, uint32_t endOffset)
{
    assert(endOffset > startOffset && (startOffset >= item.startOffset() && startOffset <= item.endOffset()));
    const auto& shape = item.shapeText(m_data);
    if(shape->direction() == Direction::Ltr) {
        fragment.shape = TextShapeView(shape, startOffset - item.startOffset(), endOffset - item.startOffset());
    } else {
        fragment.shape = TextShapeView(shape, item.endOffset() - endOffset, item.endOffset() - startOffset);
    }

    fragment.width = fragment.shape.width();
    fragment.height = fragment.element->style()->fontLineHeight();

    if(m_isVerticalText) {
        m_y += m_isUprightText ? fragment.height : fragment.width;
    } else {
        m_x += fragment.width;
    }

    if(m_isTextOnPath) {
        const auto advance = (m_isVerticalText && m_isUprightText) ? fragment.height : fragment.width;
        m_textPathCurrentOffset += advance * m_textPathScaling;
        const auto midpoint = (m_textPathFragmentOffset + m_textPathCurrentOffset) / 2.f;
        m_textPathCurrentOffset += m_textPathSpacing;
        if(m_textPathFragmentOffset > m_textPathLength || midpoint < 0.f || midpoint > m_textPathLength) {
            return;
        }

        if(m_textPathScaling != 1.f) {
            auto lengthAdjustTransform = Transform::makeTranslate(fragment.x, fragment.y);
            if(m_isVerticalText && m_isUprightText) {
                lengthAdjustTransform.scale(1.f, m_textPathScaling);
            } else {
                lengthAdjustTransform.scale(m_textPathScaling, 1.f);
            }

            lengthAdjustTransform.translate(-fragment.x, -fragment.y);
            fragment.lengthAdjustTransform = lengthAdjustTransform;
        }
    }

    m_fragments.push_back(fragment);
}

void SVGTextFragmentsBuilder::applyCharacterSpacing(float spacing)
{
    m_textPathCurrentOffset += spacing;
    if(m_isVerticalText) {
        m_y += spacing;
    } else {
        m_x += spacing;
    }
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
        auto style = fragment.element->style();
        auto origin = Point(fragment.x + fragment.xShift, fragment.y + fragment.yShift);
        auto fragmentRect = Rect(origin.x, origin.y - style->fontAscent(), fragment.width, style->fontHeight());
        if(includeStroke && style->hasStroke()) {
            SVGLengthContext lengthContext(fragment.element);
            auto strokeWidth = lengthContext.valueForLength(style->strokeWidth());
            fragmentRect.inflate(strokeWidth / 2.f);
        }

        boundingRect.unite(fragment.transform().mapRect(fragmentRect));
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
    if(decorations.isEmpty())
        return;
    auto thickness = style->textDecorationThickness();
    if(thickness <= 0.f)
        return;
    auto baseline = style->fontAscent();
    if(decorations.contains(TextDecorationLine::Underline)) {
        Point offset(origin.x, origin.y + style->underlineDistanceFromBaseline(thickness));
        paintSVGTextDecoration(context, offset, width, thickness);
    }

    if(decorations.contains(TextDecorationLine::Overline)) {
        Point offset(origin.x, origin.y - baseline);
        paintSVGTextDecoration(context, offset, width, thickness);
    }

    if(decorations.contains(TextDecorationLine::LineThrough)) {
        Point offset(origin.x, origin.y - baseline / 3.f);
        paintSVGTextDecoration(context, offset, width, thickness);
    }
}

static void paintTextFragment(const SVGRenderState& state, const SVGTextFragment& fragment, const SVGPaintServer& fill, const SVGPaintServer& stroke)
{
    const auto* style = fragment.element->style();
    if(style->visibility() != Visibility::Visible)
        return;
    state->save();
    state->addTransform(fragment.transform());

    Point origin(fragment.x + fragment.xShift, fragment.y + fragment.yShift);
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

void SVGLineLayout::serialize(std::ostream& o, int indent) const
{
    for(const auto& fragment : m_fragments) {
        for(int i = 0; i < indent; i++) {
            o << ' ';
        }

        o << "<SVGTextFragment";
        o << ':' << fragment.element->tagName();
        if(fragment.element->hasID())
            o << '#' << fragment.element->id();
        o << " rect=\'";
        o << fragment.x + fragment.xShift;
        o << ' ';
        o << fragment.y + fragment.yShift;
        o << ' ';
        o << fragment.width;
        o << ' ';
        o << fragment.height;
        o << '\'';
        if(fragment.angle) {
            o << " angle=\'";
            o << fragment.angle;
            o << '\'';
        }

        o << '>';
        fragment.shape.serialize(o);
        o << "</SVGTextFragment>\n";
    }
}

} // namespace plutobook
