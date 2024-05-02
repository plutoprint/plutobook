#include "geometry.h"

#include <cfloat>
#include <cmath>

namespace plutobook {

const Rect Rect::Empty(0, 0, 0, 0);
const Rect Rect::Invalid(0, 0, -1, -1);
const Rect Rect::Infinite(-FLT_MAX / 2.f, -FLT_MAX / 2.f, FLT_MAX, FLT_MAX);

const Transform Transform::Identity(1, 0, 0, 1, 0, 0);

Transform::Transform(float m00, float m10, float m01, float m11, float m02, float m12)
    : m00(m00), m10(m10), m01(m01), m11(m11), m02(m02), m12(m12)
{
}

Transform Transform::inverted() const
{
    float det = (this->m00 * this->m11 - this->m10 * this->m01);
    if(det == 0.f)
        return Transform();

    float inv_det = 1.f / det;
    float m00 = this->m00 * inv_det;
    float m10 = this->m10 * inv_det;
    float m01 = this->m01 * inv_det;
    float m11 = this->m11 * inv_det;
    float m02 = (this->m01 * this->m12 - this->m11 * this->m02) * inv_det;
    float m12 = (this->m10 * this->m02 - this->m00 * this->m12) * inv_det;

    return Transform(m11, -m10, -m01, m00, m02, m12);
}

Transform Transform::operator*(const Transform& transform) const
{
    float m00 = transform.m00 * this->m00 + transform.m10 * this->m01;
    float m10 = transform.m00 * this->m10 + transform.m10 * this->m11;
    float m01 = transform.m01 * this->m00 + transform.m11 * this->m01;
    float m11 = transform.m01 * this->m10 + transform.m11 * this->m11;
    float m02 = transform.m02 * this->m00 + transform.m12 * this->m01 + this->m02;
    float m12 = transform.m02 * this->m10 + transform.m12 * this->m11 + this->m12;

    return Transform(m00, m10, m01, m11, m02, m12);
}

Transform& Transform::operator*=(const Transform& transform)
{
    return (*this = *this * transform);
}

Transform& Transform::multiply(const Transform& transform)
{
    return (*this = *this * transform);
}

Transform& Transform::rotate(float angle)
{
    return multiply(rotated(angle));
}

Transform& Transform::rotate(float angle, float cx, float cy)
{
    return multiply(rotated(angle, cx, cy));
}

Transform& Transform::scale(float sx, float sy)
{
    return multiply(scaled(sx, sy));
}

Transform& Transform::shear(float shx, float shy)
{
    return multiply(sheared(shx, shy));
}

Transform& Transform::translate(float tx, float ty)
{
    return multiply(translated(tx, ty));
}

Transform& Transform::postMultiply(const Transform& transform)
{
    return (*this = transform * *this);
}

Transform& Transform::postRotate(float angle)
{
    return postMultiply(rotated(angle));
}

Transform& Transform::postRotate(float angle, float cx, float cy)
{
    return postMultiply(rotated(angle, cx, cy));
}

Transform& Transform::postScale(float sx, float sy)
{
    return postMultiply(scaled(sx, sy));
}

Transform& Transform::postShear(float shx, float shy)
{
    return postMultiply(sheared(shx, shy));
}

Transform& Transform::postTranslate(float tx, float ty)
{
    return postMultiply(translated(tx, ty));
}

Transform& Transform::identity()
{
    return (*this = Transform(1, 0, 0, 1, 0, 0));
}

Transform& Transform::invert()
{
    return (*this = inverted());
}

Point Transform::mapPoint(float x, float y) const
{
    return Point(x * m00 + y * m01 + m02, x * m10 + y * m11 + m12);
}

Point Transform::mapPoint(const Point& point) const
{
    return mapPoint(point.x, point.y);
}

Rect Transform::mapRect(const Rect& rect) const
{
    if(!rect.isValid())
        return Rect::Invalid;

    auto x1 = rect.x;
    auto y1 = rect.y;
    auto x2 = rect.x + rect.w;
    auto y2 = rect.y + rect.h;
    const Point points[] = {
        mapPoint(x1, y1), mapPoint(x2, y1),
        mapPoint(x2, y2), mapPoint(x1, y2)
    };

    auto l = points[0].x;
    auto t = points[0].y;
    auto r = points[0].x;
    auto b = points[0].y;
    for(int i = 1; i < 4; i++) {
        if(points[i].x < l) l = points[i].x;
        if(points[i].x > r) r = points[i].x;
        if(points[i].y < t) t = points[i].y;
        if(points[i].y > b) b = points[i].y;
    }

    return Rect(l, t, r - l, b - t);
}

float Transform::xScale() const
{
    return std::sqrt(m00 * m00 + m01 * m01);
}

float Transform::yScale() const
{
    return std::sqrt(m10 * m10 + m11 * m11);
}

constexpr auto pi = std::numbers::pi;

Transform Transform::rotated(float angle)
{
    auto c = std::cos(angle * pi / 180.f);
    auto s = std::sin(angle * pi / 180.f);

    return Transform(c, s, -s, c, 0, 0);
}

Transform Transform::rotated(float angle, float cx, float cy)
{
    auto c = std::cos(angle * pi / 180.f);
    auto s = std::sin(angle * pi / 180.f);

    auto x = cx * (1 - c) + cy * s;
    auto y = cy * (1 - c) - cx * s;

    return Transform(c, s, -s, c, x, y);
}

Transform Transform::scaled(float sx, float sy)
{
    return Transform(sx, 0, 0, sy, 0, 0);
}

Transform Transform::sheared(float shx, float shy)
{
    auto x = std::tan(shx * pi / 180.f);
    auto y = std::tan(shy * pi / 180.f);

    return Transform(1, y, x, 1, 0, 0);
}

Transform Transform::translated(float tx, float ty)
{
    return Transform(1, 0, 0, 1, tx, ty);
}

void Path::moveTo(float x1, float y1)
{
    m_commands.push_back(PathCommand::MoveTo);
    m_points.emplace_back(x1, y1);
}

void Path::lineTo(float x1, float y1)
{
    m_commands.push_back(PathCommand::LineTo);
    m_points.emplace_back(x1, y1);
}

void Path::cubicTo(float x1, float y1, float x2, float y2, float x3, float y3)
{
    m_commands.push_back(PathCommand::CubicTo);
    m_points.emplace_back(x1, y1);
    m_points.emplace_back(x2, y2);
    m_points.emplace_back(x3, y3);
}

void Path::close()
{
    if(!m_commands.empty() && m_commands.back() != PathCommand::Close) {
        m_commands.push_back(PathCommand::Close);
    }
}

void Path::quadTo(float cx, float cy, float x1, float y1, float x2, float y2)
{
    auto cx1 = 2.f / 3.f * x1 + 1.f / 3.f * cx;
    auto cy1 = 2.f / 3.f * y1 + 1.f / 3.f * cy;
    auto cx2 = 2.f / 3.f * x1 + 1.f / 3.f * x2;
    auto cy2 = 2.f / 3.f * y1 + 1.f / 3.f * y2;
    cubicTo(cx1, cy1, cx2, cy2, x2, y2);
}

void Path::arcTo(float cx, float cy, float rx, float ry, float xAxisRotation, bool largeArcFlag, bool sweepFlag, float x, float y)
{
    rx = std::fabs(rx);
    ry = std::fabs(ry);
    if(!rx || !ry || (cx == x && cy == y)) {
        lineTo(x, y);
        return;
    }

    float sin_th = std::sin(xAxisRotation * pi / 180.f);
    float cos_th = std::cos(xAxisRotation * pi / 180.f);

    float dx = (cx - x) / 2.f;
    float dy = (cy - y) / 2.f;
    float dx1 = cos_th * dx + sin_th * dy;
    float dy1 = -sin_th * dx + cos_th * dy;

    float dx1dx1 = dx1 * dx1;
    float dy1dy1 = dy1 * dy1;
    float rxrx = rx * rx;
    float ryry = ry * ry;
    float check = dx1dx1 / rxrx + dy1dy1 / ryry;
    if(check > 1.f) {
        rx = rx * std::sqrt(check);
        ry = ry * std::sqrt(check);
    }

    float a00 = cos_th / rx;
    float a01 = sin_th / rx;
    float a10 = -sin_th / ry;
    float a11 = cos_th / ry;

    float x0 = a00 * cx + a01 * cy;
    float y0 = a10 * cx + a11 * cy;
    float x1 = a00 * x + a01 * y;
    float y1 = a10 * x + a11 * y;

    float d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    float sfactor_sq = 1.f / d - 0.25f;
    if(sfactor_sq < 0.f) sfactor_sq = 0.f;
    float sfactor = std::sqrt(sfactor_sq);
    if(sweepFlag == largeArcFlag) sfactor = -sfactor;

    float xc = 0.5f * (x0 + x1) - sfactor * (y1 - y0);
    float yc = 0.5f * (y0 + y1) + sfactor * (x1 - x0);

    float th0 = std::atan2(y0 - yc, x0 - xc);
    float th1 = std::atan2(y1 - yc, x1 - xc);
    float th_arc = th1 - th0;
    if(th_arc < 0.f && sweepFlag)
        th_arc += 2.f * pi;
    else if(th_arc > 0.f && !sweepFlag) {
        th_arc -= 2.f * pi;
    }

    int segments = std::ceil(std::fabs(th_arc / (pi * 0.5f + 0.001f)));
    for(int i = 0; i < segments; i++) {
        float th2 = th0 + i * th_arc / segments;
        float th3 = th0 + (i + 1) * th_arc / segments;

        float a00 = cos_th * rx;
        float a01 = -sin_th * ry;
        float a10 = sin_th * rx;
        float a11 = cos_th * ry;

        float thHalf = 0.5f * (th3 - th2);
        float t = (8.f / 3.f) * std::sin(thHalf * 0.5f) * std::sin(thHalf * 0.5f) / std::sin(thHalf);
        float x1 = xc + std::cos(th2) - t * std::sin(th2);
        float y1 = yc + std::sin(th2) + t * std::cos(th2);
        float x3 = xc + std::cos(th3);
        float y3 = yc + std::sin(th3);
        float x2 = x3 + t * std::sin(th3);
        float y2 = y3 - t * std::cos(th3);

        float cx1 = a00 * x1 + a01 * y1;
        float cy1 = a10 * x1 + a11 * y1;
        float cx2 = a00 * x2 + a01 * y2;
        float cy2 = a10 * x2 + a11 * y2;
        float cx3 = a00 * x3 + a01 * y3;
        float cy3 = a10 * x3 + a11 * y3;
        cubicTo(cx1, cy1, cx2, cy2, cx3, cy3);
    }
}

void Path::addEllipse(float cx, float cy, float rx, float ry)
{
    constexpr auto kappa = 0.55228474983079339840;

    auto left = cx - rx;
    auto top = cy - ry;
    auto right = cx + rx;
    auto bottom = cy + ry;

    auto cpx = rx * kappa;
    auto cpy = ry * kappa;

    moveTo(cx, top);
    cubicTo(cx+cpx, top, right, cy-cpy, right, cy);
    cubicTo(right, cy+cpy, cx+cpx, bottom, cx, bottom);
    cubicTo(cx-cpx, bottom, left, cy+cpy, left, cy);
    cubicTo(left, cy-cpy, cx-cpx, top, cx, top);
    close();
}

void Path::addRoundedRect(const RoundedRect& rrect)
{
    addRoundedRect(rrect.rect(), rrect.radii());
}

void Path::addRoundedRect(const Rect& rect, const RectRadii& radii)
{
    if(radii.isZero()) {
        addRect(rect);
        return;
    }

    auto x1 = rect.x;
    auto x2 = rect.x + rect.w;
    auto y1 = rect.y;
    auto y2 = rect.y + rect.h;

    constexpr auto ccp = 0.447715f;

    moveTo(x1 + radii.tl.w, y1);
    lineTo(x2 - radii.tr.w, y1);
    if(radii.tr.w > 0 || radii.tr.h > 0) {
        cubicTo(x2 - radii.tr.w * ccp, y1, x2, y1 + radii.tr.h * ccp, x2, y1 + radii.tr.h);
    }

    lineTo(x2, y2 - radii.br.h);
    if(radii.br.w > 0 || radii.br.h > 0) {
        cubicTo(x2, y2 - radii.br.h * ccp, x2 - radii.br.w * ccp, y2, x2 - radii.br.w, y2);
    }

    lineTo(x1 + radii.bl.w, y2);
    if(radii.bl.w > 0 || radii.bl.h > 0) {
        cubicTo(x1 + radii.bl.w * ccp, y2, x1, y2 - radii.bl.h * ccp, x1, y2 - radii.bl.h);
    }

    lineTo(x1, y1 + radii.tl.h);
    if(radii.tl.w > 0 || radii.tl.h > 0) {
        cubicTo(x1, y1 + radii.tl.h * ccp, x1 + radii.tl.w * ccp, y1, x1 + radii.tl.w, y1);
    }

    close();
}

void Path::addRect(const Rect& rect)
{
    if(rect.isEmpty())
        return;
    auto x1 = rect.x;
    auto x2 = rect.x + rect.w;
    auto y1 = rect.y;
    auto y2 = rect.y + rect.h;

    moveTo(x1, y1);
    lineTo(x2, y1);
    lineTo(x2, y2);
    lineTo(x1, y2);
    close();
}

Rect Path::boundingRect() const
{
    if(m_commands.empty())
        return Rect();
    auto l = m_points[0].x;
    auto t = m_points[0].y;
    auto r = m_points[0].x;
    auto b = m_points[0].y;
    for(size_t i = 1; i < m_points.size(); i++) {
        if(m_points[i].x < l) l = m_points[i].x;
        if(m_points[i].x > r) r = m_points[i].x;
        if(m_points[i].y < t) t = m_points[i].y;
        if(m_points[i].y > b) b = m_points[i].y;
    }

    return Rect(l, t, r - l, b - t);
}

Path& Path::transform(const Transform& transform)
{
    for(auto& p : m_points)
        p = transform.mapPoint(p);
    return *this;
}

Path Path::transformed(const Transform& transform) const
{
    return Path(*this).transform(transform);
}

PathIterator::PathIterator(const Path& path)
    : m_commands(path.commands())
    , m_points(path.points().data())
{
}

PathCommand PathIterator::currentSegment(std::array<Point, 3>& points) const
{
    auto command = m_commands[m_index];
    switch(command) {
    case PathCommand::MoveTo:
        points[0] = m_points[0];
        m_startPoint = points[0];
        break;
    case PathCommand::LineTo:
        points[0] = m_points[0];
        break;
    case PathCommand::CubicTo:
        points[0] = m_points[0];
        points[1] = m_points[1];
        points[2] = m_points[2];
        break;
    case PathCommand::Close:
        points[0] = m_startPoint;
        break;
    }

    return command;
}

bool PathIterator::isDone() const
{
    return (m_index >= m_commands.size());
}

void PathIterator::next()
{
    switch(m_commands[m_index]) {
    case PathCommand::MoveTo:
    case PathCommand::LineTo:
        m_points += 1;
        break;
    case PathCommand::CubicTo:
        m_points += 3;
        break;
    default:
        break;
    }

    m_index += 1;
}

} // namespace plutobook
