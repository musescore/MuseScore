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
#ifndef MUSE_GLOBAL_BASEAPPLICATION_H
#define MUSE_GLOBAL_BASEAPPLICATION_H

#include "../iapplication.h"

namespace muse {
class BaseApplication : public IApplication
{
public:

    BaseApplication(const modularity::ContextPtr& ctx);

    static String appName();
    static String appTitle();
    static bool appUnstable();
    static Version appVersion();
    static Version appFullVersion();
    static String appBuild();
    static String appRevision();

    String name() const override { return appName(); }
    String title() const override { return appTitle(); }
    bool unstable() const override { return appUnstable(); }
    Version version() const override { return appVersion(); }
    Version fullVersion() const override { return appFullVersion(); }
    String build() const override { return appBuild(); }
    String revision() const override { return appRevision(); }

    void setRunMode(const RunMode& mode);
    RunMode runMode() const override;
    bool noGui() const override;

    void restart() override;

    const modularity::ContextPtr iocContext() const override;
    modularity::ModulesIoC* ioc() const override;

#ifndef NO_QT_SUPPORT
    QWindow* focusWindow() const override;
    bool notify(QObject* object, QEvent* event) override;

    Qt::KeyboardModifiers keyboardModifiers() const override;
#endif

protected:

    void removeIoC();

private:
    RunMode m_runMode = RunMode::GuiApp;
    modularity::ContextPtr m_iocContext;
};
}

#endif // MUSE_GLOBAL_BASEAPPLICATION_H
