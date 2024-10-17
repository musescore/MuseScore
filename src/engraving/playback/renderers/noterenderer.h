/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "playback/renderingcontext.h"

namespace mu::engraving {
class Note;
class NoteRenderer
{
public:
    static void render(const Note* note, const RenderingContext& ctx, muse::mpe::PlaybackEventList& result);

private:
    static void renderTiedNotes(const Note* firstNote, NominalNoteCtx& firstNoteCtx);
    static void addTiedNote(const NominalNoteCtx& tiedNoteCtx, NominalNoteCtx& firstNoteCtx);
    static void updateArticulationBoundaries(const muse::mpe::timestamp_t noteTimestamp, const muse::mpe::duration_t noteDuration,
                                             muse::mpe::ArticulationMap& articulations);
    static void applySwingIfNeed(const Note* note, NominalNoteCtx& noteCtx);

    static NominalNoteCtx buildNominalNoteCtx(const Note* note, const RenderingContext& ctx);
};
}
