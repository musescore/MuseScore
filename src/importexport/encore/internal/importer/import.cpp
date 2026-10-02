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
// Binary format documented by Leon Vinken (Enc2MusicXML, GPL v3+) building on enc2ly by Felipe Castro.

#include "ctx.h"
#include "builders.h"
#include "resolvers.h"
#include "page-layout.h"
#include "debug-dump.h"

#include "import.h"

#include "../parser/elem.h"
#include "mappers.h"
#include "../parser/ticks.h"
#include "../parser/zbot.h"
#include "emitters-tuplets.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <map>
#include <set>
#include <vector>

#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include "../parser/readers.h"

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
    // Note::setPitch only asserts the range, and drumset lookups index a 128-entry table by pitch, so
    // clamp once here, where both the main and the grace path pass.
    n->setPitch(std::clamp(semitone, 0, 127));
    n->setTpcFromPitch();
}

// score->spell() can spell a transposed pitch with double flats, so re-derive the TPC on transposing
// staves from pitch, concert key and transposition. Format agnostic: could move to a shared util.
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

// Collapse a staff's voices into voice 1 when they never sound together. All or nothing per staff, so
// the music is never altered; see ENCORE_IMPORTER.md 5.4.
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

    // No undo transaction is opened, so the commands below execute and free themselves.
    for (staff_idx_t si : candidates) {
        const track_idx_t base = si * VOICES;

        // The voice change rebuilds the destination chord and carries everything across except a
        // single-chord tremolo, so snapshot those by tick and re-attach them after.
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

    // An upper voice holding only rests is not a second voice once voice 0 fills the bar: it shows as a
    // spurious extra voice and can inflate the measure. One still holding a chord is genuine.
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
        int maxMrest = 0;
        for (const auto& ep : m.elements) {
            const EncElemType t = static_cast<EncElemType>(ep->type);
            if (t == EncElemType::NOTE || t == EncElemType::CHORD) {
                return false;
            }
            if (t == EncElemType::REST) {
                maxMrest = std::max(maxMrest, static_cast<int>(static_cast<const EncRest*>(ep.get())->mrestCount));
            }
        }
        return maxMrest > 1;
    });
    score->style().set(Sid::createMultiMeasureRests, hasMMRest);

    // Encore positions tuplet brackets/numbers flush against note heads and stems
    // with no extra vertical gap, and never pushes them outside the staff.
    score->style().set(Sid::tupletOutOfStaff,      false);
    score->style().set(Sid::tupletVHeadDistance,   0.0);
    score->style().set(Sid::tupletVStemDistance,   0.0);

    // Encore lays systems out at fixed distances from the top, so justification stays enabled with no
    // room to spread and the imported spacing survives.
    score->style().set(Sid::enableVerticalSpread, true);
    score->style().set(Sid::maxSystemSpread,      Spatium(0.0));
    score->style().set(Sid::maxStaffSpread,       Spatium(0.0));

    BuildCtx ctx{ score, enc, opts };
    buildParts(ctx);
    buildMeasures(ctx);
    buildInitialSignatures(ctx);
    emitMeasures(ctx);
    guaranteeAllMeasures(ctx);

    applyPageSetup(ctx);
    if (ctx.opts.importStaffSize) {
        applyStaffScale(score, enc);
    }

    // The passes that fit each bar to its signature can change a measure's length, which leaves the
    // score's tick map describing the lengths they had before. Everything below resolves elements by
    // tick, and a stale map answers with the measure before the one meant: rebuild it first.
    score->updateTicksAndTimeSigMap();

    resolveAll(ctx);

    // Apply the tablature import mode (link tab staves to their notation staff, or drop them).
    // Runs after notes are emitted and spanners resolved, before MIDI mapping and layout.
    applyTablatureImportMode(ctx);

    EditEnharmonicSpelling::spell(score);
    respellTransposingStaves(score);
    addTitleFrame(score, enc.titleBlock);
    // The file read path does this on load; a direct import leaves every channel at -1, which makes
    // Part::midiPort() index the mapping at -1 and crash on a straight-to-MusicXML export.
    score->rebuildMidiMapping();
    score->updateTicksAndTimeSigMap();
    score->doLayout();

    if (ctx.opts.mergeVoices) {
        mergeNonOverlappingVoices(score);
        score->doLayout();
    }

    // With imported page breaks, a first-page system may have spilled onto the next page at the
    // default staff space; shrink it just enough (<= 0.022 inch) to pull that system back.
    fitFirstPageStaffSpace(ctx);

    // doLayout caches the repeat list before the voltas are anchored, so the cached expansion replays
    // the first ending on every pass.
    score->masterScore()->invalidateRepeatList();
}

// What to tell the user about a file that failed to load, from its header alone. Encore's two
// extensions are shared with several neighbours, so whoever reaches this message has often opened
// one of those by mistake and needs the way out, not a verdict. See ENCORE_FORMAT.md §1.2.
muse::String encoreLoadErrorMessage(const QString& path)
{
    struct Foreign {
        QByteArray magic;
        const char* program;
    };
    static const std::vector<Foreign> foreign {
        { QByteArrayLiteral("ENIGMA "), "Finale" },
        { QByteArrayLiteral("Finale(R)"), "Finale" },
        { QByteArrayLiteral("SOLF"), "Melody Assistant or Harmony Assistant" },
        { QByteArray("RO\0\0", 4), "Master Tracks Pro" },
    };

    QByteArray head;
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        head = file.read(16);   // the longest signature above is nine bytes
    }
    for (const Foreign& f : foreign) {
        if (head.startsWith(f.magic)) {
            return muse::mtrc("engraving",
                              "The Encore importer cannot open %1 files. Open the file in %1 and export it as "
                              "MusicXML, then open the MusicXML file instead.")
                   .arg(muse::String::fromUtf8(f.program));
        }
    }
    return muse::mtrc("engraving",
                      "Unrecognized Encore file. The file may be corrupted or have an unsupported format. Try "
                      "opening it in Encore itself and saving it again, or exporting it as MusicXML from "
                      "there, then open the result.");
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

    QByteArray fileData = file.readAll();
    file.close();

    // ZBOT/ZBOP/ZBO6 are legacy Encore encrypted containers. Decrypt them in place with the stream
    // cipher (see parser/zbot.cpp); the result is a plain SCOW buffer that the reader
    // below parses like any other Encore file.
    if (fileData.size() >= 4 && isZbotMagic(fileData.left(4))) {
        LOGD() << "Encore: ZBOT encrypted format detected, decrypting to SCOW.";
        zbotDecrypt(fileData);
    }

    QBuffer buf(&fileData);
    buf.open(QIODevice::ReadOnly);
    QDataStream ds(&buf);
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
    // reconcileMeasureLength guarantees every voice on every staff sums to its measure length, so a
    // finalized score is always sanity-clean. Fail loudly in debug/test builds if that ever breaks,
    // rather than shipping a corrupt score silently.
    DO_ASSERT(integrity);

    return Err::NoError;
}
} // namespace mu::iex::enc
