/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_ANNOTATIONCOLLECTOR_H
#define PLUTOBOOK_ANNOTATIONCOLLECTOR_H

#include "plutobook.hpp"
#include "geometry.h"
#include "globalstring.h"

#include <map>
#include <string>

namespace plutobook {

class Box;
class BoxFrame;
class BoxModel;
class BoxLayer;
class Document;
class Element;
class FlowLineBox;
class LineBox;

// Walks the laid-out box tree the same way BoxLayer paints it, emitting a rectangle
// for every labelled element and every shaped text run instead of drawing anything.
class AnnotationCollector {
public:
    AnnotationCollector(Document* document, const AnnotationOptions& options, AnnotationList& output);

    void collectPage(uint32_t pageIndex);

private:
    // The accumulated state of the walk: how to get from the current local coordinate
    // system to page space, what remains visible there, and who the enclosing label is.
    struct State {
        Transform transform;
        Rect clip;
        std::string parentId;
    };

    void collectLayer(BoxLayer* layer, BoxLayer* rootLayer, const State& state);
    void collectLayerContents(BoxLayer* layer, BoxLayer* rootLayer, const State& state, const Point& offset);
    void collectLayerColumnContents(BoxLayer* layer, const State& state, const Point& offset);

    void collectBox(Box* box, const Point& origin, const State& state);
    void collectBoxChildren(Box* box, const Point& origin, const State& state);
    void collectLine(LineBox* line, const Point& origin, const State& state);
    void collectPageMargins(BoxLayer* pageLayer, const State& state);

    // Emits a record if the box carries a label, and returns the state its children see.
    State stateForBox(Box* box, const Rect& localRect, const State& state, AnnotationKind kind);

    // Returns true if a record was appended; a rectangle clipped away emits nothing.
    bool emit(const Rect& localRect, const State& state, AnnotationKind kind,
        std::string label, std::string id, std::string tagName, std::string text);

    // Grows the rectangle of the record already emitted for this element, so that an
    // inline element spanning several lines becomes one box instead of one per line.
    bool mergeInlineFragment(const Element* element, const Rect& localRect, const State& state);

    const Element* annotatedElement(const Box* box, std::string& label, std::string& id) const;

    Document* m_document;
    const AnnotationOptions& m_options;
    AnnotationList& m_output;

    GlobalString m_labelAttribute;
    GlobalString m_idAttribute;
    std::map<const Element*, size_t> m_inlineFragments;
    uint32_t m_pageIndex{0};
};

} // namespace plutobook

#endif // PLUTOBOOK_ANNOTATIONCOLLECTOR_H
