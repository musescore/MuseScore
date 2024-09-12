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
#pragma once

#include <set>
#include <vector>

#include "engraving/types/types.h"

#include "event.h"
#include "compatmidirenderinternal.h"

#include "log.h"

namespace mu::engraving {
class Chord;
class Drumset;
class Glissando;
class Instrument;
class Measure;
class Note;
class NoteEventList;
class Score;
class Segment;
class Trill;

class CompatMidiRender
{
public:
    static void renderScore(Score* score, EventsHolder& events, const CompatMidiRendererInternal::Context& ctx, bool expandRepeats);
    static int getControllerForSnd(Score* score, int globalSndController);
    static void createPlayEvents(const Score* score, Measure const* start = nullptr, Measure const* end = nullptr,
                                 const CompatMidiRendererInternal::Context& context = CompatMidiRendererInternal::Context());

    static int tick(const CompatMidiRendererInternal::Context& ctx, int tick);
private:
    static void createPlayEvents(const Score* score, const CompatMidiRendererInternal::Context& context, Chord* chord,
                                 Chord* prevChord = nullptr, Chord* nextChord = nullptr);
    static void createGraceNotesPlayEvents(const Score* score, const Fraction& tick, Chord* chord, int& ontime, int& trailtime);
    static std::vector<NoteEventList> renderChord(const CompatMidiRendererInternal::Context& context, Chord* chord, Chord* prevChord,
                                                  int gateTime, int ontime, int trailtime);
    static void renderArpeggio(Chord* chord, std::vector<NoteEventList>& ell, int ontime);
    static void renderTremolo(Chord* chord, std::vector<NoteEventList>& ell, int& ontime, double tremoloPartOfChord = 1.0);
    static void renderChordArticulation(const CompatMidiRendererInternal::Context& context, Chord* chord, std::vector<NoteEventList>& ell,
                                        int& gateTime, double graceOnBeatProportion, bool tremoloBefore = false);
    static void updateGateTime(const Instrument* instr, int& gateTime, const String& articulationName,
                               const CompatMidiRendererInternal::Context& context);
    static void renderGlissando(NoteEventList* events, Note* notestart, double graceOnBeatProportion, bool tremoloBefore = false);
    static bool renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, int requestedTicksPerNote,
                                       const std::vector<int>& prefix, const std::vector<int>& body, bool repeatp, bool sustainp,
                                       const std::vector<int>& suffix, int fastestFreq = 64, int slowestFreq = 8, // 64 Hz and 8 Hz
                                       double graceOnBeatProportion = 0, bool tremoloBefore = false);
    static bool renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, SymId articulationType,
                                       OrnamentStyle ornamentStyle);
    static bool renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, TrillType trillType, OrnamentStyle ornamentStyle);
    static void createSlideInNotePlayEvents(Note* note, Chord* prevChord, NoteEventList* el);
    static void createSlideOutNotePlayEvents(Note* note, NoteEventList* el, int onTime, bool hasTremolo);

    // Helpers
    static Chord* getChordFromSegment(Segment* segment, track_idx_t track);
    static Trill* findFirstTrill(Chord* chord);
    static int adjustTrailtime(int trailtime, Chord* currentChord, Chord* nextChord);
    static bool noteIsGlissandoStart(Note* note);
    static int slideLengthChordDependent(Chord* chord);
    static std::set<size_t> getNotesIndexesToRender(Chord* chord);
    static Glissando* backGlissando(Note* note);
    static bool isGlissandoValid(Glissando* glissando);
    static const Drumset* getDrumset(const Chord* chord);
    static int totalTiedNoteTicks(Note* note);
};
}
