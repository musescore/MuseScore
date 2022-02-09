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
    static const QMap<StartupModeType, QString> modeTitles {
        { StartupModeType::StartEmpty,  qtrc("appshell", "Start empty") },
        { StartupModeType::ContinueLastSession, qtrc("appshell", "Continue last session") },
        { StartupModeType::StartWithNewScore, qtrc("appshell", "Start with new score") },
        { StartupModeType::StartWithScore, qtrc("appshell", "Start with score:") }
    };

    StartModeList modes;

    for (StartupModeType type : modeTitles.keys()) {
        bool canSelectScorePath = (type == StartupModeType::StartWithScore);

        StartMode mode;
        mode.type = type;
        mode.title = modeTitles[type];
        mode.checked = configuration()->startupModeType() == type;
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
        /*
         * TODO: https://github.com/musescore/MuseScore/issues/9807
        Panel { SplashScreen, qtrc("appshell", "Show splash screen"), configuration()->needShowSplashScreen() },
         */
        Panel { Navigator, qtrc("appshell", "Show navigator"), configuration()->isNotationNavigatorVisible() },
    };

    return panels;
}

QString ProgrammeStartPreferencesModel::scorePathFilter() const
{
    return qtrc("appshell", "MuseScore File") + " (*.mscz);;"
           + qtrc("appshell", "All") + " (*)";
}

void ProgrammeStartPreferencesModel::setCurrentStartupMode(int modeIndex)
{
    StartModeList modes = allStartupModes();

    if (modeIndex < 0 || modeIndex >= modes.size()) {
        return;
    }

    StartupModeType selectedType = modes[modeIndex].type;
    if (selectedType == configuration()->startupModeType()) {
        return;
    }

    configuration()->setStartupModeType(selectedType);
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
    case Unknown:
        return;
    }

    emit panelsChanged();
}
