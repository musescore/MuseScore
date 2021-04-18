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
#include "generalpreferencesmodel.h"

#include "log.h"

using namespace mu::appshell;
using namespace mu::languages;

GeneralPreferencesModel::GeneralPreferencesModel(QObject* parent)
    : QObject(parent)
{
}

void GeneralPreferencesModel::load()
{
    languagesConfiguration()->currentLanguageCode().ch.onReceive(this, [this](const QString& languageCode) {
        emit currentLanguageCodeChanged(languageCode);
    });

    languagesService()->languages().ch.onReceive(this, [this](const LanguagesHash&) {
        emit languagesChanged(languages());
    });

    telemetryConfiguration()->isTelemetryAllowed().ch.onReceive(this, [this](bool) {
        emit isTelemetryAllowedChanged(isTelemetryAllowed());
    });
}

void GeneralPreferencesModel::openUpdateTranslationsPage()
{
    interactive()->open("musescore://home?item=add-ons&subItem=languages");
}

QVariantList GeneralPreferencesModel::languages() const
{
    ValCh<LanguagesHash> languages = languagesService()->languages();
    QList<Language> languageList = languages.val.values();

    QVariantList result;

    for (const Language& language: languageList) {
        if (language.status == LanguageStatus::Status::NoInstalled
            || language.status == LanguageStatus::Status::Undefined) {
            continue;
        }

        QVariantMap languageObj;
        languageObj["code"] = language.code;
        languageObj["name"] = language.name;
        result << languageObj;
    }

    std::sort(result.begin(), result.end(), [](const QVariant& l, const QVariant& r) {
        return l.toMap().value("code").toString() < r.toMap().value("code").toString();
    });

    return result;
}

QString GeneralPreferencesModel::currentLanguageCode() const
{
    return languagesConfiguration()->currentLanguageCode().val;
}

bool GeneralPreferencesModel::isTelemetryAllowed() const
{
    return telemetryConfiguration()->isTelemetryAllowed().val;
}

bool GeneralPreferencesModel::isAutoSave() const
{
    return false;
}

int GeneralPreferencesModel::autoSavePeriod() const
{
    return 0;
}

bool GeneralPreferencesModel::isOSCRemoteControl() const
{
    return false;
}

int GeneralPreferencesModel::oscPort() const
{
    return 0;
}

void GeneralPreferencesModel::setCurrentLanguageCode(QString currentLanguageCode)
{
    if (currentLanguageCode == this->currentLanguageCode()) {
        return;
    }

    languagesConfiguration()->setCurrentLanguageCode(currentLanguageCode);
    emit currentLanguageCodeChanged(currentLanguageCode);
}

void GeneralPreferencesModel::setIsTelemetryAllowed(bool isTelemetryAllowed)
{
    if (isTelemetryAllowed == this->isTelemetryAllowed()) {
        return;
    }

    telemetryConfiguration()->setIsTelemetryAllowed(isTelemetryAllowed);
    emit isTelemetryAllowedChanged(isTelemetryAllowed);
}

void GeneralPreferencesModel::setIsAutoSave(bool isAutoSave)
{
    NOT_IMPLEMENTED;
    emit isAutoSaveChanged(isAutoSave);
}

void GeneralPreferencesModel::setAutoSavePeriod(int autoSavePeriod)
{
    NOT_IMPLEMENTED;
    emit autoSavePeriodChanged(autoSavePeriod);
}

void GeneralPreferencesModel::setIsOSCRemoteControl(bool isOSCRemoteControl)
{
    NOT_IMPLEMENTED;
    emit isOSCRemoteControlChanged(isOSCRemoteControl);
}

void GeneralPreferencesModel::setOscPort(int oscPort)
{
    NOT_IMPLEMENTED;
    emit oscPortChanged(oscPort);
}
