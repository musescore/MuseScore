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
    constexpr InterfaceInfo(std::string_view i, std::string_view m, bool intr)
        : id(i), module(m), internal(intr) {}
};

class IModuleInterface
{
public:
    virtual ~IModuleInterface() = default;
};

class IModuleExportInterface : public IModuleInterface
{
public:
    virtual ~IModuleExportInterface() = default;

    static constexpr bool isInternalInterface() { return false; }
};

class IModuleInternalInterface : public IModuleInterface
{
public:
    virtual ~IModuleInternalInterface() = default;

    static constexpr bool isInternalInterface() { return true; }
};

class IModuleCreator
{
public:
    virtual ~IModuleCreator() = default;
    virtual std::shared_ptr<IModuleInterface> create() = 0;
};

struct IModuleExportCreator : public IModuleCreator {
    virtual ~IModuleExportCreator() = default;
    static constexpr bool isInternalInterface() { return false; }
};

struct IModuleInternalCreator : public IModuleCreator {
    virtual ~IModuleInternalCreator() = default;
    static constexpr bool isInternalInterface() { return true; }
};
}

#define INTERFACE_ID(id)                                                \
public:                                                                 \
    static constexpr mu::modularity::InterfaceInfo interfaceInfo() {    \
        constexpr std::string_view sig(IOC_FUNC_SIG);                   \
        constexpr mu::modularity::InterfaceInfo info(#id, mu::modularity::moduleNameBySig(sig), isInternalInterface());    \
        return info;                                                    \
    }                                                                   \
private:                                                                \


#define MODULE_EXPORT_INTERFACE public mu::modularity::IModuleExportInterface
#define MODULE_EXPORT_CREATOR public mu::modularity::IModuleExportCreator

#define MODULE_INTERNAL_INTERFACE public mu::modularity::IModuleInternalInterface
#define MODULE_INTERNAL_CREATOR public mu::modularity::IModuleInternalCreator

#endif // MU_MODULARITY_IMODULEEXPORT_H
