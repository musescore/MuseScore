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

#include "log.h"

namespace mu::engraving::rtti {
#define DECLARE_CLASSOF(Type) \
public: \
    static bool classof(const ElementType type) noexcept { return type == Type; } \
    static bool classof(const EngravingObject * item) noexcept { return item->type() == Type; } \

template<typename To, typename From>
bool is_classof(From* p) noexcept
{
    using ResultType = std::remove_pointer_t<To>;
    return p ? ResultType::classof(p) : false;
}

template<typename To, typename From>
auto item_cast(From* p) noexcept
{
    using ResultType = std::remove_cv_t<std::remove_pointer_t<To> >;
    return is_classof<ResultType>(p) ? static_cast<To>(p) : nullptr;
}

/* ===============================================
This visitor to foreaching the list of given types
How to use:
1. Make types list:
using SomeTypeList = TypeList<ItemA, ItemB>;

2. Create subclass like this, and implement the `doVisit` method:
class FooVisitor : public rtti::Visitor<FooVisitor>
{
public:
    template<typename T>
    static bool doVisit(EngravingItem* item, ...)
    {
        if (T::classof(item)) {
            ...
            return true;
        }
        return false;
    }
};

3. Call the `visit` method:

FooVisitor::visit(SomeTypeList {}, item, ...);

*/

template<typename ... Types> struct TypeList {};

template<typename Impl>
class Visitor
{
public:
    enum Policy {
        MayBeNotFound = 0,
        ShouldBeFound
    };

    template<typename ... Types, typename ... Args >
    static bool visit(TypeList<Types...> types, Args&& ... args)
    {
        return visit(Policy::MayBeNotFound, types, std::forward<Args>(args)...);
    }

    template<typename ... Types, typename ... Args >
    static bool visit(Policy policy, TypeList<Types...> types, Args&& ... args)
    {
        bool found = visit_helper(types, std::forward<Args>(args)...);
        if (policy == Policy::ShouldBeFound) {
            DO_ASSERT(found);
        }
        return found;
    }

private:
    template<typename ... Args>
    static bool visit_helper(TypeList<>, Args&&...) { return false; }

    template<class Head, class ... Tail, typename ... Args>
    static bool visit_helper(TypeList<Head, Tail...>, Args&& ... args)
    {
        bool found = Impl::template doVisit<Head>(args ...);
        if (!found) {
            found = visit_helper(TypeList<Tail...> {}, std::forward<Args>(args)...);
        }
        return found;
    }
};
}

#endif // MU_ENGRAVING_RTTI_H
