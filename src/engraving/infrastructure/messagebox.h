/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "modularity/ioc.h"

#ifndef ENGRAVING_NO_INTERACTIVE
#include "iinteractive.h"
#endif

namespace mu::engraving {
class MessageBox : public muse::Injectable
{
#ifndef ENGRAVING_NO_INTERACTIVE
    muse::Inject<muse::IInteractive> interactive = { this };
#endif
public:

    MessageBox(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    enum Button {
        Ok,
        Cancel
    };

    Button warning(const std::string& title, const std::string& text, const std::set<Button>& buttons = { Ok, Cancel });
};
}

#endif // MU_ENGRAVING_MESSAGEBOX_H
