/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "closeprojectscenario.h"

#include <QEventLoop>

#include "defer.h"
#include "translation.h"

#include "log.h"

using namespace mu::project;
using namespace muse;
using muse::async::Promise;

static const muse::Uri HOME_PAGE_URI("musescore://home");

bool CloseProjectScenario::closeOpenedProject(bool goToHome)
{
    if (isBusy(BusyStatus::Closing)) {
        return false;
    }

    setBusy(BusyStatus::Closing, true);
    DEFER {
        setBusy(BusyStatus::Closing, false);
    };

    INotationProjectPtr project = currentNotationProject();
    if (!project) {
        return true;
    }

    if (globalContext()->playbackState()->isPlaying()) {
        commandDispatcher()->dispatch(rcommand::Command("command://playback/stop"));
    }

    bool result = true;

    if (project->isNeedSave()) {
        IInteractive::Button btn = askAboutSavingScore(project);

        if (btn == IInteractive::Button::Cancel) {
            result = false;
        } else if (btn == IInteractive::Button::Save) {
            result = waitFor(saveProjectScenario()->saveProject());
        } else if (btn == IInteractive::Button::DontSave) {
            result = true;
        }
    }

    if (result) {
        interactive()->closeAllDialogsSync();
        globalContext()->setCurrentProject(nullptr);

        if (goToHome) {
            openHomePageIfNeed();
        }
    }

    return result;
}

bool CloseProjectScenario::isBusy(BusyStatus status) const
{
    return m_busyStatuses.contains(status);
}

muse::async::Notification CloseProjectScenario::busyChanged() const
{
    return m_busyChanged;
}

INotationProjectPtr CloseProjectScenario::currentNotationProject() const
{
    return globalContext()->currentProject();
}

void CloseProjectScenario::setBusy(BusyStatus status, bool isBusy)
{
    bool wasBusy = m_busyStatuses.contains(status);
    if (wasBusy == isBusy) {
        return;
    }

    if (isBusy) {
        m_busyStatuses.insert(status);
    } else {
        m_busyStatuses.erase(status);
    }

    m_busyChanged.notify();
}

IInteractive::Button CloseProjectScenario::askAboutSavingScore(const INotationProjectPtr& project)
{
    std::string title = muse::qtrc("project", "Do you want to save changes to the score “%1” before closing?")
                        .arg(project->displayName()).toStdString();

    std::string body = muse::trc("project", "Your changes will be lost if you don’t save them.");

    IInteractive::Result result = interactive()->warningSync(title, body, {
        IInteractive::Button::DontSave,
        IInteractive::Button::Cancel,
        IInteractive::Button::Save
    }, IInteractive::Button::Save);

    return result.standardButton();
}

Ret CloseProjectScenario::waitFor(Promise<Ret> flow)
{
    QEventLoop loop;
    Ret result;
    bool finished = false;

    flow.onResolve(this, [&result, &finished, &loop](const Ret& ret) {
        result = ret;
        finished = true;
        loop.quit();
    });

    if (!finished) {
        loop.exec();
    }

    return result;
}

void CloseProjectScenario::openHomePageIfNeed()
{
    if (interactive()->isOpened(HOME_PAGE_URI).val) {
        return;
    }

    interactive()->open(HOME_PAGE_URI);
}
