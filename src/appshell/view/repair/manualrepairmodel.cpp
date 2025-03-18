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
#include "manualrepairmodel.h"

#include "languages/languageserrors.h"

#include "log.h"
#include "translation.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::languages;

ManualRepairModel::ManualRepairModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void ManualRepairModel::load()
{
    languagesConfiguration()->currentLanguageCode().ch.onReceive(this, [this](const QString& languageCode) {
        emit currentLanguageCodeChanged(languageCode);
    });

    setIsNeedRestart(languagesService()->needRestartToApplyLanguageChange());
    languagesService()->needRestartToApplyLanguageChangeChanged().onReceive(this, [this](bool need) {
        setIsNeedRestart(need);
    });

    configuration()->startupModeTypeChanged().onNotify(this, [this]() {
        emit startupModesChanged();
    });

    configuration()->startupScorePathChanged().onNotify(this, [this]() {
        emit startupModesChanged();
    });
}

void ManualRepairModel::checkUpdateForCurrentLanguage()
{
    QString languageCode = currentLanguageCode();

    m_languageUpdateProgress = languagesService()->update(languageCode);

    m_languageUpdateProgress.progressChanged.onReceive(this, [this](int64_t current, int64_t total, const std::string& status) {
        emit receivingUpdateForCurrentLanguage(current, total, QString::fromStdString(status));
    });

    m_languageUpdateProgress.finished.onReceive(this, [this, languageCode](const ProgressResult& res) {
        if (res.ret.code() == static_cast<int>(Err::AlreadyUpToDate)) {
            QString msg = muse::qtrc("appshell/preferences", "Your version of %1 is up to date.")
                          .arg(languagesService()->language(languageCode).name);
            interactive()->info(msg.toStdString(), std::string());
        }
    });
}

QVariantList ManualRepairModel::languages() const
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

QString ManualRepairModel::currentLanguageCode() const
{
    return languagesConfiguration()->currentLanguageCode().val;
}

QStringList ManualRepairModel::keyboardLayouts() const
{
    NOT_IMPLEMENTED;
    return { "US-QWERTY", "UK-QWERTY", "QWERTZ", "AZERTY" };
}

QString ManualRepairModel::currentKeyboardLayout() const
{
    return shortcutsConfiguration()->currentKeyboardLayout();
}

bool ManualRepairModel::isOSCRemoteControl() const
{
    return false;
}

int ManualRepairModel::oscPort() const
{
    return 0;
}

void ManualRepairModel::setCurrentLanguageCode(const QString& currentLanguageCode)
{
    if (currentLanguageCode == this->currentLanguageCode()) {
        return;
    }

    languagesConfiguration()->setCurrentLanguageCode(currentLanguageCode);
    emit currentLanguageCodeChanged(currentLanguageCode);
}

void ManualRepairModel::setCurrentKeyboardLayout(const QString& keyboardLayout)
{
    if (keyboardLayout == this->currentKeyboardLayout()) {
        return;
    }

    shortcutsConfiguration()->setCurrentKeyboardLayout(keyboardLayout);
    emit currentKeyboardLayoutChanged();
}

void ManualRepairModel::setIsOSCRemoteControl(bool isOSCRemoteControl)
{
    NOT_IMPLEMENTED;
    emit isOSCRemoteControlChanged(isOSCRemoteControl);
}

void ManualRepairModel::setOscPort(int oscPort)
{
    NOT_IMPLEMENTED;
    emit oscPortChanged(oscPort);
}

bool ManualRepairModel::isNeedRestart() const
{
    return m_isNeedRestart;
}

void ManualRepairModel::setIsNeedRestart(bool newIsNeedRestart)
{
    if (m_isNeedRestart == newIsNeedRestart) {
        return;
    }
    m_isNeedRestart = newIsNeedRestart;
    emit isNeedRestartChanged();
}

QVariantList ManualRepairModel::startupModes() const
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

ManualRepairModel::StartModeList ManualRepairModel::allStartupModes() const
{
    static const QMap<StartupModeType, QString> modeTitles {
        { StartupModeType::StartEmpty,  muse::qtrc("appshell/preferences", "Start empty") },
        { StartupModeType::ContinueLastSession, muse::qtrc("appshell/preferences", "Continue last session") },
        { StartupModeType::StartWithNewScore, muse::qtrc("appshell/preferences", "Start with new score") },
        { StartupModeType::StartWithScore, muse::qtrc("appshell/preferences", "Start with score:") }
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

QStringList ManualRepairModel::scorePathFilter() const
{
    return { muse::qtrc("appshell/preferences", "MuseScore file") + " (*.mscz)",
             muse::qtrc("appshell/preferences", "All") + " (*)" };
}

void ManualRepairModel::setCurrentStartupMode(int modeIndex)
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

void ManualRepairModel::setStartupScorePath(const QString& scorePath)
{
    if (scorePath.isEmpty() || scorePath == configuration()->startupScorePath().toQString()) {
        return;
    }

    configuration()->setStartupScorePath(scorePath);
    emit startupModesChanged();
}
