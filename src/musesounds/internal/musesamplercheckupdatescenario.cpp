/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "musesamplercheckupdatescenario.h"

#include "translation.h"

using namespace mu::musesounds;

bool MuseSamplerCheckUpdateScenario::alreadyChecked() const
{
    return m_alreadyChecked;
}

void MuseSamplerCheckUpdateScenario::checkAndShowUpdateIfNeed()
{
    if (!service()->canCheckForUpdate() || multiInstancesProvider()->instances().size() != 1) {
        return;
    }

    m_alreadyChecked = true;

    if (service()->incompatibleLocalVersion()) {
        showCriticalUpdateNotification();
        return;
    }

    auto promise = service()->checkForUpdate();
    promise.onResolve(this, [this](const muse::RetVal<bool>& res) {
        if (!res.ret) {
            LOGE() << res.ret.toString();
            return;
        }

        if (res.val) {
            showNewVersionNotification();
        }
    });
}

void MuseSamplerCheckUpdateScenario::showCriticalUpdateNotification()
{
    muse::IInteractive::ButtonData notNowBtn(int(muse::IInteractive::Button::CustomButton) + 1,
                                             muse::trc("musesounds", "Not now"));

#ifdef Q_OS_LINUX
    muse::IInteractive::ButtonData launchBtn(int(muse::IInteractive::Button::CustomButton) + 2,
                                             muse::trc("musesounds", "Quit & launch MuseSounds Manager"), true /*accent*/);

    std::string msg = muse::trc("musesounds", "To apply this update, MuseScore Studio will need to close briefly and MuseSounds Manager will open. "
                                              "Your MuseSounds libraries won’t work until the update is complete.");
#else
    muse::IInteractive::ButtonData launchBtn(int(muse::IInteractive::Button::CustomButton) + 2,
                                             muse::trc("musesounds", "Quit & launch MuseHub"), true /*accent*/);

    std::string msg = muse::trc("musesounds", "To apply this update, MuseScore Studio will need to close briefly and MuseHub will open. "
                                              "Your MuseSounds libraries won’t work until the update is complete.");
#endif

    interactive()->info(muse::trc("musesounds", "MuseSounds needs an update"), msg,
                        { notNowBtn, launchBtn }, launchBtn.btn, muse::IInteractive::Option::WithIcon)
    .onResolve(this, [this, launchBtn](const muse::IInteractive::Result& res) {
        if (res.isButton(launchBtn.btn)) {
            openMuseHubAndQuit();
        }
    });
}

void MuseSamplerCheckUpdateScenario::showNewVersionNotification()
{
    muse::IInteractive::ButtonData notNowBtn(int(muse::IInteractive::Button::CustomButton) + 1,
                                             muse::trc("musesounds", "Not now"));

#ifdef Q_OS_LINUX
    muse::IInteractive::ButtonData launchBtn(int(muse::IInteractive::Button::CustomButton) + 2,
                                             muse::trc("musesounds", "Quit & launch MuseSounds Manager"), true /*accent*/);

    std::string msg = muse::trc("musesounds", "To keep MuseSounds running smoothly, "
                                              "MuseScore Studio needs to close briefly so MuseSounds Manager can apply the update. "
                                              "You’ll need to restart MuseScore Studio when the update is complete.");
#else
    muse::IInteractive::ButtonData launchBtn(int(muse::IInteractive::Button::CustomButton) + 2,
                                             muse::trc("musesounds", "Quit & launch MuseHub"), true /*accent*/);

    std::string msg = muse::trc("musesounds", "To keep MuseSounds running smoothly, "
                                              "MuseScore Studio needs to close briefly so MuseHub can apply the update. "
                                              "You’ll be prompted to relaunch MuseScore Studio when it’s ready.");
#endif

    interactive()->info(muse::trc("musesounds", "An update for MuseSounds is available"), msg,
                        { notNowBtn, launchBtn }, launchBtn.btn, muse::IInteractive::Option::WithIcon)
    .onResolve(this, [this, launchBtn](const muse::IInteractive::Result& res) {
        if (res.isButton(launchBtn.btn)) {
            openMuseHubAndQuit();
        }
    });
}

void MuseSamplerCheckUpdateScenario::openMuseHubAndQuit()
{
#ifdef Q_OS_LINUX
    if (process()->startDetached("muse-sounds-manager",
                                 { "--show-sampler-update", "--show-hidden-statuses" })) {
        dispatcher()->dispatch("quit");
    } else {
        openMuseHubWebsiteAndQuit();
    }
#else
    const muse::UriQuery MUSEHUB_URI("musehub://requestUpdateCheck?from=musescore-studio");

#ifdef Q_OS_MACOS
    if (!interactive()->canOpenApp(MUSEHUB_URI)) {
        openMuseHubWebsiteAndQuit();
        return;
    }
#endif

    auto promise = interactive()->openApp(MUSEHUB_URI);

    promise.onResolve(this, [this](const muse::Ret& ret) {
        if (ret) {
            dispatcher()->dispatch("quit");
        } else {
            openMuseHubWebsiteAndQuit();
        }
    });

    promise.onReject(this, [this](int, const std::string&) {
        openMuseHubWebsiteAndQuit();
    });
#endif
}

void MuseSamplerCheckUpdateScenario::openMuseHubWebsiteAndQuit()
{
#ifdef Q_OS_LINUX
    interactive()->openUrl(globalConfiguration()->museScoreUrl());
#else
    interactive()->openUrl(globalConfiguration()->museHubWebUrl());
#endif
    dispatcher()->dispatch("quit");
}
