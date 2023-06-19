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

#ifndef MU_MODULARITY_MODULEINFO_H
#define MU_MODULARITY_MODULEINFO_H

#include <string_view>

#ifndef IOC_FUNC_SIG
#if defined(_MSC_VER)
    #define IOC_FUNC_SIG __FUNCSIG__
#else
    #define IOC_FUNC_SIG __PRETTY_FUNCTION__
#endif
#endif

namespace mu::modularity {
constexpr std::string_view moduleNameBySig(const std::string_view& sig)
{
    constexpr std::string_view ArgBegin("(");
    constexpr std::string_view Space(" ");
    constexpr std::string_view Colon("::");

    //! NOTE Signature should be like
    //! SomeType mu::modulename::maybe::ClassName::methodName()

    std::size_t endMethod = sig.find_first_of(ArgBegin);
    if (endMethod == std::string_view::npos) {
        return sig;
    }

    std::size_t beginMethod = sig.find_last_of(Space, endMethod);
    if (beginMethod == std::string_view::npos) {
        return sig;
    }

    size_t beginModule = sig.find_first_of(Colon, beginMethod) + 2;
    size_t endModule = sig.find_first_of(Colon, beginModule);
    std::string_view module = sig.substr(beginModule, endModule - beginModule);
    return module;
}
}

#endif // MU_MODULARITY_MODULEINFO_H
