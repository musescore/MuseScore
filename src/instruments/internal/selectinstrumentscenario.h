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
#ifndef MU_INSTRUMENTS_SELECTINSTRUMENTSSCENARIO_H
#define MU_INSTRUMENTS_SELECTINSTRUMENTSSCENARIO_H

#include "iselectinstrumentscenario.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "iinteractive.h"

namespace mu::instruments {
class SelectInstrumentsScenario : public ISelectInstrumentsScenario
{
    INJECT(instruments, context::IGlobalContext, globalContext)
    INJECT(instruments, framework::IInteractive, interactive)

public:
    RetVal<InstrumentList> selectInstruments(SelectInstrumentsMode mode = SelectInstrumentsMode::None) const override;
    RetVal<Instrument> selectInstrument(const std::string& currentInstrumentId = "") const override;

private:
    RetVal<InstrumentList> selectInstruments(const QStringList& params) const;

    notation::INotationPartsPtr notationParts() const;
    notation::IDList partsInstrumentIds() const;
};
}

#endif // MU_INSTRUMENTS_SELECTINSTRUMENTSSCENARIO_H
