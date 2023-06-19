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

#ifndef MU_BRAILLE_INOTATIONBRAILLE_H
#define MU_BRAILLE_INOTATIONBRAILLE_H

#include "types/retval.h"

#include "modularity/imoduleinterface.h"

namespace mu::braille {
class INotationBraille : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotationBraille)

public:
    virtual ~INotationBraille() = default;

    virtual ValCh<std::string> brailleInfo() const = 0;
    virtual ValCh<int> cursorPosition() const = 0;
    virtual ValCh<int> currentItemPositionStart() const = 0;
    virtual ValCh<int> currentItemPositionEnd() const = 0;
    virtual ValCh<std::string> shortcut() const = 0;
    virtual ValCh<bool> enabled() const = 0;

    virtual void setEnabled(bool enabled) = 0;

    virtual void setCursorPosition(const int pos) = 0;
    virtual void setCurrentItemPosition(const int, const int) = 0;
    virtual void setShortcut(const QString&) = 0;
};
}

#endif // MU_NOTATION_INOTATIONBRAILLE_H
