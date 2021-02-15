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
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_INSTRUMENTS_INSTRUMENTSREPOSITORY_H
#define MU_INSTRUMENTS_INSTRUMENTSREPOSITORY_H

#include <QMutex>

#include "modularity/ioc.h"

#include "retval.h"
#include "async/channel.h"
#include "async/asyncable.h"
#include "framework/system/ifilesystem.h"
#include "extensions/iextensionsservice.h"

#include "instrumentstypes.h"
#include "iinstrumentsrepository.h"
#include "iinstrumentsconfiguration.h"
#include "iinstrumentsreader.h"

namespace mu::instruments {
class InstrumentsRepository : public IInstrumentsRepository, public async::Asyncable
{
    INJECT(instruments, IInstrumentsConfiguration, configuration)
    INJECT(instruments, system::IFileSystem, fileSystem)
    INJECT(instruments, IInstrumentsReader, reader)
    INJECT(instruments, extensions::IExtensionsService, extensionsService)

public:
    void init();

    RetValCh<InstrumentsMeta> instrumentsMeta() override;

private:
    void load();
    void clear();

    Transposition transposition(const QString& instrumentTemplateId) const;

    QMutex m_instrumentsMutex;
    InstrumentsMeta m_instrumentsMeta;

    async::Channel<InstrumentsMeta> m_instrumentsMetaChannel;
};
}

#endif // MU_INSTRUMENTS_INSTRUMENTSREPOSITORY_H
