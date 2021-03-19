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

QVariantList ProgrammeStartPreferencesModel::startModes() const
{
    NOT_IMPLEMENTED;

    struct StartMode
    {
        QString title;
        bool checked = false;
        bool canSelectScorePath = false;
        QString scorePath;
    };

    QList<StartMode> modes {
        { qtrc("appshell", "Start empty"), false, false, "" },
        { qtrc("appshell", "Continue last session"), false, false, "" },
        { qtrc("appshell", "Start with new score"), false, false, "" },
        { qtrc("appshell", "Start with score:"), true, true, "/tmp/test/foo.mscz" }
    };

    QVariantList result;

    for (const StartMode& mode: modes) {
        QVariantMap obj;
        obj["title"] = mode.title;
        obj["checked"] = mode.checked;
        obj["canSelectScorePath"] = mode.canSelectScorePath;
        obj["scorePath"] = mode.scorePath;

        result << obj;
    };

    return result;
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

void ProgrammeStartPreferencesModel::setCurrentStartMode(int modeIndex, const QString& scorePath)
{
    NOT_IMPLEMENTED;

    UNUSED(modeIndex);
    UNUSED(scorePath);

    emit startModesChanged();
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
