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

// User-facing import options: the under/overfill and instrument-search strategy enums and the
// EncImportOptions struct whose in-code defaults are the test fallback (GUI defaults differ).

#ifndef MU_IMPORTEXPORT_ENC_IMPORT_OPTIONS_H
#define MU_IMPORTEXPORT_ENC_IMPORT_OPTIONS_H

namespace mu {
namespace iex {
namespace enc {
enum class UnderfillStrategy {
    InvisibleRests,    // gap rests (invisible)
    VisibleRests,      // normal visible rests
    IrregularMeasure,  // set actual measure duration to match content (shipped default)
};

enum class OverfillStrategy {
    Truncate,          // remove trailing notes/rests
    StretchLastNote,   // compress the trailing tuplet / notes to fit
    IrregularMeasure,  // set actual measure duration to match content (shipped default)
};

enum class InstrumentSearchMode {
    NameAndMidi,  // name matching + MIDI fallback (shipped default)
    MidiOnly,     // skip name matching, use only MIDI program
    Piano,        // assign Grand Piano to all instruments
};

enum class TablatureImportMode {
    Separate,  // import each tablature staff as its own independent staff
    Linked,    // link a tablature staff to its notation staff, sharing notes (shipped default)
    Ignore,    // drop tablature staves entirely
};

struct EncImportOptions {
    // Layout group
    bool importPageLayout = true;   // apply page size and margins from the Encore file
    bool importPageBreaks = true;   // apply page breaks derived from the Encore LINE blocks
    bool importStaffSize  = true;   // apply staff size scaling from the Encore file

    // Layout group (continued)
    bool importSystemLocks = true;  // lock each system to Encore's line measure count

    // Text / content group
    bool importTempoTextSemantic              = true;   // map Italian tempo terms to BPM values
    bool importUnsupportedArticulationsAsText = false;  // emit unknown artic bytes as staff text

    // Instrument search
    InstrumentSearchMode instrumentSearchMode = InstrumentSearchMode::NameAndMidi;

    // Tablature handling. In-code default (test fallback) is Separate so existing fixtures keep
    // one staff per Encore staff; the shipped GUI default is Linked (see enc-importconfiguration.cpp).
    TablatureImportMode tablatureImportMode = TablatureImportMode::Separate;

    // Measure correction group. These struct values are the in-code fallback used by tests;
    // the shipped (GUI) default for both is IrregularMeasure (see enc-importconfiguration.cpp).
    UnderfillStrategy underfillMeasureStrategy = UnderfillStrategy::InvisibleRests;
    OverfillStrategy overfillMeasureStrategy  = OverfillStrategy::Truncate;
    bool firstMeasureIsPickup = true;  // shorten first measure as pickup; false = pad with rests

    // Voice consolidation: collapse a staff's non-overlapping voices back into voice 1, all or
    // nothing (a staff with genuinely overlapping voices is left untouched). In-code fallback is
    // off so existing fixtures keep their voices; the GUI default is true.
    bool mergeVoices = false;
};
} // namespace enc
} // namespace iex
} // namespace mu

#endif // MU_IMPORTEXPORT_ENC_IMPORT_OPTIONS_H
