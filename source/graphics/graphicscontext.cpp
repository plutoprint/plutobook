#include "graphicscontext.h"
#include "geometry.h"

#include <cairo.h>

#include <cmath>
#include <sstream>

namespace plutobook {

constexpr cairo_fill_rule_t to_cairo_fill_rule(FillRule fillRule)
{
    return fillRule == FillRule::NonZero ? CAIRO_FILL_RULE_WINDING : CAIRO_FILL_RULE_EVEN_ODD;
}

constexpr cairo_operator_t to_cairo_operator(BlendMode blendMode)
{
    switch(blendMode) {
    case BlendMode::Normal:
        return CAIRO_OPERATOR_OVER;
    case BlendMode::Multiply:
        return CAIRO_OPERATOR_MULTIPLY;
    case BlendMode::Screen:
        return CAIRO_OPERATOR_SCREEN;
    case BlendMode::Overlay:
        return CAIRO_OPERATOR_OVERLAY;
    case BlendMode::Darken:
        return CAIRO_OPERATOR_DARKEN;
    case BlendMode::Lighten:
        return CAIRO_OPERATOR_LIGHTEN;
    case BlendMode::ColorDodge:
        return CAIRO_OPERATOR_COLOR_DODGE;
    case BlendMode::ColorBurn:
        return CAIRO_OPERATOR_COLOR_BURN;
    case BlendMode::HardLight:
        return CAIRO_OPERATOR_HARD_LIGHT;
    case BlendMode::SoftLight:
        return CAIRO_OPERATOR_SOFT_LIGHT;
    case BlendMode::Difference:
        return CAIRO_OPERATOR_DIFFERENCE;
    case BlendMode::Exclusion:
        return CAIRO_OPERATOR_EXCLUSION;
    case BlendMode::Hue:
        return CAIRO_OPERATOR_HSL_HUE;
    case BlendMode::Saturation:
        return CAIRO_OPERATOR_HSL_SATURATION;
    case BlendMode::Color:
        return CAIRO_OPERATOR_HSL_COLOR;
    case BlendMode::Luminosity:
        return CAIRO_OPERATOR_HSL_LUMINOSITY;
    }

    return CAIRO_OPERATOR_OVER;
}

static cairo_matrix_t to_cairo_matrix(const Transform& transform)
{
    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix, transform.a, transform.b, transform.c, transform.d, transform.e, transform.f);
    return matrix;
}

static void set_cairo_stroke_data(cairo_t* cr, const StrokeData& strokeData)
{
    const auto& dashes = strokeData.dashArray();
    cairo_set_line_width(cr, strokeData.lineWidth());
    cairo_set_miter_limit(cr, strokeData.miterLimit());
    cairo_set_dash(cr, dashes.data(), dashes.size(), strokeData.dashOffset());
    switch(strokeData.lineCap()) {
    case LineCap::Butt:
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
        break;
    case LineCap::Round:
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        break;
    case LineCap::Square:
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
        break;
    }

    switch(strokeData.lineJoin()) {
    case LineJoin::Miter:
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);
        break;
    case LineJoin::Round:
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
        break;
    case LineJoin::Bevel:
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
        break;
    }
}

static void set_cairo_path(cairo_t* cr, const Path& path)
{
    PathIterator it(path);
    std::array<Point, 3> p;
    while(!it.isDone()) {
        switch(it.currentSegment(p)) {
        case PathCommand::MoveTo:
            cairo_move_to(cr, p[0].x, p[0].y);
            break;
        case PathCommand::LineTo:
            cairo_line_to(cr, p[0].x, p[0].y);
            break;
        case PathCommand::CubicTo:
            cairo_curve_to(cr, p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y);
            break;
        case PathCommand::Close:
            cairo_close_path(cr);
            break;
        }

        it.next();
    }
}

static void set_cairo_gradient(cairo_pattern_t* pattern, const GradientStops& stops, const Transform& transform, SpreadMethod method, float opacity)
{
    for(const auto& stop : stops) {
        auto red = stop.second.red() / 255.0;
        auto green = stop.second.green() / 255.0;
        auto blue = stop.second.blue() / 255.0;
        auto alpha = stop.second.alpha() / 255.0;
        cairo_pattern_add_color_stop_rgba(pattern, stop.first, red, green, blue, alpha * opacity);
    }

    switch(method) {
    case SpreadMethod::Pad:
        cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD);
        break;
    case SpreadMethod::Reflect:
        cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REFLECT);
        break;
    case SpreadMethod::Repeat:
        cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
        break;
    }

    auto matrix = to_cairo_matrix(transform);
    cairo_matrix_invert(&matrix);
    cairo_pattern_set_matrix(pattern, &matrix);
}

GraphicsContext::GraphicsContext(cairo_t* canvas)
    : m_canvas(cairo_reference(canvas))
{
}

GraphicsContext::~GraphicsContext()
{
    cairo_destroy(m_canvas);
}

void GraphicsContext::setColor(const Color& color)
{
    auto red = color.red() / 255.0;
    auto green = color.green() / 255.0;
    auto blue = color.blue() / 255.0;
    auto alpha = color.alpha() / 255.0;
    cairo_set_source_rgba(m_canvas, red, green, blue, alpha);
}

void GraphicsContext::setLinearGradient(const LinearGradientValues& values, const GradientStops& stops, const Transform& transform, SpreadMethod method, float opacity)
{
    auto pattern = cairo_pattern_create_linear(values.x1, values.y1, values.x2, values.y2);
    set_cairo_gradient(pattern, stops, transform, method, opacity);
    cairo_set_source(m_canvas, pattern);
    cairo_pattern_destroy(pattern);
}

void GraphicsContext::setRadialGradient(const RadialGradientValues& values, const GradientStops& stops, const Transform& transform, SpreadMethod method, float opacity)
{
    auto pattern = cairo_pattern_create_radial(values.fx, values.fy, 0, values.cx, values.cy, values.r);
    set_cairo_gradient(pattern, stops, transform, method, opacity);
    cairo_set_source(m_canvas, pattern);
    cairo_pattern_destroy(pattern);
}

void GraphicsContext::setPattern(cairo_surface_t* surface, const Transform& transform)
{
    auto pattern = cairo_pattern_create_for_surface(surface);
    auto matrix = to_cairo_matrix(transform);
    cairo_matrix_invert(&matrix);
    cairo_pattern_set_matrix(pattern, &matrix);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
    cairo_set_source(m_canvas, pattern);
    cairo_pattern_destroy(pattern);
}

void GraphicsContext::translate(float tx, float ty)
{
    cairo_translate(m_canvas, tx, ty);
}

void GraphicsContext::scale(float sx, float sy)
{
    cairo_scale(m_canvas, sx, sy);
}

void GraphicsContext::rotate(float angle)
{
    cairo_rotate(m_canvas, deg2rad(angle));
}

Transform GraphicsContext::getTransform() const
{
    cairo_matrix_t matrix;
    cairo_get_matrix(m_canvas, &matrix);
    return Transform(matrix.xx, matrix.yx, matrix.xy, matrix.yy, matrix.x0, matrix.y0);
}

void GraphicsContext::addTransform(const Transform& transform)
{
    cairo_matrix_t matrix = to_cairo_matrix(transform);
    cairo_transform(m_canvas, &matrix);
}

void GraphicsContext::setTransform(const Transform& transform)
{
    cairo_matrix_t matrix = to_cairo_matrix(transform);
    cairo_set_matrix(m_canvas, &matrix);
}

void GraphicsContext::resetTransform()
{
    cairo_identity_matrix(m_canvas);
}

void GraphicsContext::fillRect(const Rect& rect, FillRule fillRule)
{
    cairo_new_path(m_canvas);
    cairo_rectangle(m_canvas, rect.x, rect.y, rect.w, rect.h);
    cairo_set_fill_rule(m_canvas, to_cairo_fill_rule(fillRule));
    cairo_fill(m_canvas);
}

void GraphicsContext::fillRoundedRect(const RoundedRect& rrect, FillRule fillRule)
{
    if(!rrect.isRounded()) {
        fillRect(rrect.rect(), fillRule);
        return;
    }

    Path path;
    path.addRoundedRect(rrect);
    fillPath(path, fillRule);
}

void GraphicsContext::fillPath(const Path& path, FillRule fillRule)
{
    cairo_new_path(m_canvas);
    set_cairo_path(m_canvas, path);
    cairo_set_fill_rule(m_canvas, to_cairo_fill_rule(fillRule));
    cairo_fill(m_canvas);
}

void GraphicsContext::strokeRect(const Rect& rect, const StrokeData& strokeData)
{
    cairo_new_path(m_canvas);
    cairo_rectangle(m_canvas, rect.x, rect.y, rect.w, rect.h);
    set_cairo_stroke_data(m_canvas, strokeData);
    cairo_stroke(m_canvas);
}

void GraphicsContext::strokeRoundedRect(const RoundedRect& rrect, const StrokeData& strokeData)
{
    if(!rrect.isRounded()) {
        strokeRect(rrect.rect(), strokeData);
        return;
    }

    Path path;
    path.addRoundedRect(rrect);
    strokePath(path, strokeData);
}

void GraphicsContext::strokePath(const Path& path, const StrokeData& strokeData)
{
    cairo_new_path(m_canvas);
    set_cairo_path(m_canvas, path);
    set_cairo_stroke_data(m_canvas, strokeData);
    cairo_stroke(m_canvas);
}

void GraphicsContext::clipRect(const Rect& rect, FillRule clipRule)
{
    cairo_new_path(m_canvas);
    cairo_rectangle(m_canvas, rect.x, rect.y, rect.w, rect.h);
    cairo_set_fill_rule(m_canvas, to_cairo_fill_rule(clipRule));
    cairo_clip(m_canvas);
}

void GraphicsContext::clipRoundedRect(const RoundedRect& rrect, FillRule clipRule)
{
    if(!rrect.isRounded()) {
        clipRect(rrect.rect(), clipRule);
        return;
    }

    Path path;
    path.addRoundedRect(rrect);
    clipPath(path, clipRule);
}

void GraphicsContext::clipPath(const Path& path, FillRule clipRule)
{
    cairo_new_path(m_canvas);
    set_cairo_path(m_canvas, path);
    cairo_set_fill_rule(m_canvas, to_cairo_fill_rule(clipRule));
    cairo_clip(m_canvas);
}

void GraphicsContext::clipOutRect(const Rect& rect)
{
    double x1, y1, x2, y2;
    cairo_clip_extents(m_canvas, &x1, &y1, &x2, &y2);
    cairo_new_path(m_canvas);
    cairo_rectangle(m_canvas, x1, y1, x2 - x1, y2 - y1);
    cairo_rectangle(m_canvas, rect.x, rect.y, rect.w, rect.h);
    cairo_set_fill_rule(m_canvas, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_clip(m_canvas);
}

void GraphicsContext::clipOutRoundedRect(const RoundedRect& rrect)
{
    if(!rrect.isRounded()) {
        clipOutRect(rrect.rect());
        return;
    }

    Path path;
    path.addRoundedRect(rrect);
    clipOutPath(path);
}

void GraphicsContext::clipOutPath(const Path& path)
{
    double x1, y1, x2, y2;
    cairo_clip_extents(m_canvas, &x1, &y1, &x2, &y2);
    cairo_new_path(m_canvas);
    cairo_rectangle(m_canvas, x1, y1, x2 - x1, y2 - y1);
    set_cairo_path(m_canvas, path);
    cairo_set_fill_rule(m_canvas, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_clip(m_canvas);
}

void GraphicsContext::save()
{
    cairo_save(m_canvas);
}

void GraphicsContext::restore()
{
    cairo_restore(m_canvas);
}

void GraphicsContext::pushGroup()
{
    cairo_push_group(m_canvas);
}

void GraphicsContext::popGroup(float opacity, BlendMode blendMode)
{
    cairo_pop_group_to_source(m_canvas);
    cairo_set_operator(m_canvas, to_cairo_operator(blendMode));
    cairo_paint_with_alpha(m_canvas, opacity);
    cairo_set_operator(m_canvas, CAIRO_OPERATOR_OVER);
}

void GraphicsContext::applyMask(const ImageBuffer& maskImage)
{
    cairo_matrix_t matrix;
    cairo_get_matrix(m_canvas, &matrix);
    cairo_identity_matrix(m_canvas);
    cairo_set_source_surface(m_canvas, maskImage.surface(), maskImage.x(), maskImage.y());
    cairo_set_operator(m_canvas, CAIRO_OPERATOR_DEST_IN);
    cairo_paint(m_canvas);
    cairo_set_operator(m_canvas, CAIRO_OPERATOR_OVER);
    cairo_set_matrix(m_canvas, &matrix);
}

static void append_attribute(std::ostream& output, const std::string_view& name, const std::string_view& value)
{
    output << name << "='";
    for(auto cc : value) {
        if(cc == '\\' || cc == '\'')
            output << '\\';
        output << cc;
    }

    output << '\'';
}

void GraphicsContext::addLinkAnnotation(const std::string_view& dest, const std::string_view& uri, const Rect& rect)
{
    if(dest.empty() && uri.empty())
        return;
    double x = rect.x, y = rect.y;
    double w = rect.w, h = rect.h;
    cairo_user_to_device(m_canvas, &x, &y);
    cairo_user_to_device_distance(m_canvas, &w, &h);

    std::ostringstream ss;
    ss << "rect=[" << x << ' '  << y << ' '  << w << ' '  << h << ']';
    if(!dest.empty()) {
        append_attribute(ss, "dest", dest);
    } else if(!uri.empty()) {
        append_attribute(ss, "uri", uri);
    }

    cairo_tag_begin(m_canvas, CAIRO_TAG_LINK, ss.str().data());
    cairo_tag_end(m_canvas, CAIRO_TAG_LINK);
}

void GraphicsContext::addLinkDestination(const std::string_view& name, const Point& location)
{
    if(name.empty())
        return;
    double x = location.x;
    double y = location.y;
    cairo_user_to_device(m_canvas, &x, &y);

    std::ostringstream ss;
    append_attribute(ss, "name", name);
    ss << " x=" << x << " y=" << y;

    cairo_tag_begin(m_canvas, CAIRO_TAG_DEST, ss.str().data());
    cairo_tag_end(m_canvas, CAIRO_TAG_DEST);
}

std::unique_ptr<ImageBuffer> ImageBuffer::create(const Rect& rect)
{
    return create(rect.x, rect.y, rect.w, rect.h);
}

std::unique_ptr<ImageBuffer> ImageBuffer::create(float x, float y, float width, float height)
{
    if(width <= 0.f || height <= 0.f)
        return std::unique_ptr<ImageBuffer>(new ImageBuffer(0, 0, 1, 1));
    auto l = static_cast<int>(std::floor(x));
    auto t = static_cast<int>(std::floor(y));
    auto r = static_cast<int>(std::ceil(x + width));
    auto b = static_cast<int>(std::ceil(y + height));
    return std::unique_ptr<ImageBuffer>(new ImageBuffer(l, t, r - l, b - t));
}

const int ImageBuffer::width() const
{
    return cairo_image_surface_get_width(m_surface);
}

const int ImageBuffer::height() const
{
    return cairo_image_surface_get_height(m_surface);
}

void ImageBuffer::convertToLuminanceMask()
{
    auto width = cairo_image_surface_get_width(m_surface);
    auto height = cairo_image_surface_get_height(m_surface);
    auto stride = cairo_image_surface_get_stride(m_surface);
    auto data = cairo_image_surface_get_data(m_surface);
    for(int y = 0; y < height; y++) {
        auto pixels = reinterpret_cast<uint32_t*>(data + stride * y);
        for(int x = 0; x < width; x++) {
            auto pixel = pixels[x];
            auto a = (pixel >> 24) & 0xFF;
            auto r = (pixel >> 16) & 0xFF;
            auto g = (pixel >> 8) & 0xFF;
            auto b = (pixel >> 0) & 0xFF;
            if(a) {
                r = (r * 255) / a;
                g = (g * 255) / a;
                b = (b * 255) / a;
            }

            auto l = (r * 0.2125 + g * 0.7154 + b * 0.0721);
            pixels[x] = static_cast<uint32_t>(l * (a / 255.0)) << 24;
        }
    }
}

ImageBuffer::~ImageBuffer()
{
    cairo_destroy(m_canvas);
    cairo_surface_destroy(m_surface);
}

ImageBuffer::ImageBuffer(int x, int y, int width, int height)
    : m_surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height))
    , m_canvas(cairo_create(m_surface))
    , m_x(x), m_y(y)
{
    cairo_translate(m_canvas, -x, -y);
}

} // namespace plutobook
