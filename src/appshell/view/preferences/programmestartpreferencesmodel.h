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
#ifndef MU_APPSHELL_PROGRAMMESTARTPREFERENCESMODEL_H
#define MU_APPSHELL_PROGRAMMESTARTPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "iappshellconfiguration.h"
#include "project/iprojectconfiguration.h"

namespace mu::appshell {
class ProgrammeStartPreferencesModel : public QObject
{
    Q_OBJECT

    INJECT(IAppShellConfiguration, configuration)

    Q_PROPERTY(QVariantList startupModes READ startupModes NOTIFY startupModesChanged)
    Q_PROPERTY(QVariantList panels READ panels NOTIFY panelsChanged)

public:
    explicit ProgrammeStartPreferencesModel(QObject* parent = nullptr);

    QVariantList startupModes() const;
    QVariantList panels() const;

    Q_INVOKABLE QStringList scorePathFilter() const;

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
        Navigator
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
        StartupModeType type = StartupModeType::StartWithNewScore;
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
