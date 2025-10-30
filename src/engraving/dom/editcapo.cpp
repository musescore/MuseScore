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

#include "editcapo.h"
#include "staff.h"
#include "note.h"
#include "stringdata.h"
#include "score.h"
#include "part.h"

namespace mu::engraving {
// static
void EditCapo::applyCapoTranspose(int startTick, int endTick, UpdateCtx& ctx)
{
    staff_idx_t staffIdx = ctx.staff->idx();
    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack = startTrack + VOICES;

    for (MeasureBase* mb = ctx.staff->score()->measures()->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& seg: toMeasure(mb)->segments()) {
            if (!seg.isChordRestType()) {
                continue;
            }
            for (track_idx_t track = startTrack; track < endTrack; ++track) {
                EngravingItem* e = seg.element(track);
                if (!e || !e->isChord()) {
                    continue;
                }
                if (e->tick().ticks() < startTick || (-1 != endTick && e->tick().ticks() >= endTick)) {
                    continue;
                }

                Chord* chord = toChord(e);
                chord->sortNotes();
                ctx.stringData = chord->part()->stringData(chord->tick(), staffIdx);
                // Prefer not change strings for intervals and chords
                ctx.possibleFretConflict
                    = chord->notes().size() > 1 || std::any_of(chord->notes().begin(), chord->notes().end(),
                                                               [&](const Note* n) {
                    return n->fret() < ctx.params.fretPosition;
                });

                for (auto note: chord->notes()) {
                    if (ctx.updateIgnoredStrings) {
                        updateString(note, ctx);
                    } else {
                        update(note, ctx);
                    }
                    if (const GuitarBend* bend = note->bendFor(); bend) {
                        if (GuitarBendType::BEND == bend->type()) {
                            Note* startNote = bend->startNote();
                            Note* endNote = bend->endNote();
                            const StringData* stringData = ctx.stringData;
                            const int startString = startNote->string();
                            const int startFret = stringData->fret(startNote->pitch(), startString, startNote->staff(), startNote->tick());
                            if (startFret != startNote->fret()) {
                                startNote->setFret(startFret);
                            }

                            if (endNote->fret() != startFret) {
                                endNote->setString(startString);
                                endNote->setFret(startFret);
                            }
                            GuitarBend::fixNotesFrettingForStandardBend(bend->startNote(), bend->endNote());
                        }
                    }
                }

                for (Chord* g: chord->graceNotes()) {
                    for (Note* n: g->notes()) {
                        if (ctx.updateIgnoredStrings) {
                            updateString(n, ctx);
                        } else {
                            update(n, ctx);
                        }
                    }
                }
            }
        }
    }
}

// static
void EditCapo::updateNotationForCapoChange(const CapoParams& oldParams, const CapoParams& newParams, const Staff* staff, int startTick,
                                           int endTick)
{
    UpdateCtx ctx;
    ctx.staff = staff;
    ctx.params = newParams;
    // These two have to be set in applyCapoTranspose for every chord!
    // ctx.stringData = nullptr;
    // ctx.possibleFretConflict = false;

    const bool modeChanged = oldParams.transposeMode != newParams.transposeMode;
    const bool fretChanged = oldParams.fretPosition != newParams.fretPosition;
    const bool stringsChanged = oldParams.ignoredStrings != newParams.ignoredStrings;
    const bool activeChanged = oldParams.active != newParams.active;

    if (modeChanged) {
        handleModeChange(oldParams, newParams, startTick, endTick, ctx);
        return;
    }
    if (fretChanged) {
        handleFretChange(oldParams, newParams, startTick, endTick, ctx);
        return;
    }
    if (stringsChanged) {
        handleStringChanged(newParams, startTick, endTick, ctx);
        return;
    }
    if (activeChanged) {
        handleActiveChanged(newParams, startTick, endTick, ctx);
        return;
    }
}

// static
void EditCapo::update(Note* note, const UpdateCtx& ctx)
{
    if (muse::contains(ctx.params.ignoredStrings, (string_idx_t)note->string())) {
        note->setPitch(ctx.stringData->getPitch(note->string(), note->fret(), ctx.staff));
    } else {
        const int pitch = note->pitch() + ctx.noteOffset;
        note->setPitch(pitch);
    }
    note->setTpcFromPitch();
}

// static
void EditCapo::updateString(Note* note, const UpdateCtx& ctx)
{
    const int pitch = note->pitch();
    const int string = note->string();
    const int fret = note->fret();
    if (CapoParams::TransposeMode::PLAYBACK_ONLY != ctx.params.transposeMode) {
        if (muse::contains(ctx.params.ignoredStrings, (string_idx_t)note->string())) {
            // Already handled
            if (pitch == ctx.stringData->getPitch(string, fret, 0)) {
                return;
            }
            if (CapoParams::TransposeMode::STANDARD_ONLY == ctx.params.transposeMode) {
                note->setPitch(ctx.stringData->getPitch(note->string(), note->fret(), ctx.staff));
            }
            note->setTpcFromPitch();
            return;
        }
        if (pitch != ctx.stringData->getPitch(string, fret, -ctx.params.fretPosition)) {
            if (CapoParams::TransposeMode::STANDARD_ONLY == ctx.params.transposeMode) {
                note->setPitch(ctx.stringData->getPitch(note->string(), note->fret(), -ctx.params.fretPosition));
            }
        }
    }
    note->setTpcFromPitch();
}

// static
void EditCapo::handleModeChange(const CapoParams& oldParams, const CapoParams& newParams, int startTick, int endTick, UpdateCtx& ctx)
{
    switch (oldParams.transposeMode) {
    case CapoParams::TransposeMode::PLAYBACK_ONLY:
        if (newParams.transposeMode == CapoParams::TransposeMode::STANDARD_ONLY) {
            ctx.noteOffset = newParams.fretPosition;
        }
        break;
    case CapoParams::TransposeMode::STANDARD_ONLY:
        if (newParams.transposeMode == CapoParams::TransposeMode::PLAYBACK_ONLY) {
            ctx.noteOffset = -oldParams.fretPosition;
        } else if (newParams.transposeMode == CapoParams::TransposeMode::TAB_ONLY) {
            ctx.noteOffset = -oldParams.fretPosition;
        }
        break;
    case CapoParams::TransposeMode::TAB_ONLY:
        if (newParams.transposeMode == CapoParams::TransposeMode::STANDARD_ONLY) {
            ctx.noteOffset = newParams.fretPosition;
        }
        break;
    }
    applyCapoTranspose(startTick, endTick, ctx);
}

// static
void EditCapo::handleFretChange(const CapoParams& oldParams, const CapoParams& newParams, int startTick, int endTick, UpdateCtx& ctx)
{
    if (CapoParams::TransposeMode::STANDARD_ONLY == newParams.transposeMode) {
        ctx.noteOffset = newParams.fretPosition - oldParams.fretPosition;
        applyCapoTranspose(startTick, endTick, ctx);
    }
}

// static
void EditCapo::handleStringChanged(const CapoParams& newParams, int startTick, int endTick, UpdateCtx& ctx)
{
    ctx.updateIgnoredStrings = true;

    switch (newParams.transposeMode) {
    case CapoParams::TransposeMode::STANDARD_ONLY:
        ctx.noteOffset = newParams.fretPosition;
        applyCapoTranspose(startTick, endTick, ctx);
        break;
    case CapoParams::TransposeMode::TAB_ONLY:
    case CapoParams::TransposeMode::PLAYBACK_ONLY:
        break;
    }
}

// static
void EditCapo::handleActiveChanged(const CapoParams& newParams, int startTick, int endTick, UpdateCtx& ctx)
{
    if (!newParams.active) {
        // Treat as whatever mode to playback-only transition
        switch (newParams.transposeMode) {
        case CapoParams::TransposeMode::STANDARD_ONLY:
            ctx.noteOffset = -newParams.fretPosition;
            break;
        case CapoParams::TransposeMode::TAB_ONLY:
        case CapoParams::TransposeMode::PLAYBACK_ONLY:
            break;
        }
    } else {
        // Otherwise, as playback-only mode to previous one
        switch (newParams.transposeMode) {
        case CapoParams::TransposeMode::STANDARD_ONLY:
            ctx.noteOffset = newParams.fretPosition;
            break;
        case CapoParams::TransposeMode::TAB_ONLY:
        case CapoParams::TransposeMode::PLAYBACK_ONLY:
            break;
        }
    }
    applyCapoTranspose(startTick, endTick, ctx);
}
} // namespace mu::engraving
