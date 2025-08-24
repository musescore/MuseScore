/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "firstlaunchsetupmodel.h"

#include "translation.h"
#include "global/async/async.h"

using namespace muse;
using namespace mu;
using namespace mu::appshell;

FirstLaunchSetupModel::FirstLaunchSetupModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    m_pages = {
        Page { "ThemesPage.qml", "musescore://notation" },
        Page { "PlaybackPage.qml", "musescore://notation" },
        Page { "TutorialsPage.qml", "musescore://home?section=learn" }
    };
}

void FirstLaunchSetupModel::load()
{
    setCurrentPageIndex(0);
}

int FirstLaunchSetupModel::numberOfPages() const
{
    return m_pages.size();
}

int FirstLaunchSetupModel::currentPageIndex() const
{
    return m_currentPageIndex;
}

QVariantMap FirstLaunchSetupModel::Page::toMap() const
{
    return {
        { "url", url },
    };
}

QVariantMap FirstLaunchSetupModel::currentPage() const
{
    if (m_currentPageIndex < 0 || m_currentPageIndex >= m_pages.size()) {
        return {};
    }

    return m_pages.at(m_currentPageIndex).toMap();
}

bool FirstLaunchSetupModel::canGoBack() const
{
    return m_currentPageIndex > 0;
}

bool FirstLaunchSetupModel::canGoForward() const
{
    return m_currentPageIndex < m_pages.size() - 1;
}

bool FirstLaunchSetupModel::canFinish() const
{
    return m_currentPageIndex == m_pages.size() - 1;
}

void FirstLaunchSetupModel::setCurrentPageIndex(int index)
{
    if (index == m_currentPageIndex || index < 0 || index >= m_pages.size()) {
        return;
    }

    m_currentPageIndex = index;
    emit currentPageChanged();

    async::Async::call(this, [this]() {
        interactive()->open(m_pages.at(m_currentPageIndex).backgroundUri);
    });
}

bool FirstLaunchSetupModel::askAboutClosingEarly()
{
    const std::string title = muse::trc("appshell/gettingstarted", "Are you sure you want to cancel?");
    const std::string body = muse::qtrc("appshell/gettingstarted",
                                        "If you choose to cancel, then be sure to check out our free "
                                        "MuseSounds playback libraries on <a href=\"%1\">MuseHub.com</a>.")
                             .arg(QString::fromStdString(configuration()->museHubFreeMuseSoundsUrl()))
                             .toStdString();
    const IInteractive::Text text(body, IInteractive::TextFormat::RichText);

    static constexpr int visitMuseHubBtnId = int(IInteractive::Button::CustomButton) + 1;
    const IInteractive::ButtonData visitMuseHubBtn {
        visitMuseHubBtnId,
        muse::trc("appshell/gettingstarted", "Visit MuseHub"),
        false,
        false,
#ifdef Q_OS_WINDOWS
        IInteractive::ApplyRole
#else
        IInteractive::AcceptRole
#endif
    };

    const IInteractive::ButtonData keepGoingBtn {
        IInteractive::Button::Continue,
        muse::trc("appshell/gettingstarted", "Keep going"),
        true,
        false,
#ifdef Q_OS_WINDOWS
        IInteractive::AcceptRole
#else
        IInteractive::ContinueRole
#endif
    };

    const IInteractive::ButtonDatas buttons {
        interactive()->buttonData(IInteractive::Button::Cancel), visitMuseHubBtn, keepGoingBtn
    };

    IInteractive::Result result = interactive()->warningSync(title, text, buttons, int(IInteractive::Button::Cancel));

    if (result.isButton(visitMuseHubBtnId)) {
        interactive()->openUrl(configuration()->museHubFreeMuseSoundsUrl());
        return true;
    }

    return result.standardButton() == IInteractive::Button::Cancel;
}

void FirstLaunchSetupModel::finish()
{
    configuration()->setHasCompletedFirstLaunchSetup(true);
}
