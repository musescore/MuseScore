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

MessageBox::Button MessageBox::warning(const std::string& title, const std::string& text)
{
#ifndef NO_ENGRAVING_INTERACTIVE
    using namespace mu::framework;
    IInteractive::Result res = interactive()->warning(title, text, { IInteractive::Button::Ok, IInteractive::Button::Cancel });
    if (res.standardButton() == IInteractive::Button::Ok) {
        return MessageBox::Button::Ok;
    }

    return MessageBox::Button::Cancel;

#else
    LOGW() << "interactive disabled, will be return Ok, message: " << title << " " << text;
    return MessageBox::Button::Ok;
#endif
}
