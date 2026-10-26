/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 */
#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QFile>

#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/timesig.h"
#include "engraving/types/constants.h"
#include "utils/scorerw.h"

using namespace mu::engraving;

//! Old files keep their layout, while the default for newly created scores enables alignment.
TEST(Engraving_TempoTextAlignmentTests, defaultsAndLegacyScore)
{
    EXPECT_TRUE(MStyle().styleB(Sid::tempoAlignToGraceNotes));
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(u"note_data/grace.mscx"));
    ASSERT_NE(score, nullptr);
    EXPECT_FALSE(score->style().styleB(Sid::tempoAlignToGraceNotes));
}

//! Earlier 5.0 files share the current format version but lack the new style tag.
TEST(Engraving_TempoTextAlignmentTests, olderCurrentFormatScore)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(u"note_data/grace.mscx"));
    ASSERT_NE(score, nullptr);
    score->style().set(Sid::tempoAlignToGraceNotes, true);
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const String path = String::fromQString(directory.filePath("older-tempo-alignment.mscx"));
    ASSERT_TRUE(ScoreRW::saveScore(score.get(), path));
    QFile file(path.toQString());
    ASSERT_TRUE(file.open(QIODevice::ReadWrite));
    QByteArray xml = file.readAll();
    const QByteArray tag("<tempoAlignToGraceNotes>1</tempoAlignToGraceNotes>");
    ASSERT_TRUE(xml.contains(tag));
    xml.replace(tag, "");
    ASSERT_TRUE(file.resize(0));
    ASSERT_EQ(file.write(xml), xml.size());
    file.close();
    score.reset(ScoreRW::readScore(path, true));
    ASSERT_NE(score, nullptr);
    EXPECT_EQ(score->mscVersion(), Constants::MSC_VERSION);
    EXPECT_FALSE(score->style().styleB(Sid::tempoAlignToGraceNotes));
}

//! Both style values and per-marking overrides survive saving; reset restores style inheritance.
TEST(Engraving_TempoTextAlignmentTests, saveReloadAndReset)
{
    for (bool styleValue : { false, true }) {
        SCOPED_TRACE(styleValue);
        std::unique_ptr<MasterScore> score(ScoreRW::readScore(u"note_data/grace.mscx"));
        ASSERT_NE(score, nullptr);
        score->style().set(Sid::tempoAlignToGraceNotes, styleValue);
        Segment* segment = score->firstMeasure()->first(SegmentType::ChordRest);
        TempoText* tempo = Factory::createTempoText(segment);
        tempo->setOwnershipParent(segment);
        tempo->setTrack(0);
        tempo->setXmlText(u"Allegro");
        segment->add(tempo);
        EXPECT_EQ(tempo->alignToGraceNotes(), styleValue);
        tempo->setProperty(Pid::TEMPO_ALIGN_TO_GRACE_NOTES, !styleValue);
        tempo->setPropertyFlags(Pid::TEMPO_ALIGN_TO_GRACE_NOTES, PropertyFlags::UNSTYLED);

        QTemporaryDir directory;
        ASSERT_TRUE(directory.isValid());
        const String path = String::fromQString(directory.filePath("tempo-alignment.mscx"));
        ASSERT_TRUE(ScoreRW::saveScore(score.get(), path));
        score.reset(ScoreRW::readScore(path, true));
        ASSERT_NE(score, nullptr);
        EXPECT_EQ(score->style().styleB(Sid::tempoAlignToGraceNotes), styleValue);
        segment = score->firstMeasure()->first(SegmentType::ChordRest);
        tempo = toTempoText(segment->findAnnotation(ElementType::TEMPO_TEXT, 0, 0));
        ASSERT_NE(tempo, nullptr);
        EXPECT_EQ(tempo->alignToGraceNotes(), !styleValue);
        EXPECT_EQ(tempo->propertyFlags(Pid::TEMPO_ALIGN_TO_GRACE_NOTES), PropertyFlags::UNSTYLED);
        tempo->resetProperty(Pid::TEMPO_ALIGN_TO_GRACE_NOTES);
        EXPECT_EQ(tempo->alignToGraceNotes(), styleValue);
    }
}

//! Grace notes on another visible staff affect alignment, but visible time signatures take precedence.
TEST(Engraving_TempoTextAlignmentTests, visibleStavesAndTimeSignature)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(u"note_data/grace.mscx"));
    ASSERT_NE(score, nullptr);
    ASSERT_EQ(score->nstaves(), 2u);
    Segment* segment = score->firstMeasure()->first(SegmentType::ChordRest);
    Chord* chord = toChord(segment->element(0));
    ASSERT_NE(chord, nullptr);
    TempoText* tempo = Factory::createTempoText(segment);
    tempo->setOwnershipParent(segment);
    tempo->setTrack(4);
    tempo->setXmlText(u"Allegro");
    segment->add(tempo);
    EXPECT_FALSE(tempo->graceNoteAlignmentOffset().has_value());
    score->startCmd(muse::TranslatableString::untranslatable("Add grace note"));
    score->setGraceNote(chord, 74, NoteType::APPOGGIATURA, Constants::DIVISION / 2);
    score->endCmd();
    score->doLayout();
    EXPECT_TRUE(tempo->graceNoteAlignmentOffset().has_value());
    ASSERT_NE(tempo->alignmentTimeSignature(), nullptr);
    Segment* timeSigs = segment->prev(SegmentType::TimeSig);
    ASSERT_NE(timeSigs, nullptr);
    timeSigs->element(4)->setVisible(false);
    score->doLayout();
    EXPECT_NE(tempo->alignmentTimeSignature(), nullptr);
    timeSigs->element(0)->setVisible(false);
    score->doLayout();
    EXPECT_EQ(tempo->alignmentTimeSignature(), nullptr);
    tempo->setProperty(Pid::TEMPO_ALIGN_TO_GRACE_NOTES, true);
    score->doLayout();
    const double alignedX = tempo->pos().x();
    tempo->setProperty(Pid::TEMPO_ALIGN_TO_GRACE_NOTES, false);
    score->doLayout();
    EXPECT_GT(tempo->pos().x(), alignedX);
    score->staff(0)->setVisible(false);
    EXPECT_FALSE(tempo->graceNoteAlignmentOffset().has_value());
}
