/****************************************************************************
**                                MIT License
**
** Copyright (C) 2021 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>
**
** This file is part of KDToolBox (https://github.com/KDAB/KDToolBox).
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, ** and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice (including the next paragraph)
** shall be included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF ** CONTRACT, TORT OR OTHERWISE,
** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
****************************************************************************/

// SPDX-License-Identifier: MIT

#ifndef KDTOOLBOX_STLCONTAINERADAPTOR_H
#define KDTOOLBOX_STLCONTAINERADAPTOR_H
#pragma once

#include <vector>
#include <algorithm>
#include <iterator>

namespace KDToolBox {
namespace StlContainerAdaptor {

template<typename T, typename... Args>
struct StdVectorAdaptor : std::vector<T, Args...>
{
    using base_container = std::vector<T, Args...>;
    using base_size_type = typename base_container::size_type;
    using size_type = int;
    using value_type = typename base_container::value_type;
    // using ConstIterator = typename base_container::const_iterator;

    // Construction / RO5
    using base_container::base_container;

    StdVectorAdaptor() = default; // work around broken compilers in C++17 mode
    explicit StdVectorAdaptor(size_type count)
        : base_container(base_size_type(count))
    {
    }
    StdVectorAdaptor(size_type count, const value_type &v)
        : base_container(base_size_type(count), v)
    {
    }

    // It's a lot of churn to fix all cases
    //[[deprecated("Use clone() instead; copies of non-Qt containers are not cheap")]]
    StdVectorAdaptor(const StdVectorAdaptor &) = default;
    //[[deprecated("Use assignFrom() instead; copies of non-Qt containers are not cheap")]]
    StdVectorAdaptor &operator=(const StdVectorAdaptor &) = default;

    StdVectorAdaptor(StdVectorAdaptor &&) = default;
    StdVectorAdaptor &operator=(StdVectorAdaptor &&) = default;

    ~StdVectorAdaptor() = default;

    StdVectorAdaptor clone() const
    {
        return *this;
    }

    StdVectorAdaptor &assignFrom(const StdVectorAdaptor &other)
    {
        return *this = other;
    }
    StdVectorAdaptor &assignFrom(StdVectorAdaptor &&other)
    {
        return *this = std::move(other);
    }

    // Iterators
    decltype(auto) constBegin() const
    {
        return this->cbegin();
    }
    decltype(auto) constEnd() const
    {
        return this->cend();
    }

    // Data access
    decltype(auto) constData() const
    {
        return this->data();
    }
    decltype(auto) at(size_type i) const
    {
        return base_container::operator[](base_size_type(i));
    }
    decltype(auto) operator[](size_type i)
    {
        return base_container::operator[](base_size_type(i));
    }
    decltype(auto) operator[](size_type i) const
    {
        return base_container::operator[](base_size_type(i));
    }

    auto value(size_type i, const T &defaultValue) const
    {
        if (i >= 0 && i < size())
            return at(i);
        return defaultValue;
    }
    auto value(size_type i) const
    {
        if (i >= 0 && i < size())
            return at(i);
        return value_type();
    }

    decltype(auto) first()
    {
        return this->front();
    }
    decltype(auto) first() const
    {
        return this->front();
    }
    decltype(auto) constFirst() const
    {
        return this->front();
    }

    decltype(auto) last()
    {
        return this->back();
    }
    decltype(auto) last() const
    {
        return this->back();
    }
    decltype(auto) constLast() const
    {
        return this->back();
    }

    // Size and capacity
    decltype(auto) isEmpty() const
    {
        return this->empty();
    }
    decltype(auto) size() const
    {
        return size_type(base_container::size());
    }
    decltype(auto) count() const
    {
        return size();
    }
    decltype(auto) length() const
    {
        return size();
    }
    decltype(auto) capacity() const
    {
        return size_type(base_container::capacity());
    }
    void reserve(size_type s)
    {
        base_container::reserve(base_size_type(s));
    }
    void squeeze()
    {
        this->shrink_to_fit();
    }


    // Insertion
    void append(const value_type &v)
    {
        this->push_back(v);
    }
    void append(value_type &&v)
    {
        this->push_back(std::move(v));
    }
    void append(const StdVectorAdaptor &other)
    {
        if (this != &other) {
            this->insert(this->end(), other.begin(), other.end());
        } else {
            this->reserve(2 * size());
            std::copy(this->begin(), this->end(), std::back_inserter(*this));
        }
    }
    void prepend(const value_type &v)
    {
        this->insert(this->begin(), v);
    }
    void prepend(value_type &&v)
    {
        this->insert(this->begin(), std::move(v));
    }

    using base_container::insert;
    decltype(auto) insert(size_type position, const value_type &v)
    {
        return this->insert(this->begin() + position, v);
    }
    decltype(auto) insert(size_type position, value_type &&v)
    {
        return this->insert(this->begin() + position, std::move(v));
    }


    // Removal
    void removeFirst()
    {
        this->erase(this->begin());
    }
    void removeLast()
    {
        this->pop_back();
    }
    void remove(size_type position)
    {
        this->erase(this->begin() + position);
    }
    void remove(size_type position, size_type count)
    {
        const auto b = this->begin();
        this->erase(b + position, b + position + count);
    }
    void removeAt(size_type position)
    {
        this->erase(this->begin() + position);
    }

    template<typename AT>
    decltype(auto) removeAll(const AT &v)
    {
        const auto b = this->begin();
        const auto e = this->end();
        const auto i = std::remove(b, e, v);
        const auto result = size_type(e - i);
        this->erase(i, e);
        return result;
    }

    template<typename AT>
    bool removeOne(const AT &v)
    {
        const auto b = this->begin();
        const auto e = this->end();
        const auto i = std::find(b, e, v);
        if (i == e)
            return false;
        this->erase(i);
        return true;
    }

    decltype(auto) takeAt(size_type i)
    {
        const auto it = this->begin() + i;
        const auto result = std::move(*it);
        this->erase(it);
        return result;
    }
    decltype(auto) takeLast()
    {
        return takeAt(size() - 1);
    }
    decltype(auto) takeFirst()
    {
        return takeAt(0);
    }


    // Search
    template<typename AT>
    bool contains(const AT &v) const
    {
        return indexOf(v) >= 0;
    }

    template<typename AT>
    size_type indexOf(const AT &v, size_type from = 0) const
    {
        const auto s = size();
        if (from < 0)
            from = std::max(from + s, 0);
        if (from < s) {
            const auto b = this->begin();
            const auto e = this->end();
            const auto i = std::find(b + from, e, v);
            if (i != e)
                return size_type(i - b);
        }
        return size_type(-1);
    }

    template<typename AT>
    size_type lastIndexOf(const AT &v, size_type from = -1) const
    {
        const auto s = size();
        if (from < 0)
            from += s;
        else if (from >= s)
            from = s - 1;

        if (from >= 0) {
            const auto b = this->begin();

            const auto revB = std::make_reverse_iterator(b + from + 1);
            const auto revE = std::make_reverse_iterator(b);
            const auto i = std::find(revB, revE, v);
            if (i != revE)
                return size_type(i.base() - b) - 1;
        }
        return size_type(-1);
    }

    template<typename AT>
    bool startsWith(const AT &v) const
    {
        return this->front() == v;
    }

    template<typename AT>
    bool endsWith(const AT &v) const
    {
        return this->back() == v;
    }


    // Miscellanea
    StdVectorAdaptor &fill(const value_type &v, size_type i = -1)
    {
        if (i < 0)
            i = size();
        this->assign(base_size_type(i), v);
        return *this;
    }

    StdVectorAdaptor mid(size_type pos, size_type len = -1)
    {
        const auto s = size();

        if (len < 0)
            len = s;
        len = std::min(len, s - pos);

        const auto b = this->begin() + pos;
        const auto e = b + len;
        return StdVectorAdaptor(b, e);
    }

    void move(size_type from, size_type to)
    {
        const auto b = this->begin();
        if (from < to)
            std::rotate(b + from, b + from + 1, b + to + 1);
        else
            std::rotate(b + to, b + from, b + from + 1);
    }

    void replace(size_type pos, const value_type &v)
    {
        at(pos) = v;
    }

    void swapItemsAt(size_type i, size_type j)
    {
        qSwap(at(i), at(j));
    }


    // Misc operators
    template<typename AT>
    decltype(auto) operator<<(const typename StdVectorAdaptor<AT>::value_type &v)
    {
        push_back(v);
        return *this;
    }
    template<typename AT>
    decltype(auto) operator<<(typename StdVectorAdaptor<AT>::value_type &&v)
    {
        push_back(std::move(v));
        return *this;
    }

    template<typename AT>
    decltype(auto) operator<<(const StdVectorAdaptor<AT> &rhs)
    {
        append(rhs);
        return *this;
    }

    template<typename AT>
    decltype(auto) operator+(const StdVectorAdaptor<AT> &rhs)
    {
        StdVectorAdaptor<AT> result;
        result.reserve(size() + rhs.size());
        result.insert(result.end(), base_container::begin(), base_container::end());
        result.insert(result.end(), rhs.begin(), rhs.end());
        return result;
    }

    decltype(auto) operator+=(const StdVectorAdaptor &other)
    {
        append(other);
        return *this;
    }
};

} // namespace StlContainerAdaptor
} // namespace KDToolBox

namespace KDDockWidgets {

template<typename T, typename... Args>
using Vector = KDToolBox::StlContainerAdaptor::StdVectorAdaptor<T, Args...>;

}

#endif // KDTOOLBOX_STLCONTAINERADAPTOR_H
