/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <QByteArray>

#include "io/buffer.h"
#include "io/file.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/image.h"
#include "engraving/dom/imageStore.h"
#include "engraving/dom/linkedobjects.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tie.h"
#include "engraving/editing/editnote.h"
#include "engraving/editing/transaction/transaction.h"
#include "engraving/infrastructure/mscreader.h"
#include "engraving/infrastructure/mscwriter.h"
#include "engraving/rw/mscsaver.h"
#include "engraving/rw/write/writecontext.h"

#include "utils/scorerw.h"

using namespace muse;
using namespace muse::io;
using namespace mu::engraving;

class Engraving_MsczFileTests : public ::testing::Test
{
public:
};

TEST_F(Engraving_MsczFileTests, MsczFile_WriteRead)
{
    //! CASE Writing and reading multiple datas

    //! GIVEN Some datas

    const ByteArray originScoreData("score");
    const ByteArray originImageData("image");
    const ByteArray originThumbnailData("thumbnail");

    //! DO Write datas
    ByteArray msczData;
    {
        Buffer buf(&msczData);
        MscWriter::Params params;
        params.device = &buf;
        params.filePath = "simple1.mscz";
        params.mode = MscIoMode::Zip;

        MscWriter writer(params);
        writer.open();

        writer.writeScoreFile(originScoreData);
        writer.writeThumbnailFile(originThumbnailData);
        writer.addImageFile(u"image1.png", originImageData);
    }

    //! CHECK Read and compare with origin
    {
        Buffer buf(&msczData);
        MscReader::Params params;
        params.device = &buf;
        params.filePath = "simple1.mscz";
        params.mode = MscIoMode::Zip;

        MscReader reader(params);
        reader.open();

        ByteArray scoreData = reader.readScoreFile();
        EXPECT_EQ(scoreData, originScoreData);

        ByteArray thumbnailData = reader.readThumbnailFile();
        EXPECT_EQ(thumbnailData, originThumbnailData);

        std::vector<String> images = reader.imageFileNames();
        ByteArray imageData = reader.readImageFile(u"image1.png");
        EXPECT_EQ(images.size(), 1);
        EXPECT_EQ(images.at(0), u"image1.png");
        EXPECT_EQ(imageData, originImageData);
    }
}

class Engraving_MsczImagesTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = ScoreRW::readScore(u"parts_data/part-empty-parts.mscx");
        ASSERT_TRUE(m_score);
        ASSERT_GE(m_score->excerpts().size(), 2);
        m_part = m_score->excerpts().front()->excerptScore();
        ASSERT_TRUE(m_part);
        ChordRest* cr = m_part->findCR(Fraction(1, 4), 0);
        ASSERT_TRUE(cr && cr->isChord());
        Note* note = toChord(cr)->upNote();
        m_image = Factory::createImage(note);
        note->add(m_image);
        ASSERT_TRUE(m_image->loadFromFile(ScoreRW::rootPath() + u"/parts_data/schnee.png"));
    }

    void TearDown() override
    {
        delete m_reopened;
        delete m_score;
    }

    MasterScore* m_reopened = nullptr;
    MasterScore* m_score = nullptr;
    Score* m_part = nullptr;
    Image* m_image = nullptr;
};

TEST_F(Engraving_MsczImagesTests, ExportPart)
{
    MscSaver saver(m_score->iocContext());
    for (Score* part : { m_part, m_score->excerpts().at(1)->excerptScore() }) {
        ByteArray data;
        Buffer buffer(&data);
        MscWriter::Params params;
        params.device = &buffer;
        params.filePath = "part-image-export.mscz";
        MscWriter writer(params);
        ASSERT_TRUE(writer.open());
        ASSERT_TRUE(saver.exportPart(part, writer));
        writer.close();

        Buffer readBuffer(&data);
        MscReader::Params readParams;
        readParams.device = &readBuffer;
        readParams.filePath = params.filePath;
        MscReader reader(readParams);
        ASSERT_TRUE(reader.open());
        if (part == m_part) {
            ASSERT_EQ(reader.imageFileNames().size(), 1);
            EXPECT_EQ(reader.readImageFile(String::fromStdString(m_image->storeItem()->hashName())), m_image->storeItem()->buffer());
        } else {
            EXPECT_TRUE(reader.imageFileNames().empty());
        }

        const String exported(u"part-image-export.mscz");
        ASSERT_TRUE(File::writeFile(exported, data));
        m_reopened = ScoreRW::readScore(exported, true);
        ASSERT_TRUE(m_reopened);
        EXPECT_EQ(m_reopened->nstaves(), part->nstaves());
        if (part == m_part) {
            ChordRest* chord = m_reopened->findCR(Fraction(1, 4), 0);
            ASSERT_TRUE(chord && chord->isChord());
            Note* note = toChord(chord)->upNote();
            ASSERT_TRUE(note);
            ASSERT_EQ(note->el().size(), 1);
            ASSERT_TRUE(note->el().front()->isImage());
            EXPECT_TRUE(toImage(note->el().front())->isValid());
        }
        delete m_reopened;
        m_reopened = nullptr;
    }
}

TEST_F(Engraving_MsczImagesTests, SavePartOnlyImage)
{
    ByteArray data;
    Buffer buffer(&data);
    MscWriter::Params params;
    params.device = &buffer;
    params.filePath = "part-only-image.mscz";
    MscWriter writer(params);
    ASSERT_TRUE(writer.open());
    MscSaver saver(m_score->iocContext());
    ASSERT_TRUE(saver.writeMscz(m_score, writer, false));
    writer.close();

    Buffer readBuffer(&data);
    MscReader::Params readParams;
    readParams.device = &readBuffer;
    readParams.filePath = params.filePath;
    MscReader reader(readParams);
    ASSERT_TRUE(reader.open());
    EXPECT_EQ(reader.excerptFileNames().size(), m_score->excerpts().size());
    ASSERT_EQ(reader.imageFileNames().size(), 1);
    EXPECT_EQ(reader.readImageFile(String::fromStdString(m_image->storeItem()->hashName())), m_image->storeItem()->buffer());
}

TEST_F(Engraving_MsczImagesTests, SaveRangeExcludesPartImages)
{
    write::WriteContext context(m_score);
    context.setRange({ m_score->firstMeasure(), nullptr, 0, m_score->nstaves() });
    ByteArray data;
    Buffer buffer(&data);
    MscWriter::Params params;
    params.device = &buffer;
    params.filePath = "image-range.mscz";
    MscWriter writer(params);
    ASSERT_TRUE(writer.open());
    MscSaver saver(m_score->iocContext());
    ASSERT_TRUE(saver.writeMscz(m_score, writer, false, &context));
    writer.close();

    Buffer readBuffer(&data);
    MscReader::Params readParams;
    readParams.device = &readBuffer;
    readParams.filePath = params.filePath;
    MscReader reader(readParams);
    ASSERT_TRUE(reader.open());
    EXPECT_TRUE(reader.excerptFileNames().empty());
    EXPECT_TRUE(reader.imageFileNames().empty());
}

class Engraving_MsczPartTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = ScoreRW::readScore(u"parts_data/linked-ties-parts.mscx");
        ASSERT_TRUE(m_score);
        ASSERT_EQ(m_score->excerpts().size(), 1);
        m_part = m_score->excerpts().front()->excerptScore();
        ASSERT_TRUE(m_part);
        ASSERT_EQ(m_part->nstaves(), 2);
    }

    void TearDown() override
    {
        delete m_reopened;
        delete m_score;
    }

    void exportAndReopen()
    {
        ByteArray data;
        Buffer buffer(&data);
        MscWriter::Params params;
        params.device = &buffer;
        params.filePath = "linked-part.mscz";
        MscWriter writer(params);
        ASSERT_TRUE(writer.open());
        MscSaver saver(m_score->iocContext());
        ASSERT_TRUE(saver.exportPart(m_part, writer));
        writer.close();

        const String exported(u"linked-part.mscz");
        ASSERT_TRUE(File::writeFile(exported, data));
        m_reopened = ScoreRW::readScore(exported, true);
        ASSERT_TRUE(m_reopened);
    }

    MasterScore* m_score = nullptr;
    Score* m_part = nullptr;
    MasterScore* m_reopened = nullptr;
};

TEST_F(Engraving_MsczPartTests, ExportPartPreservesInternalLinks)
{
    ASSERT_NO_FATAL_FAILURE(exportAndReopen());
    ASSERT_TRUE(m_part->staff(0)->links());
    EXPECT_EQ(m_part->staff(0)->links()->mainElement(), m_score->staff(0));
    ASSERT_EQ(m_reopened->nstaves(), 2);
    ASSERT_TRUE(m_reopened->staff(0)->links());
    EXPECT_TRUE(m_reopened->staff(0)->links()->contains(m_reopened->staff(1)));

    ChordRest* firstChord = m_reopened->findCR(Fraction(0, 1), 0);
    ChordRest* secondChord = m_reopened->findCR(Fraction(0, 1), 4);
    ASSERT_TRUE(firstChord && firstChord->isChord());
    ASSERT_TRUE(secondChord && secondChord->isChord());
    Note* first = toChord(firstChord)->upNote();
    Note* second = toChord(secondChord)->upNote();
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(first->links());
    EXPECT_TRUE(first->links()->contains(second));
    ASSERT_TRUE(first->tieFor());
    ASSERT_TRUE(second->tieFor());
    ChordRest* firstEnd = m_reopened->findCR(Fraction(1, 4), 0);
    ChordRest* secondEnd = m_reopened->findCR(Fraction(1, 4), 4);
    ASSERT_TRUE(firstEnd && firstEnd->isChord());
    ASSERT_TRUE(secondEnd && secondEnd->isChord());
    EXPECT_EQ(first->tieFor()->endNote(), toChord(firstEnd)->upNote());
    EXPECT_EQ(second->tieFor()->endNote(), toChord(secondEnd)->upNote());
    ASSERT_TRUE(first->tieFor()->links());
    EXPECT_TRUE(first->tieFor()->links()->contains(second->tieFor()));

    const int originalPitch = first->pitch();
    m_reopened->select(first);
    m_reopened->transactionManager()->transaction(TranslatableString::untranslatable("Raise note pitch"), [&](Transaction&) {
        EditNote::upDown(m_reopened, true, UpDownMode::CHROMATIC);
    });
    EXPECT_EQ(second->pitch(), originalPitch + 1);
    m_reopened->undoRedo(true, nullptr);
    EXPECT_EQ(second->pitch(), originalPitch);
    m_reopened->undoRedo(false, nullptr);
    EXPECT_EQ(second->pitch(), originalPitch + 1);
}

TEST_F(Engraving_MsczPartTests, ExportPartOrdersLinkedHairpins)
{
    Hairpin* hairpin = Factory::createHairpin(m_score->dummy());
    hairpin->setTrack(0);
    hairpin->setTrack2(0);
    hairpin->setTick(Fraction(0, 1));
    hairpin->setTicks(Fraction(1, 2));
    m_score->addSpanner(hairpin);
    m_part->transactionManager()->transaction(TranslatableString::untranslatable("Clone linked hairpins"), [&](Transaction&) {
        Excerpt::cloneSpanner(hairpin, m_part, 4, 4);
        Excerpt::cloneSpanner(hairpin, m_part, 0, 0);
    });
    ASSERT_EQ(m_part->spannerMap().map().size(), 2);
    EXPECT_EQ(m_part->spannerMap().map().begin()->second->track(), 4);

    ASSERT_NO_FATAL_FAILURE(exportAndReopen());

    ASSERT_EQ(m_reopened->spannerMap().map().size(), 2);
    for (const auto& entry : m_reopened->spannerMap().map()) {
        Spanner* spanner = entry.second;
        EXPECT_EQ(spanner->type(), ElementType::HAIRPIN);
        EXPECT_EQ(spanner->tick(), Fraction(0, 1));
        EXPECT_EQ(spanner->ticks(), Fraction(1, 2));
        ASSERT_TRUE(spanner->links());
        EXPECT_EQ(spanner->links()->size(), 2);
    }
}
