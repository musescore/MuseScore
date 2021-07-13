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
#ifndef MU_NOTATION_ISELECTINSTRUMENTSSCENARIO_H
#define MU_NOTATION_ISELECTINSTRUMENTSSCENARIO_H

#include "modularity/imoduleexport.h"
#include "notation/notationtypes.h"
#include "retval.h"

namespace mu::notation {
class ISelectInstrumentsScenario : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISelectInstrumentsScenario)

public:
    virtual ~ISelectInstrumentsScenario() = default;

    enum class SelectInstrumentsMode {
        None,
        ShowCurrentInstruments
    };

    virtual RetVal<notation::PartInstrumentListScoreOrder> selectInstruments(SelectInstrumentsMode mode = SelectInstrumentsMode::None) const
    = 0;
    virtual RetVal<notation::Instrument> selectInstrument(const std::string& currentInstrumentId = "") const = 0;
};
}

#endif // MU_NOTATION_ISELECTINSTRUMENTSSCENARIO_H
