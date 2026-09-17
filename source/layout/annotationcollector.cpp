/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "annotationcollector.h"
#include "blockbox.h"
#include "boxlayer.h"
#include "boxview.h"
#include "document.h"
#include "inlinebox.h"
#include "linebox.h"
#include "linelayout.h"
#include "multicolumnbox.h"
#include "cssrule.h"
#include "pagebox.h"
#include "tablebox.h"
#include "textbox.h"

#include <algorithm>

namespace plutobook {

AnnotationCollector::AnnotationCollector(Document* document, const AnnotationOptions& options, AnnotationList& output)
    : m_document(document), m_options(options), m_output(output)
    , m_labelAttribute(options.labelAttribute)
    , m_idAttribute(options.idAttribute)
{
}

void AnnotationCollector::collectPage(uint32_t pageIndex)
{
    const auto& pages = m_document->pages();
    if(pageIndex >= pages.size())
        return;
    m_pageIndex = pageIndex;
    m_inlineFragments.clear();

    const auto& page = pages[pageIndex];
    auto contentRect = m_document->pageContentRectAt(pageIndex);
    if(contentRect.isEmpty())
        return;

    // The mapping PageBox::paintContents installs before rendering the document into
    // the page: the page origin and its padding, the page scale, then the slab of the
    // document flow that belongs to this page.
    State state;
    state.transform.translate(page->x() + page->paddingLeft(), page->y() + page->paddingTop());
    state.transform.scale(page->pageScale(), page->pageScale());
    state.transform.translate(-contentRect.x, -contentRect.y);
    // Layout runs in CSS pixels; the public API reports points, as PageSize does.
    state.transform.postScale(units::px, units::px);
    state.clip = state.transform.mapRect(contentRect);

    auto view = m_document->box();
    view->setCurrentPage(page.get());
    if(auto layer = view->layer())
        collectLayer(layer, layer, state);
    if(m_options.includePageMargins) {
        // The margin boxes live on the page itself, outside the document flow, so they
        // are reached from the page's own layer and in the page's own coordinates.
        State pageState;
        pageState.transform.postScale(units::px, units::px);
        pageState.clip = pageState.transform.mapRect(page->pageRect());
        if(auto pageLayer = page->layer())
            collectPageMargins(pageLayer, pageState);
    }

    view->setCurrentPage(nullptr);
}

static const char* pageMarginLabel(PageMarginType marginType)
{
    switch(marginType) {
    case PageMarginType::TopLeftCorner:
    case PageMarginType::TopLeft:
    case PageMarginType::TopCenter:
    case PageMarginType::TopRight:
    case PageMarginType::TopRightCorner:
        return "header";
    case PageMarginType::BottomRightCorner:
    case PageMarginType::BottomRight:
    case PageMarginType::BottomCenter:
    case PageMarginType::BottomLeft:
    case PageMarginType::BottomLeftCorner:
        return "footer";
    default:
        return "margin";
    }
}

void AnnotationCollector::collectPageMargins(BoxLayer* pageLayer, const State& state)
{
    for(auto child : pageLayer->children()) {
        auto marginBox = to<PageMarginBox>(child->box());
        if(marginBox == nullptr)
            continue;
        Transform transform(child->transform());
        transform.postTranslate(child->location().x, child->location().y);
        transform.postScale(marginBox->pageScale(), marginBox->pageScale());

        State current(state);
        current.transform.multiply(transform);
        if(!emit(Rect(Point(), marginBox->size()), current, AnnotationKind::Block,
            pageMarginLabel(marginBox->marginType()), std::string(), std::string(), std::string())) {
            continue;
        }

        collectBoxChildren(marginBox, Point(), current);
    }
}

void AnnotationCollector::collectLayer(BoxLayer* layer, BoxLayer* rootLayer, const State& state)
{
    Point location;
    auto currentLayer = layer;
    while(currentLayer && currentLayer != rootLayer) {
        location += currentLayer->location();
        currentLayer = currentLayer->containingLayer();
    }

    auto box = layer->box();
    if(box->isMultiColumnFlowBox()) {
        collectLayerColumnContents(layer, state, location);
        return;
    }

    if(!box->hasTransform() && !box->isPageMarginBox()) {
        collectLayerContents(layer, rootLayer, state, location);
        return;
    }

    Transform transform(layer->transform());
    transform.postTranslate(location.x, location.y);
    if(auto marginBox = to<PageMarginBox>(box))
        transform.postScale(marginBox->pageScale(), marginBox->pageScale());

    // The transform resets the coordinate system, so this layer becomes the anchor
    // that its descendants measure themselves against, exactly as painting does.
    State transformed(state);
    transformed.transform.multiply(transform);
    collectLayerContents(layer, layer, transformed, Point());
}

void AnnotationCollector::collectLayerContents(BoxLayer* layer, BoxLayer* rootLayer, const State& state, const Point& offset)
{
    auto box = layer->box();

    State current(state);
    if(box->isOverflowHidden() && !box->isSVGRootBox()) {
        Rect clipRect(offset.x, offset.y, layer->borderRect().w, layer->borderRect().h);
        current.clip.intersect(current.transform.mapRect(clipRect));
        if(current.clip.isEmpty()) {
            return;
        }
    }

    for(auto child : layer->children()) {
        if(child->zIndex() < 0) {
            collectLayer(child, rootLayer, current);
        }
    }

    // paintLayerContents subtracts the box location so that paint can add it back;
    // collectBox takes the resulting border-box origin directly.
    collectBox(box, offset, current);

    for(auto child : layer->children()) {
        if(child->zIndex() >= 0) {
            collectLayer(child, rootLayer, current);
        }
    }
}

void AnnotationCollector::collectLayerColumnContents(BoxLayer* layer, const State& state, const Point& offset)
{
    const auto& column = to<MultiColumnFlowBox>(*layer->box());
    for(auto row = column.firstRow(); row; row = row->nextRow()) {
        auto rowClip = row->visualOverflowRect();
        rowClip.translate(row->location() + offset - column.location());
        if(rowClip.isEmpty())
            continue;

        // The whole flow is walked once per column, shifted so that the column's own
        // slice lands in the row; the row clip is what discards every other slice.
        auto clip = state.transform.mapRect(rowClip);

        const auto columnCount = row->numberOfColumns();
        for(uint32_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
            auto rowRect = row->rowRectAt(columnIndex);
            auto columnRect = row->columnRectAt(columnIndex);
            auto translation = (columnRect.origin() - rowRect.origin()) + row->location() + offset - column.location();

            State current(state);
            current.clip.intersect(clip);
            if(current.clip.isEmpty())
                continue;
            current.transform.translate(translation.x, translation.y);
            collectLayerContents(layer, layer, current, Point());
        }
    }
}

void AnnotationCollector::collectBox(Box* box, const Point& origin, const State& state)
{
    State current(state);
    if(auto frame = to<BoxFrame>(box)) {
        Rect borderRect(origin, frame->size());
        current = stateForBox(box, borderRect, state, AnnotationKind::Block);
    }

    collectBoxChildren(box, origin, current);
}

void AnnotationCollector::collectBoxChildren(Box* box, const Point& origin, const State& state)
{
    if(auto block = to<BlockFlowBox>(box)) {
        if(block->isChildrenInline()) {
            if(auto lineLayout = block->lineLayout()) {
                for(const auto& line : lineLayout->lines()) {
                    collectLine(line.get(), origin, state);
                }
            }

            return;
        }
    }

    if(auto section = to<TableSectionBox>(box)) {
        for(auto rowBox : section->rows()) {
            Point rowOrigin(origin + rowBox->location());
            State rowState(stateForBox(rowBox, Rect(rowOrigin, rowBox->size()), state, AnnotationKind::Block));
            for(const auto& [col, cell] : rowBox->cells()) {
                if(cell.inColOrRowSpan())
                    continue;
                auto cellBox = cell.box();
                if(!cellBox->hasLayer()) {
                    collectBox(cellBox, rowOrigin + cellBox->location(), rowState);
                }
            }
        }

        return;
    }

    for(auto child = box->firstChild(); child; child = child->nextSibling()) {
        auto frame = to<BoxFrame>(child);
        if(frame == nullptr || frame->hasLayer())
            continue;
        collectBox(frame, origin + frame->location(), state);
    }
}

void AnnotationCollector::collectLine(LineBox* line, const Point& origin, const State& state)
{
    if(auto textLine = to<TextLineBox>(line)) {
        if(!m_options.includeTextLines || textLine->style()->visibility() != Visibility::Visible)
            return;
        std::string text;
        textLine->shape().text().toUTF8String(text);
        emit(textLine->rect().translated(origin), state, AnnotationKind::TextLine,
            std::string(), std::string(), std::string(), std::move(text));
        return;
    }

    if(auto replacedLine = to<ReplacedLineBox>(line)) {
        auto box = replacedLine->box();
        if(!box->hasLayer())
            collectBox(box, origin + box->location(), state);
        return;
    }

    auto flowLine = to<FlowLineBox>(line);
    if(flowLine == nullptr)
        return;

    State current(state);
    if(!flowLine->isRootLineBox()) {
        // One fragment of an inline element; each line it spans is its own rectangle
        // unless the caller asked for the union, which is handled after the walk.
        current = stateForBox(flowLine->box(), flowLine->rect().translated(origin), state, AnnotationKind::Inline);
    }

    for(auto child : flowLine->children()) {
        if(!child->box()->hasLayer()) {
            collectLine(child, origin, current);
        }
    }
}

AnnotationCollector::State AnnotationCollector::stateForBox(Box* box, const Rect& localRect, const State& state, AnnotationKind kind)
{
    std::string label;
    std::string id;
    auto element = annotatedElement(box, label, id);
    if(element == nullptr)
        return state;
    if(label.empty()) {
        if(!m_options.includeUnlabelledBlocks || kind != AnnotationKind::Block)
            return state;
        label.assign(element->tagName().data(), element->tagName().size());
    }

    if(kind == AnnotationKind::Inline && m_options.mergeInlineFragments
        && mergeInlineFragment(element, localRect, state)) {
        State merged(state);
        if(!id.empty())
            merged.parentId = std::move(id);
        return merged;
    }

    std::string tagName(element->tagName().data(), element->tagName().size());
    auto index = m_output.size();
    if(emit(localRect, state, kind, label, id, std::move(tagName), std::string())
        && kind == AnnotationKind::Inline && m_options.mergeInlineFragments) {
        m_inlineFragments.emplace(element, index);
    }

    State current(state);
    if(!id.empty())
        current.parentId = std::move(id);
    return current;
}

bool AnnotationCollector::emit(const Rect& localRect, const State& state, AnnotationKind kind,
    std::string label, std::string id, std::string tagName, std::string text)
{
    auto rect = state.transform.mapRect(localRect).intersected(state.clip);
    if(rect.w < m_options.minimumSize || rect.h < m_options.minimumSize)
        return false;

    Annotation annotation;
    annotation.kind = kind;
    annotation.pageIndex = m_pageIndex;
    annotation.x = rect.x;
    annotation.y = rect.y;
    annotation.width = rect.w;
    annotation.height = rect.h;
    annotation.label = std::move(label);
    annotation.id = std::move(id);
    annotation.parentId = state.parentId;
    annotation.tagName = std::move(tagName);
    annotation.text = std::move(text);
    m_output.push_back(std::move(annotation));
    return true;
}

bool AnnotationCollector::mergeInlineFragment(const Element* element, const Rect& localRect, const State& state)
{
    auto it = m_inlineFragments.find(element);
    if(it == m_inlineFragments.end())
        return false;
    auto rect = state.transform.mapRect(localRect).intersected(state.clip);
    if(rect.isEmpty())
        return true;

    auto& annotation = m_output[it->second];
    Rect united(Rect(annotation.x, annotation.y, annotation.width, annotation.height).united(rect));
    annotation.x = united.x;
    annotation.y = united.y;
    annotation.width = united.w;
    annotation.height = united.h;
    return true;
}

const Element* AnnotationCollector::annotatedElement(const Box* box, std::string& label, std::string& id) const
{
    auto element = to<Element>(box->node());
    if(element == nullptr)
        return nullptr;
    if(auto attribute = element->findAttribute(m_labelAttribute))
        label.assign(attribute->value().data(), attribute->value().size());
    if(auto attribute = element->findAttribute(m_idAttribute))
        id.assign(attribute->value().data(), attribute->value().size());
    return element;
}

} // namespace plutobook
