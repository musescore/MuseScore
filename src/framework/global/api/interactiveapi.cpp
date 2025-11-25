/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "interactiveapi.h"

#include <QUrl>

#include "global/containers.h"

using namespace muse::api;

/** APIDOC
 * User interaction - informational messages, error messages, questions and other dialogs.
 * @namespace interactive
 */

InteractiveApi::InteractiveApi(IApiEngine* e)
    : ApiObject(e)
{
}

/** APIDOC
 * Show information message
 * @method
 * @param {String} title Title
 * @param {String} text Message
 */
void InteractiveApi::info(const QString& contentTitle, const QString& text)
{
    interactive()->infoSync(contentTitle.toStdString(), text.toStdString());
}

/** APIDOC
 * Show warning message
 * @method
 * @param {String} title Title
 * @param {String} text Message
 */
void InteractiveApi::warning(const QString& contentTitle, const QString& text)
{
    interactive()->warningSync(contentTitle.toStdString(), text.toStdString());
}

/** APIDOC
 * Show error message
 * @method
 * @param {String} title Title
 * @param {String} text Message
 */
void InteractiveApi::error(const QString& contentTitle, const QString& text)
{
    interactive()->errorSync(contentTitle.toStdString(), text.toStdString());
}

using Btn = muse::IInteractive::Button;
static std::map<muse::IInteractive::Button, QString> s_buttons = {
    { Btn::NoButton, "NoButton" },
    { Btn::Ok, "Ok" },
    { Btn::Continue, "Continue" },
    { Btn::RestoreDefaults, "RestoreDefaults" },
    { Btn::Reset, "Reset" },
    { Btn::Apply, "Apply" },
    { Btn::Help, "Help" },
    { Btn::Discard, "Discard" },
    { Btn::Cancel, "Cancel" },
    { Btn::Close, "Close" },
    { Btn::Ignore, "Ignore" },
    { Btn::Retry, "Retry" },
    { Btn::Abort, "Abort" },
    { Btn::NoToAll, "NoToAll" },
    { Btn::No, "No" },
    { Btn::YesToAll, "YesToAll" },
    { Btn::Yes, "Yes" },
    { Btn::Open, "Open" },
    { Btn::DontSave, "DontSave" },
    { Btn::SaveAll, "SaveAll" },
    { Btn::Save, "Save" },
    { Btn::Next, "Next" },
    { Btn::Back, "Back" },
    { Btn::Select, "Select" },
    { Btn::Clear, "Clear" },
    { Btn::Done, "Done" }
};

static muse::IInteractive::Button buttonFromString(const QString& str)
{
    return muse::key(s_buttons, str, Btn::NoButton);
}

static QString buttonToString(const muse::IInteractive::Button& btn)
{
    return muse::value(s_buttons, btn, "NoButton");
}

std::vector<muse::IInteractive::Button> InteractiveApi::buttons(const QJSValueList& btns) const
{
    std::vector<muse::IInteractive::Button> result;
    for (const QJSValue& btn : btns) {
        QString str = btn.toString();
        int num = btn.toInt();
        result.push_back(buttonFromString(str));
    }
    return result;
}

/** APIDOC
 * Ask a question
 * @method
 * @param {String} title Title
 * @param {String} text Message
 * @param {Button[]} buttons Buttons
 * @return {Button} - selected buttons
 */
QString InteractiveApi::question(const QString& contentTitle, const QString& text, const QJSValueList& btns)
{
    IInteractive::Result res = interactive()->questionSync(contentTitle.toStdString(),
                                                           text.toStdString(),
                                                           buttons(btns));
    return buttonToString(res.standardButton());
}

/** APIDOC
 * Open URL in external browser
 * @method
 * @param {String} url URL
 */
void InteractiveApi::openUrl(const QString& url)
{
    interactive()->openUrl(QUrl(url));
}
