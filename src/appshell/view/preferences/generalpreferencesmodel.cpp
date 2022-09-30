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
#include "generalpreferencesmodel.h"

#include "languages/languageserrors.h"

#include "log.h"

using namespace mu::appshell;
using namespace mu::framework;
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

    setIsNeedRestart(languagesService()->needRestartToApplyLanguageChange());
    languagesService()->needRestartToApplyLanguageChangeChanged().onReceive(this, [this](bool need) {
        setIsNeedRestart(need);
    });

    projectConfiguration()->autoSaveEnabledChanged().onReceive(this, [this](bool enabled) {
        emit autoSaveEnabledChanged(enabled);
    });

    projectConfiguration()->autoSaveIntervalChanged().onReceive(this, [this](int minutes) {
        emit autoSaveIntervalChanged(minutes);
    });
}

void GeneralPreferencesModel::checkUpdateForCurrentLanguage()
{
    QString languageCode = currentLanguageCode();

    m_languageUpdateProgress = languagesService()->update(languageCode);

    m_languageUpdateProgress.progressChanged.onReceive(this, [this](int64_t current, int64_t total, const std::string& status) {
        emit receivingUpdateForCurrentLanguage(current, total, QString::fromStdString(status));
    });

    m_languageUpdateProgress.finished.onReceive(this, [this, languageCode](const ProgressResult& res) {
        if (res.ret.code() == static_cast<int>(Err::AlreadyUpToDate)) {
            QString msg = mu::qtrc("appshell/preferences", "Your version of %1 is up to date.")
                          .arg(languagesService()->language(languageCode).name);
            interactive()->info(msg.toStdString(), std::string());
        }
    });
}

QVariantList GeneralPreferencesModel::languages() const
{
    QList<Language> languages = languagesService()->languages().values();

    std::sort(languages.begin(), languages.end(), [](const Language& l, const Language& r) {
        return l.code < r.code;
    });

    QVariantList result;

    for (const Language& language : languages) {
        QVariantMap languageObj;
        languageObj["code"] = language.code;
        languageObj["name"] = language.name;
        result << languageObj;
    }

    if (languagesService()->hasPlaceholderLanguage()) {
        QVariantMap placeholderLanguageObj;
        placeholderLanguageObj["code"] = PLACEHOLDER_LANGUAGE_CODE;
        placeholderLanguageObj["name"] = "«Placeholder translations»";
        result.prepend(placeholderLanguageObj);
    }

    QVariantMap systemLanguageObj;
    systemLanguageObj["code"] = SYSTEM_LANGUAGE_CODE;
    systemLanguageObj["name"] = mu::qtrc("appshell/preferences", "System default");
    result.prepend(systemLanguageObj);

    return result;
}

QString GeneralPreferencesModel::currentLanguageCode() const
{
    return languagesConfiguration()->currentLanguageCode().val;
}

QStringList GeneralPreferencesModel::keyboardLayouts() const
{
    NOT_IMPLEMENTED;
    return { "US-QWERTY", "UK-QWERTY", "QWERTZ", "AZERTY" };
}

QString GeneralPreferencesModel::currentKeyboardLayout() const
{
    return shortcutsConfiguration()->currentKeyboardLayout();
}

bool GeneralPreferencesModel::isAutoSaveEnabled() const
{
    return projectConfiguration()->isAutoSaveEnabled();
}

int GeneralPreferencesModel::autoSaveInterval() const
{
    return projectConfiguration()->autoSaveIntervalMinutes();
}

bool GeneralPreferencesModel::isOSCRemoteControl() const
{
    return false;
}

int GeneralPreferencesModel::oscPort() const
{
    return 0;
}

void GeneralPreferencesModel::setCurrentLanguageCode(const QString& currentLanguageCode)
{
    if (currentLanguageCode == this->currentLanguageCode()) {
        return;
    }

    languagesConfiguration()->setCurrentLanguageCode(currentLanguageCode);
    emit currentLanguageCodeChanged(currentLanguageCode);
}

void GeneralPreferencesModel::setCurrentKeyboardLayout(const QString& keyboardLayout)
{
    if (keyboardLayout == this->currentKeyboardLayout()) {
        return;
    }

    shortcutsConfiguration()->setCurrentKeyboardLayout(keyboardLayout);
    emit currentKeyboardLayoutChanged();
}

void GeneralPreferencesModel::setAutoSaveEnabled(bool enabled)
{
    if (enabled == isAutoSaveEnabled()) {
        return;
    }

    projectConfiguration()->setAutoSaveEnabled(enabled);
    emit autoSaveEnabledChanged(enabled);
}

void GeneralPreferencesModel::setAutoSaveInterval(int minutes)
{
    if (minutes == autoSaveInterval()) {
        return;
    }

    projectConfiguration()->setAutoSaveInterval(minutes);
    emit autoSaveIntervalChanged(minutes);
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

bool GeneralPreferencesModel::isNeedRestart() const
{
    return m_isNeedRestart;
}

void GeneralPreferencesModel::setIsNeedRestart(bool newIsNeedRestart)
{
    if (m_isNeedRestart == newIsNeedRestart) {
        return;
    }
    m_isNeedRestart = newIsNeedRestart;
    emit isNeedRestartChanged();
}
