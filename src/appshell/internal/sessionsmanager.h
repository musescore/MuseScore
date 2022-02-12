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
#ifndef MU_APPSHELL_SESSIONSMANAGER_H
#define MU_APPSHELL_SESSIONSMANAGER_H

#include <optional>

#include "istartupscenario.h"

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "iappshellconfiguration.h"

#include "isessionsmanager.h"

namespace mu::project {
struct SaveLocation;
}

namespace mu::appshell {
class SessionsManager : public ISessionsManager, public async::Asyncable
{
    INJECT(appshell, actions::IActionsDispatcher, dispatcher)
    INJECT(appshell, mi::IMultiInstancesProvider, multiInstancesProvider)
    INJECT(appshell, context::IGlobalContext, globalContext)
    INJECT(appshell, IAppShellConfiguration, configuration)

public:
    void init();
    void deinit();

    bool hasProjectsForRestore() override;

    void restore() override;
    void reset() override;

private:
    void update();

    void removeProjectFromSession(const project::SaveLocation& projectPath);
    void addProjectToSession(const project::SaveLocation& projectPath);

    std::optional<project::SaveLocation> m_lastOpenedProjectLocation = std::nullopt;
};
}

#endif // MU_APPSHELL_SESSIONSMANAGER_H
