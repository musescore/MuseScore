/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "translation.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::languages;

GeneralPreferencesModel::GeneralPreferencesModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void GeneralPreferencesModel::load()
{
    languagesConfiguration()->currentLanguageCode().ch.onReceive(this, [this](const QString& languageCode) {
        emit currentLanguageCodeChanged(languageCode);
    });

    setRestartRequired(languagesService()->restartRequiredToApplyLanguage());
    languagesService()->restartRequiredToApplyLanguageChanged().onReceive(this, [this](bool required) {
        setRestartRequired(required);
    });

    configuration()->startupModeTypeChanged().onNotify(this, [this]() {
        emit currentStartupModeChanged();
    });

    configuration()->startupScorePathChanged().onNotify(this, [this]() {
        emit startupScorePathChanged();
    });

    configuration()->welcomeDialogShowOnStartupChanged().onNotify(this, [this]() {
        emit showWelcomeDialogChanged();
    });
}

void GeneralPreferencesModel::checkUpdateForCurrentLanguage()
{
    QString languageCode = currentLanguageCode();

    m_languageUpdateProgress = languagesService()->update(languageCode);

    m_languageUpdateProgress.progressChanged().onReceive(this, [this](int64_t current, int64_t total, const std::string& status) {
        emit receivingUpdateForCurrentLanguage(current, total, QString::fromStdString(status));
    });

    m_languageUpdateProgress.finished().onReceive(this, [this, languageCode](const ProgressResult& res) {
        if (res.ret.code() == static_cast<int>(Err::AlreadyUpToDate)) {
            QString msg = muse::qtrc("appshell/preferences", "Your version of %1 is up to date.")
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
    systemLanguageObj["name"] = muse::qtrc("appshell/preferences", "System default");
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
}

void GeneralPreferencesModel::setCurrentKeyboardLayout(const QString& keyboardLayout)
{
    if (keyboardLayout == this->currentKeyboardLayout()) {
        return;
    }

    shortcutsConfiguration()->setCurrentKeyboardLayout(keyboardLayout);
    emit currentKeyboardLayoutChanged();
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

bool GeneralPreferencesModel::restartRequired() const
{
    return m_restartRequired;
}

void GeneralPreferencesModel::setRestartRequired(bool restartRequired)
{
    if (m_restartRequired == restartRequired) {
        return;
    }
    m_restartRequired = restartRequired;
    emit restartRequiredChanged();
}

QVariantList GeneralPreferencesModel::startupModes() const
{
    const QVariantList result {
        QVariantMap {
            { "title", muse::qtrc("appshell/preferences", "Start empty") },
            { "value", static_cast<int>(StartupModeType::StartEmpty) },
        },
        QVariantMap {
            { "title", muse::qtrc("appshell/preferences", "Continue last session") },
            { "value", static_cast<int>(StartupModeType::ContinueLastSession) },
        },
        QVariantMap {
            { "title", muse::qtrc("appshell/preferences", "Start with new score") },
            { "value", static_cast<int>(StartupModeType::StartWithNewScore) },
        },
        QVariantMap {
            { "title", muse::qtrc("appshell/preferences", "Start with score:") },
            { "value", static_cast<int>(StartupModeType::StartWithScore) },
            { "isStartWithScore", true },
        },
    };

    return result;
}

int GeneralPreferencesModel::currentStartupMode() const
{
    return static_cast<int>(configuration()->startupModeType());
}

void GeneralPreferencesModel::setCurrentStartupMode(int modeIndex)
{
    if (modeIndex < 0 || modeIndex >= 4) {
        return;
    }

    StartupModeType selectedType = static_cast<StartupModeType>(modeIndex);
    if (selectedType == configuration()->startupModeType()) {
        return;
    }

    configuration()->setStartupModeType(selectedType);
}

QString GeneralPreferencesModel::startupScorePath() const
{
    return configuration()->startupScorePath().toQString();
}

void GeneralPreferencesModel::setStartupScorePath(const QString& scorePath)
{
    if (scorePath.isEmpty() || scorePath == startupScorePath()) {
        return;
    }

    configuration()->setStartupScorePath(scorePath);
}

QStringList GeneralPreferencesModel::scorePathFilter() const
{
    return { muse::qtrc("appshell/preferences", "MuseScore file") + " (*.mscz)",
             muse::qtrc("appshell/preferences", "All") + " (*)" };
}

bool GeneralPreferencesModel::showWelcomeDialog() const
{
    return configuration()->welcomeDialogShowOnStartup();
}

void GeneralPreferencesModel::setShowWelcomeDialog(bool show)
{
    if (configuration()->welcomeDialogShowOnStartup() == show) {
        return;
    }

    configuration()->setWelcomeDialogShowOnStartup(show);
}
