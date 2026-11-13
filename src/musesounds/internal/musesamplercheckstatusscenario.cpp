/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
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

#include "musesamplercheckstatusscenario.h"

#include "rcommand/commandtypes.h"
#include "translation.h"
#include "ui/view/iconcodes.h"

using namespace mu::musesounds;

bool MuseSamplerCheckStatusScenario::alreadyChecked() const
{
    return m_alreadyChecked;
}

void MuseSamplerCheckStatusScenario::checkAndShowStatusIfNeed()
{
    if (!multiwindowsProvider()->isFirstWindow()) {
        return;
    }

    if (service()->libInitFailed()) {
        m_alreadyChecked = true;
        if (configuration()->showLoadingFailedWarning()) {
            showLoadingFailedNotification();
        }
        return;
    }

    if (!service()->canCheckForUpdate()) {
        return;
    }

    LOGI() << "Checking for MuseSampler update...";
    m_alreadyChecked = true;

    if (service()->incompatibleLocalVersion()) {
        showCriticalUpdateNotification();
        return;
    }

    auto promise = service()->checkForUpdate();
    promise.onResolve(this, [this](const muse::RetVal<bool>& res) {
        if (!res.ret) {
            LOGE() << "Unable to check for MuseSampler update: " << res.ret.toString();
            return;
        }

        if (res.val) {
            showNewVersionNotification();
        }
    });
}

void MuseSamplerCheckStatusScenario::showLoadingFailedNotification()
{
    muse::IInteractive::ButtonData closeBtn = interactive()->buttonData(muse::IInteractive::Button::Close);
    muse::IInteractive::ButtonData troubleshootBtn(int(muse::IInteractive::Button::CustomButton) + 1,
                                                   muse::trc("musesounds", "Troubleshoot"));
    troubleshootBtn.icon = int(muse::ui::IconCode::Code::OPEN_LINK);

#ifdef Q_OS_LINUX
    muse::IInteractive::ButtonData openBtn(int(muse::IInteractive::Button::CustomButton) + 2,
                                           muse::trc("musesounds", "Open MuseSounds Manager"), true /*accent*/);

    std::string msg = muse::trc("musesounds", "To load MuseSounds, open MuseSounds Manager and check that you’re running the latest version. "
                                              "Update if needed and restart your machine.\n\n"
                                              "If the issue persists, follow the steps at the troubleshooting link below.");
#else
    muse::IInteractive::ButtonData openBtn(int(muse::IInteractive::Button::CustomButton) + 2,
                                           muse::trc("musesounds", "Open MuseHub"), true /*accent*/);

    std::string msg = muse::trc("musesounds", "To load MuseSounds, open MuseHub and check that you’re running the latest version. "
                                              "Update if needed and restart your machine.\n\n"
                                              "If the issue persists, follow the steps at the troubleshooting link below.");
#endif

#ifdef Q_OS_WIN
    const muse::IInteractive::ButtonDatas buttons { openBtn, troubleshootBtn, closeBtn };
#else
    const muse::IInteractive::ButtonDatas buttons { closeBtn, troubleshootBtn, openBtn };
#endif

    interactive()->warning(muse::trc("musesounds", "MuseSounds could not be loaded"), msg,
                           buttons, openBtn.btn,
                           muse::IInteractive::Option::WithIcon | muse::IInteractive::Option::WithDontShowAgainCheckBox)
    .onResolve(this, [this, troubleshootBtn, openBtn](const muse::IInteractive::Result& res) {
        if (!res.showAgain()) {
            configuration()->setShowLoadingFailedWarning(false);
        }

        if (res.isButton(troubleshootBtn.btn)) {
            platformInteractive()->openUrl(configuration()->museSoundsTroubleshootUrl());
        } else if (res.isButton(openBtn.btn)) {
            openMuseHub(false /*quit*/);
        }
    });
}

void MuseSamplerCheckStatusScenario::showCriticalUpdateNotification()
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
            openMuseHub(true /*quit*/);
        }
    });
}

void MuseSamplerCheckStatusScenario::showNewVersionNotification()
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
            openMuseHub(true /*quit*/);
        }
    });
}

void MuseSamplerCheckStatusScenario::openMuseHub(bool quit)
{
#ifdef Q_OS_LINUX
    if (process()->startDetached("muse-sounds-manager",
                                 { "--show-sampler-update", "--show-hidden-statuses" })) {
        if (quit) {
            commandDispatcher()->dispatch(muse::rcommand::Command("command://app/quit"));
        }
    } else {
        openMuseHubWebsite(quit);
    }
#else
    const muse::UriQuery MUSEHUB_URI("musehub://requestUpdateCheck?from=musescore-studio");

#ifdef Q_OS_MACOS
    if (!platformInteractive()->canOpenApp(MUSEHUB_URI)) {
        openMuseHubWebsite(quit);
        return;
    }
#endif

    auto promise = platformInteractive()->openApp(MUSEHUB_URI);

    promise.onResolve(this, [this, quit](const muse::Ret& ret) {
        if (!ret) {
            openMuseHubWebsite(quit);
        } else if (quit) {
            commandDispatcher()->dispatch(muse::rcommand::Command("command://app/quit"));
        }
    });

    promise.onReject(this, [this, quit](int, const std::string&) {
        openMuseHubWebsite(quit);
    });
#endif
}

void MuseSamplerCheckStatusScenario::openMuseHubWebsite(bool quit)
{
#ifdef Q_OS_LINUX
    platformInteractive()->openUrl(updateConfiguration()->appWebSiteUrl());
#else
    platformInteractive()->openUrl(globalConfiguration()->museHubWebUrl());
#endif

    if (quit) {
        commandDispatcher()->dispatch(muse::rcommand::Command("command://app/quit"));
    }
}
