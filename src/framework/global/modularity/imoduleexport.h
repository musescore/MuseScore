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

#ifndef MU_MODULARITY_IMODULEEXPORT_H
#define MU_MODULARITY_IMODULEEXPORT_H

#include <memory>
#include "moduleinfo.h"

namespace mu::modularity {
struct InterfaceInfo {
    std::string_view id;
    std::string_view module;
    bool internal = false;
    InterfaceInfo(std::string_view i, std::string_view m, bool intr)
        : id(i), module(m), internal(intr) {}
};

class IModuleExportInterface
{
public:
    virtual ~IModuleExportInterface() {}
};

struct IModuleExportCreator {
    virtual ~IModuleExportCreator() {}
    virtual std::shared_ptr<IModuleExportInterface> create() = 0;
};
}

#define DO_INTERFACE_ID(id, internal)                                           \
public:                                                                         \
    static const mu::modularity::InterfaceInfo& interfaceInfo() {               \
        static constexpr std::string_view sig(IOC_FUNC_SIG);                    \
        static const mu::modularity::InterfaceInfo info(#id, mu::modularity::moduleNameBySig(sig), internal);    \
        return info;                                                            \
    }                                                                           \
private:                                                                        \

#define INTERFACE_ID(id) DO_INTERFACE_ID(id, false)
#define INTERNAL_INTERFACE_ID(id) DO_INTERFACE_ID(id, true)

#define MODULE_EXPORT_INTERFACE public mu::modularity::IModuleExportInterface
#define MODULE_EXPORT_CREATOR public mu::modularity::IModuleExportCreator

#endif // MU_MODULARITY_IMODULEEXPORT_H
