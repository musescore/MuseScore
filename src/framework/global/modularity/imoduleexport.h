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

#define INTERFACE_ID(cls)               \
public:                                 \
    static const char* interfaceId() {  \
        static const char* id = #cls;   \
        return id;                      \
    }                                   \

namespace mu::modularity {
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

#define MODULE_EXPORT_INTERFACE public mu::modularity::IModuleExportInterface
#define MODULE_EXPORT_CREATOR public mu::modularity::IModuleExportCreator

#endif // MU_MODULARITY_IMODULEEXPORT_H
