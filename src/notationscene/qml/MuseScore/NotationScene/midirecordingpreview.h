/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#pragma once

#include <optional>

#include "draw/painter.h"
#include "draw/types/geometry.h"

#include "engraving/types/types.h"
#include "playback/iplaybackcontroller.h"

namespace mu::engraving {
class Score;
}

namespace mu::notation {
//! Draws the notes played while recording from a MIDI keyboard, like HyperScribe:
//! only a notehead, in a different color, at the pitch played and at the position where it was played.
//! It is only drawn on the view: the notes are not part of the score
class MidiRecordingPreview
{
public:
    static void paint(muse::draw::Painter* painter, const engraving::Score* score, const playback::MidiRecordingPreviewNoteList& notes);

    //! The staff line of a written pitch (0 = top line, 1 = top space, ... as Note::line())
    static int staffLine(int writtenPitch, engraving::Key key, engraving::ClefType clef);

    //! The pitch displayed on the staff for a sounding pitch (transposing instruments, concert pitch off)
    static int writtenPitch(const engraving::Score* score, engraving::staff_idx_t staffIdx, int soundingPitch,
                            const engraving::Fraction& tick);

private:
    static std::optional<double> xAtTick(const engraving::Score* score, const engraving::Fraction& tick);
};
}
