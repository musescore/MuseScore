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
#ifndef MUSE_API_INTERACTIVEAPI_H
#define MUSE_API_INTERACTIVEAPI_H

#include "api/apiobject.h"

#include "modularity/ioc.h"
#include "iinteractive.h"

namespace muse::api {
class InteractiveApi : public ApiObject
{
    Q_OBJECT

    Inject<IInteractive> interactive = { this };

public:
    explicit InteractiveApi(IApiEngine* e);

    /** APIDOC
     * Question buttons
     * @enum
     */
    enum Button {
        Ok = int(IInteractive::Button::Ok),
        Continue = int(IInteractive::Button::Continue),
        RestoreDefaults = int(IInteractive::Button::RestoreDefaults),
        Reset = int(IInteractive::Button::Reset),
        Apply = int(IInteractive::Button::Apply),
        Help = int(IInteractive::Button::Help),
        Discard = int(IInteractive::Button::Discard),
        Cancel = int(IInteractive::Button::Cancel),
        Close = int(IInteractive::Button::Close),
        Ignore = int(IInteractive::Button::Ignore),
        Retry = int(IInteractive::Button::Retry),
        Abort = int(IInteractive::Button::Abort),
        NoToAll = int(IInteractive::Button::NoToAll),
        No = int(IInteractive::Button::No),
        YesToAll = int(IInteractive::Button::YesToAll),
        Yes = int(IInteractive::Button::Yes),
        Open = int(IInteractive::Button::Open),
        DontSave = int(IInteractive::Button::DontSave),
        SaveAll = int(IInteractive::Button::SaveAll),
        Save = int(IInteractive::Button::Save),
        Next = int(IInteractive::Button::Next),
        Back = int(IInteractive::Button::Back),
        Select = int(IInteractive::Button::Select),
        Clear = int(IInteractive::Button::Clear),
        Done = int(IInteractive::Button::Done),
    };
    Q_ENUM(Button);

    Q_INVOKABLE void info(const QString& contentTitle, const QString& text);
    Q_INVOKABLE void warning(const QString& contentTitle, const QString& text);
    Q_INVOKABLE void error(const QString& contentTitle, const QString& text);

    Q_INVOKABLE QString question(const QString& contentTitle, const QString& text, const QJSValueList& buttons);

    Q_INVOKABLE void openUrl(const QString& url);

private:
    std::vector<muse::IInteractive::Button> buttons(const QJSValueList& buttons) const;
};
}

#endif // MUSE_API_INTERACTIVEAPI_H
