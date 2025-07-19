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

#include "global/modularity/ioc.h"
#include "global/iinteractive.h"
#include "notation/iinstrumentsrepository.h"

#include "global/async/asyncable.h"
#include "global/async/promise.h"

namespace mu::instrumentsscene {
class SelectInstrumentsScenario : public notation::ISelectInstrumentsScenario, public muse::async::Asyncable
{
    muse::Inject<muse::IInteractive> interactive;
    muse::Inject<notation::IInstrumentsRepository> instrumentsRepository;

public:
    muse::async::Promise<notation::PartInstrumentListScoreOrder> selectInstruments() const override;
    muse::async::Promise<notation::InstrumentTemplate> selectInstrument(
        const notation::InstrumentKey& currentInstrumentId = notation::InstrumentKey()) const override;

private:
    muse::async::Promise<notation::PartInstrumentListScoreOrder> selectInstruments(const muse::ValMap& params) const;
};
}

#endif // MU_INSTRUMENTSSCENE_SELECTINSTRUMENTSSCENARIO_H
