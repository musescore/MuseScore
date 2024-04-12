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

#ifndef MU_BRAILLE_INOTATIONBRAILLE_H
#define MU_BRAILLE_INOTATIONBRAILLE_H

#include "modularity/imoduleinterface.h"
#include "types/retval.h"

#include "brailletypes.h"

namespace mu::braille {
class INotationBraille : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotationBraille)

public:
    virtual ~INotationBraille() = default;

    virtual muse::ValCh<std::string> brailleInfo() const = 0;
    virtual muse::ValCh<int> cursorPosition() const = 0;
    virtual muse::ValCh<int> currentItemPositionStart() const = 0;
    virtual muse::ValCh<int> currentItemPositionEnd() const = 0;
    virtual muse::ValCh<std::string> keys() const = 0;
    virtual muse::ValCh<bool> enabled() const = 0;
    virtual muse::ValCh<BrailleIntervalDirection> intervalDirection() const = 0;
    virtual muse::ValCh<int> mode() const = 0;
    virtual muse::ValCh<std::string> cursorColor() const = 0;

    virtual void setEnabled(const bool enabled) = 0;
    virtual void setIntervalDirection(const BrailleIntervalDirection direction) = 0;

    virtual void setCursorPosition(const int pos) = 0;
    virtual void setCurrentItemPosition(const int, const int) = 0;
    virtual void setKeys(const QString&) = 0;

    virtual void setMode(const BrailleMode) = 0;
    virtual void toggleMode() = 0;
    virtual bool isNavigationMode() = 0;
    virtual bool isBrailleInputMode() = 0;

    virtual void setCursorColor(const QString) = 0;
};
}

#endif // MU_NOTATION_INOTATIONBRAILLE_H
