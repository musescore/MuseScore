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
#include "firstlaunchsetupmodel.h"

using namespace mu::appshell;

FirstLaunchSetupModel::FirstLaunchSetupModel(QObject* parent)
    : QObject(parent)
{
    m_pages = {
        Page { "ThemesPage.qml", "musescore://home" },
        Page { "PlaybackPage.qml", "musescore://home", /*canSkip*/ true },
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
        { "canSkip", canSkip }
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

bool FirstLaunchSetupModel::canSkip() const
{
    if (m_currentPageIndex < 0 || m_currentPageIndex >= m_pages.size()) {
        return false;
    }

    return m_pages.at(m_currentPageIndex).canSkip;
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

    interactive()->open(m_pages.at(m_currentPageIndex).backgroundUri);
}

bool FirstLaunchSetupModel::askAboutClosingEarly()
{
    IInteractive::ButtonDatas buttons {
        IInteractive::ButtonData(IInteractive::Button::Cancel, trc("global", "Cancel")),
        IInteractive::ButtonData(IInteractive::Button::Continue, trc("appshell", "Keep going"), /*accentButton*/ true)
    };

    IInteractive::Result result
        = interactive()->warning(trc("appshell", "Are you sure you want to cancel?"),
                                 trc("appshell", "If you choose to cancel, then make "
                                                 "sure to check out our free playback "
                                                 "libraries in Home > Audio."),
                                 buttons,
                                 int(IInteractive::Button::Cancel),
                                 IInteractive::Option::WithIcon);

    return result.standardButton() == IInteractive::Button::Cancel;
}

void FirstLaunchSetupModel::finish()
{
    configuration()->setHasCompletedFirstLaunchSetup(true);
}
