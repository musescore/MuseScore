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
#include "interactivetestsmodel.h"

#include <QTimer>

#include "log.h"

#include "async/async.h"

#include <QAccessible>

using namespace mu::ui;
using namespace mu::framework;

InteractiveTestsModel::InteractiveTestsModel(QObject* parent)
    : QObject(parent)
{
    ValCh<Uri> uri = interactive()->currentUri();
    setCurrentUri(uri.val);
    uri.ch.onReceive(this, [this](const Uri& uri) {
        setCurrentUri(uri);
    });
}

void InteractiveTestsModel::openSampleDialog()
{
    LOGI() << "cpp: before open";
    RetVal<Val> rv = interactive()->open("musescore://devtools/interactive/sample?color=#474747");
    LOGI() << "cpp: after open ret: " << rv.ret.toString() << ", val: " << rv.val.toString();
}

void InteractiveTestsModel::openSampleDialogAsync()
{
    LOGI() << "cpp: before open ";
    RetVal<Val> rv = interactive()->open("musescore://devtools/interactive/sample?sync=false&color=#D24373");
    LOGI() << "cpp: after open ret: " << rv.ret.toString() << ", val: " << rv.val.toString();
}

void InteractiveTestsModel::closeSampleDialog()
{
    LOGI() << "cpp: before close ";
    interactive()->close("musescore://devtools/interactive/sample");
    LOGI() << "cpp: after close";
}

void InteractiveTestsModel::openWidgetDialog()
{
    LOGI() << "cpp: before open ";
    RetVal<Val> rv = interactive()->open("musescore://devtools/interactive/testdialog?title='And from its properties'");
    LOGI() << "cpp: after open ret: " << rv.ret.toString() << ", val: " << rv.val.toString();
}

void InteractiveTestsModel::openWidgetDialogAsync()
{
    LOGI() << "cpp: before open ";
    RetVal<Val> rv = interactive()->open("musescore://devtools/interactive/testdialog?sync=false&title='And from its properties'");
    LOGI() << "cpp: after open ret: " << rv.ret.toString() << ", val: " << rv.val.toString();
}

void InteractiveTestsModel::closeWidgetDialog()
{
    LOGI() << "cpp: before close ";
    interactive()->close("musescore://devtools/interactive/testdialog");
    LOGI() << "cpp: after close";
}

void InteractiveTestsModel::setCurrentUri(const Uri& uri)
{
    m_currentUri = QString::fromStdString(uri.toString());
    emit currentUriChanged(m_currentUri);
}

QString InteractiveTestsModel::currentUri() const
{
    return m_currentUri;
}

void InteractiveTestsModel::question()
{
    IInteractive::Result result = interactive()->question(
        "Do you really want to delete the 'xxx' workspace?", "",
        { interactive()->buttonData(IInteractive::Button::No),
          interactive()->buttonData(IInteractive::Button::Yes) }, 0,
        IInteractive::Option::WithIcon);

    if (result.standartButton() == IInteractive::Button::Yes) {
        LOGI() << "Yes!!";
    } else {
        LOGI() << "No!!";
    }
}

void InteractiveTestsModel::customQuestion()
{
    int maybeBtn = int(IInteractive::Button::CustomButton) + 1;
    IInteractive::Result result = interactive()->question("Test", "It works?", {
        IInteractive::ButtonData(maybeBtn, "Maybe"),
        interactive()->buttonData(IInteractive::Button::No)
    });

    if (result.button() == maybeBtn) {
        LOGI() << "Maybe!!";
    } else {
        LOGE() << "No!!";
    }
}

void InteractiveTestsModel::information()
{
    IInteractive::Result result = interactive()->info("Tuplet cannot cross barlines", "", {}, 0,
                                                      IInteractive::Option::WithIcon | IInteractive::Option::WithShowAgain);
    LOGD() << interactive()->buttonData(result.standartButton()).text;
}

void InteractiveTestsModel::warning()
{
    int noSaveButton = int(IInteractive::Button::CustomButton) + 1;
    int saveButton = noSaveButton + 1;

    IInteractive::Result result = interactive()->warning("Do you want to save changes to the score “Untitled” before closing?",
                                                         "Your changes will be lost if you don’t save them.",
                                                         { IInteractive::ButtonData(noSaveButton, "Don’t save"),
                                                           interactive()->buttonData(IInteractive::Button::Cancel),
                                                           IInteractive::ButtonData(saveButton, "Save", true) }, saveButton,
                                                         IInteractive::Option::WithIcon);

    if (result.button() == noSaveButton) {
        LOGI() << "Don’t save!!";
    } else if (result.button() == saveButton) {
        LOGE() << "Save!!";
    } else {
        LOGE() << "Cancel!!";
    }
}

void InteractiveTestsModel::critical()
{
    IInteractive::Result result = interactive()->error("Cannot read file C:/Users/Username/Desktop/Composition.mscz",
                                                       "An error has occured when trying to open this file",  {}, 0,
                                                       IInteractive::Option::WithIcon);
    LOGD() << interactive()->buttonData(result.standartButton()).text;
}

void InteractiveTestsModel::require()
{
    RetVal<Val> rv = interactive()->open("musescore://devtools/interactive/sample?title='Test'");
    if (rv.ret) {
        LOGI() << "received: " << rv.val.toString();
    } else if (check_ret(rv.ret, Ret::Code::Cancel)) {
        LOGI() << "was cancelled";
    } else {
        LOGE() << "some error: " << rv.ret.code();
    }
}
