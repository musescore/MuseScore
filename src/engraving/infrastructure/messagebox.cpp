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
#include "messagebox.h"

#include "log.h"

using namespace mu::engraving;

MessageBox::Button MessageBox::warning(const std::string& title, const std::string& text, const std::set<Button>& buttons,
                                       bool withIcon, bool withDontShowAgainCheckBox)
{
    return MessageBox::openDialog("WARNING", title, text, buttons, withIcon, withDontShowAgainCheckBox);
}

MessageBox::Button MessageBox::error(const std::string& title, const std::string& text, const std::set<Button>& buttons,
                                     bool withIcon, bool withDontShowAgainCheckBox)
{
    return MessageBox::openDialog("ERROR", title, text, buttons, withIcon, withDontShowAgainCheckBox);
}

MessageBox::Button MessageBox::openDialog(const std::string& type, const std::string& title, const std::string& text,
                                          const std::set<Button>& buttons, bool withIcon, bool withDontShowAgainCheckBox)
{
#ifndef ENGRAVING_NO_INTERACTIVE
    using namespace mu::framework;

    std::vector<IInteractive::Button> realButtons;
    if (buttons.find(Cancel) != buttons.cend()) {
        realButtons.push_back(IInteractive::Button::Cancel);
    }
    if (buttons.find(Ok) != buttons.cend() || realButtons.empty()) {
        realButtons.push_back(IInteractive::Button::Ok);
    }

    IInteractive::Options options = {};
    if (withIcon) {
        options |= IInteractive::Option::WithIcon;
    }

    if (withDontShowAgainCheckBox) {
        options |= IInteractive::Option::WithDontShowAgainCheckBox;
    }

    IInteractive::Result res;
    if (type == "WARNING") {
        res = interactive()->warning(title, text, realButtons, IInteractive::Button::Ok, options);
    } else if (type == "ERROR") {
        res = interactive()->error(title, text, realButtons, IInteractive::Button::Ok, options);
    } else {
        res = int(IInteractive::Button::Cancel);
    }

    if (res.standardButton() == IInteractive::Button::Ok) {
        return MessageBox::Button::Ok;
    }

    return MessageBox::Button::Cancel;

#else
    UNUSED(buttons);
    LOGW() << "interactive disabled, will be return Ok, message: " << title << " " << text;
    return MessageBox::Button::Ok;
#endif
}
