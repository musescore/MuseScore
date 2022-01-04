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
#ifndef MU_APPSHELL_STARTUPSCENARIO_H
#define MU_APPSHELL_STARTUPSCENARIO_H

#include "istartupscenario.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "iappshellconfiguration.h"
#include "actions/iactionsdispatcher.h"
#include "async/asyncable.h"

namespace mu::appshell {
class StartupScenario : public IStartupScenario, public async::Asyncable
{
    INJECT(appshell, IAppShellConfiguration, configuration)
    INJECT(appshell, framework::IInteractive, interactive)
    INJECT(appshell, actions::IActionsDispatcher, dispatcher)

public:

    void setSessionType(const QString& sessionType) override;
    void setStartupScorePath(const io::path& path) override;

    void run() override;
    bool startupCompleted() const override;

private:
    void onStartupPageOpened(StartupSessionType sessionType);

    StartupSessionType resolveStartupSessionType() const;
    Uri startupPageUri(StartupSessionType sessionType) const;

    void openScore(const io::path& path);

    QString m_sessionType;
    io::path m_startupScorePath;
    bool m_startupCompleted = false;
};
}

#endif // MU_APPSHELL_STARTUPSCENARIO_H
