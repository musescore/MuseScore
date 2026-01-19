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

#pragma once

#include "api/apiobject.h"

#include "modularity/ioc.h"
#include "interactive/iinteractive.h"

namespace muse::interactive {
class InteractiveApi : public api::ApiObject
{
    Q_OBJECT

    ContextInject<IInteractive> interactive = { this };

public:
    explicit InteractiveApi(api::IApiEngine* e);

    /** APIDOC
     * Question buttons
     * @global
     * @enum
     */
    enum ButtonCode {
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
    Q_ENUM(ButtonCode);

    Q_INVOKABLE void info(const QString& contentTitle, const QString& text);
    Q_INVOKABLE void warning(const QString& contentTitle, const QString& text);
    Q_INVOKABLE void error(const QString& contentTitle, const QString& text);

    Q_INVOKABLE QString question(const QString& contentTitle, const QString& text, const QJSValueList& buttons);

    Q_INVOKABLE void openUrl(const QString& url);

private:
    std::vector<muse::IInteractive::Button> buttons(const QJSValueList& buttons) const;
};
}
