//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_INSTRUMENTS_ISELECTINSTRUMENTSSCENARIO_H
#define MU_INSTRUMENTS_ISELECTINSTRUMENTSSCENARIO_H

#include "modularity/imoduleexport.h"
#include "instrumentstypes.h"
#include "retval.h"

namespace mu::instruments {
class ISelectInstrumentsScenario : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISelectInstrumentsScenario)

public:
    virtual ~ISelectInstrumentsScenario() = default;

    enum class SelectInstrumentsMode {
        None,
        ShowCurrentInstruments
    };

    virtual RetVal<InstrumentList> selectInstruments(SelectInstrumentsMode mode = SelectInstrumentsMode::None) const = 0;
    virtual RetVal<Instrument> selectInstrument(const std::string& currentInstrumentId = "") const = 0;
};
}

#endif // MU_INSTRUMENTS_ISELECTINSTRUMENTSSCENARIO_H
