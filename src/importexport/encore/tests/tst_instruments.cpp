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

// Instrument resolution: name/MIDI/clef routing to templates, drumset detection, TK-block name/MIDI/Key
// decoding across format variants, key transposition and octave clefs. See ENCORE_IMPORTER.md §Instrument routing.

#include <gtest/gtest.h>

#include "engraving/dom/chord.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/tuplet.h"

#include "engraving/dom/instrtemplate.h"
#include "importexport/encore/internal/importer/mappers.h"

#include "testbase.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

class Tst_Instruments : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

// ===========================================================================
// FIX: name + MIDI scoring lets "Bass" + GM 32 (Acoustic Bass) beat the choral Bass template (program 52).
// ===========================================================================
TEST_F(Tst_Instruments, instrument_name_midi_tiebreaks_to_acoustic_bass)
{
    MasterScore* score = readEncoreScore("instruments_instr_bass_midi_tiebreak.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"acoustic-bass"))
        << "Bass + program 32 must reach the instrumental template, not the choral voice";
    delete score;
}

// ===========================================================================
// FIX: a name/MIDI match may land on a tablature template variant ("Classical
// Guitar (tablature)"), but Encore stores a normal clef. The importer must swap
// to the standard-notation sibling so the staff is a normal pitched staff.
// ===========================================================================
TEST_F(Tst_Instruments, tab_template_swapped_to_standard_when_clef_not_tab)
{
    MasterScore* score = readEncoreScore("instruments_tab_template_forced_standard.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->staves().empty());
    const StaffType* st = score->staff(0)->staffType(Fraction(0, 1));
    ASSERT_NE(st, nullptr);
    EXPECT_NE(st->group(), StaffGroup::TAB)
        << "Encore stores a normal clef, so the tablature template must be swapped to standard notation";
    delete score;
}

// ===========================================================================
// FIX: when Encore stores EncClefType::TAB the importer must swap a standard
// "Classical Guitar" match to the tablature sibling so the staff is tablature.
// ===========================================================================
TEST_F(Tst_Instruments, standard_template_swapped_to_tablature_when_clef_tab)
{
    MasterScore* score = readEncoreScore("instruments_tab_clef_keeps_tablature.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->staves().empty());
    const StaffType* st = score->staff(0)->staffType(Fraction(0, 1));
    ASSERT_NE(st, nullptr);
    EXPECT_EQ(st->group(), StaffGroup::TAB)
        << "EncClefType::TAB must select the tablature template variant";
    delete score;
}

// ===========================================================================
// FEATURE: RHYTHM staffType (=2) in LINE block routes to snare-drum regardless of name or midiProgram.
// ===========================================================================
TEST_F(Tst_Instruments, instrument_rhythm_staff_routes_to_snare_drum)
{
    MasterScore* score = readEncoreScore("instruments_rhythm_staff_snare.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"snare-drum"))
        << "RHYTHM staffType must route to snare-drum template (step 6), not Grand Piano";
    EXPECT_NE(inst->drumset(), nullptr)
        << "Snare-drum instrument must carry a drumset";
    delete score;
}

// ===========================================================================
// FEATURE: EncClefType::PERC on first staff routes to drumset regardless of name or midiProgram.
// ===========================================================================
TEST_F(Tst_Instruments, instrument_perc_clef_routes_to_drumset)
{
    MasterScore* score = readEncoreScore("instruments_instr_perc_clef_drumset.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"drumset"))
        << "PERC clef must override name+MIDI and route to drumset (primary path)";
    delete score;
}

// ===========================================================================
// FEATURE: keyword fallback path. "Drums" (contains "drum") triggers step 4 and routes
// to drumset regardless of midiProgram=1 (which would otherwise give Grand Piano).
// ===========================================================================
TEST_F(Tst_Instruments, instrument_name_drums_english_routes_to_drumset)
{
    MasterScore* score = readEncoreScore("instruments_instr_drums_name_drumset.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"drumset"))
        << "'Drums' must reach drumset via findDrumsetTemplate, not fall back to piano";
    delete score;
}

// ===========================================================================
// FIX: percussion tracks store midiProgram=1 (Grand Piano) regardless of instrument.
// "Percusión" must reach drumset via findDrumsetTemplate (localized name), not MIDI fallback.
// ===========================================================================
TEST_F(Tst_Instruments, instrument_name_routes_percussion_to_drumset)
{
    MasterScore* score = readEncoreScore("instruments_instr_percussion_drumset.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"drumset"))
        << "Encore 'Percusión' must reach the drum-kit template, not the MIDI piano fallback";
    delete score;
}

// ===========================================================================
// FIX: matching is diacritics-insensitive. Template "Laúd" (id=laud) must be reached
// by files that write "Laud" (no accent), as real corpora frequently do.
// ===========================================================================
TEST_F(Tst_Instruments, instrument_name_diacritics_insensitive_match)
{
    MasterScore* score = readEncoreScore("instruments_instr_laud_accent.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"laud"))
        << "Encore 'Laud' must match the laud template whose trackName carries the diacritic";
    delete score;
}

// ===========================================================================
// FEATURE: TK block instrument name encoding (UTF-16 probe for v0xC4)
// ===========================================================================

// A large-varsize TK block selects two-byte (UTF-16) name decoding directly, no probe needed.
TEST_F(Tst_Instruments, tk_utf16_name_charsize_reads_full_name)
{
    MasterScore* score = readEncoreScore("instruments_tk_utf16_name.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());

    const String longName = score->parts()[0]->longName();
    EXPECT_EQ(longName, String(u"Bandurria"))
        << "UTF-16 TK name (charSize=TWO_BYTES by offset) must be fully decoded";

    delete score;
}

// A small-varsize TK block defaults to one-byte, but a NUL second byte probes as UTF-16 and upgrades it.
TEST_F(Tst_Instruments, tk_probe_upgrades_onebyte_to_utf16)
{
    MasterScore* score = readEncoreScore("instruments_tk_probe_utf16.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());

    const String longName = score->parts()[0]->longName();
    EXPECT_EQ(longName, String(u"Bandurria"))
        << "Probe must upgrade ONE_BYTE charSize to TWO_BYTES for UTF-16 content";

    delete score;
}

// A non-NUL second byte keeps one-byte (Latin-1) decoding rather than misreading it as UTF-16.
TEST_F(Tst_Instruments, tk_probe_keeps_onebyte_for_latin1)
{
    MasterScore* score = readEncoreScore("instruments_tk_onebyte_name.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "ONE_BYTE TK name file must produce a clean score";
    EXPECT_EQ(score->parts().size(), 1u)
        << "ONE_BYTE TK name file must produce exactly 1 part";
    delete score;
}

// ===========================================================================
// FIX: compact v0xC4 (no TK blocks) stores MIDI at offset 390 (step 276), not the TK formula (offset 2278).
// Also: findTemplateByMidi prefers "common" genre: MIDI 68 is Oboe (common) before Dulzaina (world).
// ===========================================================================
TEST_F(Tst_Instruments, compact_no_tk_midi_reads_from_compact_area)
{
    MasterScore* score = readEncoreScore("instruments_compact_no_tk_midi_oboe.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"oboe"))
        << "Compact file (no TK blocks): MIDI 69 must read from compact offset and resolve to oboe";
    delete score;
}

// ===========================================================================
// FIX: MIDI step 5 must fire even when name is empty. Old guard skipped step 5 for names < 4 chars,
// forcing Grand Piano even with a valid midiProgram. TK00 zeroed + MIDI 43 must resolve to Cello.
// ===========================================================================
TEST_F(Tst_Instruments, instrument_empty_name_midi_resolves_to_cello)
{
    MasterScore* score = readEncoreScore("instruments_instr_empty_name_midi_cello.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"violoncello"))
        << "Empty name + MIDI 43 (Cello) must resolve to violoncello via step5, not Grand Piano";
    delete score;
}

// ===========================================================================
// FIX: a matched template with no <trackName> (e.g. the bare "recorder"
// template, whose UI name comes from muse_instruments, not instruments.xml)
// must not leave the imported part with an empty track name. The importer
// backfills the track name from the Encore instrument name so the mixer and
// the Instruments panel show a name instead of a blank entry.
// ===========================================================================
TEST_F(Tst_Instruments, instrument_recorder_midi75_keeps_track_name)
{
    MasterScore* score = readEncoreScore("instruments_instr_recorder_midi75_trackname.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"recorder"))
        << "MIDI 75 must resolve to the recorder template via step5";
    EXPECT_EQ(inst->trackName(), String(u"Recorder"))
        << "recorder template has no trackName; importer must derive the sounding "
        "instrument name from the template id, not from the Encore part label";
    EXPECT_EQ(inst->nameAsPlainText(), String(u"Txistu"))
        << "the Encore instrument name stays as the part long name";
    delete score;
}

// ===========================================================================
// FIX: Bb clarinet (MIDI 72, Key=0) must not fall back to Grand Piano, and
// its transposition must be zeroed (encKey=0 means 'sounds as written' in
// Encore so no chromatic shift should be applied at display time).
// ===========================================================================
TEST_F(Tst_Instruments, instrument_clarinet_midi72_key0_resolves_not_piano)
{
    MasterScore* score = readEncoreScore("instruments_instr_clarinet_midi72_key0.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"bb-clarinet"))
        << "Clarinete + MIDI 72 + Key=0 must resolve to bb-clarinet (step2), not Grand Piano";
    EXPECT_EQ(inst->transpose().chromatic, 0)
        << "encKey=0 means Encore stores written pitch with no shift; template transposition "
        "must be zeroed so notes display at their Encore written pitch";
    delete score;
}

// ===========================================================================
// FIX: nameless Bb clarinet (MIDI 72, Key=0) must not fall back to Grand Piano.
// When name is too short to trigger step 2, only step 5 (MIDI-only) can fire.
// The old transposition filter in step 5 rejected bb-clarinet (tmplChr=-2)
// whenever encKey==0, causing piano fallback.
// ===========================================================================
TEST_F(Tst_Instruments, instrument_empty_name_midi_clarinet_resolves_not_piano)
{
    MasterScore* score = readEncoreScore("instruments_instr_empty_name_midi_clarinet.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"bb-clarinet"))
        << "Empty name + MIDI 72 + Key=0 must resolve to bb-clarinet (step5), not Grand Piano";
    delete score;
}

// ===========================================================================
// REGRESSION: Bb clarinet with Key=-2 (correctly configured) must still work.
// Guards against a fix for Key=0 accidentally breaking the correct-Key path.
// ===========================================================================
TEST_F(Tst_Instruments, instrument_clarinet_midi72_key_neg2_resolves_correctly)
{
    MasterScore* score = readEncoreScore("instruments_instr_clarinet_midi72_key_neg2.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"bb-clarinet"))
        << "Clarinete + MIDI 72 + Key=-2 must still resolve to bb-clarinet via transposition filter";
    delete score;
}

// ===========================================================================
// FIX: compact v0xC4 with short header (LINE block before offset 390) must not read MIDI from inside LINE.
// Byte 0x30=48 at offset 390 is LINE layout data; guard must fall back to Grand Piano.
// ===========================================================================
TEST_F(Tst_Instruments, compact_short_header_ignores_line_data_as_midi)
{
    MasterScore* score = readEncoreScore("instruments_compact_short_header_no_midi.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_NE(inst->id(), String(u"timpani"))
        << "Byte 0x30=48 inside LINE block must NOT select Timpani; no MIDI program in pre-LINE area";
    delete score;
}

TEST_F(Tst_Instruments, instrument_name_recovery_without_tk_block)
{
    // instrumentCount=2, 1 TK block (TK00 "Bandurria"). "Guitarra" has no TK04 header but its
    // UTF-16 LE name is at NAME_BASE + 1*NAME_STEP; importer must detect and recover it.
    MasterScore* score = readEncoreScore("instruments_name_recovery.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_GE(score->parts().size(), 2u)
        << "Both instruments must be created (1 TK block + 1 recovered)";

    const String part1Name = score->parts()[1]->longName();
    EXPECT_EQ(part1Name, String(u"Guitarra"))
        << "Instrument name must be recovered from formula position, not 'Soprano Guitar'";

    delete score;
}

TEST_F(Tst_Instruments, staff_hidden_flag)
{
    // LINE staffData byte +19 = showByte; 0x00 means hidden. Importer must call part->setShow(false).
    MasterScore* score = readEncoreScore("instruments_staff_hidden.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());

    EXPECT_FALSE(score->parts()[0]->show())
        << "Staff with showByte=0x00 must be hidden (part->show()==false)";

    delete score;
}

TEST_F(Tst_Instruments, instrument_count_padding)
{
    // header instrumentCount=2 but only 1 TK block. Instruments vector must be padded to instrumentCount
    // so both parts are created (padded entry uses MIDI fallback).
    MasterScore* score = readEncoreScore("instruments_instrument_count_padding.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "score must be clean: " << ret.text();

    EXPECT_EQ(score->parts().size(), 2u)
        << "Both instruments must be imported even when only 1 TK block exists";

    delete score;
}

TEST_F(Tst_Instruments, transposition_filter_prefers_compatible_key)
{
    // The transposition filter is a preference, not a hard rejection: when no compatible template exists it
    // falls back to the best name+MIDI match rather than returning nullptr (which would give Grand Piano).
    MasterScore* score = readEncoreScore("instruments_instrument_count_padding.enc");
    ASSERT_NE(score, nullptr);
    delete score;

    using namespace mu::iex::enc;

    const InstrumentTemplate* filtered = findEncoreInstrumentTemplate(
        QStringLiteral("Dulzaina 2"), -1, 0);
    ASSERT_NE(filtered, nullptr)
        << "When no transposition-compatible match exists, must fall back to best name match "
        "(not nullptr) to avoid Grand Piano fallback";
    EXPECT_EQ(filtered->transpose.chromatic, 6)
        << "Castilian Dulzaina (chromatic=6) is the only dulzaina template, so it is the fallback";

    const InstrumentTemplate* unfiltered = findEncoreInstrumentTemplate(
        QStringLiteral("Dulzaina 2"), -1);
    ASSERT_NE(unfiltered, nullptr);
    EXPECT_EQ(unfiltered->transpose.chromatic, 6)
        << "Unfiltered call must also find Castilian Dulzaina (chromatic=6)";
}

// Regression: a distinctive name match (a needle unique to a single template, e.g.
// "Dulzaina" -> only "Castilian Dulzaina") must NOT be overridden by the MIDI program.
// The instrument is named "Dulzaina" with MIDI 69 (Oboe); before the fix the contains-only
// match was treated as weak and replaced by Oboe. Ambiguous substrings ("Bajo") still defer
// to MIDI; see instrument_name_midi_tiebreaks_to_acoustic_bass and the tuba test.
TEST_F(Tst_Instruments, unique_name_match_not_overridden_by_midi)
{
    MasterScore* score = readEncoreScore("instruments_unique_name_beats_midi.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_TRUE(inst->id().contains(String(u"dulzaina")))
        << "unique name 'Dulzaina' must win over the Oboe MIDI program; got "
        << inst->id().toStdString();
    delete score;
}

// FEATURE: last-resort fuzzy (edit-distance) name match for spelling typos the exact/contains
// search misses. "Clarynet" (one substitution from "Clarinet") has no substring match and no
// MIDI program, so without the fuzzy pass it falls back to Grand Piano; it must map to a clarinet.
TEST_F(Tst_Instruments, fuzzy_name_match_typo)
{
    MasterScore* score = readEncoreScore("instruments_fuzzy_name_match.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_TRUE(inst->id().contains(String(u"clarinet")))
        << "typo 'Clarynet' must fuzzy-match a clarinet, not fall back to piano; got "
        << inst->id().toStdString();
    delete score;
}

// When a file has no TK blocks, instrument names are recovered from fixed offsets; the "Part N" fallback
// must be applied only after recovery so it does not pre-empt a name present in the file.
TEST_F(Tst_Instruments, no_tk_blocks_name_recovered_from_fixed_offset)
{
    MasterScore* score = readEncoreScore("instruments_no_tk_name_recovered.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Part* part = score->parts().front();
    ASSERT_NE(part, nullptr);
    const QString longName = part->longName().toQString();
    EXPECT_EQ(longName, QString("Dulzaina"))
        << "Instrument name 'Dulzaina' stored at NAME_BASE=202 (UTF-16) must be "
        "recovered when TK blocks are absent; without fix it stays 'Part 1'";
    delete score;
}

TEST_F(Tst_Instruments, no_tk_blocks_name_recovered_latin1_encoding)
{
    // instruments_no_tk_name_latin1.enc: TK00 magic zeroed, "Tamboril" written
    // as Latin-1 (b0='T', b1='a' → not UTF-16, is Latin-1) at NAME_BASE=202.
    MasterScore* score = readEncoreScore("instruments_no_tk_name_latin1.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Part* part = score->parts().front();
    ASSERT_NE(part, nullptr);
    const QString longName = part->longName().toQString();
    EXPECT_EQ(longName, QString("Tamboril"))
        << "Latin-1 instrument name at NAME_BASE=202 must be recovered when TK blocks absent";
    delete score;
}

TEST_F(Tst_Instruments, no_tk_blocks_name_falls_back_to_part_n_when_not_recoverable)
{
    // instruments_no_tk_name_fallback.enc: TK00 magic zeroed, offset 202 is 0x00
    // (b0 < 0x20 → recoverMissingNames skips it). "Part 1" fallback must fire.
    MasterScore* score = readEncoreScore("instruments_no_tk_name_fallback.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Part* part = score->parts().front();
    ASSERT_NE(part, nullptr);
    const QString longName = part->longName().toQString();
    EXPECT_FALSE(longName.isEmpty())
        << "Part must have a non-empty name even when name recovery fails";
    delete score;
}

TEST_F(Tst_Instruments, small_tk_midi_read_from_correct_offset)
{
    // smallTK layout stores the MIDI program at a different offset; reading the wrong one lands in the
    // zero-padded name area (program 0 -> Grand Piano). MIDI 49 must resolve to a non-Piano template.
    MasterScore* score = readEncoreScore("instruments_small_tk_midi49.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_NE(inst->id(), String(u"grand-piano"))
        << "MIDI 49 must be read from smallTK offset (contentFilePos+offset+76=390), "
        "not zero-padded name area; result must not be Grand Piano fallback";
    delete score;
}

TEST_F(Tst_Instruments, small_tk_key_read_from_correct_offset)
{
    // smallTK layout stores the Key at a distinct offset the reader used to skip; +6 must be read and applied.
    MasterScore* score = readEncoreScore("instruments_small_tk_key6.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->transpose().chromatic, 6)
        << "Key=6 must be read from smallTK offset (contentFilePos+varSize+53=367)";
    delete score;
}

TEST_F(Tst_Instruments, total_size_tk_midi_read_from_content_offset)
{
    // Encore 4.x TK varSize is the TOTAL block size, so the stride equals varSize and MIDI sits at
    // content[60]. Both instruments' programs must be read from there, not the 5.x-layout offset.
    MasterScore* score = readEncoreScore("instruments_total_size_tk_two_instrs.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_GE(static_cast<int>(score->parts().size()), 2)
        << "File has 2 instruments; both must be parsed";
    const Instrument* inst0 = score->parts()[0]->instrument();
    const Instrument* inst1 = score->parts()[1]->instrument();
    ASSERT_NE(inst0, nullptr);
    ASSERT_NE(inst1, nullptr);
    EXPECT_NE(inst0->id(), String(u"grand-piano"))
        << "MIDI=49 must be read from total-size TK content[60]; must not fall back to Grand Piano";
    EXPECT_NE(inst1->id(), String(u"grand-piano"))
        << "MIDI=34 must be read from total-size TK content[60]; must not fall back to Grand Piano";
    delete score;
}

// An empty name on a real TK block is authoritative: the importer must fall back to "Part N" rather than
// probe the formula offset (where unrelated bytes would produce a garbage name).
TEST_F(Tst_Instruments, tk_empty_name_is_authoritative_not_recovered)
{
    MasterScore* score = readEncoreScore("instruments_tk_empty_name_authoritative.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_GE(static_cast<int>(score->parts().size()), 2)
        << "File declares 2 instruments; both must appear as parts";
    EXPECT_EQ(score->parts()[1]->longName(), String(u"Part 2"))
        << "An empty name on a real TK block is authoritative; instrument 1 "
        "must fall back to 'Part 2', not recover garbage from the formula offset";
    delete score;
}

TEST_F(Tst_Instruments, no_tk_blocks_reads_midi_and_key_from_large_tk_offsets)
{
    // instruments_no_tk_blocks_midi_key.enc: TK00 magic zeroed, MIDI=69 at 2278, Key=6 at 2255.
    // Expected: oboe template selected (MIDI 68 0-indexed = oboe), transposition +6.
    MasterScore* score = readEncoreScore("instruments_no_tk_blocks_midi_key.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"oboe"))
        << "MIDI=69 (oboe) must be read from large-TK offset 2278 even when TK blocks are absent";
    EXPECT_EQ(inst->transpose().chromatic, 6)
        << "Key=6 must be read from large-TK offset 2255 and applied as transposition";
    delete score;
}

TEST_F(Tst_Instruments, no_tk_blocks_large_tk_layout_reads_all_instrument_names)
{
    // No-TK files with a large-TK layout must use the large stride when recovering names, or the second
    // instrument reads from empty bytes and falls back to "Part 2".
    MasterScore* score = readEncoreScore("instruments_no_tk_large_tk_two_names.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_GE(static_cast<int>(score->parts().size()), 2)
        << "File declares 2 instruments; both must appear as parts";
    const QString name0 = score->parts()[0]->longName().toQString();
    const QString name1 = score->parts()[1]->longName().toQString();
    EXPECT_EQ(name0, QString("Oboe"))
        << "Instrument 0 name must be read from NAME_BASE=202 with step=2158";
    EXPECT_EQ(name1, QString("Cello"))
        << "Instrument 1 name must be read from NAME_BASE+2158=2360 (not 202+112=314)";
    delete score;
}

// A MIDI program in the GM percussive range with a name matching no template must route to drumset,
// not fall back to Grand Piano.
TEST_F(Tst_Instruments, gm_perc_range_midi_program_routes_to_drumset)
{
    MasterScore* score = readEncoreScore("instruments_gm_perc_range_taiko.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"drumset"))
        << "MIDI program 116 (Taiko Drum, GM Percussive range 113-128) must route "
        "to drumset even when the instrument name matches nothing";
    EXPECT_NE(inst->drumset(), nullptr)
        << "Drumset instrument must carry a drumset object";
    delete score;
}

// MIDI lookup must use only each template's primary channel, so MIDI 44 (Tremolo Strings) does not match
// acoustic-bass via its tremolo secondary channel.
TEST_F(Tst_Instruments, midi44_does_not_resolve_to_acoustic_bass_via_tremolo_channel)
{
    MasterScore* score = readEncoreScore("instruments_compact_no_tk_midi_oboe.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"oboe"))
        << "MIDI 69 must resolve to oboe; acoustic-bass must not match via tremolo channel";
    delete score;
}

// Trailing punctuation must be stripped from the name needle so "Bandurr. I" matches Bandurria.
TEST_F(Tst_Instruments, abbreviated_name_with_trailing_dot_matches_bandurria)
{
    MasterScore* score = readEncoreScore("instruments_abbreviated_name_bandurr.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts().front()->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->id(), String(u"bandurria"))
        << "Name 'Bandurr. I' must resolve to bandurria after punctuation stripping";
    delete score;
}

TEST_F(Tst_Instruments, orchestra_sanity_check)
{
    MasterScore* score = readEncoreScore("kordorkestro.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "kordorkestro should pass sanityCheck: " << ret.text();
    delete score;
}

// Tokenizer: a trailing ordinal after a non-space separator ("Trumpet-1") must be stripped so
// the base name "Trumpet" still matches. MIDI 41 (Violin) is the wrong answer if it is not.
TEST_F(Tst_Instruments, instrument_name_trailing_number_after_dash_stripped)
{
    MasterScore* score = readEncoreScore("instruments_name_trailing_number.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts()[0]->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_TRUE(inst->id().contains(String(u"trumpet")))
        << "\"Trumpet-1\" must match a Trumpet (trailing \"-1\" stripped), not the MIDI fallback; got "
        << inst->id().toStdString();
    delete score;
}

// Tokenizer: words split on '-' (not only spaces), so "French-Horn" yields the needle "horn"
// and matches the Horn template. MIDI 41 (Violin) is the wrong answer if the split fails.
TEST_F(Tst_Instruments, instrument_name_splits_on_dash_separator)
{
    MasterScore* score = readEncoreScore("instruments_name_dash_separator.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts()[0]->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_TRUE(inst->id().contains(String(u"horn")))
        << "\"French-Horn\" must split on '-' and match a Horn, not the MIDI fallback; got "
        << inst->id().toStdString();
    delete score;
}

// Weak (substring-only) name match must not let a treble bugle sharing the tuba's MIDI program
// outrank the GM instrument. "Contrabass" + MIDI 59 (Tuba) resolves to a bass-clef instrument,
// not the treble "Contrabass Bugle". Mirrors the Spanish "Bajo" -> "Clarín contrabajo" case.
TEST_F(Tst_Instruments, instrument_weak_substring_name_defers_to_midi)
{
    MasterScore* score = readEncoreScore("instruments_weak_name_defers_to_midi.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    const Instrument* inst = score->parts()[0]->instrument();
    ASSERT_NE(inst, nullptr);
    EXPECT_NE(inst->id(), String(u"contrabass-bugle"))
        << "weak substring name match must not pick the treble bugle";
    EXPECT_TRUE(inst->id().contains(String(u"tuba")))
        << "must defer to MIDI program 59 (Tuba); got " << inst->id().toStdString();
    delete score;
}

// Oboe with keyTransposeSemitones=5: instrument transposition must be chromatic=5, diatonic=3.
TEST_F(Tst_Instruments, key_transposition_non_octave_oboe)
{
    MasterScore* score = readEncoreScore("importer_transp_oboe_jota.enc");
    ASSERT_NE(score, nullptr);
    Staff* staff = score->staff(0);
    ASSERT_NE(staff, nullptr);
    const Interval iv = staff->part()->instrument()->transpose();
    EXPECT_EQ(iv.chromatic, 5);
    EXPECT_EQ(iv.diatonic, 3);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    delete score;
}

// ===========================================================================
// FIX: the importer never rebuilt the MIDI mapping, so every part's channel
// stayed at -1. Reading the score into an .mscz hides this (the file read path
// rebuilds the mapping on load), but exporting straight to MusicXML calls
// Part::midiPort(), which indexes m_midiMapping[-1] and crashes. After import,
// every part must own a valid (>= 0) MIDI channel and a safe MIDI port.
// ===========================================================================
TEST_F(Tst_Instruments, midi_mapping_is_built_for_every_part)
{
    MasterScore* score = readEncoreScore("instruments_compact_tk_ignores_key_byte.enc");
    ASSERT_NE(score, nullptr);
    ASSERT_FALSE(score->parts().empty());
    for (const Part* part : score->parts()) {
        const Instrument* inst = part->instrument();
        ASSERT_NE(inst, nullptr);
        EXPECT_GE(inst->channel(0)->channel(), 0)
            << "import must assign a MIDI channel; -1 makes midiPort() index out of bounds";
        EXPECT_GE(part->midiPort(), 0)
            << "midiPort() must be valid so MusicXML export does not crash";
    }
    delete score;
}

// Name-confidence matcher tests, calling the matcher directly. A confident match (exact, unique, or an
// unambiguous normalized/plural/fuzzy match) must be flagged confident so the weak-name -> MIDI override
// does not discard it for a placeholder program.
TEST_F(Tst_Instruments, plural_name_depluralized_and_unique)
{
    using namespace mu::iex::enc;
    bool exact = false, unique = false;
    // A plural name must collapse to its singular stem and match the unique template confidently.
    const InstrumentTemplate* t = findEncoreInstrumentTemplate(
        QStringLiteral("Bandurrias"), -1, ENC_KEY_NO_FILTER, &exact, &unique);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->id, String(u"bandurria"));
    EXPECT_TRUE(exact || unique)
        << "a depluralized stem must match the Bandurria template confidently, not weakly";
}

TEST_F(Tst_Instruments, attached_digit_name_normalized)
{
    using namespace mu::iex::enc;
    bool exact = false, unique = false;
    // "Bandurria1" (digit attached, no separator) must normalize to "Bandurria".
    const InstrumentTemplate* t = findEncoreInstrumentTemplate(
        QStringLiteral("Bandurria1"), -1, ENC_KEY_NO_FILTER, &exact, &unique);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->id, String(u"bandurria"));
    EXPECT_TRUE(exact || unique) << "attached part number must be stripped to a confident match";
}

TEST_F(Tst_Instruments, fuzzy_unique_name_reported_unique)
{
    using namespace mu::iex::enc;
    bool exact = false, unique = false;
    // "Acordeon" fuzzy-matches only "accordion"; a single close template is confident.
    const InstrumentTemplate* t = findEncoreInstrumentTemplate(
        QStringLiteral("Acordeon"), -1, ENC_KEY_NO_FILTER, &exact, &unique);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->id, String(u"accordion"));
    EXPECT_TRUE(unique) << "a fuzzy match that is the only template above threshold must be unique";
}

TEST_F(Tst_Instruments, ambiguous_name_still_not_unique)
{
    using namespace mu::iex::enc;
    bool exact = false, unique = false;
    // Regression guard: "Guitarra" contains-matches many guitar templates, so it must stay
    // non-unique and keep deferring to the MIDI program (Classical Guitar via MIDI 25).
    const InstrumentTemplate* t = findEncoreInstrumentTemplate(
        QStringLiteral("Guitarra"), -1, ENC_KEY_NO_FILTER, &exact, &unique);
    ASSERT_NE(t, nullptr);
    EXPECT_FALSE(exact);
    EXPECT_FALSE(unique) << "an ambiguous substring name must remain weak so MIDI can correct it";
}

// Class B: a configured MIDI program that no template carries as its primary sound must fall
// back to the nearest template in the same General MIDI family, not to Grand Piano.
TEST_F(Tst_Instruments, gm_family_fallback_for_unmapped_program)
{
    using namespace mu::iex::enc;
    constexpr int kPizzicatoStrings0 = 45;   // GM 46, 0-indexed; Strings family (40..47)
    EXPECT_EQ(findTemplateByMidi(kPizzicatoStrings0), nullptr)
        << "precondition: no template has Pizzicato Strings as its primary program";
    const InstrumentTemplate* t = findTemplateByMidiFamily(kPizzicatoStrings0);
    ASSERT_NE(t, nullptr);
    ASSERT_FALSE(t->channel.empty());
    const int prog = t->channel.front().program();
    EXPECT_GE(prog, 40);
    EXPECT_LE(prog, 47) << "family fallback must stay within the Strings family, not Grand Piano";
}

// Regression: SCO5 (big-endian macOS Encore 5) frames every block's size big-endian
// except the TK instrument blocks, whose size is little-endian ("70 00 00 00" = 112).
// Reading it big-endian and masking to 16 bits yielded 0, so the name-scan loop ran
// zero times and only the first instrument name survived (recovered by position); the
// rest imported as "Part N". The importer must undo the big-endian read of the TK size.
TEST_F(Tst_Instruments, sco5_tk_block_instrument_names)
{
    MasterScore* score = readEncoreScore("instruments_sco5_tk_names.enc");
    ASSERT_NE(score, nullptr) << "Failed to load instruments_sco5_tk_names.enc";
    ASSERT_EQ(score->parts().size(), 2u) << "expected 2 instruments";
    EXPECT_EQ(score->parts().at(0)->longName(), String(u"CORNETA 1"));
    EXPECT_EQ(score->parts().at(1)->longName(), String(u"TROMPETA 2"))
        << "second TK-block name must import, not fall back to a default";
    delete score;
}
