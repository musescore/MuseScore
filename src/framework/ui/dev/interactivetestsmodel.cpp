//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "interactivetestsmodel.h"
#include "log.h"

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
    IInteractive::Button btn = interactive()->question("Test", "It works?", {
        IInteractive::Button::Yes,
        IInteractive::Button::No });

    if (btn == IInteractive::Button::Yes) {
        LOGI() << "Yes!!";
    } else {
        LOGI() << "No!!";
    }
}

void InteractiveTestsModel::customQuestion()
{
    int maybeBtn = int(IInteractive::Button::CustomButton) + 1;
    int btn = interactive()->question("Test", "It works?",{
        IInteractive::ButtonData(maybeBtn, "Maybe"),
        interactive()->buttonData(IInteractive::Button::No)
    });

    if (btn == maybeBtn) {
        LOGI() << "Maybe!!";
    } else {
        LOGE() << "No!!";
    }
}

void InteractiveTestsModel::information()
{
    interactive()->message(IInteractive::Type::Info, "Test", "This is info text");
}

void InteractiveTestsModel::warning()
{
    interactive()->message(IInteractive::Type::Warning, "Test", "This is warning text");
}

void InteractiveTestsModel::critical()
{
    interactive()->message(IInteractive::Type::Critical, "Test", "This is critical text");
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
