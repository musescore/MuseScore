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

// Build score parts and staves, matching each Encore instrument to the best MuseScore instrument template.

#include "builders.h"
#include "ctx.h"
#include "import.h"
#include "../parser/elem.h"
#include "mappers.h"
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <QDataStream>
#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stringdata.h"
#include "engraving/engravingerrors.h"
#include "log.h"

using namespace mu::engraving;

namespace mu::iex::enc {
// Build a human-readable instrument name from a template id slug
// ("bass-clarinet" -> "Bass Clarinet"). Used only when a matched template
// carries no track name of its own (see applyInstrumentOrFallback).
static String humanizeTemplateId(const String& id)
{
    QString out;
    bool startWord = true;
    for (const QChar& ch : id.toQString()) {
        if (ch == u'-' || ch == u'_') {
            out.append(u' ');
            startWord = true;
        } else {
            out.append(startWord ? ch.toUpper() : ch);
            startWord = false;
        }
    }
    return String(out);
}

// Which matching rule selected the instrument template, in source-evaluation order.
// Used only for the debug log; an enum (not a bare int + parallel label array) keeps the
// label switch compiler-checked so reordering or adding a step cannot index out of range.
enum class MatchStep {
    None,
    PercClef,       // PERC clef or GM percussive range -> drumset
    NameMidiScore,  // name+MIDI score match
    DrumsetName,    // name scoring over drumset templates
    PercKeyword,    // generic percussion keyword in the name -> drumset
    RhythmStaff,    // RHYTHM staff -> snare-drum
    MidiProgram,    // MIDI program lookup
    MidiFamily,     // nearest template in the same GM family (fallback before Grand Piano)
};

static const char* matchStepLabel(MatchStep step)
{
    switch (step) {
    case MatchStep::None:          return "";
    case MatchStep::PercClef:      return "PERC clef";
    case MatchStep::NameMidiScore: return "name+MIDI score";
    case MatchStep::DrumsetName:   return "drumset name";
    case MatchStep::PercKeyword:   return "perc keyword";
    case MatchStep::RhythmStaff:   return "RHYTHM staff";
    case MatchStep::MidiProgram:   return "MIDI program";
    case MatchStep::MidiFamily:    return "MIDI family";
    }
    return "";
}

// Describe an Encore MIDI program for the debug log using MuseScore's own instrument names
// (localized like everything else: the template trackName is translated at load). Encore stores
// a 1-indexed GM program; map it to the template whose primary sound is that program and show its
// track name. No hardcoded GM table; an unmapped program shows just the number.
static std::string midiProgramInfo(const EncInstrument& instr)
{
    if (instr.midiProgram <= 0) {
        return "MIDI program none";
    }
    std::string label = "MIDI program " + std::to_string(instr.midiProgram);
    if (const InstrumentTemplate* gm = findTemplateByMidi(instr.midiProgram - 1)) {
        label += " (" + gm->trackName.toStdString() + ")";
    }
    return label;
}

// Apply a found template (or fallback to Grand Piano) and set the instrument's long name.
static void applyInstrumentOrFallback(Part* part, const InstrumentTemplate* tmpl,
                                      const EncInstrument& instr, MatchStep matchStep)
{
    auto setInstrName = [&](Instrument& ins) {
        if (!instr.name.isEmpty()) {
            ins.setLongName(String(instr.name));
        }
        ins.setShortName(String());
        ins.instrumentLabel().setAllowGroupName(false);
    };
    if (tmpl) {
        Instrument ins = Instrument::fromTemplate(tmpl);
        setInstrName(ins);
        // A few generic templates carry no track name in instruments.xml (their UI name comes from
        // the muse instruments, unavailable here). Derive it from the template id so the mixer and
        // Instruments panel are not left blank.
        if (ins.trackName().isEmpty()) {
            ins.setTrackName(humanizeTemplateId(tmpl->id));
        }
        LOGD() << "  instrument \"" << instr.name.toStdString() << "\" (" << midiProgramInfo(instr)
               << "): step(" << matchStepLabel(matchStep) << ") -> " << ins.trackName().toStdString();
        part->setInstrument(ins);
        return;
    }
    // Grand Piano fallback.
    const InstrumentTemplate* pianoTmpl = searchTemplateForMidiProgram(0, 0, false);
    if (pianoTmpl) {
        LOGD() << "  instrument \"" << instr.name.toStdString() << "\" (" << midiProgramInfo(instr)
               << "): no match -> fallback: Grand Piano";
        Instrument ins = Instrument::fromTemplate(pianoTmpl);
        setInstrName(ins);
        part->setInstrument(ins);
    } else {
        LOGD() << "  instrument \"" << instr.name.toStdString() << "\" (" << midiProgramInfo(instr)
               << "): no match, no piano template -> bare MIDI";
        part->setMidiProgram(0, 0);
        if (!instr.name.isEmpty()) {
            part->setPlainLongName(String(instr.name));
        }
        part->setPlainShortName(String());
    }
}

// Step 2: name+MIDI score with transposition filter. A contains-only (substring) name match can
// outrank the right GM instrument via the MIDI bonus, so when the name match is neither exact nor
// unique, prefer the MIDI-program instrument if it differs. Exact and unique name matches are kept.
static const InstrumentTemplate* tryNameMidiScore(const EncInstrument& instr, int encMidi,
                                                  int encKey, bool isRhythm)
{
    bool nameExact = false;
    bool nameUnique = false;
    const InstrumentTemplate* nameTmpl = findEncoreInstrumentTemplate(instr.name, encMidi, encKey, &nameExact, &nameUnique);
    if (nameTmpl && !nameExact && !nameUnique && !isRhythm && instr.midiProgram > 0) {
        const InstrumentTemplate* midiTmpl = findTemplateByMidi(instr.midiProgram - 1);
        if (midiTmpl && midiTmpl != nameTmpl) {
            LOGD() << "  instrument \"" << instr.name.toStdString()
                   << "\": weak name match \"" << nameTmpl->trackName.toStdString()
                   << "\" overridden by MIDI " << instr.midiProgram << " -> "
                   << midiTmpl->trackName.toStdString();
            nameTmpl = midiTmpl;
        }
    }
    if (!nameTmpl && !instr.name.trimmed().isEmpty()) {
        const InstrumentTemplate* rejected = findEncoreInstrumentTemplate(instr.name, encMidi);
        if (rejected) {
            LOGD() << "  instrument \"" << instr.name.toStdString()
                   << "\": MIDI " << instr.midiProgram << " match \""
                   << rejected->trackName.toStdString()
                   << "\" rejected (template chromatic=" << rejected->transpose.chromatic
                   << " vs encKey=" << encKey << "), trying MIDI";
        }
    }
    return nameTmpl;
}

// Step 4: generic percussion keyword in the instrument name.
static bool nameHasPercKeyword(const EncInstrument& instr)
{
    const QString lname = instr.name.toLower();
    return lname.contains(QStringLiteral("perc"))
           || lname.contains(QStringLiteral("drum"))
           || lname.contains(QStringLiteral("bater"));
}

// Step 6: MIDI program lookup (logs a transposition mismatch against the Encore key).
static const InstrumentTemplate* tryMidiProgram(const EncInstrument& instr, int encKey)
{
    if (instr.midiProgram <= 0) {
        return nullptr;
    }
    const InstrumentTemplate* midiTmpl = findTemplateByMidi(instr.midiProgram - 1);
    if (midiTmpl) {
        const int tmplChr = midiTmpl->transpose.chromatic;
        const bool transpMismatch = (tmplChr % 12 != 0) && (encKey % 12 != 0)
                                    && ((((encKey % 12) + 12) % 12) != (((tmplChr % 12) + 12) % 12));
        if (transpMismatch) {
            LOGD() << "  instrument \"" << instr.name.toStdString()
                   << "\": MIDI " << instr.midiProgram << " match \""
                   << midiTmpl->trackName.toStdString()
                   << "\" transposition differs (template chromatic=" << tmplChr
                   << " vs encKey=" << encKey << ")";
        }
    }
    return midiTmpl;
}

static const InstrumentTemplate* applyBestInstrument(Part* part,
                                                     const EncInstrument& instr,
                                                     bool isPercByClef,
                                                     bool isRhythm,
                                                     bool encWantsTab,
                                                     InstrumentSearchMode searchMode)
{
    // Piano mode: skip all matching and go straight to fallback.
    if (searchMode == InstrumentSearchMode::Piano) {
        applyInstrumentOrFallback(part, nullptr, instr, MatchStep::None);
        return nullptr;
    }

    const int encMidi = instr.midiProgram > 0 ? instr.midiProgram - 1 : -1;
    const int encKey  = static_cast<int>(instr.keyTransposeSemitones);
    const bool nameTooShort = instr.name.trimmed().size() < 4;
    const bool useNameSearch = (searchMode == InstrumentSearchMode::NameAndMidi);

    const InstrumentTemplate* tmpl = nullptr;
    MatchStep matchStep = MatchStep::None;
    auto tryStep = [&](MatchStep step, const InstrumentTemplate* candidate) {
        if (!tmpl && candidate) {
            tmpl = candidate;
            matchStep = step;
        }
    };

    // Step 1: PERC clef or GM Percussive range (113 to 128 1-indexed) -> drumset.
    static constexpr int GM_PERC_FIRST = 113;
    if (isPercByClef || instr.midiProgram >= GM_PERC_FIRST) {
        tryStep(MatchStep::PercClef, searchTemplate(String(u"drumset")));
    }

    // Steps 2-4: name-based matching (skipped in MidiOnly mode).
    if (useNameSearch) {
        if (!tmpl) {
            tryStep(MatchStep::NameMidiScore, tryNameMidiScore(instr, encMidi, encKey, isRhythm));
        }
        // Step 3: name scoring over drumset templates.
        if (!nameTooShort) {
            tryStep(MatchStep::DrumsetName, findDrumsetTemplate(instr.name));
        }
        // Step 4: generic percussion keywords.
        if (!tmpl && !nameTooShort && nameHasPercKeyword(instr)) {
            tryStep(MatchStep::PercKeyword, searchTemplate(String(u"drumset")));
        }
    }

    // Step 5 (RHYTHM): snare-drum; skip MIDI to avoid program-0 piano override.
    if (isRhythm) {
        tryStep(MatchStep::RhythmStaff, searchTemplate(String(u"snare-drum")));
    }

    // Step 6: MIDI program lookup (skipped for RHYTHM staves).
    if (!tmpl && !isRhythm) {
        tryStep(MatchStep::MidiProgram, tryMidiProgram(instr, encKey));
    }

    // Step 7: nearest template in the same GM family. Catches configured programs that no
    // template carries as its primary sound (Pizzicato/Tremolo Strings, Muted Trumpet, Synth
    // Bass, Voice Oohs, …) so the part keeps its category instead of falling back to Grand Piano.
    if (!tmpl && !isRhythm && encMidi >= 0) {
        tryStep(MatchStep::MidiFamily, findTemplateByMidiFamily(encMidi));
    }

    // Encore stores the staff's notation/tablature choice in the clef; a name/MIDI match may
    // land on the wrong variant (e.g. "Classical Guitar (tablature)" when Encore uses a normal
    // clef). Swap to the standard or tablature sibling to match Encore's clef.
    if (tmpl && !isRhythm && (tmpl->staffGroup == StaffGroup::TAB) != encWantsTab) {
        if (const InstrumentTemplate* variant = findInstrumentVariant(tmpl, encWantsTab)) {
            LOGD() << "  instrument \"" << instr.name.toStdString() << "\": clef-driven variant "
                   << (encWantsTab ? "tablature" : "standard") << " -> " << variant->trackName.toStdString();
            tmpl = variant;
        }
    }

    applyInstrumentOrFallback(part, tmpl, instr, matchStep);
    return tmpl;
}

// Pick a TAB StaffType preset by string count (FULL for 4/5/6, COMMON for 7-10, else 6FULL);
// the line count is set separately via Staff::setLines.
static StaffTypes tabPresetForStringCount(int n)
{
    switch (n) {
    case 4:  return StaffTypes::TAB_4FULL;
    case 5:  return StaffTypes::TAB_5FULL;
    case 6:  return StaffTypes::TAB_6FULL;
    case 7:  return StaffTypes::TAB_7COMMON;
    case 8:  return StaffTypes::TAB_8COMMON;
    case 9:  return StaffTypes::TAB_9COMMON;
    case 10: return StaffTypes::TAB_10COMMON;
    default: return StaffTypes::TAB_6FULL;
    }
}

// Make an Encore tab staff a real MuseScore tab: attach StringData + a TAB StaffType so notes
// auto-fret at layout. Tuning source: Encore's stored tuning, else template StringData, else guitar.
static void setupTablatureStaff(Staff* staff, Instrument* instrument, const EncTabTuning& tuning)
{
    if (!staff || !instrument) {
        return;
    }
    const char* source = "Encore tuning";
    std::vector<int> pitches = tuning.hasData ? tuning.openStringPitches : std::vector<int>();
    if (pitches.empty()) {
        const StringData* tmplSd = instrument->stringData();
        if (tmplSd && tmplSd->strings() > 0) {
            for (const instrString& is : tmplSd->stringList()) {
                pitches.push_back(is.pitch);
            }
            source = "template";
        }
    }
    if (pitches.empty()) {
        pitches = { 40, 45, 50, 55, 59, 64 };   // standard 6-string guitar (sounding pitches)
        source = "default guitar";
    }
    const int nStrings = static_cast<int>(pitches.size());

    std::vector<instrString> strings;
    strings.reserve(pitches.size());
    std::string tuningStr;
    for (int p : pitches) {
        strings.push_back(instrString(p));
        tuningStr += std::to_string(p) + " ";
    }
    static constexpr int kTabFrets = 24;
    instrument->setStringData(StringData(kTabFrets, strings));

    const Fraction t0(0, 1);
    if (const StaffType* preset = StaffType::preset(tabPresetForStringCount(nStrings))) {
        staff->setStaffType(t0, *preset);
    }
    staff->setLines(t0, nStrings);
    LOGD() << "  tab staff: " << nStrings << " strings, tuning [ " << tuningStr << "] (" << source << ")";
}

void buildParts(BuildCtx& ctx)
{
    MasterScore* score = ctx.score;
    const EncRoot& enc = ctx.enc;
    const char* searchModeLabel
        =ctx.opts.instrumentSearchMode == InstrumentSearchMode::MidiOnly ? "MIDI only"
          : ctx.opts.instrumentSearchMode == InstrumentSearchMode::Piano ? "Grand Piano for all"
          : "name then MIDI";
    LOGD() << "---- Instrument assignment (using " << searchModeLabel << ") ----";
    int cumStaffIdx = 0;  // running index into enc.lines[0].staffData
    for (const auto& instr : enc.instruments) {
        int ns = instr.nstaves > 0 ? instr.nstaves : 1;
        Part* part = new Part(score);

        const EncLineStaffData* lsd = lineStaffDataAt(enc, cumStaffIdx);
        const bool isPercByClef = lsd && lsd->clef == EncClefType::PERC;
        const bool isRhythm = lsd && lsd->staffType == EncStaffType::RHYTHM;
        // Tablature only when Encore explicitly marks it (clef==TAB or staffType==TAB). v0xA6
        // has no per-staff clef in the LINE block, so it defaults to standard notation.
        const bool encWantsTab = lsd && (lsd->clef == EncClefType::TAB
                                         || lsd->staffType == EncStaffType::TAB);
        const InstrumentTemplate* tmpl = applyBestInstrument(part, instr, isPercByClef, isRhythm,
                                                             encWantsTab, ctx.opts.instrumentSearchMode);

        const bool showFromLine = !lsd || lsd->showStaff;
        if (!showFromLine) {
            part->setShow(false);
        }

        const int pitchOffset = static_cast<int>(instr.keyTransposeSemitones);
        // Transposition handling depends on the offset:
        //  - non-octave and positive octave: set on the instrument so the display keeps the written
        //    pitch under a plain clef (the octave is a playback transposition, no 8va clef).
        //  - negative octave: left to the octave-down clef from pickStaffClef()/applyOctaveToClef()
        //    plus the template's own transposition.
        Instrument* instrument = part->instrument();
        if (instrument) {
            if (pitchOffset != 0 && (std::abs(pitchOffset) % 12 != 0 || pitchOffset > 0)) {
                const Interval iv(pitchOffset);
                instrument->setTranspose(iv);
                static const char* const keyNames[] = {
                    "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"
                };
                const int keyIdx = ((pitchOffset % 12) + 12) % 12;
                LOGD() << "  instrument \"" << instr.name.toStdString()
                       << "\": transposition in " << keyNames[keyIdx]
                       << " (chromatic=" << iv.chromatic << " diatonic=" << iv.diatonic << ")";
            } else if (pitchOffset == 0) {
                // encKey=0 means "sounds as written" (ENCORE_FORMAT.md). Zero out any transposition
                // the template carries, including octave offsets: notes are already at written pitch,
                // so a template offset would shift the display by an octave.
                const Interval tmplT = instrument->transpose();
                if (!tmplT.isZero()) {
                    instrument->setTranspose(Interval(0, 0));
                    LOGD() << "  instrument \"" << instr.name.toStdString()
                           << "\": encKey=0 (sounds as written) → zeroing template transposition";
                }
            }
        }
        for (int s = 0; s < ns; ++s) {
            Staff* staff = Factory::createStaff(part);
            score->appendStaff(staff);
            if (tmpl) {
                staff->init(tmpl, nullptr, s);
                // Encore does not store bracket/brace grouping data, so remove any
                // bracket the template may have set to avoid spurious cross-part brackets.
                staff->setBracketType(0, BracketType::NO_BRACKET);
                staff->setBracketSpan(0, 0);
                // Safety net: if no standard sibling template was found above and the staff is
                // still tablature though Encore stores standard notation, force a standard staff.
                if (!encWantsTab && staff->staffType(Fraction(0, 1))->group() == StaffGroup::TAB) {
                    if (const StaffType* stdType = StaffType::getDefaultPreset(StaffGroup::STANDARD)) {
                        staff->setStaffType(Fraction(0, 1), *stdType);
                    }
                }
            }
            // Tablature is per staff (one Encore instrument may carry both a notation and a tab staff).
            const EncLineStaffData* staffLsd = lineStaffDataAt(enc, cumStaffIdx + s);
            const bool staffWantsTab = staffLsd && (staffLsd->clef == EncClefType::TAB
                                                    || staffLsd->staffType == EncStaffType::TAB);
            if (staffWantsTab) {
                setupTablatureStaff(staff, instrument, instr.tabTuning.hasData ? instr.tabTuning : enc.tabTuning);
            }
            ctx.staffPitchOffset.push_back(pitchOffset);
            ClefType cClef = ClefType::INVALID;
            if (tmpl) {
                cClef = tmpl->clefType(static_cast<staff_idx_t>(s)).concertClef;
            }
            ctx.staffTemplateConcertClef.push_back(cClef);
            ++ctx.totalStaves;
        }
        score->appendPart(part);
        cumStaffIdx += ns;
    }
    if (ctx.totalStaves == 0) {
        Part* part = new Part(score);
        part->setMidiProgram(0, 0);
        Staff* staff = Factory::createStaff(part);
        score->appendStaff(staff);
        score->appendPart(part);
        ctx.totalStaves = 1;
    }
}

// --- Tablature import mode post-pass -----------------------------------------

static bool encStaffIsTab(const EncRoot& enc, int i)
{
    const EncLineStaffData* d = lineStaffDataAt(enc, i);
    return d && (d->clef == EncClefType::TAB || d->staffType == EncStaffType::TAB);
}

static bool encStaffIsNotation(const EncRoot& enc, int i)
{
    const EncLineStaffData* d = lineStaffDataAt(enc, i);
    return d && d->staffType == EncStaffType::MELODY
           && d->clef != EncClefType::TAB && d->clef != EncClefType::PERC;
}

// A tab staff is a derived view with no notes of its own (frets come from the notation staff),
// so a mixed-file tab staff imports empty; this detects that emptiness.
static bool staffHasChords(Score* score, staff_idx_t staffIdx)
{
    for (Segment* s = score->firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (voice_idx_t v = 0; v < VOICES; ++v) {
            const EngravingItem* e = s->element(staffIdx * VOICES + v);
            if (e && e->isChord()) {
                return true;
            }
        }
    }
    return false;
}

static void removeTabStaff(Score* score, Staff* tab)
{
    // Full removal path (raw removeStaff leaves measures inconsistent -> layout crash). A tab staff is
    // its own single-staff part, so drop the whole part; fall back to per-staff removal if it shares one.
    Part* part = tab->part();
    if (part->nstaves() == 1) {
        score->cmdRemovePart(part);
    } else {
        score->cmdRemoveStaff(tab->idx());
    }
}

// Turn a tab staff into a standard 5-line notation staff: drop the StringData and swap the TAB
// StaffType and clef for standard ones. Used by Ignore mode on a tab-only score, where there is no
// notation staff to fall back to, so the tab's notes are shown as notation (guitar-family G8vb clef).
static void convertTabStaffToStandard(Score* score, Staff* staff)
{
    const Fraction t0(0, 1);
    if (const StaffType* stdType = StaffType::getDefaultPreset(StaffGroup::STANDARD)) {
        staff->setStaffType(t0, *stdType);
    }
    staff->setLines(t0, 5);
    if (Instrument* ins = staff->part()->instrument()) {
        ins->setStringData(StringData());
    }
    staff->setDefaultClefType(ClefTypeList(ClefType::G8_VB));
    if (Measure* m = score->tick2measure(t0)) {
        if (Segment* seg = m->findSegment(SegmentType::HeaderClef, t0)) {
            if (Clef* clef = toClef(seg->element(staff->idx() * VOICES))) {
                clef->setClefType(ClefType::G8_VB);
            }
        }
    }
}

// Merge an empty tab staff into its notation staff as one instrument (guitar+tab idiom): clone the
// notation's notes as linked clones (the tab renders them as frets), reparent the tab, drop its part.
static void linkTabToNotation(Score* score, Staff* notation, Staff* tab, bool notationVisible, bool tabVisible)
{
    Part* notPart = notation->part();
    Part* tabPart = tab->part();

    // Clear the tab's rest fill + key signature before cloning so the clone lands in an empty staff;
    // otherwise stale rests overflow an irregular bar and the key signature duplicates.
    const track_idx_t tabBase = tab->idx() * VOICES;
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        for (Segment* seg = m->first(); seg; seg = seg->next()) {
            const SegmentType st = seg->segmentType();
            if (st != SegmentType::ChordRest && st != SegmentType::KeySig) {
                continue;
            }
            for (voice_idx_t v = 0; v < VOICES; ++v) {
                if (EngravingItem* e = seg->element(tabBase + v)) {
                    seg->remove(e);
                    delete e;
                }
            }
        }
    }

    Excerpt::cloneStaff(notation, tab, true);

    if (const StringData* sd = tabPart->instrument()->stringData()) {
        notPart->instrument()->setStringData(*sd);
    }

    tabPart->removeStaff(tab);
    notPart->appendStaff(tab);
    tab->setPart(notPart);
    score->removePart(tabPart);

    notPart->setShow(true);
    notation->setVisible(notationVisible);
    tab->setVisible(tabVisible);
}

void applyTablatureImportMode(BuildCtx& ctx)
{
    if (ctx.opts.tablatureImportMode == TablatureImportMode::Separate) {
        return;   // buildParts already produced independent tab staves
    }
    Score* score = ctx.score;
    const EncRoot& enc = ctx.enc;
    const bool ignore = ctx.opts.tablatureImportMode == TablatureImportMode::Ignore;
    LOGD() << "---- Tablature (" << (ignore ? "Ignore" : "Linked") << " mode) ----";

    // Snapshot the staff list because the merge/removal below mutates it. Identify targets by
    // pointer and by their pre-mutation index (which still maps 1:1 to the Encore staff order).
    const std::vector<Staff*> staves(score->staves().begin(), score->staves().end());
    const int n = static_cast<int>(staves.size());

    std::vector<Staff*> tabsToRemove;
    struct Pair {
        Staff* notation;
        Staff* tab;
        bool notationVisible;
        bool tabVisible;
    };
    std::vector<Pair> pairs;

    for (int i = 0; i < n; ++i) {
        if (!encStaffIsTab(enc, i)) {
            continue;
        }
        if (ignore) {
            tabsToRemove.push_back(staves[i]);
            LOGD() << "  drop tab staff " << i;
            continue;
        }
        // Pair the tab with the notation staff immediately above and merge, but only for separate
        // single-staff parts with the tab empty and the notation carrying the music.
        if (i > 0 && encStaffIsNotation(enc, i - 1)
            && staves[i]->part() != staves[i - 1]->part()
            && staves[i - 1]->part()->nstaves() == 1 && staves[i]->part()->nstaves() == 1
            && !staffHasChords(score, static_cast<staff_idx_t>(i))
            && staffHasChords(score, static_cast<staff_idx_t>(i - 1))) {
            const EncLineStaffData* notLsd = lineStaffDataAt(enc, i - 1);
            const EncLineStaffData* tabLsd = lineStaffDataAt(enc, i);
            pairs.push_back({ staves[i - 1], staves[i],
                              !notLsd || notLsd->showStaff, !tabLsd || tabLsd->showStaff });
            LOGD() << "  linked tab: staff " << i << " <- notation staff " << (i - 1);
        }
    }

    if (ignore) {
        // A tab-only score (every staff is tablature) has no notation staff to fall back to. "Ignore"
        // means no tablature staff, so show the notes as standard notation rather than dropping them
        // (an empty score has no playable part and crashes playback).
        if (static_cast<int>(tabsToRemove.size()) >= n) {
            for (Staff* t : tabsToRemove) {
                convertTabStaffToStandard(score, t);
            }
            return;
        }
        for (Staff* t : tabsToRemove) {
            removeTabStaff(score, t);
        }
        // Dropping the tab can leave only hidden parts (e.g. a tab shown over a hidden notation
        // staff); an all-hidden score has no playable part and crashes playback. Reveal what remains.
        const bool anyShown = std::any_of(score->parts().begin(), score->parts().end(),
                                          [](const Part* p) { return p->show(); });
        if (!anyShown) {
            for (Part* p : score->parts()) {
                p->setShow(true);
            }
        }
        return;
    }
    for (const Pair& p : pairs) {
        linkTabToNotation(score, p.notation, p.tab, p.notationVisible, p.tabVisible);
    }
}
} // namespace mu::iex::enc
