//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "programmestartpreferencesmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::appshell;

ProgrammeStartPreferencesModel::ProgrammeStartPreferencesModel(QObject* parent)
    : QObject(parent)
{
}

QVariantList ProgrammeStartPreferencesModel::startupModes() const
{
    QVariantList result;

    for (const StartMode& mode: allStartupModes()) {
        QVariantMap obj;
        obj["title"] = mode.title;
        obj["checked"] = mode.checked;
        obj["canSelectScorePath"] = mode.canSelectScorePath;
        obj["scorePath"] = mode.scorePath;

        result << obj;
    }

    return result;
}

ProgrammeStartPreferencesModel::StartModeList ProgrammeStartPreferencesModel::allStartupModes() const
{
    static const QMap<StartupSessionType, QString> sessionTitles {
        { StartupSessionType::StartEmpty,  qtrc("appshell", "Start empty") },
        { StartupSessionType::ContinueLastSession, qtrc("appshell", "Continue last session") },
        { StartupSessionType::StartWithNewScore, qtrc("appshell", "Start with new score") },
        { StartupSessionType::StartWithScore, qtrc("appshell", "Start with score:") }
    };

    StartModeList modes;

    for (StartupSessionType type : sessionTitles.keys()) {
        bool canSelectScorePath = (type == StartupSessionType::StartWithScore);

        StartMode mode;
        mode.sessionType = type;
        mode.title = sessionTitles[type];
        mode.checked = configuration()->startupSessionType() == type;
        mode.scorePath = canSelectScorePath ? configuration()->startupScorePath().toQString() : QString();
        mode.canSelectScorePath = canSelectScorePath;

        modes << mode;
    }

    return modes;
}

QVariantList ProgrammeStartPreferencesModel::panels() const
{
    QVariantList result;

    for (const Panel& panel: allPanels()) {
        QVariantMap obj;
        obj["title"] = panel.title;
        obj["visible"] = panel.visible;

        result << obj;
    }

    return result;
}

ProgrammeStartPreferencesModel::PanelList ProgrammeStartPreferencesModel::allPanels() const
{
    PanelList panels {
        Panel { SplashScreen, qtrc("appshell", "Show splash screen"), configuration()->needShowSplashScreen() },
        Panel { Navigator, qtrc("appshell", "Show navigator"), configuration()->isNotationNavigatorVisible().val },
        Panel { Tours, qtrc("appshell", "Show tours"), configuration()->needShowTours() }
    };

    return panels;
}

QString ProgrammeStartPreferencesModel::scorePathFilter() const
{
    return qtrc("appshell", "MuseScore Files") + " (*.mscz *.mscx);;"
           + qtrc("appshell", "All") + " (*)";
}

void ProgrammeStartPreferencesModel::setCurrentStartupMode(int modeIndex)
{
    StartModeList modes = allStartupModes();

    if (modeIndex < 0 || modeIndex >= modes.size()) {
        return;
    }

    StartupSessionType selectedType = modes[modeIndex].sessionType;
    if (selectedType == configuration()->startupSessionType()) {
        return;
    }

    configuration()->setStartupSessionType(selectedType);
    emit startupModesChanged();
}

void ProgrammeStartPreferencesModel::setStartupScorePath(const QString& scorePath)
{
    if (scorePath.isEmpty() || scorePath == configuration()->startupScorePath().toQString()) {
        return;
    }

    configuration()->setStartupScorePath(scorePath);

    emit startupModesChanged();
}

void ProgrammeStartPreferencesModel::setPanelVisible(int panelIndex, bool visible)
{
    PanelList panels = allPanels();

    if (panelIndex < 0 || panelIndex >= panels.size()) {
        return;
    }

    Panel panel = panels[panelIndex];

    switch (panel.type) {
    case SplashScreen:
        configuration()->setNeedShowSplashScreen(visible);
        break;
    case Navigator:
        configuration()->setIsNotationNavigatorVisible(visible);
        break;
    case Tours:
        configuration()->setNeedShowTours(visible);
        break;
    case Unknown:
        return;
    }

    emit panelsChanged();
}
