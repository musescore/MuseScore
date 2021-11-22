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
#ifndef MU_UI_IACCESSIBLEINTERFACEREGISTER_H
#define MU_UI_IACCESSIBLEINTERFACEREGISTER_H

#include "modularity/imoduleexport.h"

#include <QAccessibleInterface>

namespace mu::accessibility {
class IAccessibleInterfaceRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAccessibleInterfaceRegister)

public:
    virtual ~IAccessibleInterfaceRegister() = default;

    using InterfaceGetter = std::function<QAccessibleInterface* (QObject*)>;

    virtual void registerInterfaceGetter(const std::string& className, const InterfaceGetter& interfaceGetter) = 0;
    virtual InterfaceGetter interfaceGetter(const std::string& className) const = 0;
};
}

#endif // MU_UI_IACCESSIBLEINTERFACEREGISTER_H
