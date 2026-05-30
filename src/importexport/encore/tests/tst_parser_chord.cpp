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

// Unit tests for the numeric chord decoding in EncChordSym::chordName() and EncMidiCc::read(),
// exercised in isolation by building the parser structs directly. See ENCORE_FORMAT.md §CHORD symbol element.

#include <gtest/gtest.h>

#include <QByteArray>
#include <QDataStream>

#include "../internal/parser/elem.h"

using namespace mu::iex::enc;

// Helper: build a numeric-only chord (tipo bit 0 clear = no text).
static EncChordSym makeNumeric(quint8 toniko, quint8 radiko, quint8 baso = 0, quint8 tipo = 0)
{
    EncChordSym cs;
    cs.toniko = toniko;
    cs.tipo   = tipo;
    cs.radiko = radiko;
    cs.baso   = baso;
    return cs;
}

// Helper: build a text-mode chord (tipo bit 0 set).
static EncChordSym makeText(const QString& teksto)
{
    EncChordSym cs;
    cs.tipo   = 1;
    cs.teksto = teksto;
    return cs;
}

// Root note encoding (radiko).

TEST(Tst_EncChordSym, all_natural_roots)
{
    EXPECT_EQ(makeNumeric(0, 0x00).chordName(), "C");
    EXPECT_EQ(makeNumeric(0, 0x01).chordName(), "D");
    EXPECT_EQ(makeNumeric(0, 0x02).chordName(), "E");
    EXPECT_EQ(makeNumeric(0, 0x03).chordName(), "F");
    EXPECT_EQ(makeNumeric(0, 0x04).chordName(), "G");
    EXPECT_EQ(makeNumeric(0, 0x05).chordName(), "A");
    EXPECT_EQ(makeNumeric(0, 0x06).chordName(), "B");
}

TEST(Tst_EncChordSym, sharp_roots)
{
    EXPECT_EQ(makeNumeric(0, 0x10).chordName(), "C#");
    EXPECT_EQ(makeNumeric(0, 0x14).chordName(), "G#");
    EXPECT_EQ(makeNumeric(0, 0x15).chordName(), "A#");
}

TEST(Tst_EncChordSym, flat_roots)
{
    EXPECT_EQ(makeNumeric(0, 0x21).chordName(), "Db");
    EXPECT_EQ(makeNumeric(0, 0x22).chordName(), "Eb");
    EXPECT_EQ(makeNumeric(0, 0x26).chordName(), "Bb");
}

TEST(Tst_EncChordSym, invalid_root_returns_empty)
{
    EXPECT_TRUE(makeNumeric(0, 0x07).chordName().isEmpty()) << "radiko=7 has no note name";
    EXPECT_TRUE(makeNumeric(0, 0x0F).chordName().isEmpty()) << "radiko=0x0F has no note name";
    EXPECT_TRUE(makeNumeric(0, 0x17).chordName().isEmpty()) << "sharp + invalid nibble";
}

// Chord quality (toniko index into kChordQuality[]).

TEST(Tst_EncChordSym, major_no_suffix)
{
    EXPECT_EQ(makeNumeric(0, 0x00).chordName(), "C");
    EXPECT_EQ(makeNumeric(0, 0x05).chordName(), "A");
}

TEST(Tst_EncChordSym, minor)
{
    EXPECT_EQ(makeNumeric(1, 0x05).chordName(), "Am");
    EXPECT_EQ(makeNumeric(1, 0x00).chordName(), "Cm");
}

TEST(Tst_EncChordSym, augmented)
{
    EXPECT_EQ(makeNumeric(2, 0x00).chordName(), "C+");
}

TEST(Tst_EncChordSym, diminished)
{
    EXPECT_EQ(makeNumeric(3, 0x00).chordName(), "Cdim");
}

TEST(Tst_EncChordSym, diminished7_index4)
{
    // toniko=4 is diminished 7 in Encore's palette (not dominant 7).
    EXPECT_EQ(makeNumeric(4, 0x03).chordName(), "Fdim7");
}

TEST(Tst_EncChordSym, dominant7_index24)
{
    EXPECT_EQ(makeNumeric(24, 0x03).chordName(), "F7");
}

TEST(Tst_EncChordSym, maj7)
{
    EXPECT_EQ(makeNumeric(12, 0x00).chordName(), "Cmaj7");
}

TEST(Tst_EncChordSym, ninth_sharp11_index34)
{
    // toniko=34 is 9(#11) (was mislabeled 11 by an off-by-one in the table).
    EXPECT_EQ(makeNumeric(34, 0x00).chordName(), "C9(#11)");
}

TEST(Tst_EncChordSym, minor7)
{
    EXPECT_EQ(makeNumeric(55, 0x01).chordName(), "Dm7");
}

TEST(Tst_EncChordSym, sus4)
{
    EXPECT_EQ(makeNumeric(47, 0x04).chordName(), "Gsus4");
}

TEST(Tst_EncChordSym, sus2)
{
    EXPECT_EQ(makeNumeric(45, 0x04).chordName(), "Gsus2");
}

TEST(Tst_EncChordSym, seventh_sus4_index48)
{
    // toniko=48 is 7sus4 (the table used to map it to 9sus4).
    EXPECT_EQ(makeNumeric(48, 0x01).chordName(), "D7sus4");
}

TEST(Tst_EncChordSym, half_diminished)
{
    EXPECT_EQ(makeNumeric(57, 0x05).chordName(), "Am7(b5)");
}

TEST(Tst_EncChordSym, out_of_range_toniko_treated_as_major)
{
    // toniko=64 is beyond the 0-63 table; degrades to just the root.
    EXPECT_EQ(makeNumeric(64, 0x00).chordName(), "C");
}

// Bass note (tipo bit 1 = bass present, baso same encoding as radiko).

TEST(Tst_EncChordSym, slash_chord_with_bass)
{
    EncChordSym cs = makeNumeric(0, 0x00, 0x04, 0x02);
    EXPECT_EQ(cs.chordName(), "C/G");
}

TEST(Tst_EncChordSym, slash_chord_minor_with_flat_bass)
{
    EncChordSym cs = makeNumeric(1, 0x05, 0x02, 0x02);
    EXPECT_EQ(cs.chordName(), "Am/E");
}

TEST(Tst_EncChordSym, bass_ignored_when_tipo_bit1_clear)
{
    // baso is set but tipo bit 1 is clear: bass must not appear.
    EncChordSym cs = makeNumeric(0, 0x00, 0x04, 0x00);
    EXPECT_EQ(cs.chordName(), "C");
}

// Text mode (tipo bit 0 set): teksto is returned verbatim.

TEST(Tst_EncChordSym, text_mode_returns_teksto)
{
    EXPECT_EQ(makeText("Am").chordName(), "Am");
    EXPECT_EQ(makeText("G7").chordName(), "G7");
    EXPECT_EQ(makeText("Cmaj7(b5)").chordName(), "Cmaj7(b5)");
}

TEST(Tst_EncChordSym, text_mode_ignores_numeric_fields)
{
    // Even with radiko pointing to a different note, teksto wins.
    EncChordSym cs = makeText("F#m7");
    cs.toniko = 0;
    cs.radiko = 0x00;   // would decode to "C" without text
    EXPECT_EQ(cs.chordName(), "F#m7");
}

TEST(Tst_EncChordSym, empty_teksto_falls_through_to_numeric)
{
    EncChordSym cs;
    cs.tipo   = 0;       // no text flag
    cs.teksto = {};
    cs.toniko = 1;
    cs.radiko = 0x05;    // A minor
    EXPECT_EQ(cs.chordName(), "Am");
}

// EncMidiCc inline MIDI Control Change events. read() is entered at d[3] (caller already consumed
// tick + typeVoice), so the hand-built buffer starts at the size byte.
// See ENCORE_FORMAT.md §MIDI control change (type 11).
TEST(Tst_EncMidiCc, decodes_controller_and_value)
{
    QByteArray bytes;
    bytes.append(char(12));      // d[3] size = 12
    bytes.append(char(0));       // d[4] rawStaff
    bytes.append(char(0xB0));    // d[5] CC marker (channel 0)
    bytes.append(4, char(0));    // d[6..9] zeros
    bytes.append(char(64));      // d[10] controller = sustain pedal
    bytes.append(char(127));     // d[11] value = ON

    QDataStream ds(bytes);
    ds.setByteOrder(QDataStream::LittleEndian);
    EncMidiCc cc(0, static_cast<quint8>(EncElemType::MIDI_CC), 0);
    cc.read(ds);

    EXPECT_EQ(cc.size, 12);
    EXPECT_EQ(cc.controller, 64);
    EXPECT_EQ(cc.value, 127);
}

TEST(Tst_EncMidiCc, short_element_leaves_controller_value_zero)
{
    // A malformed/short element (size < 12) must not read controller/value (stream stays aligned).
    QByteArray bytes;
    bytes.append(char(4));       // d[3] size = 4 (too short)
    bytes.append(char(0));       // d[4] rawStaff
    QDataStream ds(bytes);
    ds.setByteOrder(QDataStream::LittleEndian);
    EncMidiCc cc(0, static_cast<quint8>(EncElemType::MIDI_CC), 0);
    cc.read(ds);

    EXPECT_EQ(cc.controller, 0);
    EXPECT_EQ(cc.value, 0);
}
