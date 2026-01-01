/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_GRAPHICSCONTEXT_H
#define PLUTOBOOK_GRAPHICSCONTEXT_H

#include "boxstyle.h"

#include <memory>

typedef struct _cairo cairo_t;
typedef struct _cairo_surface cairo_surface_t;

namespace plutobook {

class Path;
class ImageBuffer;

using GradientStop = std::pair<float, Color>;
using GradientStops = std::vector<GradientStop>;

struct LinearGradientValues {
    float x1 = 0.f;
    float y1 = 0.f;
    float x2 = 0.f;
    float y2 = 0.f;
};

struct RadialGradientValues {
    float fx = 0.f;
    float fy = 0.f;
    float cx = 0.f;
    float cy = 0.f;
    float r = 0.f;
};

enum class SpreadMethod {
    Pad,
    Reflect,
    Repeat
};

using DashArray = std::vector<double>;

class StrokeData {
public:
    explicit StrokeData(float lineWidth = 1.f) : m_lineWidth(lineWidth) {}

    void setLineWidth(float lineWidth) { m_lineWidth = lineWidth; }
    float lineWidth() const { return m_lineWidth; }

    void setMiterLimit(float miterLimit) { m_miterLimit = miterLimit; }
    float miterLimit() const { return m_miterLimit; }

    void setDashOffset(float dashOffset) { m_dashOffset = dashOffset; }
    float dashOffset() const { return m_dashOffset; }

    void setDashArray(DashArray dashArray) { m_dashArray = std::move(dashArray); }
    const DashArray& dashArray() const { return m_dashArray; }

    void setLineCap(LineCap lineCap) { m_lineCap = lineCap; }
    LineCap lineCap() const { return m_lineCap; }

    void setLineJoin(LineJoin lineJoin) { m_lineJoin = lineJoin; }
    LineJoin lineJoin() const { return m_lineJoin; }

private:
    float m_lineWidth;
    float m_miterLimit{10.f};
    float m_dashOffset{0.f};
    LineCap m_lineCap{LineCap::Butt};
    LineJoin m_lineJoin{LineJoin::Miter};
    DashArray m_dashArray;
};

class GraphicsContext {
public:
    GraphicsContext() = delete;
    explicit GraphicsContext(cairo_t* canvas);
    ~GraphicsContext();

    void setColor(const Color& color);
    void setLinearGradient(const LinearGradientValues& values, const GradientStops& stops, const Transform& transform, SpreadMethod method, float opacity);
    void setRadialGradient(const RadialGradientValues& values, const GradientStops& stops, const Transform& transform, SpreadMethod method, float opacity);
    void setPattern(cairo_surface_t* surface, const Transform& transform);

    void translate(float tx, float ty);
    void scale(float sx, float sy);
    void rotate(float angle);

    Transform getTransform() const;
    void addTransform(const Transform& transform);
    void setTransform(const Transform& transform);
    void resetTransform();

    void fillRect(const Rect& rect, FillRule fillRule = FillRule::NonZero);
    void fillRoundedRect(const RoundedRect& rrect, FillRule fillRule = FillRule::NonZero);
    void fillPath(const Path& path, FillRule fillRule = FillRule::NonZero);

    void strokeRect(const Rect& rect, const StrokeData& strokeData);
    void strokeRoundedRect(const RoundedRect& rrect, const StrokeData& strokeData);
    void strokePath(const Path& path, const StrokeData& strokeData);

    void clipRect(const Rect& rect, FillRule clipRule = FillRule::NonZero);
    void clipRoundedRect(const RoundedRect& rrect, FillRule clipRule = FillRule::NonZero);
    void clipPath(const Path& path, FillRule clipRule = FillRule::NonZero);

    void clipOutRect(const Rect& rect);
    void clipOutRoundedRect(const RoundedRect& rrect);
    void clipOutPath(const Path& path);

    void save();
    void restore();

    void pushGroup();
    void popGroup(float opacity, BlendMode blendMode = BlendMode::Normal);
    void applyMask(const ImageBuffer& maskImage);

    void addLinkAnnotation(const std::string_view& dest, const std::string_view& uri, const Rect& rect);
    void addLinkDestination(const std::string_view& name, const Point& location);

    cairo_t* canvas() const { return m_canvas; }

private:
    GraphicsContext(const GraphicsContext&) = delete;
    GraphicsContext& operator=(const GraphicsContext&) = delete;
    cairo_t* m_canvas;
};

class ImageBuffer {
public:
    static std::unique_ptr<ImageBuffer> create(const Rect& rect);
    static std::unique_ptr<ImageBuffer> create(float x, float y, float width, float height);

    cairo_surface_t* surface() const { return m_surface; }
    cairo_t* canvas() const { return m_canvas; }

    const int x() const { return m_x; }
    const int y() const { return m_y; }

    const int width() const;
    const int height() const;

    void convertToLuminanceMask();

    ~ImageBuffer();

private:
    ImageBuffer(int x, int y, int width, int height);
    cairo_surface_t* m_surface;
    cairo_t* m_canvas;
    const int m_x;
    const int m_y;
};

} // namespace plutobook

#endif // PLUTOBOOK_GRAPHICSCONTEXT_H
