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

#include "writecontext.h"
#include "containers.h"

using namespace mu::engraving;
using namespace mu::engraving::write;

int WriteContext::assignLocalIndex(const Location& mainElementLocation)
{
    return m_linksIndexer.assignLocalIndex(mainElementLocation);
}

void WriteContext::setLidLocalIndex(int lid, int localIndex)
{
    m_lidLocalIndices.insert({ lid, localIndex });
}

int WriteContext::lidLocalIndex(int lid) const
{
    return muse::value(m_lidLocalIndices, lid, 0);
}

bool WriteContext::canWrite(const EngravingItem* e) const
{
    if (!_clipboardmode) {
        return true;
    }
    return _filter.canSelect(e);
}

bool WriteContext::canWriteNoteIdx(size_t noteIdx, size_t totalNotesInChord) const
{
    if (!_clipboardmode) {
        return true;
    }
    const Selection& sel = m_score->selection();
    return _filter.canSelectNoteIdx(noteIdx, totalNotesInChord, sel.rangeContainsMultiNoteChords());
}

bool WriteContext::canWriteVoice(track_idx_t track) const
{
    if (!_clipboardmode) {
        return true;
    }
    return _filter.canSelectVoice(track);
}
