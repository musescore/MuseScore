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
#ifndef MU_IMPORTEXPORT_MIDICONFIGURATION_H
#define MU_IMPORTEXPORT_MIDICONFIGURATION_H

#include "async/asyncable.h"
#include "io/path.h"
#include "../imidiconfiguration.h"

namespace mu::iex::midi {
class MidiConfiguration : public IMidiImportExportConfiguration, public muse::async::Asyncable
{
public:
    void init();

    // import
    int midiShortestNote() const override; // ticks
    void setMidiShortestNote(int ticks) override;
    muse::async::Channel<int> midiShortestNoteChanged() const override;

    bool midiChannel9isDrum() const override;
    void setMidiChannel9isDrum(bool isDrum) override;
    muse::async::Channel<bool> midiChannel9isDrumChanged() const override;

    void setMidiImportOperationsFile(const std::optional<muse::io::path_t>& filePath) const override;

    // export
    bool isExpandRepeats() const override;
    void setExpandRepeats(bool expand) override;

    bool isMidiExportRpns() const override;
    void setIsMidiExportRpns(bool exportRpns) override;

private:
    muse::async::Channel<int> m_midiShortestNoteChanged;
    muse::async::Channel<bool> m_midiChannel9isDrumChanged;
};
}

#endif // MU_IMPORTEXPORT_MIDICONFIGURATION_H
