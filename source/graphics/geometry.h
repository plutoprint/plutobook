#ifndef PLUTOBOOK_GEOMETRY_H
#define PLUTOBOOK_GEOMETRY_H

#include <array>
#include <tuple>
#include <algorithm>
#include <vector>
#include <cstdint>

namespace plutobook {

class Point {
public:
    constexpr Point() = default;
    constexpr Point(float x, float y) : x(x), y(y) {}

    constexpr void move(float dx, float dy) { x += dx; y += dy; }
    constexpr void move(float d) { move(d, d); }
    constexpr void move(const Point& p) { move(p.x, p.y); }

    constexpr void scale(float sx, float sy) { x *= sx; y *= sy; }
    constexpr void scale(float s) { scale(s, s); }

    constexpr float dot(const Point& p) const { return x * p.x + y * p.y; }

public:
    float x{0};
    float y{0};
};

constexpr Point operator+(const Point& a, const Point& b)
{
    return Point(a.x + b.x, a.y + b.y);
}

constexpr Point operator-(const Point& a, const Point& b)
{
    return Point(a.x - b.x, a.y - b.y);
}

constexpr Point operator-(const Point& a)
{
    return Point(-a.x, -a.y);
}

constexpr Point& operator+=(Point& a, const Point& b)
{
    a.move(b);
    return a;
}

constexpr Point& operator-=(Point& a, const Point& b)
{
    a.move(-b);
    return a;
}

constexpr float operator*(const Point& a, const Point& b)
{
    return a.dot(b);
}

constexpr bool operator==(const Point& a, const Point& b)
{
    return std::tie(a.x, a.y) == std::tie(b.x, b.y);
}

constexpr bool operator!=(const Point& a, const Point& b)
{
    return std::tie(a.x, a.y) != std::tie(b.x, b.y);
}

constexpr bool operator<(const Point& a, const Point& b)
{
    return std::tie(a.x, a.y) < std::tie(b.x, b.y);
}

constexpr bool operator>(const Point& a, const Point& b)
{
    return std::tie(a.x, a.y) > std::tie(b.x, b.y);
}

constexpr bool operator<=(const Point& a, const Point& b)
{
    return std::tie(a.x, a.y) <= std::tie(b.x, b.y);
}

constexpr bool operator>=(const Point& a, const Point& b)
{
    return std::tie(a.x, a.y) >= std::tie(b.x, b.y);
}

class Size {
public:
    constexpr Size() = default;
    constexpr Size(float w, float h) : w(w), h(h) {}

    constexpr void expand(float dw, float dh) { w += dw; h += dh; }
    constexpr void expand(float d) { expand(d, d); }
    constexpr void expand(const Size& s) { expand(s.w, s.h); }

    constexpr void scale(float sw, float sh) { w *= sw; h *= sh; }
    constexpr void scale(float s) { scale(s, s); }

    constexpr bool isEmpty() const { return w <= 0.f || h <= 0.f; }
    constexpr bool isZero() const { return w <= 0.f && h <= 0.f; }
    constexpr bool isValid() const { return w >= 0.f && h >= 0.f; }

public:
    float w{0};
    float h{0};
};

constexpr Size operator+(const Size& a, const Size& b)
{
    return Size(a.w + b.w, a.h + b.h);
}

constexpr Size operator-(const Size& a, const Size& b)
{
    return Size(a.w - b.w, a.h - b.h);
}

constexpr Size operator-(const Size& a)
{
    return Size(-a.w, -a.h);
}

constexpr Size& operator+=(Size& a, const Size& b)
{
    a.expand(b);
    return a;
}

constexpr Size& operator-=(Size& a, const Size& b)
{
    a.expand(-b);
    return a;
}

constexpr bool operator==(const Size& a, const Size& b)
{
    return std::tie(a.w, a.h) == std::tie(b.w, b.h);
}

constexpr bool operator!=(const Size& a, const Size& b)
{
    return std::tie(a.w, a.h) != std::tie(b.w, b.h);
}

constexpr bool operator<(const Size& a, const Size& b)
{
    return std::tie(a.w, a.h) < std::tie(b.w, b.h);
}

constexpr bool operator>(const Size& a, const Size& b)
{
    return std::tie(a.w, a.h) > std::tie(b.w, b.h);
}

constexpr bool operator<=(const Size& a, const Size& b)
{
    return std::tie(a.w, a.h) <= std::tie(b.w, b.h);
}

constexpr bool operator>=(const Size& a, const Size& b)
{
    return std::tie(a.w, a.h) >= std::tie(b.w, b.h);
}

class RectOutsets {
public:
    constexpr RectOutsets() = default;
    constexpr explicit RectOutsets(float outset) : RectOutsets(outset, outset, outset, outset) {}
    constexpr RectOutsets(float top, float right, float bottom, float left)
        : t(top), r(right), b(bottom), l(left)
    {}

public:
    float t{0};
    float r{0};
    float b{0};
    float l{0};
};

class Rect {
public:
    constexpr Rect() = default;
    constexpr explicit Rect(const Size& size) : Rect(size.w, size.h) {}
    constexpr Rect(float width, float height) : Rect(0, 0, width, height) {}
    constexpr Rect(const Point& origin, const Size& size) : Rect(origin.x, origin.y, size.w, size.h) {}
    constexpr Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

    constexpr void move(float dx, float dy) { x += dx; y += dy; }
    constexpr void move(float d) { move(d, d); }
    constexpr void move(const Point& p) { move(p.x, p.y); }

    constexpr void expand(float t, float r, float b, float l) { x -= l; y -= t; w += l + r; h += t + b; }
    constexpr void shrink(float t, float r, float b, float l) { expand(-t, -r, -b, -l); }

    constexpr void expand(const RectOutsets& outsets) { expand(outsets.t, outsets.r, outsets.b, outsets.l); }
    constexpr void shrink(const RectOutsets& outsets) { shrink(outsets.t, outsets.r, outsets.b, outsets.l); }

    constexpr Rect operator+(const RectOutsets& outsets) const;
    constexpr Rect operator-(const RectOutsets& outsets) const;

    constexpr Rect& operator+=(const RectOutsets& outsets);
    constexpr Rect& operator-=(const RectOutsets& outsets);

    constexpr void scale(float sx, float sy) { x *= sx; y *= sy; w *= sx; h *= sy; }
    constexpr void scale(float s) { scale(s, s); }

    constexpr void inflate(float dx, float dy) { x -= dx; y -= dy; w += dx * 2.f; h += dy * 2.f; }
    constexpr void inflate(float d) { inflate(d, d); }

    constexpr Rect moved(float dx, float dy) const { return Rect(x + dx, y + dy, w, h); }
    constexpr Rect moved(const Point& point) const { return moved(point.x, point.y); }

    constexpr Rect intersected(const Rect& rect) const;
    constexpr Rect united(const Rect& rect) const;

    constexpr Rect& intersect(const Rect& o);
    constexpr Rect& unite(const Rect& o);

    constexpr bool contains(const Point& p) const { return p.x >= x && p.x < right() && p.y >= y && p.y < bottom(); }
    constexpr bool contains(const Rect& r) const { return x <= r.x && y <= r.y && right() >= r.right() && bottom() >= r.bottom(); }
    constexpr bool intersects(const Rect& r) const { return !isEmpty() && !r.isEmpty() && x < r.right() && r.x < right() && y < r.bottom() && r.y < bottom(); }

    constexpr float right() const { return x + w; }
    constexpr float bottom() const { return y + h; }

    constexpr Point origin() const { return Point(x, y); }
    constexpr Size size() const { return Size(w, h); }

    constexpr Point topLeft() const { return Point(x, y); }
    constexpr Point topRight() const { return Point(x + w, y); }
    constexpr Point bottomLeft() const { return Point(x, y + h); }
    constexpr Point bottomRight() const { return Point(x + w, y + h); }

    constexpr bool isEmpty() const { return w <= 0.f || h <= 0.f; }
    constexpr bool isZero() const { return w <= 0.f && h <= 0.f; }
    constexpr bool isValid() const { return w >= 0.f && h >= 0.f; }

    static const Rect Empty;
    static const Rect Invalid;
    static const Rect Infinite;

public:
    float x{0};
    float y{0};
    float w{0};
    float h{0};
};

constexpr Rect Rect::operator+(const RectOutsets& outsets) const
{
    Rect rect(*this);
    rect.expand(outsets);
    return rect;
}

constexpr Rect Rect::operator-(const RectOutsets& outsets) const
{
    Rect rect(*this);
    rect.shrink(outsets);
    return rect;
}

constexpr Rect& Rect::operator+=(const RectOutsets& outsets)
{
    expand(outsets);
    return *this;
}

constexpr Rect& Rect::operator-=(const RectOutsets& outsets)
{
    shrink(outsets);
    return *this;
}

constexpr Rect Rect::intersected(const Rect& rect) const
{
    if(!rect.isValid())
        return *this;
    if(!isValid())
        return rect;
    auto l = std::max(x, rect.x);
    auto t = std::max(y, rect.y);
    auto r = std::min(x + w, rect.x + rect.w);
    auto b = std::min(y + h, rect.y + rect.h);
    if(l >= r || t >= b)
        return Rect::Empty;
    return Rect(l, t, r - l, b - t);
}

constexpr Rect Rect::united(const Rect& rect) const
{
    if(!rect.isValid())
        return *this;
    if(!isValid())
        return rect;
    auto l = std::min(x, rect.x);
    auto t = std::min(y, rect.y);
    auto r = std::max(x + w, rect.x + rect.w);
    auto b = std::max(y + h, rect.y + rect.h);
    return Rect(l, t, r - l, b - t);
}

constexpr Rect& Rect::intersect(const Rect& o)
{
    *this = intersected(o);
    return *this;
}

constexpr Rect& Rect::unite(const Rect& o)
{
    *this = united(o);
    return *this;
}

constexpr bool operator==(const Rect& a, const Rect& b)
{
    return std::tie(a.x, a.y, a.w, a.h) == std::tie(b.x, b.y, b.w, b.h);
}

constexpr bool operator!=(const Rect& a, const Rect& b)
{
    return std::tie(a.x, a.y, a.w, a.h) != std::tie(b.x, b.y, b.w, b.h);
}

constexpr bool operator<(const Rect& a, const Rect& b)
{
    return std::tie(a.x, a.y, a.w, a.h) < std::tie(b.x, b.y, b.w, b.h);
}

constexpr bool operator>(const Rect& a, const Rect& b)
{
    return std::tie(a.x, a.y, a.w, a.h) > std::tie(b.x, b.y, b.w, b.h);
}

constexpr bool operator<=(const Rect& a, const Rect& b)
{
    return std::tie(a.x, a.y, a.w, a.h) <= std::tie(b.x, b.y, b.w, b.h);
}

constexpr bool operator>=(const Rect& a, const Rect& b)
{
    return std::tie(a.x, a.y, a.w, a.h) >= std::tie(b.x, b.y, b.w, b.h);
}

class RectRadii {
public:
    constexpr RectRadii() = default;
    constexpr explicit RectRadii(float radius) : RectRadii(radius, radius) {}
    constexpr RectRadii(float rx, float ry)
        : tl(rx, ry), tr(rx, ry), bl(rx, ry), br(rx, ry)
    {}

    constexpr explicit RectRadii(const Size& radii) : RectRadii(radii, radii, radii, radii) {}
    constexpr RectRadii(const Size& tl, const Size& tr, const Size& bl, const Size& br)
        : tl(tl), tr(tr), bl(bl), br(br)
    {}

    constexpr void constrain(float width, float height);

    constexpr void expand(float t, float r, float b, float l);
    constexpr void shrink(float t, float r, float b, float l) { expand(-t, -r, -b, -l); }

    constexpr void expand(const RectOutsets& outsets) { expand(outsets.t, outsets.r, outsets.b, outsets.l); }
    constexpr void shrink(const RectOutsets& outsets) { shrink(outsets.t, outsets.r, outsets.b, outsets.l); }

    constexpr RectRadii operator+(const RectOutsets& outsets) const;
    constexpr RectRadii operator-(const RectOutsets& outsets) const;

    constexpr RectRadii& operator+=(const RectOutsets& outsets);
    constexpr RectRadii& operator-=(const RectOutsets& outsets);

    constexpr bool isZero() const { return tl.isZero() && tr.isZero() && bl.isZero() && br.isZero(); };

public:
    Size tl;
    Size tr;
    Size bl;
    Size br;
};

constexpr void RectRadii::constrain(float width, float height)
{
    float factor = 1.f;
    auto horizontalSum = std::max(tl.w + tr.w, bl.w + br.w);
    if(horizontalSum > width)
        factor = std::min(factor, width / horizontalSum);
    auto verticalSum = std::max(tl.h + bl.h, tr.h + br.h);
    if(verticalSum > height) {
        factor = std::min(factor, height / verticalSum);
    }

    if(factor == 1.f)
        return;
    tl.scale(factor);
    tr.scale(factor);
    bl.scale(factor);
    br.scale(factor);

    if(tl.isEmpty()) tl = Size();
    if(tr.isEmpty()) tr = Size();
    if(bl.isEmpty()) bl = Size();
    if(br.isEmpty()) br = Size();
}

constexpr void RectRadii::expand(float t, float r, float b, float l)
{
    if(!tl.isEmpty()) tl.expand(l, t);
    if(!tr.isEmpty()) tr.expand(r, t);
    if(!bl.isEmpty()) bl.expand(l, b);
    if(!br.isEmpty()) br.expand(r, b);

    if(tl.isEmpty()) tl = Size();
    if(tr.isEmpty()) tr = Size();
    if(bl.isEmpty()) bl = Size();
    if(br.isEmpty()) br = Size();
}

constexpr RectRadii RectRadii::operator+(const RectOutsets& outsets) const
{
    RectRadii radii(*this);
    radii.expand(outsets);
    return radii;
}

constexpr RectRadii RectRadii::operator-(const RectOutsets& outsets) const
{
    RectRadii radii(*this);
    radii.shrink(outsets);
    return radii;
}

constexpr RectRadii& RectRadii::operator+=(const RectOutsets& outsets)
{
    expand(outsets);
    return *this;
}

constexpr RectRadii& RectRadii::operator-=(const RectOutsets& outsets)
{
    shrink(outsets);
    return *this;
}

class RoundedRect {
public:
    constexpr RoundedRect() = default;
    constexpr RoundedRect(const Rect& rect, const RectRadii& radii)
        : m_rect(rect), m_radii(radii)
    {}

    constexpr bool isRounded() const { return !m_radii.isZero(); }

    constexpr void expand(float t, float r, float b, float l);
    constexpr void shrink(float t, float r, float b, float l) { expand(-t, -r, -b, -l); }

    constexpr void expand(const RectOutsets& outsets) { expand(outsets.t, outsets.r, outsets.b, outsets.l); }
    constexpr void shrink(const RectOutsets& outsets) { shrink(outsets.t, outsets.r, outsets.b, outsets.l); }

    constexpr RoundedRect operator+(const RectOutsets& outsets) const;
    constexpr RoundedRect operator-(const RectOutsets& outsets) const;

    constexpr RoundedRect& operator+=(const RectOutsets& outsets);
    constexpr RoundedRect& operator-=(const RectOutsets& outsets);

    constexpr const Rect& rect() const { return m_rect; }
    constexpr const RectRadii& radii() const { return m_radii; }

private:
    Rect m_rect;
    RectRadii m_radii;
};

constexpr void RoundedRect::expand(float t, float r, float b, float l)
{
    m_rect.expand(t, r, b, l);
    m_radii.expand(t, r, b, l);
}

constexpr RoundedRect RoundedRect::operator+(const RectOutsets& outsets) const
{
    RoundedRect rect(*this);
    rect.expand(outsets);
    return rect;
}

constexpr RoundedRect RoundedRect::operator-(const RectOutsets& outsets) const
{
    RoundedRect rect(*this);
    rect.shrink(outsets);
    return rect;
}

constexpr RoundedRect& RoundedRect::operator+=(const RectOutsets& outsets)
{
    expand(outsets);
    return *this;
}

constexpr RoundedRect& RoundedRect::operator-=(const RectOutsets& outsets)
{
    shrink(outsets);
    return *this;
}

class Transform {
public:
    Transform() = default;
    Transform(float a, float b, float c, float d, float e, float f);

    Transform inverted() const;
    Transform operator*(const Transform& transform) const;
    Transform& operator*=(const Transform& transform);

    Transform& multiply(const Transform& transform);
    Transform& rotate(float angle);
    Transform& rotate(float angle, float cx, float cy);
    Transform& scale(float sx, float sy);
    Transform& shear(float shx, float shy);
    Transform& translate(float tx, float ty);

    Transform& postMultiply(const Transform& transform);
    Transform& postRotate(float angle);
    Transform& postRotate(float angle, float cx, float cy);
    Transform& postScale(float sx, float sy);
    Transform& postShear(float shx, float shy);
    Transform& postTranslate(float tx, float ty);

    Transform& invert();

    Point mapPoint(float x, float y) const;
    Point mapPoint(const Point& point) const;
    Rect mapRect(const Rect& rect) const;

    float xScale() const;
    float yScale() const;

    static Transform makeRotate(float angle);
    static Transform makeRotate(float angle, float cx, float cy);
    static Transform makeScale(float sx, float sy);
    static Transform makeShear(float shx, float shy);
    static Transform makeTranslate(float tx, float ty);

    static const Transform Identity;

public:
    float a{1};
    float b{0};
    float c{0};
    float d{1};
    float e{0};
    float f{0};
};

enum class PathCommand : uint8_t {
    MoveTo,
    LineTo,
    CubicTo,
    Close
};

class Path {
public:
    Path() = default;

    void moveTo(float x1, float y1);
    void lineTo(float x1, float y1);
    void cubicTo(float x1, float y1, float x2, float y2, float x3, float y3);
    void close();
    void clear();

    void addEllipse(float cx, float cy, float rx, float ry);
    void addRoundedRect(const RoundedRect& rrect);
    void addRoundedRect(const Rect& rect, const RectRadii& radii);
    void addRect(const Rect& rect);

    Rect boundingRect() const;

    Path& transform(const Transform& transform);
    Path transformed(const Transform& transform) const;

    bool isEmpty() const { return m_commands.empty(); }

    const std::vector<PathCommand>& commands() const { return m_commands; }
    const std::vector<Point>& points() const { return m_points; }

private:
    std::vector<PathCommand> m_commands;
    std::vector<Point> m_points;
};

inline void Path::clear()
{
    m_commands.clear();
    m_points.clear();
}

class PathIterator {
public:
    explicit PathIterator(const Path& path);

    PathCommand currentSegment(std::array<Point, 3>& points) const;
    bool isDone() const;
    void next();

private:
    mutable Point m_startPoint;
    const std::vector<PathCommand>& m_commands;
    const Point* m_points{nullptr};
    size_t m_index{0};
};

} // namespace plutobook

#endif // PLUTOBOOK_GEOMETRY_H
