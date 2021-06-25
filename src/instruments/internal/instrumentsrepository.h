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
#ifndef MU_INSTRUMENTS_INSTRUMENTSREPOSITORY_H
#define MU_INSTRUMENTS_INSTRUMENTSREPOSITORY_H

#include <QMutex>

#include "modularity/ioc.h"

#include "retval.h"
#include "async/channel.h"
#include "async/asyncable.h"
#include "extensions/iextensionsservice.h"

#include "instrumentstypes.h"
#include "iinstrumentsrepository.h"
#include "iinstrumentsconfiguration.h"

namespace mu::instruments {
class InstrumentsRepository : public IInstrumentsRepository, public async::Asyncable
{
    INJECT(instruments, IInstrumentsConfiguration, configuration)
    INJECT(instruments, extensions::IExtensionsService, extensionsService)

public:
    void init();

    RetValCh<InstrumentsMeta> instrumentsMeta() override;

private:
    void load();
    void clear();
    void fillInstrumentsMeta();

    InstrumentsMeta m_instrumentsMeta;
    QMutex m_instrumentsMutex;
    async::Channel<InstrumentsMeta> m_instrumentsMetaChannel;
};
}

#endif // MU_INSTRUMENTS_INSTRUMENTSREPOSITORY_H
