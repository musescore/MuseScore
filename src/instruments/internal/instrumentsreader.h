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
#ifndef MU_INSTRUMENTS_INSTRUMENTSREADER_H
#define MU_INSTRUMENTS_INSTRUMENTSREADER_H

#include "modularity/ioc.h"
#include "retval.h"
#include "framework/system/ifilesystem.h"

#include "instrumentstypes.h"
#include "iinstrumentsreader.h"

namespace Ms {
class XmlReader;
}

namespace mu::instruments {
class InstrumentsReader : public IInstrumentsReader
{
    INJECT(instruments, system::IFileSystem, fileSystem)

public:
    RetVal<InstrumentsMeta> readMeta(const io::path& path) const override;

private:
    struct GroupMeta
    {
        InstrumentGroup group;
        InstrumentTemplateMap templates;
    };

    void loadGroupMeta(Ms::XmlReader& reader, InstrumentsMeta& generalMeta, int groupIndex) const;
    MidiArticulation readArticulation(Ms::XmlReader& reader) const;
    InstrumentGenre readGenre(Ms::XmlReader& reader) const;
    InstrumentTemplate readInstrumentTemplate(Ms::XmlReader& reader, InstrumentsMeta& generalMeta) const;

    int readStaffIndex(Ms::XmlReader& reader) const;
    PitchRange readPitchRange(Ms::XmlReader& reader) const;
    MidiAction readMidiAction(Ms::XmlReader& reader) const;
    StringData readStringData(Ms::XmlReader& reader) const;

    void fillByDeffault(Instrument& instrument) const;
    void initInstrument(Instrument& sourceInstrument, const Instrument& destinationInstrument) const;
};
}

#endif // MU_INSTRUMENTS_INSTRUMENTSREADER_H
