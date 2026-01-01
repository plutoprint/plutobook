/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "textshape.h"
#include "fontresource.h"
#include "graphicscontext.h"
#include "geometry.h"
#include "textbreakiterator.h"

#include <algorithm>

#include <unicode/uchar.h>
#include <unicode/uscript.h>
#include <cairo-ft.h>
#include <hb-ft.h>

namespace plutobook {

TextShapeRunGlyphDataList::TextShapeRunGlyphDataList(Heap* heap, size_t size)
    : m_data(new (heap) TextShapeRunGlyphData[size])
    , m_size(size)
{
}

std::unique_ptr<TextShapeRun> TextShapeRun::create(Heap* heap, const SimpleFontData* fontData, uint32_t offset, uint32_t length, float width, TextShapeRunGlyphDataList glyphs)
{
    return std::unique_ptr<TextShapeRun>(new (heap) TextShapeRun(fontData, offset, length, width, std::move(glyphs)));
}

TextShapeRun::TextShapeRun(const SimpleFontData* fontData, uint32_t offset, uint32_t length, float width, TextShapeRunGlyphDataList glyphs)
    : m_fontData(fontData)
    , m_offset(offset)
    , m_length(length)
    , m_width(width)
    , m_glyphs(std::move(glyphs))
{
}

float TextShapeRun::positionForOffset(uint32_t offset, Direction direction) const
{
    assert(offset <= m_length);
    uint32_t glyphIndex = 0;
    float position = 0;
    const auto numGlyphs = m_glyphs.size();
    if(direction == Direction::Rtl) {
        while(glyphIndex < numGlyphs && m_glyphs[glyphIndex].characterIndex > offset) {
            position += m_glyphs[glyphIndex++].advance;
        }

        if(glyphIndex == numGlyphs || m_glyphs[glyphIndex].characterIndex < offset)
            return position;
        auto characterIndex = m_glyphs[glyphIndex].characterIndex;
        while(glyphIndex < numGlyphs - 1 && characterIndex == m_glyphs[glyphIndex + 1].characterIndex) {
            position += m_glyphs[glyphIndex++].advance;
        }

        position += m_glyphs[glyphIndex].advance;
    } else {
        while(glyphIndex < numGlyphs && m_glyphs[glyphIndex].characterIndex < offset) {
            position += m_glyphs[glyphIndex].advance;
            ++glyphIndex;
        }
    }

    return position;
}

float TextShapeRun::positionForVisualOffset(uint32_t offset, Direction direction) const
{
    assert(offset < m_length);
    if(direction == Direction::Rtl)
        offset = m_length - offset - 1;
    return positionForOffset(offset, direction);
}

uint32_t TextShapeRun::offsetForPosition(float position, Direction direction) const
{
    assert(position >= 0.f && position <= m_width);
    if(position <= 0.f)
        return direction == Direction::Ltr ? 0 : m_length;
    uint32_t glyphIndex = 0;
    float currentPosition = 0.f;
    const auto numGlyphs = m_glyphs.size();
    while(glyphIndex < numGlyphs) {
        currentPosition += m_glyphs[glyphIndex].advance;

        auto characterIndex = m_glyphs[glyphIndex].characterIndex;
        while(glyphIndex < numGlyphs - 1 && characterIndex == m_glyphs[glyphIndex + 1].characterIndex) {
            currentPosition += m_glyphs[++glyphIndex].advance;
        }

        if((direction == Direction::Ltr && position < currentPosition)
            || (direction == Direction::Rtl && position <= currentPosition)) {
            return characterIndex;
        }

        ++glyphIndex;
    }

    return direction == Direction::Rtl ? 0 : m_length;
}

constexpr int kMaxGlyphs = 1 << 16;
constexpr int kMaxCharacters = kMaxGlyphs;

#define HB_TO_FLT(v) (static_cast<float>(v) / (1 << 16))

static bool isEmojiCodepoint(uint32_t codepoint, FontVariantEmoji variantEmoji)
{
    return variantEmoji != FontVariantEmoji::Text && codepoint > 0xFF && u_hasBinaryProperty(codepoint, UCHAR_EMOJI);
}

RefPtr<TextShape> TextShape::createForText(const UString& text, Direction direction, bool disableSpacing, const BoxStyle* style)
{
    const auto& font = style->font();
    auto fontFeatures = style->fontFeatures();
    auto fontVariantEmoji = style->fontVariantEmoji();
    auto letterSpacing = disableSpacing ? 0 : style->letterSpacing();
    auto wordSpacing = disableSpacing ? 0 : style->wordSpacing();
    auto heap = style->heap();

    auto hbBuffer = hb_buffer_create();
    auto hbDirection = direction == Direction::Ltr ? HB_DIRECTION_LTR : HB_DIRECTION_RTL;
    auto textBuffer = (const uint16_t*)(text.getBuffer());

    float totalWidth = 0.f;
    int startIndex = 0;
    int totalLength = text.length();
    TextShapeRunList textRuns(heap);

    CharacterBreakIterator iterator(text);
    while(totalLength > 0) {
        auto errorCode = U_ZERO_ERROR;
        auto character = text.char32At(startIndex);
        auto fontData = font->getFontData(character, isEmojiCodepoint(character, fontVariantEmoji));
        auto scriptCode = uscript_getScript(character, &errorCode);
        if(!fontData || U_FAILURE(errorCode))
            break;
        auto nextIndex = iterator.nextBreakOpportunity(startIndex, totalLength);
        auto endIndex = startIndex + std::min(totalLength, kMaxCharacters);
        for(; nextIndex < endIndex; nextIndex = iterator.nextBreakOpportunity(nextIndex, endIndex)) {
            auto nextCharacter = text.char32At(nextIndex);
            if(treatAsZeroWidthSpace(nextCharacter))
                continue;
            auto nextFontData = font->getFontData(nextCharacter, isEmojiCodepoint(nextCharacter, fontVariantEmoji));
            auto nextScriptCode = uscript_getScript(nextCharacter, &errorCode);
            if(fontData != nextFontData || U_FAILURE(errorCode))
                break;
            if(scriptCode == USCRIPT_INHERITED || scriptCode == USCRIPT_COMMON)
                scriptCode = nextScriptCode;
            if(scriptCode != nextScriptCode && nextScriptCode != USCRIPT_INHERITED && nextScriptCode != USCRIPT_COMMON
                && !uscript_hasScript(nextCharacter, scriptCode)) {
                break;
            }
        }

        assert(nextIndex > startIndex);
        auto numCharacters = nextIndex - startIndex;
        auto scriptName = uscript_getShortName(scriptCode);
        auto hbScript = hb_script_from_string(scriptName, -1);

        std::vector<hb_feature_t> hbFeatures;
        auto addFeatures = [&hbFeatures](const FontFeatureList& features) {
            for(const auto& feature : features) {
                hb_feature_t hbFeature;
                hbFeature.tag = feature.first.value();
                hbFeature.value = feature.second;
                hbFeature.start = 0;
                hbFeature.end = static_cast<unsigned>(-1);
                hbFeatures.push_back(hbFeature);
            }
        };

        addFeatures(fontFeatures);
        addFeatures(fontData->features());

        hb_buffer_reset(hbBuffer);
        hb_buffer_add_utf16(hbBuffer, textBuffer + startIndex, numCharacters, 0, numCharacters);
        hb_buffer_set_direction(hbBuffer, hbDirection);
        hb_buffer_set_script(hbBuffer, hbScript);
        hb_shape(fontData->hbFont(), hbBuffer, hbFeatures.data(), hbFeatures.size());

        auto glyphInfos = hb_buffer_get_glyph_infos(hbBuffer, nullptr);
        auto glyphPositions = hb_buffer_get_glyph_positions(hbBuffer, nullptr);
        auto numGlyphs = hb_buffer_get_length(hbBuffer);

        float width = 0.f;
        TextShapeRunGlyphDataList glyphs(heap, numGlyphs);
        for(size_t index = 0; index < numGlyphs; ++index) {
            const auto& glyphInfo = glyphInfos[index];
            const auto& glyphPosition = glyphPositions[index];

            auto& glyphData = glyphs[index];
            glyphData.glyphIndex = glyphInfo.codepoint;
            glyphData.characterIndex = glyphInfo.cluster;
            glyphData.xOffset = HB_TO_FLT(glyphPosition.x_offset);
            glyphData.yOffset = -HB_TO_FLT(glyphPosition.y_offset);
            glyphData.advance = HB_TO_FLT(glyphPosition.x_advance - glyphPosition.y_advance);

            if(!disableSpacing) {
                auto character = text.charAt(startIndex + glyphData.characterIndex);
                if(letterSpacing && !treatAsZeroWidthSpace(character))
                    glyphData.advance += letterSpacing;
                if(wordSpacing && treatAsSpace(character)) {
                    glyphData.advance += wordSpacing;
                }
            }

            width += glyphData.advance;
        }

        auto textRun = TextShapeRun::create(heap, fontData, startIndex, numCharacters, width, std::move(glyphs));
        totalWidth += width;
        startIndex += numCharacters;
        totalLength -= numCharacters;
        textRuns.push_back(std::move(textRun));
    }

    hb_buffer_destroy(hbBuffer);
    if(direction == Direction::Rtl)
        std::reverse(textRuns.begin(), textRuns.end());
    return adoptPtr(new (heap) TextShape(text, direction, totalWidth, std::move(textRuns)));
}

RefPtr<TextShape> TextShape::createForTabs(const UString& text, Direction direction, const BoxStyle* style)
{
    const auto& font = style->font();
    auto heap = style->heap();

    float totalWidth = 0.f;
    int startIndex = 0;
    int totalLength = text.length();

    TextShapeRunList runs(heap);
    if(auto fontData = font->getFontData(kSpaceCharacter, false)) {
        auto tabWidth = style->tabWidth(fontData->spaceWidth());
        auto spaceGlyph = fontData->spaceGlyph();
        while(totalLength > 0) {
            auto numGlyphs = std::min(totalLength, kMaxGlyphs);
            TextShapeRunGlyphDataList glyphs(heap, numGlyphs);
            for(int index = 0; index < numGlyphs; ++index) {
                assert(text[index + startIndex] == kTabulationCharacter);
                auto& glyphData = glyphs[index];
                glyphData.glyphIndex = spaceGlyph;
                glyphData.characterIndex = direction == Direction::Ltr ? index : numGlyphs - index - 1;
                glyphData.xOffset = 0.f;
                glyphData.yOffset = 0.f;
                glyphData.advance = tabWidth;
            }

            auto run = TextShapeRun::create(heap, fontData, startIndex, numGlyphs, numGlyphs * tabWidth, std::move(glyphs));
            totalWidth += run->width();
            startIndex += numGlyphs;
            totalLength -= numGlyphs;
            runs.push_back(std::move(run));
        }
    }

    return adoptPtr(new (heap) TextShape(text, direction, totalWidth, std::move(runs)));
}

uint32_t TextShape::offsetForPosition(float position) const
{
    auto currentOffset = m_direction == Direction::Ltr ? 0 : m_text.length();
    if(position <= 0.f)
        return currentOffset;
    float currentPosition = 0;
    for(const auto& run : m_runs) {
        if(m_direction == Direction::Rtl)
            currentOffset -= run->length();
        auto runPosition = position - currentPosition;
        if(runPosition >= 0.f && runPosition <= run->width())
            return currentOffset + run->offsetForPosition(runPosition, m_direction);
        if(m_direction == Direction::Ltr)
            currentOffset += run->length();
        currentPosition += run->width();
    }

    return currentOffset;
}

float TextShape::positionForOffset(uint32_t offset) const
{
    auto currentOffset = offset;
    if(m_direction == Direction::Rtl && offset < m_text.length()) {
        currentOffset = m_text.length() - offset - 1;
    }

    float position = 0;
    float currentPosition = 0;
    for(const auto& run : m_runs) {
        if(currentOffset < run->length()) {
            position = currentPosition + run->positionForVisualOffset(currentOffset, m_direction);
            break;
        }

        currentOffset -= run->length();
        currentPosition += run->width();
    }

    if(!position && offset == m_text.length())
        return m_direction == Direction::Rtl ? 0.f : m_width;
    return position;
}

TextShape::~TextShape() = default;

TextShape::TextShape(const UString& text, Direction direction, float width, TextShapeRunList runs)
    : m_text(text)
    , m_direction(direction)
    , m_width(width)
    , m_runs(std::move(runs))
{
}

UString TextShapeView::text() const
{
    if(m_shape)
        return m_shape->text().tempSubString(m_startOffset, length());
    return UString();
}

uint32_t TextShapeView::expansionOpportunityCount() const
{
    if(m_startOffset == m_endOffset)
        return 0;
    uint32_t count = 0;
    auto direction = m_shape->direction();
    const auto& text = m_shape->text();
    for(const auto& run : m_shape->runs()) {
        const auto& glyphs = run->glyphs();
        for(uint32_t glyphIndex = 0; glyphIndex < glyphs.size(); ++glyphIndex) {
            const auto& glyph = glyphs[glyphIndex];
            auto characterIndex = glyph.characterIndex + run->offset();
            if((direction == Direction::Ltr && characterIndex >= m_endOffset)
                || (direction == Direction::Rtl && characterIndex < m_startOffset)) {
                break;
            }

            if((direction == Direction::Ltr && characterIndex >= m_startOffset)
                || (direction == Direction::Rtl && characterIndex < m_endOffset)) {
                auto character = text.charAt(characterIndex);
                if(treatAsSpace(character)) {
                    ++count;
                }
            }
        }
    }

    return count;
}

void TextShapeView::maxAscentAndDescent(float& maxAscent, float& maxDescent) const
{
    if(m_startOffset == m_endOffset)
        return;
    auto direction = m_shape->direction();
    for(const auto& run : m_shape->runs()) {
        const auto& glyphs = run->glyphs();
        for(uint32_t glyphIndex = 0; glyphIndex < glyphs.size(); ++glyphIndex) {
            const auto& glyph = glyphs[glyphIndex];
            auto characterIndex = glyph.characterIndex + run->offset();
            if((direction == Direction::Ltr && characterIndex >= m_endOffset)
                || (direction == Direction::Rtl && characterIndex < m_startOffset)) {
                break;
            }

            if((direction == Direction::Ltr && characterIndex >= m_startOffset)
                || (direction == Direction::Rtl && characterIndex < m_endOffset)) {
                maxAscent = std::max(maxAscent, run->fontData()->ascent());
                maxDescent = std::max(maxDescent, run->fontData()->descent());
            }
        }
    }
}

float TextShapeView::width(float expansion) const
{
    if(m_startOffset == m_endOffset)
        return 0.f;
    float width = 0.f;
    auto direction = m_shape->direction();
    const auto& text = m_shape->text();
    for(const auto& run : m_shape->runs()) {
        const auto& glyphs = run->glyphs();
        for(uint32_t glyphIndex = 0; glyphIndex < glyphs.size(); ++glyphIndex) {
            const auto& glyph = glyphs[glyphIndex];
            auto characterIndex = glyph.characterIndex + run->offset();
            if((direction == Direction::Ltr && characterIndex >= m_endOffset)
                || (direction == Direction::Rtl && characterIndex < m_startOffset)) {
                break;
            }

            if((direction == Direction::Ltr && characterIndex >= m_startOffset)
                || (direction == Direction::Rtl && characterIndex < m_endOffset)) {
                auto character = text.charAt(characterIndex);
                if(expansion && treatAsSpace(character))
                    width += expansion;
                width += glyph.advance;
            }
        }
    }

    return width;
}

float TextShapeView::draw(GraphicsContext& context, const Point& origin, float expansion) const
{
    if(m_startOffset == m_endOffset)
        return 0.f;
    auto canvas = context.canvas();
    auto direction = m_shape->direction();
    auto offset = origin;
    const auto& text = m_shape->text();
    for(const auto& run : m_shape->runs()) {
        const auto& glyphs = run->glyphs();
        auto glyphBuffer = cairo_glyph_allocate(glyphs.size());
        uint32_t numGlyphs = 0;
        for(uint32_t glyphIndex = 0; glyphIndex < glyphs.size(); ++glyphIndex) {
            const auto& glyph = glyphs[glyphIndex];
            auto characterIndex = glyph.characterIndex + run->offset();
            if((direction == Direction::Ltr && characterIndex >= m_endOffset)
                || (direction == Direction::Rtl && characterIndex < m_startOffset)) {
                break;
            }

            if((direction == Direction::Ltr && characterIndex >= m_startOffset)
                || (direction == Direction::Rtl && characterIndex < m_endOffset)) {
                auto character = text.charAt(characterIndex);
                if(!treatAsZeroWidthSpace(character)) {
                    glyphBuffer[numGlyphs].index = glyph.glyphIndex;
                    glyphBuffer[numGlyphs].x = offset.x + glyph.xOffset;
                    glyphBuffer[numGlyphs].y = offset.y + glyph.yOffset;
                    numGlyphs++;
                }

                offset.x += glyph.advance;
                if(expansion && treatAsSpace(character)) {
                    offset.x += expansion;
                }
            }
        }

        cairo_set_scaled_font(canvas, run->fontData()->font());
        cairo_show_glyphs(canvas, glyphBuffer, numGlyphs);
        cairo_glyph_free(glyphBuffer);
    }

    return offset.x - origin.x;
}

} // namespace plutobook
