#include "boxlayer.h"
#include "graphicscontext.h"
#include "box.h"

namespace plutobook {

std::unique_ptr<BoxLayer> BoxLayer::create(BoxModel* box, BoxLayer* parent)
{
    return std::unique_ptr<BoxLayer>(new (box->heap()) BoxLayer(box, parent));
}

void BoxLayer::layout()
{
    m_borderRect = m_box->borderBoundingBox();
    if(auto box = to<BoxFrame>(m_box))
        m_borderRect.move(box->location());
    if(!m_box->isPositioned()) {
        auto parent = m_box->parentBox();
        while(parent && !parent->hasLayer()) {
            if(auto box = to<BoxFrame>(parent)) {
                if(!box->isTableRowBox()) {
                    m_borderRect.move(box->location());
                }
            }

            parent = parent->parentBox();
        }

        if(parent && parent->isTableRowBox()) {
            auto& box = to<BoxFrame>(*parent);
            m_borderRect.move(-box.location());
        }
    }

    if(m_box->isRelPositioned()) {
        m_borderRect.move(m_box->relativePositionOffset());
    }

    m_transform = Transform::Identity;
    if(m_box->hasTransform()) {
        m_transform = m_box->style()->getTransform(m_borderRect.w, m_borderRect.h);
    }

    auto compare_func = [](auto& a, auto& b) { return a->zIndex() < b->zIndex(); };
    std::stable_sort(m_children.begin(), m_children.end(), compare_func);
    m_overflowRect = m_box->visualOverflowRect();
    for(auto child : m_children) {
        child->layout();
        if(!m_box->isOverflowHidden()) {
            auto overflowRect = child->transform().mapRect(child->overflowRect());
            overflowRect.move(child->location());
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
        auto position = currentLayer->box()->position();
        auto parentLayer = currentLayer->parent();
        if(position == Position::Fixed) {
            for(; parentLayer; parentLayer = parentLayer->parent()) {
                auto box = parentLayer->box();
                if(box->isBoxView() || box->hasTransform()) {
                    break;
                }
            }
        } else if(position == Position::Absolute) {
            for(; parentLayer; parentLayer = parentLayer->parent()) {
                auto box = parentLayer->box();
                if(box->isBoxView() || box->isPositioned() || box->isRelPositioned() || box->hasTransform()) {
                    break;
                }
            }
        }

        location += currentLayer->location();
        currentLayer = parentLayer;
    }

    if(m_box->position() == Position::Fixed && !rootLayer->parent()) {
        location.x += std::max(0.f, rect.x);
        location.y += std::max(0.f, rect.y);
    }

    if(!m_box->hasTransform()) {
        paintLayerContents(rootLayer, context, rect, location);
        return;
    }

    Transform transform(m_transform);
    transform.postTranslate(location.x, location.y);
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
    if(compositing && !m_box->isSVGRootBox())
        context.pushGroup();
    Point adjustedOffset(offset);
    if(auto box = to<BoxFrame>(m_box)) {
        adjustedOffset -= box->location();
    }

    PaintInfo paintInfo(context, rect);
    if(m_box->isBoxView())
        m_box->paintRootBackground(paintInfo);
    for(auto child : m_children) {
        if(child->zIndex() < 0) {
            child->paintLayer(rootLayer, context, rect);
        }
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
