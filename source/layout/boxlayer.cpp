/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "boxlayer.h"
#include "inlinebox.h"
#include "pagebox.h"
#include "multicolumnbox.h"
#include "graphicscontext.h"

namespace plutobook {

std::unique_ptr<BoxLayer> BoxLayer::create(BoxModel* box, BoxLayer* parent)
{
    return std::unique_ptr<BoxLayer>(new (box->heap()) BoxLayer(box, parent));
}

BoxLayer* BoxLayer::containingLayer() const
{
    auto parentLayer = parent();
    if(m_box->style()->position() == Position::Fixed) {
        while(parentLayer && !parentLayer->box()->canContainFixedPositionedBoxes()) {
            parentLayer = parentLayer->parent();
        }
    } else if(m_box->style()->position() == Position::Absolute) {
        while(parentLayer && !parentLayer->box()->canContainAbsolutePositionedBoxes()) {
            parentLayer = parentLayer->parent();
        }
    }

    return parentLayer;
}

void BoxLayer::updatePosition()
{
    m_borderRect = m_box->borderBoundingBox();
    if(m_box->isPositioned()) {
        auto container = containingLayer();
        if(container && container->box()->isRelativePositioned()) {
            if(auto box = to<InlineBox>(container->box())) {
                m_borderRect.translate(box->relativePositionedInlineOffset(m_box));
            }
        }
    } else {
        auto parent = m_box->parentBox();
        while(parent && !parent->hasLayer()) {
            if(auto box = to<BoxFrame>(parent))
                m_borderRect.translate(box->location());
            parent = parent->parentBox();
        }
    }

    if(m_box->isRelativePositioned()) {
        m_borderRect.translate(m_box->relativePositionOffset());
    }

    if(m_box->hasTransform()) {
        m_transform = m_box->style()->getTransform(m_borderRect.w, m_borderRect.h);
    }

    auto compare_func = [](const auto& a, const auto& b) { return a->zIndex() < b->zIndex(); };
    std::stable_sort(m_children.begin(), m_children.end(), compare_func);
    m_overflowRect = m_box->visualOverflowRect();
    for(auto child : m_children) {
        child->updatePosition();
        if(!m_box->isOverflowHidden() && !child->box()->isFixedPositioned()
            && !child->box()->isMultiColumnFlowBox()) {
            auto overflowRect = child->transform().mapRect(child->overflowRect());
            overflowRect.translate(child->location());
            m_overflowRect.unite(overflowRect);
        }
    }
}

void BoxLayer::paint(GraphicsContext& context, const Rect& rect)
{
    paintLayer(this, context, rect);
}

void BoxLayer::paintLayer(BoxLayer* rootLayer, GraphicsContext& context, const Rect& rect)
{
    Point location;
    auto currentLayer = this;
    while(currentLayer && currentLayer != rootLayer) {
        location += currentLayer->location();
        currentLayer = currentLayer->containingLayer();
    }

    if(m_box->isMultiColumnFlowBox()) {
        assert(m_box->style()->position() == Position::Static && !m_box->hasTransform());
        paintLayerColumnContents(rootLayer, context, rect, location);
        return;
    }

    if(m_box->style()->position() == Position::Fixed && !rootLayer->parent()) {
        location.x += std::max(0.f, rect.x);
        location.y += std::max(0.f, rect.y);
    }

    if(!m_box->hasTransform() && !m_box->isPageMarginBox()) {
        paintLayerContents(rootLayer, context, rect, location);
        return;
    }

    Transform transform(m_transform);
    transform.postTranslate(location.x, location.y);
    if(auto marginBox = to<PageMarginBox>(m_box))
        transform.postScale(marginBox->pageScale(), marginBox->pageScale());
    Rect rectangle = transform.inverted().mapRect(rect);

    context.save();
    context.addTransform(transform);
    paintLayerContents(this, context, rectangle, Point());
    context.restore();
}

void BoxLayer::paintLayerContents(BoxLayer* rootLayer, GraphicsContext& context, const Rect& rect, const Point& offset)
{
    Rect clipRect(offset.x, offset.y, m_borderRect.w, m_borderRect.h);
    auto clipping = m_box->isOverflowHidden() && !m_box->isSVGRootBox();
    if(m_box->isPositioned()) {
        auto clip = m_box->style()->clip();
        if(!clip.left().isAuto()) {
            auto value = clip.left().calc(m_borderRect.w);
            clipRect.x += value;
            clipRect.w -= value;
            clipping = true;
        }

        if(!clip.right().isAuto()) {
            clipRect.w -= m_borderRect.w - clip.right().calc(m_borderRect.w);
            clipping = true;
        }

        if(!clip.top().isAuto()) {
            auto value = clip.top().calc(m_borderRect.h);
            clipRect.y += value;
            clipRect.h -= value;
            clipping = true;
        }

        if(!clip.bottom().isAuto()) {
            clipRect.h -= m_borderRect.h - clip.bottom().calc(m_borderRect.h);
            clipping = true;
        }
    }

    if(clipping) {
        if(clipRect.isEmpty())
            return;
        context.save();
        context.clipRect(clipRect);
    }

    auto compositing = m_opacity < 1.f || m_box->style()->hasBlendMode();
    if(compositing && !m_box->isSVGRootBox()) {
        context.pushGroup();
    }

    PaintInfo paintInfo(context, rect);
    m_box->paintRootBackground(paintInfo);
    for(auto child : m_children) {
        if(child->zIndex() < 0) {
            child->paintLayer(rootLayer, context, rect);
        }
    }

    Point adjustedOffset(offset);
    if(m_box->isBoxFrame() && !m_box->isPageBox()) {
        const auto& box = to<BoxFrame>(*m_box);
        adjustedOffset.translate(-box.location());
    }

    m_box->paint(paintInfo, adjustedOffset, PaintPhase::Decorations);
    m_box->paint(paintInfo, adjustedOffset, PaintPhase::Floats);
    m_box->paint(paintInfo, adjustedOffset, PaintPhase::Contents);
    m_box->paint(paintInfo, adjustedOffset, PaintPhase::Outlines);
    for(auto child : m_children) {
        if(child->zIndex() >= 0) {
            child->paintLayer(rootLayer, context, rect);
        }
    }

    if(compositing && !m_box->isSVGRootBox())
        context.popGroup(m_opacity, m_box->style()->blendMode());
    if(clipping) {
        context.restore();
    }
}

void BoxLayer::paintLayerColumnContents(BoxLayer* rootLayer, GraphicsContext& context, const Rect& rect, const Point& offset)
{
    const auto& column = to<MultiColumnFlowBox>(*m_box);
    for(auto row = column.firstRow(); row; row = row->nextRow()) {
        auto clipRect = row->visualOverflowRect();
        clipRect.translate(row->location() + offset - column.location());
        if(clipRect.isEmpty())
            continue;
        context.save();
        context.clipRect(clipRect);

        const auto columnCount = row->numberOfColumns();
        for(uint32_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
            auto rowRect = row->rowRectAt(columnIndex);
            auto columnRect = row->columnRectAt(columnIndex);
            auto translation = (columnRect.origin() - rowRect.origin()) + row->location() + offset - column.location();
            auto rectangle = rect.translated(-translation);

            context.save();
            context.translate(translation.x, translation.y);
            paintLayerContents(this, context, rectangle, Point());
            context.restore();
        }

        row->paintColumnRules(context, offset - column.location());
        context.restore();
    }
}

BoxLayer::BoxLayer(BoxModel* box, BoxLayer* parent)
    : m_box(box)
    , m_parent(parent)
    , m_children(box->heap())
    , m_zIndex(box->style()->zIndex().value_or(0))
    , m_opacity(box->style()->opacity())
{
    if(parent) {
        parent->m_children.push_back(this);
    }
}

} // namespace plutobook
