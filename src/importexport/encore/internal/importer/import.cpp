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

// Top-level Encore (.enc) import: read the file, build the score, and run whole-score fix-up passes.
// Binary format reverse-engineered by Leon Vinken (Enc2MusicXML, GPL v3+) building on enc2ly by Felipe Castro.

#include "ctx.h"
#include "builders.h"
#include "resolvers.h"
#include "page-layout.h"
#include "debug-dump.h"

#include "import.h"

#include "../parser/elem.h"
#include "mappers.h"
#include "../parser/ticks.h"
#include "emitters-tuplets.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <map>
#include <set>
#include <vector>

#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/system.h"
#include "engraving/dom/volta.h"
#include "engraving/dom/mscore.h"
#include "engraving/types/spatium.h"
#include "engraving/engravingerrors.h"

#include "engraving/editing/editenharmonicspelling.h"
#include "engraving/editing/implodeexplode.h"
#include "engraving/editing/editvoice.h"
#include "engraving/editing/transaction/transaction.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::iex::enc {
// faceValue low nibble: 1=whole, 2=half ... 8=256th; 0 and 9..15 are invalid.
// High nibble carries unrelated flags.
bool isValidFaceValue(quint8 faceValue)
{
    const quint8 fv = faceValue & 0x0F;
    return fv > 0 && fv <= 8;
}

void applyConcertPitch(Note* n, int semitone)
{
    // A transposed or garbage Encore semitone can land outside MIDI's [0,127]. Note::setPitch
    // only asserts the range (no clamp), and downstream drumset lookups index a 128-entry table
    // by pitch, so an out-of-range value is undefined behaviour. Clamp once, here, at the single
    // choke point both the main and grace note paths go through.
    n->setPitch(std::clamp(semitone, 0, 127));
    n->setTpcFromPitch();
}

// score->spell() re-spells the whole score with a context heuristic that can spell transposed
// pitches with double-flats instead of the plain note the key wants. Re-derive the TPC of notes on
// transposing staves from pitch + concert key + transposition (pitch unchanged); leave others as is.
// TODO: format-agnostic, reads no Encore data; candidate to promote to a shared importexport util.
static void respellTransposingStaves(MasterScore* score)
{
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (track_idx_t t = 0; t < score->ntracks(); ++t) {
                EngravingItem* e = s->element(t);
                if (!e || !e->isChord()) {
                    continue;
                }
                Chord* chord = toChord(e);
                if (!chord->staff() || chord->staff()->transpose(chord->tick()).isZero()) {
                    continue;   // non-transposing staff: keep spell()'s spelling
                }
                for (Chord* gc : chord->graceNotes()) {
                    for (Note* n : gc->notes()) {
                        n->setTpcFromPitch();
                    }
                }
                for (Note* n : chord->notes()) {
                    n->setTpcFromPitch();
                }
            }
        }
    }
}

// Map Encore score-size (1 to 4) to MuseScore Staff Properties Scale (Pid::MAG): 1=60%, 2=75%,
// 3=100%, 4=130%. Global spatium is not changed.
static void applyStaffScale(MasterScore* score, const EncRoot& enc)
{
    static const double kScaleBySize[4] = { 0.60, 0.75, 1.00, 1.30 };
    staff_idx_t msStaffIdx = 0;
    for (size_t instrIdx = 0; instrIdx < enc.instruments.size(); ++instrIdx) {
        const int sz = staffDisplaySize(enc, static_cast<int>(instrIdx));
        const double scale = kScaleBySize[sz - 1];
        const int ns = enc.instruments[instrIdx].nstaves > 0 ? enc.instruments[instrIdx].nstaves : 1;
        for (int s = 0; s < ns && msStaffIdx < score->staves().size(); ++s, ++msStaffIdx) {
            score->staves()[msStaffIdx]->setProperty(Pid::MAG, PropertyValue(scale));
        }
    }
}

// Collapse a staff's voices back into voice 1 when they never sound at the same time (the
// engraving equivalent of "move to voice 1" + Tools > Implode). All-or-nothing per staff:
// a staff is collapsible only if every voice fits into voice 1 with no timing change (notes may
// merge into a chord only at identical onset+duration), so the music is never altered.
// TODO: format-agnostic, reads no Encore data; candidate to promote to a shared importexport util.
static void mergeNonOverlappingVoices(MasterScore* score)
{
    // Pass 1: find the staves that carry notes in more than voice 0 and whose voices
    // can be flattened without a timing conflict.
    std::vector<staff_idx_t> candidates;
    for (staff_idx_t si = 0; si < score->nstaves(); ++si) {
        const track_idx_t base = si * VOICES;
        bool hasUpperVoiceNotes = false;
        std::set<std::pair<int, int> > intervals;   // distinct (startTick, endTick) of chords
        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                for (voice_idx_t v = 0; v < VOICES; ++v) {
                    EngravingItem* e = s->element(base + v);
                    if (!e || !e->isChord()) {
                        continue;
                    }
                    if (v != 0) {
                        hasUpperVoiceNotes = true;
                    }
                    const Chord* c = toChord(e);
                    const int start = c->tick().ticks();
                    const int end   = (c->tick() + c->actualTicks()).ticks();
                    intervals.insert({ start, end });
                }
            }
        }
        if (!hasUpperVoiceNotes) {
            continue;   // already a single voice, nothing to do
        }
        // The distinct intervals (identical ones, i.e. chord candidates, are deduped
        // by the set) must not overlap. Sweep in start order: an interval that begins
        // before the furthest end seen so far overlaps a different one => conflict.
        bool collapsible = true;
        int maxEnd = -1;
        for (const std::pair<int, int>& iv : intervals) {   // std::set is ordered by (start, end)
            if (iv.first < maxEnd) {
                collapsible = false;
                break;
            }
            maxEnd = std::max(maxEnd, iv.second);
        }
        if (collapsible) {
            candidates.push_back(si);
        }
    }

    Measure* first = score->firstMeasure();
    Measure* last  = score->lastMeasure();
    if (!first || !last) {
        return;
    }

    // Pass 2: collapse each candidate staff. No undo transaction is opened, so the
    // editing commands below execute immediately and free themselves (see
    // UndoStack::pushAndPerform); the surrounding ScoreLoad keeps that path quiet.
    // (May be empty; the stale-rest cleanup below still runs.)
    for (staff_idx_t si : candidates) {
        const track_idx_t base = si * VOICES;

        // The voice change below rebuilds the destination chord from scratch; it carries
        // articulations, lyrics and slurs across but not a single-chord tremolo, so a
        // tremolo on a moved upper-voice chord would be lost. Snapshot every tremolo on
        // the staff (keyed by onset tick) and re-attach it after the collapse.
        std::map<int, TremoloType> tremolosByTick;
        for (Measure* m = first; m; m = m->nextMeasure()) {
            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                for (voice_idx_t v = 0; v < VOICES; ++v) {
                    EngravingItem* e = s->element(base + v);
                    if (e && e->isChord()) {
                        if (TremoloSingleChord* trem = toChord(e)->tremoloSingleChord()) {
                            tremolosByTick[s->tick().ticks()] = trem->tremoloType();
                        }
                    }
                }
            }
        }

        // Move every note on the staff into voice 1, filling its rests and merging
        // simultaneous same-duration notes into chords.
        score->deselectAll();
        score->select(first, SelectType::RANGE, si);
        score->select(last, SelectType::RANGE, si);
        EditVoice::changeSelectedElementsVoice(score->transactionManager()->currentOrDummyTransaction(), score, 0);

        // Drop the now-empty upper voices (their leftover rests) by imploding the
        // single staff onto voice 1.
        score->deselectAll();
        score->select(first, SelectType::RANGE, si);
        score->select(last, SelectType::RANGE, si);
        ImplodeExplode::implode(score);

        // Re-attach any tremolo whose chord was moved into voice 1 (and so lost it).
        for (const auto& [tick, type] : tremolosByTick) {
            const Fraction f = Fraction::fromTicks(tick);
            Measure* m = score->tick2measure(f);
            if (!m) {
                continue;
            }
            Segment* s = m->findSegment(SegmentType::ChordRest, f);
            if (!s) {
                continue;
            }
            for (voice_idx_t v = 0; v < VOICES; ++v) {
                EngravingItem* e = s->element(base + v);
                if (e && e->isChord() && !toChord(e)->tremoloSingleChord()) {
                    Chord* c = toChord(e);
                    TremoloSingleChord* trem = Factory::createTremoloSingleChord(c);
                    trem->setTremoloType(type);
                    c->add(trem);
                    break;
                }
            }
        }
    }

    // Final pass: drop redundant upper-voice rests. An upper voice (index >= 1) holding only rests
    // in a measure is not a real second voice (voice 0 already fills the bar after the collapse);
    // it would show as a spurious extra voice and can inflate the measure length. An upper voice
    // still carrying a chord is a genuine overlapping voice and is left untouched.
    for (staff_idx_t si = 0; si < score->nstaves(); ++si) {
        const track_idx_t base = si * VOICES;
        std::vector<Rest*> staleRests;
        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            for (voice_idx_t v = 1; v < VOICES; ++v) {
                bool voiceHasChord = false;
                std::vector<Rest*> voiceRests;
                for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                    EngravingItem* e = s->element(base + v);
                    if (!e) {
                        continue;
                    }
                    if (e->isChord()) {
                        voiceHasChord = true;
                        break;
                    }
                    if (e->isRest()) {
                        voiceRests.push_back(toRest(e));
                    }
                }
                if (!voiceHasChord) {
                    staleRests.insert(staleRests.end(), voiceRests.begin(), voiceRests.end());
                }
            }
        }
        for (Rest* r : staleRests) {
            score->removeElement(r);
        }
    }
    score->deselectAll();
}

static void buildScore(MasterScore* score, const EncRoot& enc, const EncImportOptions& opts)
{
    ScoreLoad sl;   // import edits run outside any undo transaction; see mergeNonOverlappingVoices

    score->style().set(Sid::chordsXmlFile, true);
    score->chordList()->read(u"chords.xml");

    // Enable multi-measure rests only when the file uses them (any REST with mrestCount > 1);
    // otherwise show individual whole rests.
    const bool hasMMRest = std::any_of(enc.measures.begin(), enc.measures.end(),
                                       [](const EncMeasure& m) {
        if (m.elements.empty()) {
            return false;
        }
        for (const auto& ep : m.elements) {
            if (static_cast<EncElemType>(ep->type) != EncElemType::REST) {
                return false;
            }
        }
        return static_cast<const EncRest*>(m.elements[0].get())->mrestCount > 1;
    });
    score->style().set(Sid::createMultiMeasureRests, hasMMRest);

    // Encore positions tuplet brackets/numbers flush against note heads and stems
    // with no extra vertical gap, and never pushes them outside the staff.
    score->style().set(Sid::tupletOutOfStaff,      false);
    score->style().set(Sid::tupletVHeadDistance,   0.0);
    score->style().set(Sid::tupletVStemDistance,   0.0);

    // Encore does not stretch systems and staves to fill the page: it lays them out at fixed
    // distances from the top. Keep vertical justification enabled but allow it no extra room
    // (max system/staff spread = 0), so the imported spacing matches Encore instead of being
    // spread to fill the page.
    score->style().set(Sid::enableVerticalSpread, true);
    score->style().set(Sid::maxSystemSpread,      Spatium(0.0));
    score->style().set(Sid::maxStaffSpread,       Spatium(0.0));

    BuildCtx ctx{ score, enc, opts };
    buildParts(ctx);
    buildMeasures(ctx);
    buildInitialSignatures(ctx);
    emitMeasures(ctx);

    if (ctx.opts.importStaffSize) {
        applyStaffScale(score, enc);
    }

    resolveAll(ctx);

    EditEnharmonicSpelling::spell(score);
    respellTransposingStaves(score);
    addTitleFrame(score, enc.titleBlock);
    // Assign MIDI ports/channels to every part. The file read path does this on load,
    // but a direct import builds the score in memory without it, leaving each channel
    // at -1; that makes Part::midiPort() index m_midiMapping[-1] and crash on a
    // straight-to-MusicXML export.
    score->rebuildMidiMapping();
    score->setUpTempoMap();
    score->doLayout();

    if (ctx.opts.mergeVoices) {
        mergeNonOverlappingVoices(score);
        score->doLayout();
    }

    // doLayout computes and caches the repeat list; at that point voltas may not yet be
    // anchored, so the cached expansion ignores 1st/2nd endings and replays the 1st
    // ending on every pass. The file read path invalidates the repeat list after load
    // for the same reason; do the same here so playback right after import is correct.
    score->masterScore()->invalidateRepeatList();
}

muse::String encoreLoadErrorMessage(const QString& path)
{
    QByteArray head;
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        head = file.read(5);
    }
    const QByteArray magic = head.left(4);
    const muse::String name = muse::String::fromQString(QFileInfo(path).fileName());

    // Older encrypted Encore container (ZBOT/ZBOP/ZBO6).
    if (magic == "ZBOT" || magic == "ZBOP" || magic == "ZBO6") {
        return muse::mtrc("engraving",
                          "“%1” is in an older, encrypted Encore format (%2) that this importer cannot read. "
                          "Open it in Encore and save it again, then import the saved file.")
               .arg(name).arg(muse::String::fromQString(QString::fromLatin1(magic)));
    }
    // Recognizable Encore header, but the file could not be parsed: unsupported variant, damaged,
    // or empty.
    if (magic == "SCOW" || magic == "SCO5") {
        const QString ver = QStringLiteral("%1").arg(
            head.size() >= 5 ? static_cast<unsigned char>(head[4]) : 0, 2, 16, QChar('0'));
        return muse::mtrc("engraving",
                          "“%1” could not be read as an Encore file (format version 0x%2). It may be damaged "
                          "or use an unsupported variant. Try opening it in Encore and saving it again, then "
                          "import the saved file.")
               .arg(name).arg(muse::String::fromQString(ver));
    }
    // No recognizable Encore header at all.
    return muse::mtrc("engraving",
                      "“%1” is not a recognized Encore file. Its header does not match any known Encore "
                      "format, so it may be corrupted or a different type of file.")
           .arg(name);
}

Err importEncore(MasterScore* score, const QString& path, const EncImportOptions& opts)
{
    if (!QFileInfo::exists(path)) {
        return Err::FileNotFound;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return Err::FileOpenError;
    }

    // ZBOT/ZBOP/ZBO6 are older encrypted Encore containers (only the first 42 bytes decrypt with
    // a known XOR key; see ENCORE_FORMAT.md). Not supported here. See MuseScore#24341.
    {
        QByteArray magic4 = file.read(4);
        file.seek(0);
        if (magic4 == "ZBOT" || magic4 == "ZBOP" || magic4 == "ZBO6") {
            LOGW() << "Encore: encrypted format (" << magic4.toStdString()
                   << ") is not supported; re-save the file in Encore as an unencrypted file first.";
            return Err::FileBadFormat;
        }
    }

    QDataStream ds(&file);
    ds.setByteOrder(QDataStream::LittleEndian);

    EncRoot enc;
    if (!enc.read(ds)) {
        return Err::FileBadFormat;
    }

    if (enc.instruments.empty() || enc.measures.empty()) {
        return Err::FileBadFormat;
    }

    logEncRootInfo(enc);
    buildScore(score, enc, opts);

    muse::Ret integrity = score->sanityCheck();
    if (!integrity) {
        LOGW() << "Encore import: score corruption detected:\n" << integrity.text();
    }

    return Err::NoError;
}
} // namespace mu::iex::enc
