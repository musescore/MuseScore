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

#include <gtest/gtest.h>

#include "engraving/tests/utils/scorerw.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"

using namespace mu;
using namespace mu::engraving;

extern engraving::Err importOve(MasterScore*, const QString& name);

static const String OVE_DIR(u"data/");

class Ove_Tests : public ::testing::Test
{
public:
    MasterScore* oveRead(const char* file, engraving::Err& err);
};

//---------------------------------------------------------
//   collect
//   every element of the given type in the score
//---------------------------------------------------------

static std::vector<EngravingItem*> collect(Score* score, ElementType type)
{
    std::vector<EngravingItem*> found;
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        for (Segment* s = m->first(); s; s = s->next()) {
            for (EngravingItem* e : s->elist()) {
                if (e && e->type() == type) {
                    found.push_back(e);
                }
            }
            for (EngravingItem* e : s->annotations()) {
                if (e && e->type() == type) {
                    found.push_back(e);
                }
            }
        }
    }
    return found;
}

//---------------------------------------------------------
//   oveRead
//   read an Overture file and report the error importOve() rejects it with;
//   returns nullptr when it is rejected
//---------------------------------------------------------

MasterScore* Ove_Tests::oveRead(const char* file, engraving::Err& err)
{
    auto importFunc = [&err](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        err = importOve(score, path.toQString());
        return err;
    };

    return ScoreRW::readScore(OVE_DIR + String::fromUtf8(file) + u".ove", false, importFunc);
}

// The two unpatched files the malformed fixtures below are derived from. Also this
// module's first tests of any kind, so they pin that plain Overture 4 files still import.
TEST_F(Ove_Tests, oveCondEnding) {
    engraving::Err err = engraving::Err::UnknownError;
    MasterScore* score = oveRead("cond-ending", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, engraving::Err::NoError);
    EXPECT_GT(score->nstaves(), 0u);
    EXPECT_GT(collect(score, ElementType::CHORD).size(), 0u);
}

// This one carries no notes -- it exists to exercise bar numbering -- so it is measured by
// its measures rather than its chords.
TEST_F(Ove_Tests, oveCondBarNumber) {
    engraving::Err err = engraving::Err::UnknownError;
    MasterScore* score = oveRead("cond-bar-number", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, engraving::Err::NoError);
    EXPECT_GT(score->nstaves(), 0u);
    EXPECT_TRUE(score->firstMeasure());
}

// Each glissando is kept by the note it starts on and ends on the note the file names, also
// when that note is in a later measure.
TEST_F(Ove_Tests, oveGlissando) {
    engraving::Err err = engraving::Err::UnknownError;
    MasterScore* score = oveRead("glissando", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, engraving::Err::NoError);

    struct Expected {
        Fraction tick;
        int pitch;
        Fraction tick2;
        int pitch2;
    };
    const std::vector<Expected> expected = {
        { Fraction(1, 4), 60, Fraction(3, 4), 84 },
        { Fraction(1, 1), 64, Fraction(5, 2), 86 },
        { Fraction(2, 1), 72, Fraction(11, 4), 84 },
    };

    std::vector<Glissando*> glissandos;
    for (EngravingItem* e : collect(score, ElementType::CHORD)) {
        for (Note* note : toChord(e)->notes()) {
            for (Spanner* sp : note->spannerFor()) {
                if (sp->isGlissando()) {
                    glissandos.push_back(toGlissando(sp));
                }
            }
        }
    }
    ASSERT_EQ(glissandos.size(), expected.size());

    for (size_t i = 0; i < expected.size(); ++i) {
        Glissando* g = glissandos[i];
        ASSERT_TRUE(g->startElement() && g->startElement()->isNote());
        ASSERT_TRUE(g->endElement() && g->endElement()->isNote());
        const Note* startNote = toNote(g->startElement());
        const Note* endNote = toNote(g->endElement());

        EXPECT_EQ(g->ownershipParent(), startNote);
        EXPECT_TRUE(muse::contains(endNote->spannerBack(), static_cast<Spanner*>(g)));

        EXPECT_EQ(g->tick(), expected[i].tick);
        EXPECT_EQ(startNote->pitch(), expected[i].pitch);
        EXPECT_EQ(g->tick2(), expected[i].tick2);
        EXPECT_EQ(endNote->pitch(), expected[i].pitch2);
        EXPECT_EQ(g->track(), startNote->track());
        EXPECT_EQ(g->track2(), endNote->track());

        EXPECT_EQ(g->glissandoType(), GlissandoType::STRAIGHT);
        EXPECT_EQ(g->text(), u"gliss.");
    }
}

//---------------------------------------------------------
//   malformed files
//
//   Each file below is a documented byte patch of cond-ending.ove. The Overture container
//   is a chain of name(4) + size(4) chunks whose data blocks are parsed by BarsParse, so
//   a chunk size or an item size is all it takes to reach the guards under test.
//---------------------------------------------------------

// BarsParse::parseCond(): an item whose declared size of 3 underflows both the 11 byte and
// the 7 byte header subtraction, wrapping to roughly 4 billion. The item is a Tempo, whose
// parser ignores the length it is given, so without the guard the file is accepted.
TEST_F(Ove_Tests, oveCondBlockUnderflow) {
    engraving::Err err = engraving::Err::UnknownError;
    EXPECT_FALSE(oveRead("cond-block-underflow", err));
    EXPECT_EQ(err, engraving::Err::FileUnknownError);
}

// BarsParse::parseBdat(): the same underflow, on the 7 byte header subtraction. The item is
// a Note, and parseNoteRest()'s `while (cursor < length + 1)` simply skips a negative
// length, so without the guard the file is accepted with the note silently dropped.
TEST_F(Ove_Tests, oveBdatBlockUnderflow) {
    engraving::Err err = engraving::Err::UnknownError;
    EXPECT_FALSE(oveRead("bdat-block-underflow", err));
    EXPECT_EQ(err, engraving::Err::FileUnknownError);
}
