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
#ifndef MU_ENGRAVING_MESSAGEBOX_H
#define MU_ENGRAVING_MESSAGEBOX_H

#include <set>
#include <string>

#ifndef ENGRAVING_NO_INTERACTIVE
#include "modularity/ioc.h"
#include "iinteractive.h"
#endif

namespace mu::engraving {
class MessageBox
{
#ifndef ENGRAVING_NO_INTERACTIVE
    INJECT_STATIC(framework::IInteractive, interactive)
#endif
public:

    enum Button {
        Ok,
        Cancel
    };

    static Button warning(const std::string& title, const std::string& text, const std::set<Button>& buttons = { Ok, Cancel },
                          bool withIcon = true, bool withDontShowAgainCheckBox = false);
    static Button error(const std::string& title, const std::string& text, const std::set<Button>& buttons = { Ok }, bool withIcon = true,
                        bool withDontShowAgainCheckBox = false);
private:
    static Button openDialog(const std::string& type, const std::string& title, const std::string& text,
                             const std::set<Button>& buttons = { Ok, Cancel }, bool withIcon = true,
                             bool withDontShowAgainCheckBox = false);
};
}

#endif // MU_ENGRAVING_MESSAGEBOX_H
