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
#include "applicationactioncontroller.h"

#include <QCoreApplication>
#include <QCloseEvent>
#include <QWindow>

#include "translation.h"

using namespace mu::appshell;
using namespace mu::framework;
using namespace mu::actions;

void ApplicationActionController::init()
{
    dispatcher()->reg(this, "quit", this, &ApplicationActionController::quit);

    dispatcher()->reg(this, "fullscreen", this, &ApplicationActionController::toggleFullScreen);

    dispatcher()->reg(this, "about", this, &ApplicationActionController::openAboutDialog);
    dispatcher()->reg(this, "about-qt", this, &ApplicationActionController::openAboutQtDialog);
    dispatcher()->reg(this, "about-musicxml", this, &ApplicationActionController::openAboutMusicXMLDialog);
    dispatcher()->reg(this, "online-handbook", this, &ApplicationActionController::openOnlineHandbookPage);
    dispatcher()->reg(this, "ask-help", this, &ApplicationActionController::openAskForHelpPage);
    dispatcher()->reg(this, "report-bug", this, &ApplicationActionController::openBugReportPage);
    dispatcher()->reg(this, "leave-feedback", this, &ApplicationActionController::openLeaveFeedbackPage);
    dispatcher()->reg(this, "preference-dialog", this, &ApplicationActionController::openPreferencesDialog);

    dispatcher()->reg(this, "revert-factory", this, &ApplicationActionController::revertToFactorySettings);

    qApp->installEventFilter(this);
}

bool ApplicationActionController::eventFilter(QObject* watched, QEvent* event)
{
    QCloseEvent* closeEvent = dynamic_cast<QCloseEvent*>(event);
    if (closeEvent && watched == mainWindow()->qWindow()) {
        quit();
        closeEvent->ignore();
        return true;
    }

    return QObject::eventFilter(watched, event);
}

mu::ValCh<bool> ApplicationActionController::isFullScreen() const
{
    ValCh<bool> result;
    result.ch = m_fullScreenChannel;
    result.val = mainWindow()->isFullScreen();

    return result;
}

void ApplicationActionController::quit()
{
    if (projectFilesController()->closeOpenedProject()) {
        QCoreApplication::quit();
    }
}

void ApplicationActionController::toggleFullScreen()
{
    mainWindow()->toggleFullScreen();
    bool isFullScreen = mainWindow()->isFullScreen();
    m_fullScreenChannel.send(isFullScreen);
}

void ApplicationActionController::openAboutDialog()
{
    interactive()->open("musescore://about/musescore");
}

void ApplicationActionController::openAboutQtDialog()
{
    QApplication::aboutQt();
}

void ApplicationActionController::openAboutMusicXMLDialog()
{
    interactive()->open("musescore://about/musicxml");
}

void ApplicationActionController::openOnlineHandbookPage()
{
    std::string handbookUrl = configuration()->handbookUrl();
    interactive()->openUrl(handbookUrl);
}

void ApplicationActionController::openAskForHelpPage()
{
    std::string askForHelpUrl = configuration()->askForHelpUrl();
    interactive()->openUrl(askForHelpUrl);
}

void ApplicationActionController::openBugReportPage()
{
    std::string bugReportUrl = configuration()->bugReportUrl();
    interactive()->openUrl(bugReportUrl);
}

void ApplicationActionController::openLeaveFeedbackPage()
{
    std::string leaveFeedbackUrl = configuration()->leaveFeedbackUrl();
    interactive()->openUrl(leaveFeedbackUrl);
}

void ApplicationActionController::openPreferencesDialog()
{
    if (multiInstancesProvider()->isPreferencesAlreadyOpened()) {
        multiInstancesProvider()->activateWindowWithOpenedPreferences();
        return;
    }

    interactive()->open("musescore://preferences");
}

void ApplicationActionController::revertToFactorySettings()
{
    std::string question = trc("appshell", "This will reset all your preferences.\n"
                                           "Custom palettes, custom shortcuts, and the list of recent scores will be deleted. "
                                           "Reverting will not remove any scores from your computer.\n"
                                           "Are you sure you want to proceed?");

    IInteractive::Result result = interactive()->question(std::string(), question, {
        IInteractive::Button::Yes,
        IInteractive::Button::No
    });

    if (result.standardButton() == IInteractive::Button::Yes) {
        configuration()->revertToFactorySettings();
    }
}
