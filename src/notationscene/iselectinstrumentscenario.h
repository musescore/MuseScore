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
#ifndef MU_NOTATION_ISELECTINSTRUMENTSSCENARIO_H
#define MU_NOTATION_ISELECTINSTRUMENTSSCENARIO_H

#include "modularity/imoduleinterface.h"
#include "notation/notationtypes.h"
#include "global/async/promise.h"

namespace mu::notation {
class ISelectInstrumentsScenario : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISelectInstrumentsScenario)

public:
    virtual ~ISelectInstrumentsScenario() = default;

    virtual muse::async::Promise<PartInstrumentListScoreOrder> selectInstruments() const = 0;
    virtual muse::async::Promise<InstrumentTemplate> selectInstrument(
        const InstrumentKey& currentInstrumentKey = InstrumentKey()) const = 0;
};
}

#endif // MU_NOTATION_ISELECTINSTRUMENTSSCENARIO_H
