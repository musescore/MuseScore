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

// Pure lookups from Encore values to MuseScore types: clefs, key/time signatures, dynamics,
// articulations, tempo terms, instrument-template matching, and the frame/signature builders.

#ifndef MU_IMPORTEXPORT_ENC_IMPORT_MAPPING_H
#define MU_IMPORTEXPORT_ENC_IMPORT_MAPPING_H

#include <QString>
#include <vector>

#include "engraving/dom/clef.h"
#include "engraving/dom/timesig.h"

#include "engraving/types/symid.h"
#include "engraving/types/types.h"
#include "../parser/elem.h"

namespace mu::engraving {
class MasterScore;
class Measure;
class Score;
class InstrumentTemplate;
}

namespace mu::iex::enc {
mu::engraving::ClefType encClef2MuseScore(EncClefType ct);

// Encore time-signature glyph byte to MuseScore TimeSigType. Common time (0x43 'C' / 0x63 'c')
// in 4/4 maps to FOUR_FOUR; everything else is NORMAL (numeric). Cut time (alla breve) uses a
// glyph value that is not yet confirmed in the format, so it currently shows as numeric 2/2.
mu::engraving::TimeSigType encTimeSigGlyph2Type(quint8 glyph, mu::engraving::Fraction ts);

// Pick octave-decorated clef when Encore's plain G/F plus a NEGATIVE octave Key implies one
// (e.g. keyOffset=-12 -> G8_VB/F8_VB; -24 -> G15_MB/F15_MB). Positive octave Keys keep the plain
// clef (the octave is carried as a playback transposition; see builders-parts.cpp).
mu::engraving::ClefType pickStaffClef(EncClefType encClef, int keyOffsetSemitones);

// Return the octave-down variant of an already-resolved MuseScore clef for a NEGATIVE octave Key
// (-12/-24). Returns the input clef for non-octave or positive offsets, or when the clef has no
// octave variant.
mu::engraving::ClefType applyOctaveToClef(mu::engraving::ClefType base, int keyOffsetSemitones);

int encKeyToFifths(quint8 key);

void addInitialKeySig(mu::engraving::MasterScore* score, int staffIdx, quint8 encKey);
void addInitialTimeSig(mu::engraving::MasterScore* score, int nstaves, mu::engraving::Fraction ts,
                       mu::engraving::TimeSigType tsType = mu::engraving::TimeSigType::NORMAL);
void addInitialClef(mu::engraving::MasterScore* score, int staffIdx, EncClefType ct);
void addInitialClef(mu::engraving::MasterScore* score, int staffIdx, mu::engraving::ClefType clef);

QString normalizeEncoreInstrName(const QString& name);

// Sentinel for findEncoreInstrumentTemplate: skip the transposition compatibility filter.
// Valid Encore key offsets are in [-33, +24]; 0x7FFFFFFF is outside that range.
constexpr int ENC_KEY_NO_FILTER = 0x7FFFFFFF;

// Find best non-drumset template by name+MIDI score; applies transposition filter when encKeySemitones != ENC_KEY_NO_FILTER.
// When outExactName is non-null, it is set to true if the returned template matched the instrument
// name exactly (track/long/short name equality) rather than only via a substring ("contains").
// When outUniqueName is non-null, it is set to true if the returned template matched via a
// "distinctive" needle, i.e. a word no other template's name contains (e.g. "Dulzaina" hits only
// "Castilian Dulzaina"). Such a contains-match is as trustworthy as an exact match.
const mu::engraving::InstrumentTemplate* findEncoreInstrumentTemplate(
    const QString& encName, int encMidiProgram = -1, int encKeySemitones = ENC_KEY_NO_FILTER, bool* outExactName = nullptr,
    bool* outUniqueName = nullptr);

// Same as findEncoreInstrumentTemplate but restricted to useDrumset templates.
const mu::engraving::InstrumentTemplate* findDrumsetTemplate(const QString& encName);

// MIDI-only lookup among non-drumset templates; prefers "common" genre when multiple share the same program.
const mu::engraving::InstrumentTemplate* findTemplateByMidi(int encMidiProgram0indexed);

// Fallback when findTemplateByMidi finds no exact program match: returns the nearest template
// within the same General MIDI family (16 families of 8 programs), preferring the "common"
// genre on ties. Keeps the instrument's category (Strings, Brass, Bass, …) instead of falling
// back to Grand Piano for programs no template carries as its primary sound (Pizzicato/Tremolo
// Strings, Muted Trumpet, Synth Bass, Voice Oohs, …).
const mu::engraving::InstrumentTemplate* findTemplateByMidiFamily(int encMidiProgram0indexed);

// Given a matched template, return its standard-notation or tablature sibling (e.g.
// "Classical Guitar" <-> "Classical Guitar (tablature)"). Returns the input if it already
// matches wantTab, or nullptr when no sibling exists. Siblings are matched by shared
// musicXmlId, then by track name with any trailing "(...)" variant suffix removed.
const mu::engraving::InstrumentTemplate* findInstrumentVariant(
    const mu::engraving::InstrumentTemplate* base, bool wantTab);

// Return BPS if text is a standard Italian tempo term (Allegro, Andante, ...).
// Return 0 for relative marks (a tempo, Tempo I). Return -1 if not a tempo mark.
double encTextToTempoBps(const QString& text);

} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_IMPORT_MAPPING_H
