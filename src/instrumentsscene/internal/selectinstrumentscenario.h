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
#ifndef MU_INSTRUMENTSSCENE_SELECTINSTRUMENTSSCENARIO_H
#define MU_INSTRUMENTSSCENE_SELECTINSTRUMENTSSCENARIO_H

#include "notation/iselectinstrumentscenario.h"
#include "notation/iinstrumentsrepository.h"
#include "modularity/ioc.h"
#include "iinteractive.h"

namespace mu::instrumentsscene {
class SelectInstrumentsScenario : public notation::ISelectInstrumentsScenario
{
    INJECT(muse::IInteractive, interactive)
    INJECT(notation::IInstrumentsRepository, instrumentsRepository)

public:
    muse::RetVal<notation::PartInstrumentListScoreOrder> selectInstruments() const override;
    muse::RetVal<notation::InstrumentTemplate> selectInstrument(
        const notation::InstrumentKey& currentInstrumentId = notation::InstrumentKey()) const override;

private:
    muse::RetVal<notation::PartInstrumentListScoreOrder> selectInstruments(const muse::StringList& params) const;
};
}

#endif // MU_INSTRUMENTSSCENE_SELECTINSTRUMENTSSCENARIO_H
