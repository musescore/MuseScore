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
#pragma once

#include <QObject>

#include "progress.h"

#include "modularity/ioc.h"
#include "iappshellconfiguration.h"
#include "async/asyncable.h"

#include "global/iinteractive.h"
#include "languages/ilanguagesconfiguration.h"
#include "languages/ilanguagesservice.h"
#include "shortcuts/ishortcutsconfiguration.h"

namespace mu::appshell {
class GeneralPreferencesModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QVariantList languages READ languages NOTIFY languagesChanged)
    Q_PROPERTY(QString currentLanguageCode READ currentLanguageCode WRITE setCurrentLanguageCode NOTIFY currentLanguageCodeChanged)

    Q_PROPERTY(QStringList keyboardLayouts READ keyboardLayouts CONSTANT)
    Q_PROPERTY(QString currentKeyboardLayout READ currentKeyboardLayout WRITE setCurrentKeyboardLayout NOTIFY currentKeyboardLayoutChanged)

    Q_PROPERTY(bool isOSCRemoteControl READ isOSCRemoteControl WRITE setIsOSCRemoteControl NOTIFY isOSCRemoteControlChanged)
    Q_PROPERTY(int oscPort READ oscPort WRITE setOscPort NOTIFY oscPortChanged)

    Q_PROPERTY(bool restartRequired READ restartRequired WRITE setRestartRequired NOTIFY restartRequiredChanged)

    Q_PROPERTY(QVariantList startupModes READ startupModes CONSTANT)
    Q_PROPERTY(int currentStartupMode READ currentStartupMode WRITE setCurrentStartupMode NOTIFY currentStartupModeChanged)
    Q_PROPERTY(QString startupScorePath READ startupScorePath WRITE setStartupScorePath NOTIFY startupScorePathChanged)
    Q_PROPERTY(QStringList scorePathFilter READ scorePathFilter CONSTANT)

    Q_PROPERTY(bool showWelcomeDialog READ showWelcomeDialog WRITE setShowWelcomeDialog NOTIFY showWelcomeDialogChanged)

    muse::Inject<IAppShellConfiguration> configuration = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<muse::languages::ILanguagesConfiguration> languagesConfiguration = { this };
    muse::Inject<muse::languages::ILanguagesService> languagesService = { this };
    muse::Inject<muse::shortcuts::IShortcutsConfiguration> shortcutsConfiguration = { this };

public:
    explicit GeneralPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void checkUpdateForCurrentLanguage();

    QVariantList languages() const;
    QString currentLanguageCode() const;

    QStringList keyboardLayouts() const;
    QString currentKeyboardLayout() const;

    bool isOSCRemoteControl() const;
    int oscPort() const;
    bool restartRequired() const;

    QVariantList startupModes() const;
    int currentStartupMode() const;
    QString startupScorePath() const;
    QStringList scorePathFilter() const;

    bool showWelcomeDialog() const;
    void setShowWelcomeDialog(bool show);

public slots:
    void setCurrentLanguageCode(const QString& currentLanguageCode);
    void setCurrentKeyboardLayout(const QString& keyboardLayout);
    void setIsOSCRemoteControl(bool isOSCRemoteControl);
    void setOscPort(int oscPort);
    void setRestartRequired(bool restartRequired);
    void setCurrentStartupMode(int mode);
    void setStartupScorePath(const QString& scorePath);

signals:
    void languagesChanged(QVariantList languages);
    void currentLanguageCodeChanged(QString currentLanguageCode);
    void currentKeyboardLayoutChanged();
    void isOSCRemoteControlChanged(bool isOSCRemoteControl);
    void oscPortChanged(int oscPort);

    void receivingUpdateForCurrentLanguage(int current, int total, QString status);
    void restartRequiredChanged();

    void currentStartupModeChanged();
    void startupScorePathChanged();

    void showWelcomeDialogChanged();

private:
    muse::Progress m_languageUpdateProgress;

    bool m_restartRequired = false;
};
}
