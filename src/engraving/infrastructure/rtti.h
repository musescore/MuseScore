/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_RTTI_H
#define MU_ENGRAVING_RTTI_H

#include <type_traits>

namespace mu::engraving::rtti {
#define DECLARE_CLASSOF(Type) \
public: \
    static bool classof(const ElementType type) noexcept { return type == Type; } \
    static bool classof(const EngravingObject * item) noexcept { return item->type() == Type; } \

template<typename To, typename From>
bool is_classof(From* p) noexcept
{
    using ResultType = std::remove_pointer_t<To>;
    return ResultType::classof(p);
}

template<typename To, typename From>
auto item_cast(From* p) noexcept
{
    using ResultType = std::remove_cv_t<std::remove_pointer_t<To> >;
    return is_classof<ResultType>(p) ? static_cast<To>(p) : nullptr;
}

/* ===============================================
This visitor to foreaching the list of given types
How use:
1. Make types list

using SomeTypeList = TypeList<ItemA, ItemB>;

2. Write visit function with name 'somename_visit'
   (there may be some number of arguments)

template<typename T>
bool foo_visit(EngravingItem* item, ...)
{
    if (T::classof(item)) {
        ...
        return true;
    }
    return false;
}

3. Declare visiter with some name

DECLARE_VISITER(foo)

4. Call visiter

foo_visiter(SomeTypeList {}, item, ...);

*/

template<typename ... Types> struct TypeList {};

#define DECLARE_VISITER(name) \
    template<typename ... Arg> bool name##_visiter(rtti::TypeList<>, Arg & ...) { return false; } \
    template<class Head, class ... Tail, typename ... Arg> bool name##_visiter(rtti::TypeList<Head, Tail...>, Arg & ...); \
    template<class Head, class ... Tail, typename ... Arg> bool name##_visiter(rtti::TypeList<Head, Tail...>, Arg & ... a) \
    { \
        bool found = name##_visit<Head>(a ...); \
        if (!found) { \
            found = name##_visiter(rtti::TypeList<Tail...> {}, a ...); \
        } \
        return found; \
    } \

}

#endif // MU_ENGRAVING_RTTI_H
