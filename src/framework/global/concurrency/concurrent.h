/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_GLOBAL_CONCURRENT_H
#define MU_GLOBAL_CONCURRENT_H

#include <QtConcurrent>

namespace mu {
class Concurrent
{
public:

    template<typename Functor>
    static void run(Functor functor)
    {
        QtConcurrent::run(functor);
    }

    // not const object, not const fn
    template<typename Class>
    static void run(Class* object, void (Class::* fn)())
    {
        QtConcurrent::run(object, fn);
    }

    template<typename Class, typename Param1, typename Arg1>
    static void run(Class* object, void (Class::* fn)(Param1), const Arg1& arg1)
    {
        QtConcurrent::run(object, fn, arg1);
    }

    template<typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2>
    static void run(Class* object, void (Class::* fn)(Param1, Param2), const Arg1& arg1, const Arg2& arg2)
    {
        QtConcurrent::run(object, fn, arg1, arg2);
    }

    // not const object, const fn
    template<typename Class>
    static void run(Class* object, void (Class::* fn)() const)
    {
        QtConcurrent::run(object, fn);
    }

    template<typename Class, typename Param1, typename Arg1>
    static void run(Class* object, void (Class::* fn)(Param1) const, const Arg1& arg1)
    {
        QtConcurrent::run(object, fn, arg1);
    }

    template<typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2>
    static void run(Class* object, void (Class::* fn)(Param1, Param2) const, const Arg1& arg1, const Arg2& arg2)
    {
        QtConcurrent::run(object, fn, arg1, arg2);
    }
};
}

#endif // MU_GLOBAL_CONCURRENT_H
