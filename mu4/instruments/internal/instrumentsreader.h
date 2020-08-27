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

namespace mu {
namespace instruments {
class InstrumentsReader : public IInstrumentsReader
{
    INJECT(instruments, framework::IFileSystem, fileSystem)

public:
    RetVal<InstrumentsMeta> readMeta(const io::path& path) const override;

private:
    struct GroupMeta
    {
        InstrumentGroup group;
        InstrumentTemplateHash templates;
    };

    GroupMeta readGroupMeta(Ms::XmlReader& reader, InstrumentsMeta& generalMeta) const;
    MidiArticulation readArticulation(Ms::XmlReader& reader) const;
    InstrumentGenre readGenre(Ms::XmlReader& reader) const;
    InstrumentTemplate readInstrumentTemplate(Ms::XmlReader& reader, InstrumentsMeta& generalMeta) const;

    int readStaffIndex(Ms::XmlReader& reader) const;
    PitchRange readPitchRange(Ms::XmlReader& reader) const;
    MidiAction readMidiAction(Ms::XmlReader& reader) const;
    StringData readStringData(Ms::XmlReader& reader) const;

    void fillByDeffault(Instrument &instrument) const;
};
}
}

#endif // MU_INSTRUMENTS_INSTRUMENTSREADER_H
