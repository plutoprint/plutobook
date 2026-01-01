/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_TEXTSHAPE_H
#define PLUTOBOOK_TEXTSHAPE_H

#include "pointer.h"
#include "heapstring.h"
#include "ustring.h"

#include <memory>
#include <vector>

namespace plutobook {

struct TextShapeRunGlyphData : public HeapMember {
    uint16_t glyphIndex;
    uint16_t characterIndex;
    float xOffset;
    float yOffset;
    float advance;
};

class TextShapeRunGlyphDataList {
public:
    explicit TextShapeRunGlyphDataList(Heap* heap, size_t size);

    TextShapeRunGlyphData& operator[](size_t index) { return m_data[index]; }
    const TextShapeRunGlyphData& operator[](size_t index) const { return m_data[index]; }
    size_t size() const { return m_size; }

private:
    std::unique_ptr<TextShapeRunGlyphData[]> m_data;
    size_t m_size;
};

class SimpleFontData;

enum class Direction : uint8_t;

class TextShapeRun : public HeapMember {
public:
    static std::unique_ptr<TextShapeRun> create(Heap* heap, const SimpleFontData* fontData, uint32_t offset, uint32_t length, float width, TextShapeRunGlyphDataList glyphs);

    const SimpleFontData* fontData() const { return m_fontData; }
    uint32_t offset() const { return m_offset; }
    uint32_t length() const { return m_length; }
    float width() const { return m_width; }
    const TextShapeRunGlyphDataList& glyphs() const { return m_glyphs; }

    float positionForOffset(uint32_t offset, Direction direction) const;
    float positionForVisualOffset(uint32_t offset, Direction direction) const;
    uint32_t offsetForPosition(float position, Direction direction) const;

private:
    TextShapeRun(const SimpleFontData* fontData, uint32_t offset, uint32_t length, float width, TextShapeRunGlyphDataList glyphs);
    const SimpleFontData* m_fontData;
    uint32_t m_offset;
    uint32_t m_length;
    float m_width;
    TextShapeRunGlyphDataList m_glyphs;
};

using TextShapeRunList = std::pmr::vector<std::unique_ptr<TextShapeRun>>;

class BoxStyle;

class TextShape : public HeapMember, public RefCounted<TextShape> {
public:
    static RefPtr<TextShape> createForText(const UString& text, Direction direction, bool disableSpacing, const BoxStyle* style);
    static RefPtr<TextShape> createForTabs(const UString& text, Direction direction, const BoxStyle* style);

    const UString& text() const { return m_text; }
    uint32_t length() const { return m_text.length(); }
    Direction direction() const { return m_direction; }
    float width() const { return m_width; }
    const TextShapeRunList& runs() const { return m_runs; }

    uint32_t offsetForPosition(float position) const;
    float positionForOffset(uint32_t offset) const;

    ~TextShape();

private:
    TextShape(const UString& text, Direction direction, float width, TextShapeRunList runs);
    UString m_text;
    Direction m_direction;
    float m_width;
    TextShapeRunList m_runs;
};

class GraphicsContext;
class Point;

class TextShapeView {
public:
    TextShapeView() = default;
    explicit TextShapeView(const RefPtr<TextShape>& shape)
        : m_shape(shape), m_startOffset(0), m_endOffset(shape->length())
    {}

    TextShapeView(const RefPtr<TextShape>& shape, uint32_t startOffset, uint32_t endOffset)
        : m_shape(shape), m_startOffset(startOffset), m_endOffset(endOffset)
    {
        assert(endOffset >= startOffset && endOffset <= shape->length());
    }

    const RefPtr<TextShape>& shape() const { return m_shape; }
    uint32_t startOffset() const { return m_startOffset; }
    uint32_t endOffset() const { return m_endOffset; }
    uint32_t length() const { return m_endOffset - m_startOffset; }
    UString text() const;

    uint32_t expansionOpportunityCount() const;
    void maxAscentAndDescent(float& maxAscent, float& maxDescent) const;
    float width(float expansion = 0.f) const;
    float draw(GraphicsContext& context, const Point& origin, float expansion) const;

private:
    RefPtr<TextShape> m_shape;
    uint32_t m_startOffset{0};
    uint32_t m_endOffset{0};
};

} // namespace plutobook

#endif // PLUTOBOOK_TEXTSHAPE_H
