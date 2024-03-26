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
#ifndef MU_GLOBAL_IAPPLICATION_H
#define MU_GLOBAL_IAPPLICATION_H

#include <string>

#include "modularity/imoduleinterface.h"

#include "types/version.h"

#ifndef NO_QT_SUPPORT
class QObject;
class QEvent;
class QWindow;
#endif

namespace mu {
class IApplication : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IApplication)
public:
    virtual ~IApplication() = default;

    enum class RunMode {
        GuiApp,
        ConsoleApp,
        AudioPluginRegistration,
    };

    virtual String name() const = 0;

    virtual bool unstable() const = 0;
    virtual Version version() const = 0;
    virtual Version fullVersion() const = 0; // with suffix
    virtual String build() const = 0;
    virtual String revision() const = 0;

    virtual void setRunMode(const RunMode& mode) = 0;
    virtual RunMode runMode() const = 0;
    virtual bool noGui() const = 0;

    virtual void restart() = 0;

#ifndef NO_QT_SUPPORT
    virtual QWindow* focusWindow() const = 0;
    virtual bool notify(QObject* object, QEvent* event) = 0;
#endif
};
}

#endif // MU_GLOBAL_IAPPLICATION_H
