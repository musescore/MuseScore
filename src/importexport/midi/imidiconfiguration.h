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
#ifndef MU_IMPORTEXPORT_IMIDIIMPORTEXPORTCONFIGURATION_H
#define MU_IMPORTEXPORT_IMIDIIMPORTEXPORTCONFIGURATION_H

#include <optional>

#include "modularity/imoduleinterface.h"
#include "async/channel.h"
#include "io/path.h"

namespace mu::iex::midi {
class IMidiImportExportConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiImportExportConfiguration)

public:
    virtual ~IMidiImportExportConfiguration() = default;

    // import
    virtual int midiShortestNote() const = 0; //ticks
    virtual void setMidiShortestNote(int ticks) = 0;
    virtual muse::async::Channel<int> midiShortestNoteChanged() const = 0;

    virtual void setMidiImportOperationsFile(const std::optional<muse::io::path_t>& filePath) const = 0;

    // export
    virtual bool isExpandRepeats() const = 0;
    virtual void setExpandRepeats(bool expand) = 0;

    virtual bool isMidiExportRpns() const = 0;
    virtual void setIsMidiExportRpns(bool exportRpns) = 0;

    virtual bool isMidiSpaceLyrics() const = 0;
    virtual void setIsMidiSpaceLyrics(bool spaceLyrics) = 0;
};
}

#endif // MU_IMPORTEXPORT_IMIDIIMPORTEXPORTCONFIGURATION_H
