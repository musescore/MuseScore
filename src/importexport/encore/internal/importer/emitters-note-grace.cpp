/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Emit grace-note chords and attach them to their principal chord.

#include "emitters-internal.h"
#include "mappers.h"
#include "../parser/ticks.h"
#include "durations.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"

namespace mu::iex::enc {
using namespace mu::engraving;

// Create a detached grace Chord for the EncNote and queue or attach it; returns true if handled
// as a grace (caller must return). Grace chords must be parented under a Chord, not a Segment, or
// pagePos crashes. See ENCORE_IMPORTER.md §Grace and cue notes.
bool tryHandleGraceNote(BuildCtx& ctx, MeasEmitCtx& mc, NoteElemCtx& ec,
                        const EncNote* en)
{
    if (!isValidFaceValue(en->faceValue)) {
        return false;
    }
    if (en->graceType() == EncGraceType::NORMAL && !en->isInnerGrace) {
        return false;
    }
    // An explicitly-flagged grace (acciaccatura/appoggiatura) may carry any written value: Encore
    // lets you turn a quarter or half note into a grace. Only the v0xA6 inferred-grace path
    // (isInnerGrace, graceType NORMAL) keeps the historical "shorter than a quarter" guard.
    if (en->graceType() == EncGraceType::NORMAL && (en->faceValue & 0x0F) < 4) {
        return false;
    }

    const auto trackKey = ec.trackKey;

    // Roll back per-track tick state so the next note is not detected as a chord extension of this grace.
    if (ec.savedPrevMidiTick >= 0) {
        ctx.scratch.prevMidiTick[trackKey] = ec.savedPrevMidiTick;
    } else {
        ctx.scratch.prevMidiTick.erase(trackKey);
    }
    if (ec.hadLastChordPos) {
        ctx.scratch.lastChordPos[trackKey] = ec.savedLastChordPos;
    } else {
        ctx.scratch.lastChordPos.erase(trackKey);
    }

    // Each grace chord member is a separate note at the same tick, but the grace path rolls
    // prevMidiTick back so isChordExt never fires for the second member. Merge it into this
    // track's last grace chord, else a 2-note grace chord splits into two single-note graces.
    {
        auto gcIt = ctx.scratch.lastGraceChord.find(trackKey);
        auto tkIt = ctx.scratch.lastGraceTick.find(trackKey);
        if (gcIt != ctx.scratch.lastGraceChord.end() && gcIt->second && !gcIt->second->notes().empty()
            && tkIt != ctx.scratch.lastGraceTick.end() && tkIt->second == static_cast<int>(en->tick)) {
            Note* member = Factory::createNote(gcIt->second);
            applyConcertPitch(member, en->semiTonePitch + ctx.staffPitchOffset[ec.staffIdx]);
            if (en->isMuted()) {
                member->setPlay(false);
            }
            gcIt->second->add(member);
            return true;
        }
    }

    const bool appoggiatura = (en->graceType() == EncGraceType::APPOGGIATURA);
    const bool beamedGroup = (en->grace1 & 0x10);

    // Classify the grace against the principal notes of its own voice/measure:
    //  - principalAtOrAfter: a principal note at or after the grace -> grace-before, ornaments it.
    //  - contiguousNoteBefore: a principal note whose written span reaches the grace tick with no
    //    silence between -> grace-after, belongs to that preceding note.
    // A grace preceded by silence with nothing at/after it is neither: it falls through to the
    // grace-before path and, via the cross-barline pending carry, ornaments the next bar's downbeat.
    bool principalAtOrAfter = false;
    bool contiguousNoteBefore = false;
    if (mc.encMeas) {
        const int graceTick = static_cast<int>(en->tick);
        for (const auto& elp : mc.encMeas->elements) {
            if (static_cast<EncElemType>(elp->type) != EncElemType::NOTE
                || elp->staffIdx != ec.staffIdx || elp->voice != ec.voice) {
                continue;
            }
            const EncNote* n = static_cast<const EncNote*>(elp.get());
            if (n->graceType() != EncGraceType::NORMAL) {
                continue;   // only principal notes bound the decision
            }
            if (static_cast<int>(n->tick) >= graceTick) {
                principalAtOrAfter = true;
            } else if (static_cast<int>(n->tick) + faceValue2ticks(n->faceValue & 0x0F) >= graceTick) {
                contiguousNoteBefore = true;
            }
        }
    }

    // A no-slash small note (appoggiatura) with no principal note to ornament is a cue note, not a
    // grace: hand it back to the normal path, which keeps its full value and draws it small. Only an
    // appoggiatura adjacent to a principal is a real grace; acciaccaturas (slash) are always graces.
    if (appoggiatura && !principalAtOrAfter && !contiguousNoteBefore) {
        return false;
    }

    // grace-after only when a contiguous principal note precedes and nothing sits at/after the grace.
    Chord* precedingChord = nullptr;
    if (!appoggiatura && !ec.isChordExt && !principalAtOrAfter && contiguousNoteBefore) {
        for (Segment* s = mc.measure->last(SegmentType::ChordRest); s; s = s->prev(SegmentType::ChordRest)) {
            if (s->tick() >= ec.elemTick) {
                continue;
            }
            EngravingItem* el = s->element(ec.track);
            if (el && el->isChord() && !toChord(el)->isGrace()) {
                precedingChord = toChord(el);
                break;
            }
        }
    }
    const bool afterMode = (precedingChord != nullptr);

    // Grace figure: appoggiatura keeps its written type; a lone acciaccatura is the slashed eighth;
    // a beamed group keeps its written figure unslashed (Encore does not slash beamed groups).
    // Grace-after uses the *_AFTER variants.
    NoteType graceNoteType;
    if (appoggiatura) {
        graceNoteType = NoteType::APPOGGIATURA;
    } else if (afterMode) {
        switch (en->faceValue & 0x0F) {
        case 6:  graceNoteType = NoteType::GRACE32_AFTER;
            break;
        case 5:  graceNoteType = NoteType::GRACE16_AFTER;
            break;
        default: graceNoteType = beamedGroup ? NoteType::GRACE16_AFTER : NoteType::GRACE8_AFTER;
            break;
        }
    } else if (beamedGroup) {
        switch (en->faceValue & 0x0F) {
        case 3:  graceNoteType = NoteType::GRACE4;
            break;
        case 6:  graceNoteType = NoteType::GRACE32;
            break;
        default: graceNoteType = NoteType::GRACE16;
            break;
        }
    } else {
        graceNoteType = NoteType::ACCIACCATURA;
    }

    // A lone slashed acciaccatura is drawn as an eighth regardless of stored value, else a small
    // stored value (e.g. a 128th) renders as a many-flagged glyph.
    const bool eighthGlyph = (graceNoteType == NoteType::ACCIACCATURA
                              || graceNoteType == NoteType::GRACE8_AFTER);
    DurationType graceDt = eighthGlyph ? DurationType::V_EIGHTH
                           : realDuration2DurationType(en->realDuration, en->faceValue);
    Chord* gc = Factory::createChord(ctx.score->dummy()->segment());
    gc->setTrack(ec.track);
    TDuration gdur(graceDt);
    gc->setDurationType(gdur);
    gc->setTicks(gdur.fraction());
    gc->setDots(0);
    gc->setNoteType(graceNoteType);

    Note* gnote = Factory::createNote(gc);
    applyConcertPitch(gnote, en->semiTonePitch + ctx.staffPitchOffset[ec.staffIdx]);
    if (en->isMuted()) {
        gnote->setPlay(false);   // Encore per-note mute flag
    }
    gc->add(gnote);

    ctx.scratch.lastGraceChord[trackKey] = gc;
    ctx.scratch.lastGraceTick[trackKey] = static_cast<int>(en->tick);

    // Articulations are applied after attachment: a detached grace chord only sees the dummy
    // segment, where fermatas cannot anchor.

    // Grace-after: attach immediately to the preceding principal chord (stays in this bar).
    if (afterMode) {
        gc->setGraceIndex(precedingChord->graceNotes().size());
        precedingChord->add(gc);
        applyNoteArticulations(ctx, gnote, gc, en, ec.track, mc);
        return true;
    }

    // Retroactive attachment: main note already placed at elemTick (isChordExt=TRUE).
    if (ec.isChordExt) {
        Segment* existingSeg = mc.measure->getSegment(SegmentType::ChordRest, ec.elemTick);
        if (existingSeg) {
            EngravingItem* existingEl = existingSeg->element(ec.track);
            if (existingEl && existingEl->isChord()) {
                gc->setGraceIndex(0);
                toChord(existingEl)->add(gc);
                applyNoteArticulations(ctx, gnote, gc, en, ec.track, mc);
                ctx.scratch.graceStolenTicks[trackKey] += faceValue2ticks(en->faceValue & 0x0F);
                return true;
            }
        }
    }

    // Grace-before: queue for the next principal chord.
    ctx.scratch.pendingGraces[trackKey].push_back({ gc, en, mc.measure });
    ctx.scratch.graceStolenTicks[trackKey] += faceValue2ticks(en->faceValue & 0x0F);
    return true;
}
} // namespace mu::iex::enc
