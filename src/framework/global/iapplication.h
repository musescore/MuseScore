/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#ifndef MUSE_GLOBAL_IAPPLICATION_H
#define MUSE_GLOBAL_IAPPLICATION_H

#include "modularity/imoduleinterface.h"

#include "types/string.h"
#include "types/version.h"
#include "modularity/ioc.h"

#ifndef NO_QT_SUPPORT
class QObject;
class QEvent;
class QWindow;
#endif

namespace muse {
class IApplication : MODULE_GLOBAL_INTERFACE
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
    virtual String title() const = 0;

    virtual bool unstable() const = 0;
    virtual Version version() const = 0;
    virtual Version fullVersion() const = 0; // with suffix
    virtual String build() const = 0;
    virtual String revision() const = 0;

    virtual RunMode runMode() const = 0;
    virtual bool noGui() const = 0;

    virtual void setup() = 0;
    virtual void finish() = 0;
    virtual void restart() = 0;

    virtual modularity::ContextPtr setupNewContext(const StringList& args = {}) = 0;
    virtual void destroyContext(const modularity::ContextPtr& ctx) = 0;
    virtual int contextCount() const = 0;
    virtual std::vector<modularity::ContextPtr> contexts() const = 0;

    // to remove
    virtual const modularity::ContextPtr iocContext() const = 0;
    virtual modularity::ModulesIoC* ioc() const = 0;
    // ---------

    virtual void processEvents() = 0;

#ifndef NO_QT_SUPPORT
    virtual QWindow* focusWindow() const = 0;
    virtual bool notify(QObject* object, QEvent* event) = 0;

    virtual Qt::KeyboardModifiers keyboardModifiers() const = 0;
#endif
};
}

#endif // MUSE_GLOBAL_IAPPLICATION_H
