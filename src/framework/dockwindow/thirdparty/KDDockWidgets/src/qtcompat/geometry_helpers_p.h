/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <cmath>
#include <algorithm>

/// QSize, QPoint and QRect replacements for Flutter
/// The goal is that Qt users can still use Qt's classes, to avoid porting churn.
/// For Flutter, we use these classes, which have the same API (or at least a subset) as Qt's

namespace KDDockWidgets {

class Size
{
public:
    Size() = default;
    Size(int width, int height)
        : m_width(width)
        , m_height(height)
    {
    }

    int width() const
    {
        return m_width;
    }

    int height() const
    {
        return m_height;
    }

    void setWidth(int w)
    {
        m_width = w;
    }

    void setHeight(int h)
    {
        m_height = h;
    }

    bool isNull() const
    {
        return m_width == 0 && m_height == 0;
    }

    bool isEmpty() const
    {
        return m_width <= 0 || m_height <= 0;
    }

    bool isValid() const
    {
        return m_width >= 0 && m_height >= 0;
    }

    Size expandedTo(Size sz) const
    {
        return { std::max(m_width, sz.m_width), std::max(m_height, sz.height()) };
    }

    Size boundedTo(Size sz) const
    {
        return { std::min(m_width, sz.m_width), std::min(m_height, sz.height()) };
    }

    bool operator==(Size other) const
    {
        return m_width == other.m_width && m_height == other.m_height;
    }

    bool operator!=(Size other) const
    {
        return !operator==(other);
    }

private:
    int m_width = -1;
    int m_height = -1;
};

class Point
{
public:
    Point() = default;
    Point(int x, int y)
        : m_x(x)
        , m_y(y)
    {
    }

    int x() const
    {
        return m_x;
    }

    int y() const
    {
        return m_y;
    }

    void setX(int x)
    {
        m_x = x;
    }

    void setY(int y)
    {
        m_y = y;
    }

    bool isNull() const
    {
        return m_x == 0 && m_y == 0;
    }

    int manhattanLength() const
    {
        return std::abs(x()) + std::abs(y());
    }

    bool operator==(Point other) const
    {
        return m_x == other.m_x && m_y == other.m_y;
    }

    bool operator!=(Point other) const
    {
        return !operator==(other);
    }

private:
    int m_x = 0;
    int m_y = 0;
};

class Margins
{
public:
    Margins() = default;
    Margins(int l, int t, int r, int b)
        : m_left(l)
        , m_top(t)
        , m_right(r)
        , m_bottom(b)
    {
    }

    int left() const
    {
        return m_left;
    }

    int top() const
    {
        return m_top;
    }

    int right() const
    {
        return m_right;
    }

    int bottom() const
    {
        return m_bottom;
    }

private:
    int m_left = 0;
    int m_top = 0;
    int m_right = 0;
    int m_bottom = 0;
};

class Rect
{
public:
    Rect() = default;
    Rect(Point pos, Size size)
        : m_pos(pos)
        , m_size(size)
    {
    }

    Rect(int x, int y, int width, int height)
        : m_pos(Point(x, y))
        , m_size(Size(width, height))
    {
    }

    Point pos() const
    {
        return m_pos;
    };

    Size size() const
    {
        return m_size;
    }

    int width() const
    {
        return m_size.width();
    }

    int height() const
    {
        return m_size.height();
    }

    void setWidth(int w)
    {
        m_size.setWidth(w);
    }

    void setHeight(int h)
    {
        m_size.setHeight(h);
    }

    int x() const
    {
        return m_pos.x();
    }

    int y() const
    {
        return m_pos.y();
    }

    void setSize(Size sz)
    {
        m_size = sz;
    }

    Point topLeft() const
    {
        return m_pos;
    }

    int top() const
    {
        return y();
    }

    int bottom() const
    {
        return y() + height() - 1;
    }

    int right() const
    {
        return x() + width() - 1;
    }

    bool isNull() const
    {
        return m_pos.isNull() && m_size.isNull();
    }

    bool isValid() const
    {
        return !isEmpty();
    }

    bool isEmpty() const
    {
        return m_size.isEmpty();
    }

    void moveTop(int y)
    {
        m_pos.setY(y);
    }

    void moveLeft(int x)
    {
        m_pos.setX(x);
    }

    void moveTopLeft(Point pt)
    {
        m_pos = pt;
    }

    void moveTo(Point pt)
    {
        moveTopLeft(pt);
    }

    void moveTo(int x, int y)
    {
        moveTopLeft({ x, y });
    }

    void setLeft(int x)
    {
        const int delta = m_pos.x() - x;
        const int width = m_size.width();
        m_pos.setX(x);
        m_size.setWidth(width + delta);
    }

    void setRight(int r)
    {
        const int delta = r - right();
        const int width = m_size.width();
        m_size.setWidth(width + delta);
    }

    void setTop(int y)
    {
        const int delta = m_pos.y() - y;
        const int height = m_size.height();
        m_pos.setY(y);
        m_size.setHeight(height + delta);
    }

    void setTopLeft(Point pt)
    {
        setLeft(pt.x());
        setTop(pt.y());
    }

    void setBottom(int b)
    {
        const int delta = b - bottom();
        const int height = m_size.height();
        m_size.setHeight(height + delta);
    }

    void setX(int x)
    {
        setLeft(x);
    }

    void setY(int y)
    {
        setTop(y);
    }

    Rect marginsAdded(Margins m) const
    {
        Rect result = *this;
        result.adjust(-m.left(), -m.top(), m.right(), m.bottom());
        return result;
    }

    Rect intersected(Rect other) const
    {
        const int maxLeft = std::max(x(), other.x());
        const int minRight = std::min(right(), other.right());
        if (maxLeft > minRight)
            return Rect();

        const int maxTop = std::max(y(), other.y());
        const int minBottom = std::min(bottom(), other.bottom());
        if (maxTop > minBottom)
            return Rect();

        const int w = minRight - maxLeft + 1;
        const int h = minBottom - maxTop + 1;
        return Rect(maxLeft, maxTop, w, h);
    }

    int left() const
    {
        return m_pos.x();
    }

    Point bottomLeft() const
    {
        return { left(), bottom() };
    }

    Point bottomRight() const
    {
        return { right(), bottom() };
    }

    Point topRight() const
    {
        return { right(), top() };
    }

    bool contains(Point pt) const
    {
        return pt.x() >= x() && pt.y() >= y() && pt.x() <= right() && pt.y() <= bottom();
    }

    bool contains(Rect other) const
    {
        return contains(other.topLeft()) && contains(other.bottomRight());
    }

    Rect adjusted(int l, int t, int r, int b) const
    {
        Rect result = *this;
        result.adjust(l, t, r, b);
        return result;
    }

    void adjust(int l, int t, int r, int b);


    Point center() const
    {
        return { left() + ((right() - left()) / 2), top() + ((bottom() - top()) / 2) };
    }

    void moveCenter(Point pt);

    void moveRight(int r)
    {
        const int delta = r - right();
        moveLeft(left() + delta);
    }

    void moveBottom(int b)
    {
        const int delta = b - bottom();
        moveTop(top() + delta);
    }

    bool intersects(Rect other) const
    {
        return !intersected(other).isNull();
    }

    void translate(Point pt);

    bool operator==(Rect other) const
    {
        return m_pos == other.m_pos && m_size == other.m_size;
    }

    bool operator!=(Rect other) const
    {
        return !operator==(other);
    }

private:
    Point m_pos;
    Size m_size = { 0, 0 };
};

inline const Point operator-(Point pt1, Point pt2)
{
    return { pt1.x() - pt2.x(), pt1.y() - pt2.y() };
}

inline const Point operator+(Point pt1, Point pt2)
{
    return { pt1.x() + pt2.x(), pt1.y() + pt2.y() };
}

inline const Size operator+(Size sz1, Size sz2)
{
    return { sz1.width() + sz2.width(), sz1.height() + sz2.height() };
}

inline const Size operator-(Size sz1, Size sz2)
{
    return { sz1.width() - sz2.width(), sz1.height() - sz2.height() };
}

inline void Rect::translate(Point pt)
{
    moveTopLeft(topLeft() + pt);
}

inline void Rect::moveCenter(Point pt)
{
    const Point delta = pt - center();
    moveTopLeft(topLeft() + delta);
}

inline void Rect::adjust(int l, int t, int r, int b)
{
    setLeft(left() + l);
    setTop(top() + t);
    setRight(right() + r);
    setBottom(bottom() + b);
}

}
