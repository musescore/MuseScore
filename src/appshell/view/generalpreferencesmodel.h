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
#ifndef MU_APPSHELL_GENERALPREFERENCESMODEL_H
#define MU_APPSHELL_GENERALPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "global/iinteractive.h"
#include "languages/ilanguagesservice.h"
#include "telemetry/itelemetryconfiguration.h"

namespace mu::appshell {
class GeneralPreferencesModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, framework::IInteractive, interactive)
    INJECT(appshell, languages::ILanguagesService, languagesService)
    INJECT(appshell, telemetry::ITelemetryConfiguration, telemetryConfiguration)

    Q_PROPERTY(QVariantList languages READ languages NOTIFY languagesChanged)
    Q_PROPERTY(QString currentLanguageCode READ currentLanguageCode WRITE setCurrentLanguageCode NOTIFY currentLanguageCodeChanged)

    Q_PROPERTY(bool isTelemetryAllowed READ isTelemetryAllowed WRITE setIsTelemetryAllowed NOTIFY isTelemetryAllowedChanged)
    Q_PROPERTY(bool isAutoSave READ isAutoSave WRITE setIsAutoSave NOTIFY isAutoSaveChanged)
    Q_PROPERTY(int autoSavePeriod READ autoSavePeriod WRITE setAutoSavePeriod NOTIFY autoSavePeriodChanged)
    Q_PROPERTY(bool isOSCRemoteControl READ isOSCRemoteControl WRITE setIsOSCRemoteControl NOTIFY isOSCRemoteControlChanged)
    Q_PROPERTY(int oscPort READ oscPort WRITE setOscPort NOTIFY oscPortChanged)

public:
    explicit GeneralPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void openUpdateTranslationsPage();

    QVariantList languages() const;
    QString currentLanguageCode() const;

    bool isTelemetryAllowed() const;
    bool isAutoSave() const;
    int autoSavePeriod() const;
    bool isOSCRemoteControl() const;
    int oscPort() const;

public slots:
    void setCurrentLanguageCode(QString currentLanguageCode);
    void setIsTelemetryAllowed(bool isTelemetryAllowed);
    void setIsAutoSave(bool isAutoSave);
    void setAutoSavePeriod(int sutoSavePeriod);
    void setIsOSCRemoteControl(bool isOSCRemoteControl);
    void setOscPort(int oscPort);

signals:
    void languagesChanged(QVariantList languages);
    void currentLanguageCodeChanged(QString currentLanguageCode);
    void isTelemetryAllowedChanged(bool isTelemetryAllowed);
    void isAutoSaveChanged(bool isAutoSave);
    void isOSCRemoteControlChanged(bool isOSCRemoteControl);
    void oscPortChanged(int oscPort);
    void autoSavePeriodChanged(int autoSavePeriod);
};
}

#endif // MU_APPSHELL_GENERALPREFERENCESMODEL_H
