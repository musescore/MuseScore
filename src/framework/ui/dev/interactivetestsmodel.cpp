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
#include <QAccessible>

#include "async/async.h"

#include "global/progress.h"

#include "log.h"

using namespace muse::ui;

struct ProgressData
{
    muse::Progress progress;
    QTimer timer;
    int total = 10;
    int current = 0;
};

InteractiveTestsModel::InteractiveTestsModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
}

void InteractiveTestsModel::init()
{
    ValCh<Uri> uri = interactive()->currentUri();
    setCurrentUri(uri.val);
    uri.ch.onReceive(this, [this](const Uri& uri) {
        setCurrentUri(uri);
    });
}

void InteractiveTestsModel::showProgress()
{
    ProgressData* pd = new ProgressData();
    pd->timer.setInterval(1000);
    connect(&pd->timer, &QTimer::timeout, [pd]() {
        pd->current++;
        pd->progress.progress(pd->current, pd->total, "bla-bla");

        if (pd->current >= pd->total) {
            pd->timer.stop();
            pd->progress.finish(ProgressResult::make_ok(Val()));

            async::Async::call(nullptr, [pd]() {
                delete pd;
            });
        }
    });

    interactive()->showProgress("Progress sample", &pd->progress);

    pd->timer.start();
    pd->progress.start();
}

void InteractiveTestsModel::openSampleDialogSync()
{
    LOGI() << "cpp: before open";
    RetVal<Val> rv = interactive()->openSync("muse://devtools/interactive/sample?color=#474747");
    LOGI() << "cpp: after open ret: " << rv.ret.toString() << ", val: " << rv.val.toString();
}

void InteractiveTestsModel::openSampleDialogAsync()
{
    LOGI() << "cpp: before open ";
    interactive()->open("muse://devtools/interactive/sample?sync=false&color=#D24373");
    LOGI() << "cpp: after open";
}

void InteractiveTestsModel::closeSampleDialog()
{
    LOGI() << "cpp: before close ";
    interactive()->close(Uri("muse://devtools/interactive/sample"));
    LOGI() << "cpp: after close";
}

void InteractiveTestsModel::openSampleDialogAsyncWithPromise()
{
    auto promise = interactive()->open("muse://devtools/interactive/sample?color=#D24358");
    promise.onResolve(this, [](const Val& val) {
        LOGI() << "success val: " << val.toString();
    }).onReject(this, [](int code, const std::string& err) {
        LOGE() << "faile or cancel code: " << code << ", err: " << err;
    });
}

void InteractiveTestsModel::openWidgetDialog()
{
    LOGI() << "cpp: before open ";
    RetVal<Val> rv = interactive()->openSync("muse://devtools/interactive/testdialog?title='And from its properties'");
    LOGI() << "cpp: after open ret: " << rv.ret.toString() << ", val: " << rv.val.toString();
}

void InteractiveTestsModel::openWidgetDialogAsync()
{
    LOGI() << "cpp: before open ";
    RetVal<Val> rv = interactive()->openSync("muse://devtools/interactive/testdialog?sync=false&title='And from its properties'");
    LOGI() << "cpp: after open ret: " << rv.ret.toString() << ", val: " << rv.val.toString();
}

void InteractiveTestsModel::closeWidgetDialog()
{
    LOGI() << "cpp: before close ";
    interactive()->close(Uri("muse://devtools/interactive/testdialog"));
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
    IInteractive::Result result = interactive()->questionSync(
        "Do you really want to delete the 'xxx' workspace?", "",
        { interactive()->buttonData(IInteractive::Button::No),
          interactive()->buttonData(IInteractive::Button::Yes) }, 0,
        IInteractive::Option::WithIcon);

    if (result.standardButton() == IInteractive::Button::Yes) {
        LOGI() << "Yes!!";
    } else {
        LOGI() << "No!!";
    }
}

void InteractiveTestsModel::questionByUri()
{
    auto promise = interactive()->question(
        "Do you really want to delete the 'xxx' workspace?", "",
        { IInteractive::Button::No, IInteractive::Button::Yes }, IInteractive::Button::NoButton,
        IInteractive::Option::WithIcon);

    promise.onResolve(this, [](const IInteractive::Result& res) {
        if (res.standardButton() == IInteractive::Button::Yes) {
            LOGI() << "Yes!!";
        } else {
            LOGI() << "No!!";
        }
    }).onReject(this, [](int code, const std::string& err) {
        LOGE() << code << err;
    });
}

void InteractiveTestsModel::customQuestion()
{
    int maybeBtn = int(IInteractive::Button::CustomButton) + 1;
    IInteractive::Result result = interactive()->questionSync("Test", "It works?", {
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
    auto result = interactive()->info("Tuplet cannot cross barlines",
                                      std::string(), {}, 0,
                                      IInteractive::Option::WithIcon
                                      | IInteractive::Option::WithDontShowAgainCheckBox)
                  .onResolve(this, [this](const IInteractive::Result& res) {
        LOGD() << interactive()->buttonData(res.standardButton()).text;
    });
}

void InteractiveTestsModel::warning()
{
    IInteractive::Result result = interactive()->warningSync("Do you want to save changes to the score “Untitled” before closing?",
                                                             "Your changes will be lost if you don’t save them.",
                                                             { interactive()->buttonData(IInteractive::Button::DontSave),
                                                               interactive()->buttonData(IInteractive::Button::Save),
                                                               interactive()->buttonData(IInteractive::Button::Cancel) });

    if (result.standardButton() == IInteractive::Button::DontSave) {
        LOGI() << "Don’t save!!";
    } else if (result.standardButton() == IInteractive::Button::Save) {
        LOGE() << "Save!!";
    } else {
        LOGE() << "Cancel!!";
    }
}

void InteractiveTestsModel::critical()
{
    IInteractive::Result result = interactive()->errorSync("Cannot read file C:/Users/Username/Desktop/Composition.mscz",
                                                           std::string("An error has occurred when trying to open this file"));
    LOGD() << interactive()->buttonData(result.standardButton()).text;
}

void InteractiveTestsModel::criticalWithDetailedText()
{
    IInteractive::Text text;
    text.text = std::string("An error has occurred when trying to open this file");
    text.detailedText = std::string("Permission denied");
    IInteractive::Result result = interactive()->errorSync("Cannot read file C:/Users/Username/Desktop/Composition.mscz", text);
    LOGD() << interactive()->buttonData(result.standardButton()).text;
}

void InteractiveTestsModel::require()
{
    RetVal<Val> rv = interactive()->openSync("muse://devtools/interactive/sample?title='Test'");
    if (rv.ret) {
        LOGI() << "received: " << rv.val.toString();
    } else if (check_ret(rv.ret, Ret::Code::Cancel)) {
        LOGI() << "was cancelled";
    } else {
        LOGE() << "some error: " << rv.ret.code();
    }
}
