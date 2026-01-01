/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_BOXLAYER_H
#define PLUTOBOOK_BOXLAYER_H

#include "heapstring.h"
#include "geometry.h"

#include <memory>

namespace plutobook {

class GraphicsContext;
class BoxModel;
class BoxLayer;

using BoxLayerList = std::pmr::vector<BoxLayer*>;

class BoxLayer : public HeapMember {
public:
    static std::unique_ptr<BoxLayer> create(BoxModel* box, BoxLayer* parent);

    BoxModel* box() const { return m_box; }
    BoxLayer* parent() const { return m_parent; }
    BoxLayer* containingLayer() const;

    int zIndex() const { return m_zIndex; }

    float staticTop() const { return m_staticTop; }
    float staticLeft() const { return m_staticLeft; }

    void setStaticTop(float top) { m_staticTop = top; }
    void setStaticLeft(float left) { m_staticLeft = left; }

    const Transform& transform() const { return m_transform; }
    const Rect& borderRect() const { return m_borderRect; }
    const Rect& overflowRect() const { return m_overflowRect; }

    float overflowTop() const { return m_overflowRect.y; }
    float overflowBottom() const { return m_overflowRect.bottom(); }
    float overflowLeft() const { return m_overflowRect.x; }
    float overflowRight() const { return m_overflowRect.right(); }

    Point location() const { return m_borderRect.origin(); }
    Size size() const { return m_borderRect.size(); }

    void updatePosition();

    void paint(GraphicsContext& context, const Rect& rect);
    void paintLayer(BoxLayer* rootLayer, GraphicsContext& context, const Rect& rect);
    void paintLayerContents(BoxLayer* rootLayer, GraphicsContext& context, const Rect& rect, const Point& offset);
    void paintLayerColumnContents(BoxLayer* rootLayer, GraphicsContext& context, const Rect& rect, const Point& offset);

private:
    BoxLayer(BoxModel* box, BoxLayer* parent);
    BoxModel* m_box;
    BoxLayer* m_parent;
    BoxLayerList m_children;
    int m_zIndex;
    float m_opacity;
    float m_staticTop{0};
    float m_staticLeft{0};
    Transform m_transform;
    Rect m_borderRect;
    Rect m_overflowRect;
};

} // namespace plutobook

#endif // PLUTOBOOK_BOXLAYER_H
