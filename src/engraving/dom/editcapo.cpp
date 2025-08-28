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
                        EditCapo::updateString(note, ctx);
                    } else {
                        EditCapo::update(note, ctx);
                    }
                }

                for (Chord* g: chord->graceNotes()) {
                    for (Note* n: g->notes()) {
                        if (ctx.updateIgnoredStrings) {
                            EditCapo::updateString(n, ctx);
                        } else {
                            EditCapo::update(n, ctx);
                        }
                    }
                }
            }
        }
    }
}

// static
void EditCapo::updateNotationForCapoChange(const CapoParams& oldParams, const CapoParams& newParams, const Staff* staff,
                                           int startTick, int endTick)
{
    UpdateCtx ctx;

    const bool modeChanged = oldParams.transposeMode != newParams.transposeMode;
    const bool fretChanged = oldParams.fretPosition != newParams.fretPosition;
    const bool stringsChanged = oldParams.ignoredStrings != newParams.ignoredStrings;
    const bool activeChanged = oldParams.active != newParams.active;

    if (modeChanged) {
        EditCapo::handleModeChange(oldParams, newParams, startTick, endTick, ctx);
        return;
    }
    if (fretChanged) {
        EditCapo::handleFretChange(oldParams, newParams, startTick, endTick, ctx);
        return;
    }
    if (stringsChanged) {
        EditCapo::handleStringChanged(newParams, startTick, endTick, ctx);
        return;
    }
    if (activeChanged) {
        EditCapo::handleActiveChanged(newParams, startTick, endTick, ctx);
        return;
    }
}

void EditCapo::update(Note* note, const UpdateCtx& ctx)
{
    if (muse::contains(ctx.params.ignoredStrings, (string_idx_t)note->string())) {
        note->setPitch(ctx.stringData->getPitch(note->string(), note->fret(), ctx.staff));
    } else {
        const int pitch = note->pitch() + ctx.noteOffset;
        note->setPitch(pitch);

        int string = note->string();
        int fret = note->fret();

        if (ctx.updateFrets) {
            if (ctx.possibleFretConflict) {
                if (CapoParams::TransposeMode::TAB_ONLY == ctx.params.transposeMode) {
                    if (fret + ctx.fretOffset >= 0) {
                        note->setFret(fret + ctx.fretOffset);
                    } else {
                        ctx.stringData->convertPitch(pitch, ctx.fretOffset, &string, &fret);
                        note->setString(string);
                        note->setFret(fret);
                    }
                } else if (CapoParams::TransposeMode::STANDARD_ONLY == ctx.params.transposeMode) {
                    ctx.stringData->convertPitch(pitch, ctx.fretOffset, &string, &fret);
                    note->setString(string);
                    note->setFret(fret);
                }
            } else {
                ctx.stringData->convertPitch(pitch, ctx.fretOffset, &string, &fret);
                note->setString(string);
                note->setFret(fret);
            }
        }
    }
    if (note->fret() < 0) {
        note->setFretConflict(true);
    }
    note->setTpcFromPitch();
}

void EditCapo::updateString(mu::engraving::Note* note, const mu::engraving::EditCapo::UpdateCtx& ctx)
{
    const int pitch = note->pitch();
    int string = note->string();
    int fret = note->fret();
    if (CapoParams::TransposeMode::PLAYBACK_ONLY != ctx.params.transposeMode) {
        if (muse::contains(ctx.params.ignoredStrings, (string_idx_t)note->string())) {
            // Already handled
            if (pitch == ctx.stringData->getPitch(string, fret, 0)) {
                return;
            }
            if (CapoParams::TransposeMode::STANDARD_ONLY == ctx.params.transposeMode) {
                note->setPitch(ctx.stringData->getPitch(note->string(), note->fret(), ctx.staff));
            } else {
                // The safest way
                // We're in the tab-only mode and fret now is real fret - capo fret.
                // If string is ignored, reverse that.
                note->setFret(fret + ctx.params.fretPosition);
            }
            note->setTpcFromPitch();
            return;
        }
        if (pitch != ctx.stringData->getPitch(string, fret, -ctx.params.fretPosition)) {
            if (CapoParams::TransposeMode::STANDARD_ONLY == ctx.params.transposeMode) {
                note->setPitch(ctx.stringData->getPitch(note->string(), note->fret(), -ctx.params.fretPosition));
            } else {
                ctx.stringData->convertPitch(pitch, -ctx.params.fretPosition, &string, &fret);
                note->setString(string);
                note->setFret(fret);
            }
        }
    }
    note->setTpcFromPitch();
}

void
EditCapo::handleModeChange(const CapoParams& oldParams, const CapoParams& newParams, int startTick, int endTick,
                           UpdateCtx& ctx)
{
    switch (oldParams.transposeMode) {
    case CapoParams::TransposeMode::PLAYBACK_ONLY:
        if (newParams.transposeMode == CapoParams::TransposeMode::STANDARD_ONLY) {
            ctx.noteOffset = newParams.fretPosition;
        } else if (newParams.transposeMode == CapoParams::TransposeMode::TAB_ONLY) {
            ctx.fretOffset = -newParams.fretPosition;
            ctx.updateFrets = true;
        }
        break;
    case CapoParams::TransposeMode::STANDARD_ONLY:
        if (newParams.transposeMode == CapoParams::TransposeMode::PLAYBACK_ONLY) {
            ctx.noteOffset = -oldParams.fretPosition;
        } else if (newParams.transposeMode == CapoParams::TransposeMode::TAB_ONLY) {
            ctx.noteOffset = -oldParams.fretPosition;
            ctx.fretOffset = -newParams.fretPosition;
            ctx.updateFrets = true;
        }
        break;
    case CapoParams::TransposeMode::TAB_ONLY:
        if (newParams.transposeMode == CapoParams::TransposeMode::PLAYBACK_ONLY) {
            ctx.updateFrets = true;
        } else if (newParams.transposeMode == CapoParams::TransposeMode::STANDARD_ONLY) {
            ctx.noteOffset = newParams.fretPosition;
            ctx.fretOffset = -newParams.fretPosition;
            ctx.updateFrets = true;
        }
        break;
    }
    EditCapo::applyCapoTranspose(startTick, endTick, ctx);
}

void
EditCapo::handleFretChange(const CapoParams& oldParams, const CapoParams& newParams, int startTick, int endTick,
                           EditCapo::UpdateCtx& ctx)
{
    if (CapoParams::TransposeMode::TAB_ONLY == newParams.transposeMode) {
        ctx.fretOffset = -newParams.fretPosition;
        ctx.updateFrets = true;
        EditCapo::applyCapoTranspose(startTick, endTick, ctx);
    } else if (CapoParams::TransposeMode::STANDARD_ONLY == newParams.transposeMode) {
        ctx.noteOffset = newParams.fretPosition - oldParams.fretPosition;
        EditCapo::applyCapoTranspose(startTick, endTick, ctx);
    }
}

void EditCapo::handleStringChanged(const CapoParams& newParams, int startTick, int endTick, EditCapo::UpdateCtx& ctx)
{
    ctx.updateIgnoredStrings = true;

    switch (newParams.transposeMode) {
    case CapoParams::TransposeMode::STANDARD_ONLY:
        ctx.noteOffset = newParams.fretPosition;
        break;
    case CapoParams::TransposeMode::TAB_ONLY:
        ctx.fretOffset = -newParams.fretPosition;
        ctx.updateFrets = true;
        break;
    case CapoParams::TransposeMode::PLAYBACK_ONLY:
        break;
    }
    EditCapo::applyCapoTranspose(startTick, endTick, ctx);
}

void EditCapo::handleActiveChanged(const CapoParams& newParams, int startTick, int endTick,
                                   UpdateCtx& ctx)
{
    if (!newParams.active) {
        // Treat as whatever mode to playback-only transition
        switch (newParams.transposeMode) {
        case CapoParams::TransposeMode::STANDARD_ONLY:
            ctx.noteOffset = -newParams.fretPosition;
            EditCapo::applyCapoTranspose(startTick, endTick, ctx);
            break;
        case CapoParams::TransposeMode::TAB_ONLY:
            ctx.updateFrets = true;
            EditCapo::applyCapoTranspose(startTick, endTick, ctx);
            break;
        case CapoParams::TransposeMode::PLAYBACK_ONLY:
            break;
        }
    } else {
        // Otherwise, as playback-only mode to previous one
        switch (newParams.transposeMode) {
        case CapoParams::TransposeMode::STANDARD_ONLY:
            ctx.noteOffset = newParams.fretPosition;
            EditCapo::applyCapoTranspose(startTick, endTick, ctx);
            break;
        case CapoParams::TransposeMode::TAB_ONLY:
            ctx.fretOffset = -newParams.fretPosition;
            ctx.updateFrets = true;
            EditCapo::applyCapoTranspose(startTick, endTick, ctx);
            break;
        case CapoParams::TransposeMode::PLAYBACK_ONLY:
            break;
        }
    }
}
} // namespace mu::engraving
