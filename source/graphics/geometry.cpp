/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "geometry.h"

#include <cfloat>
#include <cmath>

namespace plutobook {

const Rect Rect::Empty(0, 0, 0, 0);
const Rect Rect::Invalid(0, 0, -1, -1);
const Rect Rect::Infinite(-FLT_MAX / 2.f, -FLT_MAX / 2.f, FLT_MAX, FLT_MAX);

const Transform Transform::Identity(1, 0, 0, 1, 0, 0);

Transform::Transform(float a, float b, float c, float d, float e, float f)
    : a(a), b(b), c(c), d(d), e(e), f(f)
{
}

Transform Transform::inverted() const
{
    float det = (a * d - b * c);
    if(det == 0.f) {
        return Transform();
    }

    float inv_det = 1.f / det;
    float aa = a * inv_det;
    float bb = b * inv_det;
    float cc = c * inv_det;
    float dd = d * inv_det;
    float ee = (c * f - d * e) * inv_det;
    float ff = (b * e - a * f) * inv_det;

    return Transform(dd, -bb, -cc, aa, ee, ff);
}

Transform Transform::operator*(const Transform& transform) const
{
    float aa = transform.a * a + transform.b * c;
    float bb = transform.a * b + transform.b * d;
    float cc = transform.c * a + transform.d * c;
    float dd = transform.c * b + transform.d * d;
    float ee = transform.e * a + transform.f * c + e;
    float ff = transform.e * b + transform.f * d + f;

    return Transform(aa, bb, cc, dd, ee, ff);
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
    return multiply(makeRotate(angle));
}

Transform& Transform::rotate(float angle, float cx, float cy)
{
    return multiply(makeRotate(angle, cx, cy));
}

Transform& Transform::scale(float sx, float sy)
{
    return multiply(makeScale(sx, sy));
}

Transform& Transform::shear(float shx, float shy)
{
    return multiply(makeShear(shx, shy));
}

Transform& Transform::translate(float tx, float ty)
{
    return multiply(makeTranslate(tx, ty));
}

Transform& Transform::postMultiply(const Transform& transform)
{
    return (*this = transform * *this);
}

Transform& Transform::postRotate(float angle)
{
    return postMultiply(makeRotate(angle));
}

Transform& Transform::postRotate(float angle, float cx, float cy)
{
    return postMultiply(makeRotate(angle, cx, cy));
}

Transform& Transform::postScale(float sx, float sy)
{
    return postMultiply(makeScale(sx, sy));
}

Transform& Transform::postShear(float shx, float shy)
{
    return postMultiply(makeShear(shx, shy));
}

Transform& Transform::postTranslate(float tx, float ty)
{
    return postMultiply(makeTranslate(tx, ty));
}

Transform& Transform::invert()
{
    return (*this = inverted());
}

Point Transform::mapPoint(float x, float y) const
{
    return Point(x * a + y * c + e, x * b + y * d + f);
}

Point Transform::mapPoint(const Point& point) const
{
    return mapPoint(point.x, point.y);
}

Rect Transform::mapRect(const Rect& rect) const
{
    if(!rect.isValid()) {
        return Rect::Invalid;
    }

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
    return std::sqrt(a * a + c * c);
}

float Transform::yScale() const
{
    return std::sqrt(b * b + d * d);
}

Transform Transform::makeRotate(float angle)
{
    auto c = std::cos(deg2rad(angle));
    auto s = std::sin(deg2rad(angle));

    return Transform(c, s, -s, c, 0, 0);
}

Transform Transform::makeRotate(float angle, float cx, float cy)
{
    auto c = std::cos(deg2rad(angle));
    auto s = std::sin(deg2rad(angle));

    auto x = cx * (1 - c) + cy * s;
    auto y = cy * (1 - c) - cx * s;

    return Transform(c, s, -s, c, x, y);
}

Transform Transform::makeScale(float sx, float sy)
{
    return Transform(sx, 0, 0, sy, 0, 0);
}

Transform Transform::makeShear(float shx, float shy)
{
    auto x = std::tan(deg2rad(shx));
    auto y = std::tan(deg2rad(shy));

    return Transform(1, y, x, 1, 0, 0);
}

Transform Transform::makeTranslate(float tx, float ty)
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

void Path::addEllipse(float cx, float cy, float rx, float ry)
{
    constexpr auto kappa = 0.552285f;

    auto x1 = cx - rx;
    auto y1 = cy - ry;
    auto x2 = cx + rx;
    auto y2 = cy + ry;

    auto cpx = rx * kappa;
    auto cpy = ry * kappa;

    moveTo(cx, y1);
    cubicTo(cx + cpx, y1, x2, cy - cpy, x2, cy);
    cubicTo(x2, cy + cpy, cx + cpx, y2, cx, y2);
    cubicTo(cx - cpx, y2, x1, cy + cpy, x1, cy);
    cubicTo(x1, cy - cpy, cx - cpx, y1, cx, y1);
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
