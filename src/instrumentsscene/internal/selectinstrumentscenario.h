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
#ifndef MU_INSTRUMENTSSCENE_SELECTINSTRUMENTSSCENARIO_H
#define MU_INSTRUMENTSSCENE_SELECTINSTRUMENTSSCENARIO_H

#include "notation/iselectinstrumentscenario.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "iinteractive.h"

namespace mu::instrumentsscene {
class SelectInstrumentsScenario : public notation::ISelectInstrumentsScenario
{
    INJECT(instruments, context::IGlobalContext, globalContext)
    INJECT(instruments, framework::IInteractive, interactive)

public:
    RetVal<notation::PartInstrumentListScoreOrder> selectInstruments(SelectInstrumentsMode mode = SelectInstrumentsMode::None) const
    override;
    RetVal<notation::Instrument> selectInstrument(const std::string& currentInstrumentId = "") const override;

private:
    RetVal<notation::PartInstrumentListScoreOrder> selectInstruments(const QStringList& params) const;

    notation::INotationPartsPtr notationParts() const;
    notation::IDList partsIds() const;
    notation::ScoreOrder scoreOrder() const;
};
}

#endif // MU_INSTRUMENTSSCENE_SELECTINSTRUMENTSSCENARIO_H
