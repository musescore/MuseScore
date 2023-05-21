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

#ifndef MU_MODULARITY_IOC_H
#define MU_MODULARITY_IOC_H

#include <memory>
#include "modulesioc.h"

#define INJECT(Interface, getter) \
private: \
    mutable std::shared_ptr<Interface> mutable_##getter = nullptr; \
public: \
    std::shared_ptr<Interface> getter() const {  \
        if (!mutable_##getter) { \
            static const std::string_view sig(IOC_FUNC_SIG); \
            mutable_##getter = mu::modularity::ioc()->resolve<Interface>(mu::modularity::moduleNameBySig(sig), sig); \
        } \
        return mutable_##getter; \
    } \
    void set##getter(std::shared_ptr<Interface> impl) { mutable_##getter = impl; } \

#define INJECT_STATIC(Interface, getter) \
public: \
    static std::shared_ptr<Interface>& getter() {  \
        static std::shared_ptr<Interface> static_##getter = nullptr; \
        if (!static_##getter) { \
            static const std::string_view sig(IOC_FUNC_SIG); \
            static_##getter = mu::modularity::ioc()->resolve<Interface>(mu::modularity::moduleNameBySig(sig), sig); \
        } \
        return static_##getter; \
    } \
    static void set##getter(std::shared_ptr<Interface> impl) { \
        std::shared_ptr<Interface>& static_##getter = getter(); \
        static_##getter = impl; \
    } \

namespace mu::modularity {
inline ModulesIoC* ioc()
{
    return ModulesIoC::instance();
}
}

#endif // MU_MODULARITY_IOC_H
