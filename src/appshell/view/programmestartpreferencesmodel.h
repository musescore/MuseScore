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
#ifndef MU_APPSHELL_PROGRAMMESTARTPREFERENCESMODEL_H
#define MU_APPSHELL_PROGRAMMESTARTPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "iappshellconfiguration.h"
#include "userscores/iuserscoresconfiguration.h"

namespace mu::appshell {
class ProgrammeStartPreferencesModel : public QObject
{
    Q_OBJECT

    INJECT(appshell, IAppShellConfiguration, configuration)

    Q_PROPERTY(QVariantList startupModes READ startupModes NOTIFY startupModesChanged)
    Q_PROPERTY(QVariantList panels READ panels NOTIFY panelsChanged)

public:
    explicit ProgrammeStartPreferencesModel(QObject* parent = nullptr);

    QVariantList startupModes() const;
    QVariantList panels() const;

    Q_INVOKABLE QString scorePathFilter() const;

    Q_INVOKABLE void setCurrentStartupMode(int modeIndex);
    Q_INVOKABLE void setStartupScorePath(const QString& scorePath);
    Q_INVOKABLE void setPanelVisible(int panelIndex, bool visible);

signals:
    void startupModesChanged();
    void panelsChanged();

private:
    enum PanelType {
        Unknown,
        SplashScreen,
        Navigator,
        Tours
    };

    struct Panel
    {
        PanelType type = Unknown;
        QString title;
        bool visible = false;
    };

    using PanelList = QList<Panel>;

    struct StartMode
    {
        StartupSessionType sessionType = StartupSessionType::StartWithNewScore;
        QString title;
        bool checked = false;
        bool canSelectScorePath = false;
        QString scorePath;
    };

    using StartModeList = QList<StartMode>;

    PanelList allPanels() const;
    StartModeList allStartupModes() const;
};
}

#endif // MU_APPSHELL_GENERALPREFERENCESMODEL_H
