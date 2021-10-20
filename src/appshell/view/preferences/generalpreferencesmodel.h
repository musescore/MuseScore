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
#ifndef MU_APPSHELL_GENERALPREFERENCESMODEL_H
#define MU_APPSHELL_GENERALPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "global/iinteractive.h"
#include "languages/ilanguagesconfiguration.h"
#include "languages/ilanguagesservice.h"
#include "project/iprojectconfiguration.h"
#include "telemetry/itelemetryconfiguration.h"

namespace mu::appshell {
class GeneralPreferencesModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, framework::IInteractive, interactive)
    INJECT(appshell, languages::ILanguagesConfiguration, languagesConfiguration)
    INJECT(appshell, languages::ILanguagesService, languagesService)
    INJECT(appshell, project::IProjectConfiguration, projectConfiguration)
    INJECT(appshell, telemetry::ITelemetryConfiguration, telemetryConfiguration)

    Q_PROPERTY(QVariantList languages READ languages NOTIFY languagesChanged)
    Q_PROPERTY(QString currentLanguageCode READ currentLanguageCode WRITE setCurrentLanguageCode NOTIFY currentLanguageCodeChanged)

    Q_PROPERTY(bool isTelemetryAllowed READ isTelemetryAllowed WRITE setIsTelemetryAllowed NOTIFY isTelemetryAllowedChanged)
    Q_PROPERTY(bool isAutoSaveEnabled READ isAutoSaveEnabled WRITE setAutoSaveEnabled NOTIFY autoSaveEnabledChanged)
    Q_PROPERTY(int autoSaveInterval READ autoSaveInterval WRITE setAutoSaveInterval NOTIFY autoSaveIntervalChanged)
    Q_PROPERTY(bool isOSCRemoteControl READ isOSCRemoteControl WRITE setIsOSCRemoteControl NOTIFY isOSCRemoteControlChanged)
    Q_PROPERTY(int oscPort READ oscPort WRITE setOscPort NOTIFY oscPortChanged)

public:
    explicit GeneralPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void openUpdateTranslationsPage();

    QVariantList languages() const;
    QString currentLanguageCode() const;

    bool isTelemetryAllowed() const;
    bool isAutoSaveEnabled() const;
    int autoSaveInterval() const;
    bool isOSCRemoteControl() const;
    int oscPort() const;

public slots:
    void setCurrentLanguageCode(QString currentLanguageCode);
    void setIsTelemetryAllowed(bool isTelemetryAllowed);
    void setAutoSaveEnabled(bool enabled);
    void setAutoSaveInterval(int minutes);
    void setIsOSCRemoteControl(bool isOSCRemoteControl);
    void setOscPort(int oscPort);

signals:
    void languagesChanged(QVariantList languages);
    void currentLanguageCodeChanged(QString currentLanguageCode);
    void isTelemetryAllowedChanged(bool isTelemetryAllowed);
    void autoSaveEnabledChanged(bool enabled);
    void autoSaveIntervalChanged(int minutes);
    void isOSCRemoteControlChanged(bool isOSCRemoteControl);
    void oscPortChanged(int oscPort);
};
}

#endif // MU_APPSHELL_GENERALPREFERENCESMODEL_H
