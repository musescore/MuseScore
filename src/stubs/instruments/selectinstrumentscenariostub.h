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
#ifndef MU_INSTRUMENTS_SELECTINSTRUMENTSSCENARIOSTUB_H
#define MU_INSTRUMENTS_SELECTINSTRUMENTSSCENARIOSTUB_H

#include "instruments/iselectinstrumentscenario.h"

namespace mu::instruments {
class SelectInstrumentsScenario : public ISelectInstrumentsScenario
{
public:
    RetVal<InstrumentList> selectInstruments(SelectInstrumentsMode mode = SelectInstrumentsMode::None) const override;
    RetVal<Instrument> selectInstrument(const std::string& currentInstrumentId = "") const override;
};
}

#endif // MU_INSTRUMENTS_SELECTINSTRUMENTSSCENARIOSTUB_H
