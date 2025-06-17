/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "io/file.h"

#include "engraving/tests/utils/scorerw.h"
#include "engraving/tests/utils/scorecomp.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/excerpt.h"

#include "iengravingconfiguration.h"

using namespace mu;
using namespace mu::engraving;

static const String GUITARPRO_DIR(u"guitarbendimporter_data/");

namespace mu::iex::guitarpro {
extern Err importGTP(MasterScore*, muse::io::IODevice* io, const muse::modularity::ContextPtr& iocCtx, bool createLinkedTabForce = false,
                     bool experimental = false);
class GuitarBendImporter_Tests : public ::testing::Test, public muse::Injectable
{
public:
    muse::Inject<mu::engraving::IEngravingConfiguration> engravingConfiguration = { this };

    GuitarBendImporter_Tests();
    void SetUp() override;
    void TearDown() override;
    void gpReadTest(const String& folderName, const String& extension);
};

GuitarBendImporter_Tests::GuitarBendImporter_Tests()
    : muse::Injectable(muse::modularity::globalCtx())
{
}

void GuitarBendImporter_Tests::SetUp()
{
    engravingConfiguration()->setExperimentalGuitarBendImport(true);
}

void GuitarBendImporter_Tests::TearDown()
{
    engravingConfiguration()->setExperimentalGuitarBendImport(false);
}

void GuitarBendImporter_Tests::gpReadTest(const String& fileName, const String& extension)
{
    String gpFileName = GUITARPRO_DIR + fileName + u"." + extension;
    String refFileName = GUITARPRO_DIR + fileName + u"-" + extension + u".mscx";

    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> Err {
        muse::io::File file(path);
        return importGTP(score, &file, muse::modularity::globalCtx(), true, true);
    };

    MasterScore* score = ScoreRW::readScore(gpFileName, false, importFunc);
    EXPECT_TRUE(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"guitarbend-" + fileName + u"-" + extension + u".mscx", refFileName));

    delete score;
}

TEST_F(GuitarBendImporter_Tests, gpSlightBend) {
    gpReadTest(u"slight_bend", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gp5SlightBend) {
    gpReadTest(u"slight_bend", u"gp5");
}

TEST_F(GuitarBendImporter_Tests, gpPrebend) {
    gpReadTest(u"prebend", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gp5Prebend) {
    gpReadTest(u"prebend", u"gp5");
}

// TODO: primary staff is ok, in linked staff notes are mixed
TEST_F(GuitarBendImporter_Tests, DISABLED_gpTiedBendChord) {
    gpReadTest(u"tied_bend_chord", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpSlightBendChord) {
    gpReadTest(u"slight_bend_chord", u"gp");
}

// TODO: primary staff is ok, in linked staff notes are mixed
TEST_F(GuitarBendImporter_Tests, DISABLED_gpPrebendChord) {
    gpReadTest(u"prebend_chord", u"gp");
}

#ifdef SPLIT_CHORD_DURATIONS
TEST_F(GuitarBendImporter_Tests, gpBend_1) {
    gpReadTest(u"bend_1", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpBend_2) {
    gpReadTest(u"bend_2", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpBend_3) {
    gpReadTest(u"bend_3", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpBend_4) {
    gpReadTest(u"bend_4", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpBendRelease_1) {
    gpReadTest(u"bend+release_1", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpPrebendBend) {
    gpReadTest(u"prebend+bend", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpTiedBend_1) {
    gpReadTest(u"tied_bend_1", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpTiedBend_2) {
    gpReadTest(u"tied_bend_2", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpTiedPrebendBend) {
    gpReadTest(u"tied_prebend+bend", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpBendRelease2Notes) {
    gpReadTest(u"bend_release_2_notes", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpBendRelease3Notes) {
    gpReadTest(u"bend_release_3_notes", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpBendsCombinations) {
    gpReadTest(u"bends_combinations", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpTuples1Bend) {
    gpReadTest(u"tuples_1_bend", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpTuplesTiedBends) {
    gpReadTest(u"tuples_tied_bends", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpDottedDurations) {
    gpReadTest(u"dotted_durations", u"gp");
}
#else
TEST_F(GuitarBendImporter_Tests, gpSimpleBend) {
    gpReadTest(u"simple_bend", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpBendRelease) {
    gpReadTest(u"bend_release", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpPrebendBend) {
    gpReadTest(u"prebend_bend", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpSimpleBendChord) {
    gpReadTest(u"simple_bend_chord", u"gp");
}

TEST_F(GuitarBendImporter_Tests, gpBendsTied1) {
    gpReadTest(u"bends_tied_1", u"gp");
}
TEST_F(GuitarBendImporter_Tests, gpBendsTied2) {
    gpReadTest(u"bends_tied_2", u"gp");
}
TEST_F(GuitarBendImporter_Tests, gpBendsTied3) {
    gpReadTest(u"bends_tied_3", u"gp");
}
TEST_F(GuitarBendImporter_Tests, gpBendHold) {
    gpReadTest(u"bend_hold", u"gp");
}
#endif
}
