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
#include <gmock/gmock.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "mpe/events.h"
#include "mpe/mpetypes.h"
#include "mpe/tests/utils/articulationutils.h"
#include "mpe/tests/mocks/articulationprofilesrepositorymock.h"

#include "global/io/buffer.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/segment.h"
#include "engraving/editing/transaction/undostack.h"
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "engraving/infrastructure/mscreader.h"
#include "engraving/infrastructure/mscwriter.h"
#include "engraving/playback/playbackmodel.h"
#include "engraving/playback/practiceemphasis.h"
#include "engraving/playback/practiceemphasisdata.h"
#include "engraving/rw/mscloader.h"
#include "engraving/rw/mscsaver.h"

#include "utils/scorerw.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

using namespace mu::engraving;
using namespace muse::mpe;
using namespace muse;

namespace {
const String SCORE_FILE("playback/playbackmodel_data/practice_emphasis/ssatb-one-bar.mscx");

struct HeardNote {
    int midi = 0;
    staff_layer_idx_t staff = 0;
    voice_layer_idx_t voice = 0;
    dynamic_level_t level = 0;
};

int midiFromPitchLevel(pitch_level_t pitch)
{
    return ZERO_PITCH_LEVEL_MIDI_EQUIVALENT + static_cast<int>(pitch / PITCH_LEVEL_STEP);
}

std::vector<HeardNote> heardNotes(const PlaybackData& data)
{
    std::vector<HeardNote> notes;
    for (const auto& pair : data.originEvents) {
        for (const PlaybackEvent& event : pair.second) {
            if (!std::holds_alternative<muse::mpe::NoteEvent>(event)) {
                continue;
            }
            const muse::mpe::NoteEvent& note = std::get<muse::mpe::NoteEvent>(event);
            HeardNote heard;
            heard.midi = midiFromPitchLevel(note.pitchCtx().nominalPitchLevel);
            heard.staff = note.arrangementCtx().staffLayerIndex;
            heard.voice = note.arrangementCtx().voiceLayerIndex;
            heard.level = note.expressionCtx().nominalDynamicLevel;
            notes.push_back(heard);
        }
    }
    return notes;
}

const HeardNote* findMidi(const std::vector<HeardNote>& notes, int midi)
{
    for (const HeardNote& note : notes) {
        if (note.midi == midi) {
            return &note;
        }
    }
    return nullptr;
}

double curveOut(const DynamicAutomationLayers& layers, layer_idx_t key)
{
    auto it = layers.find(key);
    EXPECT_NE(it, layers.end()) << "missing dynamic layer " << key;
    if (it == layers.end() || it->second.empty()) {
        return 0.0;
    }
    return it->second.begin()->second.outValue;
}

Note* findPitch(Score* score, int pitch)
{
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
            for (track_idx_t track = 0; track < score->ntracks(); ++track) {
                EngravingItem* element = segment->element(track);
                if (!element || !element->isChord()) {
                    continue;
                }
                for (Note* note : toChord(element)->notes()) {
                    if (note->pitch() == pitch) {
                        return note;
                    }
                }
            }
        }
    }
    return nullptr;
}

int displayedMeasureNumber(const Measure* measure)
{
    return measure->measureNumber() + measure->measureNumberOffset() + 1;
}

const PracticeEmphasisData::PartRule* findPart(const std::vector<PracticeEmphasisData::PartRule>& parts, const std::string& id)
{
    for (const PracticeEmphasisData::PartRule& part : parts) {
        if (part.id == id) {
            return &part;
        }
    }
    return nullptr;
}

void expectHeard(const std::vector<HeardNote>& notes, int midi, staff_layer_idx_t staff, voice_layer_idx_t voice, dynamic_level_t level)
{
    const HeardNote* heard = findMidi(notes, midi);
    if (!heard) {
        ADD_FAILURE() << "missing midi " << midi;
        return;
    }
    EXPECT_EQ(heard->staff, staff) << midi;
    EXPECT_EQ(heard->voice, voice) << midi;
    EXPECT_EQ(heard->level, level) << midi;
}

bool writeMscz(MasterScore* score, ByteArray& out)
{
    muse::io::Buffer buffer(&out);
    MscWriter::Params params;
    params.device = &buffer;
    params.filePath = "practice-emphasis-roundtrip.mscz";
    params.mode = MscIoMode::Zip;

    MscWriter writer(params);
    if (!writer.open()) {
        return false;
    }

    MscSaver saver(score->iocContext());
    const bool wrote = saver.writeMscz(score, writer, false);
    writer.close();
    return wrote && !writer.hasError();
}

bool containerHasEmphasis(const ByteArray& mscz, ByteArray* json = nullptr)
{
    ByteArray copy = mscz;
    muse::io::Buffer buffer(&copy);
    MscReader::Params params;
    params.device = &buffer;
    params.filePath = "practice-emphasis-roundtrip.mscz";
    params.mode = MscIoMode::Zip;

    MscReader reader(params);
    if (!reader.open()) {
        return false;
    }
    const ByteArray data = reader.readPracticeEmphasisJsonFile();
    if (data.empty()) {
        return false;
    }
    if (json) {
        *json = data;
    }
    return true;
}
}

class Engraving_PracticeEmphasisTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_defaultProfile = std::make_shared<ArticulationsProfile>();
        m_repositoryMock = std::make_shared<NiceMock<ArticulationProfilesRepositoryMock> >();
        ON_CALL(*m_repositoryMock, defaultProfile(_)).WillByDefault(Return(m_defaultProfile));
    }

    PlaybackData loadPart(MasterScore* score, size_t partIndex)
    {
        const Part* part = score->parts().at(partIndex);
        PlaybackModel model(modularity::globalCtx());
        model.profilesRepository.set(m_repositoryMock);
        model.load(score);
        return model.resolveTrackPlaybackData(part->id(), part->instrumentId());
    }

    ArticulationsProfilePtr m_defaultProfile;
    std::shared_ptr<NiceMock<ArticulationProfilesRepositoryMock> > m_repositoryMock;
};

TEST(Engraving_PracticeEmphasis, DynamicStepMatchesSixDecibels)
{
    const dynamic_level_t mf = dynamicLevelFromType(muse::mpe::DynamicType::mf);
    EXPECT_EQ(PracticeEmphasis::scaleDynamicLevel(mf, 0.f), mf);
    EXPECT_EQ(PracticeEmphasis::scaleDynamicLevel(mf, 6.f), dynamicLevelFromType(muse::mpe::DynamicType::f));
    EXPECT_EQ(PracticeEmphasis::scaleDynamicLevel(mf, -12.f), dynamicLevelFromType(muse::mpe::DynamicType::p));
    EXPECT_EQ(PracticeEmphasis::scaleDynamicLevel(dynamicLevelFromType(muse::mpe::DynamicType::fffffffff), 24.f),
              dynamicLevelFromType(muse::mpe::DynamicType::fffffffff));
    EXPECT_EQ(PracticeEmphasis::scaleDynamicLevel(dynamicLevelFromType(muse::mpe::DynamicType::ppppppppp), -24.f),
              dynamicLevelFromType(muse::mpe::DynamicType::ppppppppp));
}

TEST_F(Engraving_PracticeEmphasisTests, AltoVoiceIsLoudOnTheCompanionLayer)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 2);
    score->initAutomation();

    PlaybackData before = loadPart(score, 0);
    const std::vector<HeardNote> written = heardNotes(before);
    const HeardNote* soprano = findMidi(written, 76);
    const HeardNote* alto = findMidi(written, 67);
    ASSERT_TRUE(soprano);
    ASSERT_TRUE(alto);
    const dynamic_level_t sopranoWritten = soprano->level;
    const dynamic_level_t altoWritten = alto->level;
    const dynamic_level_t loud = PracticeEmphasis::scaleDynamicLevel(altoWritten, PracticeEmphasisData::DEFAULT_EMPHASIZED_DB);
    const dynamic_level_t quiet = PracticeEmphasis::scaleDynamicLevel(sopranoWritten, PracticeEmphasisData::DEFAULT_OTHER_DB);
    const staff_layer_idx_t altoCompanion = static_cast<staff_layer_idx_t>(PracticeEmphasis::EMPHASIS_STAFF_OFFSET);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPlaybackActive(true);
    emphasis.setPartSelected(score, "A1", true);
    emphasis.setPartSelected(score, "A2", true);

    PlaybackData after = loadPart(score, 0);
    const std::vector<HeardNote> heard = heardNotes(after);
    const HeardNote* sopranoAfter = findMidi(heard, 76);
    const HeardNote* altoAfter = findMidi(heard, 67);
    const HeardNote* altoSecond = findMidi(heard, 65);
    ASSERT_TRUE(sopranoAfter);
    ASSERT_TRUE(altoAfter);
    ASSERT_TRUE(altoSecond);

    EXPECT_EQ(sopranoAfter->level, quiet);
    EXPECT_EQ(sopranoAfter->staff, 0);
    EXPECT_EQ(sopranoAfter->voice, 0);
    EXPECT_EQ(altoAfter->level, loud);
    EXPECT_NE(altoAfter->level, altoWritten);
    EXPECT_EQ(altoSecond->level, altoAfter->level);
    EXPECT_EQ(altoAfter->staff, altoCompanion);
    EXPECT_EQ(altoAfter->voice, 1);
    EXPECT_NE(after.dynamics.find(makeLayerIdx(altoCompanion, 1)), after.dynamics.end());
    EXPECT_GT(curveOut(after.dynamics, makeLayerIdx(altoCompanion, 1)), curveOut(after.dynamics, makeLayerIdx(0, 0)));

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, DivisiSplitsTheChordAndKeepsTheSharedUnison)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);
    score->initAutomation();

    PlaybackData writtenWomen = loadPart(score, 0);
    PlaybackData writtenMen = loadPart(score, 1);
    const std::vector<HeardNote> womenWrittenNotes = heardNotes(writtenWomen);
    const std::vector<HeardNote> menWrittenNotes = heardNotes(writtenMen);
    ASSERT_TRUE(findMidi(womenWrittenNotes, 76));
    ASSERT_TRUE(findMidi(menWrittenNotes, 60));
    const dynamic_level_t writtenLevel = findMidi(womenWrittenNotes, 76)->level;
    const dynamic_level_t tenorWritten = findMidi(menWrittenNotes, 60)->level;
    const dynamic_level_t loud = PracticeEmphasis::scaleDynamicLevel(writtenLevel, 6.f);
    const dynamic_level_t quiet = PracticeEmphasis::scaleDynamicLevel(writtenLevel, -12.f);
    const dynamic_level_t tenorQuiet = PracticeEmphasis::scaleDynamicLevel(tenorWritten, -12.f);
    const staff_layer_idx_t companion = static_cast<staff_layer_idx_t>(PracticeEmphasis::EMPHASIS_STAFF_OFFSET);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPlaybackActive(true);
    emphasis.setGains(6.f, -12.f);

    auto run = [&](const char* partId) {
        emphasis.setPartSelected(score, "S1", std::string(partId) == "S1");
        emphasis.setPartSelected(score, "S2", std::string(partId) == "S2");
        return std::pair<PlaybackData, PlaybackData> { loadPart(score, 0), loadPart(score, 1) };
    };

    {
        auto [women, men] = run("S1");
        const std::vector<HeardNote> notes = heardNotes(women);
        const HeardNote* e5 = findMidi(notes, 76);
        const HeardNote* c5 = findMidi(notes, 72);
        const HeardNote* d5 = findMidi(notes, 74);
        const HeardNote* alto = findMidi(notes, 67);
        ASSERT_TRUE(e5);
        ASSERT_TRUE(c5);
        ASSERT_TRUE(d5);
        ASSERT_TRUE(alto);

        EXPECT_EQ(e5->staff, companion);
        EXPECT_EQ(e5->level, loud);
        EXPECT_EQ(d5->staff, companion);
        EXPECT_EQ(d5->level, loud);
        EXPECT_EQ(c5->staff, 0);
        EXPECT_EQ(c5->level, quiet);
        EXPECT_EQ(c5->voice, 0);
        EXPECT_EQ(alto->staff, 0);
        EXPECT_EQ(alto->voice, 1);
        EXPECT_EQ(alto->level, quiet);

        const std::vector<HeardNote> menNotes = heardNotes(men);
        const HeardNote* tenor = findMidi(menNotes, 60);
        const HeardNote* bass = findMidi(menNotes, 48);
        ASSERT_TRUE(tenor);
        ASSERT_TRUE(bass);
        EXPECT_EQ(tenor->staff, 1);
        EXPECT_EQ(tenor->level, tenorQuiet);
        EXPECT_EQ(bass->staff, 1);
        EXPECT_EQ(bass->voice, 1);
        EXPECT_EQ(bass->level, tenorQuiet);

        EXPECT_NE(women.dynamics.find(makeLayerIdx(companion, 0)), women.dynamics.end());
        EXPECT_GT(curveOut(women.dynamics, makeLayerIdx(companion, 0)), curveOut(women.dynamics, makeLayerIdx(0, 0)));
    }

    {
        auto [women, men] = run("S2");
        const std::vector<HeardNote> notes = heardNotes(women);
        const HeardNote* e5 = findMidi(notes, 76);
        const HeardNote* c5 = findMidi(notes, 72);
        const HeardNote* d5 = findMidi(notes, 74);
        ASSERT_TRUE(e5);
        ASSERT_TRUE(c5);
        ASSERT_TRUE(d5);

        // S2 keeps the lower divisi note and the shared unison. S1's E5 stays quiet.
        EXPECT_EQ(c5->staff, companion);
        EXPECT_EQ(c5->level, loud);
        EXPECT_EQ(d5->staff, companion);
        EXPECT_EQ(d5->level, loud);
        EXPECT_EQ(e5->staff, 0);
        EXPECT_EQ(e5->level, quiet);
        EXPECT_EQ(findMidi(notes, 67)->level, quiet);
        const std::vector<HeardNote> menNotes = heardNotes(men);
        ASSERT_TRUE(findMidi(menNotes, 43));
        EXPECT_EQ(findMidi(menNotes, 43)->level, tenorQuiet);
        EXPECT_GT(curveOut(women.dynamics, makeLayerIdx(companion, 0)), curveOut(women.dynamics, makeLayerIdx(0, 0)));
    }

    delete score;
}

TEST(Engraving_PracticeEmphasis, PartTemplatesMapChoirToStaffAndVoice)
{
    const std::vector<PracticeEmphasisData::PartRule> open = PracticeEmphasisData::templatesForStaffCount(4);
    const std::vector<PracticeEmphasisData::PartRule> closed = PracticeEmphasisData::templatesForStaffCount(2);
    const std::vector<PracticeEmphasisData::PartRule> oneStaff = PracticeEmphasisData::templatesForStaffCount(1);
    ASSERT_EQ(open.size(), 8);
    ASSERT_EQ(closed.size(), 8);
    ASSERT_EQ(oneStaff.size(), 8);

    const PracticeEmphasisData::PartRule* openS1 = findPart(open, "S1");
    const PracticeEmphasisData::PartRule* openB2 = findPart(open, "B2");
    ASSERT_TRUE(openS1);
    ASSERT_TRUE(openB2);
    EXPECT_EQ(openS1->staffIndex, 0);
    EXPECT_EQ(openS1->voice, 0);
    EXPECT_EQ(openS1->role, PracticeEmphasisData::Role::Top);
    EXPECT_TRUE(openS1->builtin);
    EXPECT_EQ(openB2->staffIndex, 3);
    EXPECT_EQ(openB2->voice, 0);
    EXPECT_EQ(openB2->role, PracticeEmphasisData::Role::Lower);

    const PracticeEmphasisData::PartRule* closedA1 = findPart(closed, "A1");
    const PracticeEmphasisData::PartRule* closedT2 = findPart(closed, "T2");
    const PracticeEmphasisData::PartRule* closedB1 = findPart(closed, "B1");
    ASSERT_TRUE(closedA1);
    ASSERT_TRUE(closedT2);
    ASSERT_TRUE(closedB1);
    EXPECT_EQ(closedA1->staffIndex, 0);
    EXPECT_EQ(closedA1->voice, 1);
    EXPECT_EQ(closedT2->staffIndex, 1);
    EXPECT_EQ(closedT2->voice, 0);
    EXPECT_EQ(closedT2->role, PracticeEmphasisData::Role::Lower);
    EXPECT_EQ(closedB1->staffIndex, 1);
    EXPECT_EQ(closedB1->voice, 1);

    const PracticeEmphasisData::PartRule* oneB2 = findPart(oneStaff, "B2");
    ASSERT_TRUE(oneB2);
    EXPECT_EQ(oneB2->staffIndex, 0);
    EXPECT_EQ(oneB2->voice, 3);
    EXPECT_TRUE(PracticeEmphasisData::templatesForStaffCount(0).empty());
}

TEST_F(Engraving_PracticeEmphasisTests, UnionKeepsASharedNoteUntilEveryPartIsCleared)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);
    score->initAutomation();
    ASSERT_EQ(displayedMeasureNumber(score->firstMeasure()), 1);

    PlaybackData writtenWomen = loadPart(score, 0);
    PlaybackData writtenMen = loadPart(score, 1);
    const dynamic_level_t writtenLevel = findMidi(heardNotes(writtenWomen), 76)->level;
    const dynamic_level_t tenorWritten = findMidi(heardNotes(writtenMen), 60)->level;
    const dynamic_level_t loud = PracticeEmphasis::scaleDynamicLevel(writtenLevel, PracticeEmphasisData::DEFAULT_EMPHASIZED_DB);
    const dynamic_level_t quiet = PracticeEmphasis::scaleDynamicLevel(writtenLevel, PracticeEmphasisData::DEFAULT_OTHER_DB);
    const dynamic_level_t tenorQuiet = PracticeEmphasis::scaleDynamicLevel(tenorWritten, PracticeEmphasisData::DEFAULT_OTHER_DB);
    const staff_layer_idx_t sopranoCompanion = PracticeEmphasis::EMPHASIS_STAFF_OFFSET;
    const staff_layer_idx_t tenorCompanion = static_cast<staff_layer_idx_t>(1 + PracticeEmphasis::EMPHASIS_STAFF_OFFSET);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPlaybackActive(true);
    emphasis.setGains(PracticeEmphasisData::DEFAULT_EMPHASIZED_DB, PracticeEmphasisData::DEFAULT_OTHER_DB);
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setPartSelected(score, "S2", true);

    {
        const std::vector<HeardNote> women = heardNotes(loadPart(score, 0));
        expectHeard(women, 76, sopranoCompanion, 0, loud);
        expectHeard(women, 72, sopranoCompanion, 0, loud);
        expectHeard(women, 74, sopranoCompanion, 0, loud);
        expectHeard(women, 67, 0, 1, quiet);
        const std::vector<HeardNote> men = heardNotes(loadPart(score, 1));
        expectHeard(men, 60, 1, 0, tenorQuiet);
        expectHeard(men, 48, 1, 1, tenorQuiet);
    }

    // S2 leaves. S1 still sings the top note and the written unison, so those stay loud.
    emphasis.setPartSelected(score, "S2", false);
    {
        const std::vector<HeardNote> women = heardNotes(loadPart(score, 0));
        expectHeard(women, 76, sopranoCompanion, 0, loud);
        expectHeard(women, 74, sopranoCompanion, 0, loud);
        expectHeard(women, 72, 0, 0, quiet);
        EXPECT_TRUE(emphasis.partSelected("S1"));
        EXPECT_FALSE(emphasis.partSelected("S2"));
    }

    emphasis.setPartSelected(score, "S1", false);
    emphasis.setPartSelected(score, "S2", true);
    {
        const std::vector<HeardNote> women = heardNotes(loadPart(score, 0));
        expectHeard(women, 72, sopranoCompanion, 0, loud);
        expectHeard(women, 74, sopranoCompanion, 0, loud);
        expectHeard(women, 76, 0, 0, quiet);
    }

    // A one-note chord belongs to both parts of that voice. Clearing T2 leaves T1's claim.
    emphasis.setPartSelected(score, "S2", false);
    emphasis.setPartSelected(score, "T1", true);
    emphasis.setPartSelected(score, "T2", true);
    {
        const std::vector<HeardNote> men = heardNotes(loadPart(score, 1));
        const dynamic_level_t tenorLoud = PracticeEmphasis::scaleDynamicLevel(tenorWritten, PracticeEmphasisData::DEFAULT_EMPHASIZED_DB);
        expectHeard(men, 60, tenorCompanion, 0, tenorLoud);
        expectHeard(men, 59, tenorCompanion, 0, tenorLoud);
        expectHeard(men, 48, 1, 1, tenorQuiet);
    }
    emphasis.setPartSelected(score, "T2", false);
    {
        const std::vector<HeardNote> men = heardNotes(loadPart(score, 1));
        const dynamic_level_t tenorLoud = PracticeEmphasis::scaleDynamicLevel(tenorWritten, PracticeEmphasisData::DEFAULT_EMPHASIZED_DB);
        expectHeard(men, 60, tenorCompanion, 0, tenorLoud);
        expectHeard(men, 59, tenorCompanion, 0, tenorLoud);
    }

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, EditsAndRangeChangeWhoIsLoud)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);
    score->initAutomation();

    PlaybackData writtenWomen = loadPart(score, 0);
    const dynamic_level_t writtenLevel = findMidi(heardNotes(writtenWomen), 76)->level;
    const dynamic_level_t loud = PracticeEmphasis::scaleDynamicLevel(writtenLevel, 8.f);
    const dynamic_level_t quiet = PracticeEmphasis::scaleDynamicLevel(writtenLevel, -18.f);
    const staff_layer_idx_t companion = PracticeEmphasis::EMPHASIS_STAFF_OFFSET;

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPlaybackActive(true);
    emphasis.setGains(8.f, -18.f);
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setEditingPartId(score, "S1");

    Note* e5 = findPitch(score, 76);
    Note* c5 = findPitch(score, 72);
    Note* d5 = findPitch(score, 74);
    Note* alto = findPitch(score, 67);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(c5);
    ASSERT_TRUE(d5);
    ASSERT_TRUE(alto);
    EXPECT_TRUE(emphasis.isEmphasized(e5));
    EXPECT_TRUE(emphasis.isEmphasized(d5));
    EXPECT_FALSE(emphasis.isEmphasized(c5));
    EXPECT_FALSE(emphasis.isEmphasized(alto));

    emphasis.editNote(d5);
    EXPECT_FALSE(emphasis.isEmphasized(d5));
    const PracticeEmphasisData::PartRule* soprano = findPart(emphasis.document().parts, "S1");
    ASSERT_TRUE(soprano);
    EXPECT_TRUE(std::find(soprano->removed.begin(), soprano->removed.end(), d5->eid().toStdString()) != soprano->removed.end());

    emphasis.editNote(d5);
    EXPECT_TRUE(emphasis.isEmphasized(d5));
    soprano = findPart(emphasis.document().parts, "S1");
    ASSERT_TRUE(soprano);
    EXPECT_TRUE(soprano->removed.empty());

    emphasis.editNote(c5);
    EXPECT_TRUE(emphasis.isEmphasized(c5));
    soprano = findPart(emphasis.document().parts, "S1");
    ASSERT_TRUE(soprano);
    EXPECT_TRUE(std::find(soprano->added.begin(), soprano->added.end(), c5->eid().toStdString()) != soprano->added.end());
    emphasis.resetPartEdits("S1");
    EXPECT_FALSE(emphasis.isEmphasized(c5));
    soprano = findPart(emphasis.document().parts, "S1");
    ASSERT_TRUE(soprano);
    EXPECT_TRUE(soprano->added.empty());
    EXPECT_TRUE(soprano->removed.empty());

    emphasis.editNote(alto);
    {
        const std::vector<HeardNote> women = heardNotes(loadPart(score, 0));
        expectHeard(women, 76, companion, 0, loud);
        expectHeard(women, 74, companion, 0, loud);
        expectHeard(women, 72, 0, 0, quiet);
        expectHeard(women, 67, companion, 1, loud);
        expectHeard(women, 65, 0, 1, quiet);
    }

    emphasis.setRange(2, 2);
    EXPECT_FALSE(emphasis.isEmphasized(e5));
    EXPECT_TRUE(emphasis.isEmphasized(alto));
    {
        const std::vector<HeardNote> women = heardNotes(loadPart(score, 0));
        expectHeard(women, 76, 0, 0, quiet);
        expectHeard(women, 74, 0, 0, quiet);
        expectHeard(women, 67, companion, 1, loud);
    }

    emphasis.setRange(1, 1);
    EXPECT_TRUE(emphasis.isEmphasized(e5));
    emphasis.setRange(0, 0);
    EXPECT_TRUE(emphasis.isEmphasized(e5));

    delete score;
}

TEST(Engraving_PracticeEmphasis, ThreeNoteChordSplitsTopFromTheRestAndAUnisonIsShared)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    ASSERT_TRUE(score);

    Chord* divided = Factory::createChord(score->dummy());
    divided->setTrack(0);
    Note* low = Factory::createNote(divided);
    Note* middle = Factory::createNote(divided);
    Note* high = Factory::createNote(divided);
    low->setPitch(64);
    middle->setPitch(67);
    high->setPitch(72);
    divided->add(low);
    divided->add(middle);
    divided->add(high);

    Chord* unison = Factory::createChord(score->dummy());
    unison->setTrack(0);
    Note* shared = Factory::createNote(unison);
    shared->setPitch(70);
    unison->add(shared);

    Chord* tiedPitch = Factory::createChord(score->dummy());
    tiedPitch->setTrack(0);
    Note* firstTop = Factory::createNote(tiedPitch);
    Note* secondTop = Factory::createNote(tiedPitch);
    firstTop->setPitch(70);
    secondTop->setPitch(70);
    tiedPitch->add(firstTop);
    tiedPitch->add(secondTop);

    PracticeEmphasisData::Document document;
    PracticeEmphasisData::PartRule soprano1;
    soprano1.id = "S1";
    soprano1.staffIndex = 0;
    soprano1.voice = 0;
    soprano1.role = PracticeEmphasisData::Role::Top;
    soprano1.selected = true;
    soprano1.builtin = true;
    PracticeEmphasisData::PartRule soprano2 = soprano1;
    soprano2.id = "S2";
    soprano2.role = PracticeEmphasisData::Role::Lower;
    soprano2.selected = true;
    document.parts = { soprano1, soprano2 };
    score->practiceEmphasis().setDocument(document, false);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    EXPECT_TRUE(emphasis.isEmphasized(high));
    EXPECT_TRUE(emphasis.isEmphasized(middle));
    EXPECT_TRUE(emphasis.isEmphasized(low));
    EXPECT_TRUE(emphasis.isEmphasized(shared));
    EXPECT_TRUE(emphasis.isEmphasized(firstTop));
    EXPECT_TRUE(emphasis.isEmphasized(secondTop));

    emphasis.setPartSelected(score, "S2", false);
    EXPECT_TRUE(emphasis.isEmphasized(high));
    EXPECT_FALSE(emphasis.isEmphasized(middle));
    EXPECT_FALSE(emphasis.isEmphasized(low));
    EXPECT_TRUE(emphasis.isEmphasized(shared));
    EXPECT_TRUE(emphasis.isEmphasized(firstTop));
    EXPECT_FALSE(emphasis.isEmphasized(secondTop));

    emphasis.setPartSelected(score, "S1", false);
    emphasis.setPartSelected(score, "S2", true);
    EXPECT_FALSE(emphasis.isEmphasized(high));
    EXPECT_TRUE(emphasis.isEmphasized(middle));
    EXPECT_TRUE(emphasis.isEmphasized(low));
    EXPECT_TRUE(emphasis.isEmphasized(shared));
    EXPECT_FALSE(emphasis.isEmphasized(firstTop));
    EXPECT_TRUE(emphasis.isEmphasized(secondTop));

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, SidecarRoundTripsInsideTheScoreAndLeavesNotationAlone)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);
    score->initAutomation();
    ASSERT_TRUE(score->practiceEmphasis().isDefault());

    ByteArray untouched;
    ASSERT_TRUE(writeMscz(score, untouched));
    EXPECT_FALSE(containerHasEmphasis(untouched));

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPlaybackActive(true);
    emphasis.setEmphasisMode(true);
    emphasis.setGains(8.f, -18.f);
    emphasis.setRange(1, 1);
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setPartSelected(score, "S2", true);
    emphasis.setPartSelected(score, "S2", false);

    Note* d5 = findPitch(score, 74);
    Note* alto = findPitch(score, 67);
    Note* e5 = findPitch(score, 76);
    ASSERT_TRUE(d5);
    ASSERT_TRUE(alto);
    ASSERT_TRUE(e5);
    const auto d5Color = d5->color();
    emphasis.setEditingPartId(score, "S1");
    emphasis.editNote(d5);
    emphasis.editNote(alto);
    EXPECT_EQ(d5->color(), d5Color);
    EXPECT_FALSE(emphasis.isDefault());

    PracticeEmphasisData::Document stored = emphasis.document();
    PracticeEmphasisData parsed;
    EXPECT_TRUE(parsed.fromJson(emphasis.toJson()));
    EXPECT_EQ(parsed.document(), stored);

    const muse::ByteArray garbage("not-json");
    EXPECT_FALSE(parsed.fromJson(garbage));
    EXPECT_EQ(parsed.document(), stored);

    ByteArray mscz;
    ASSERT_TRUE(writeMscz(score, mscz));
    ByteArray json;
    ASSERT_TRUE(containerHasEmphasis(mscz, &json));
    PracticeEmphasisData fromContainer;
    EXPECT_TRUE(fromContainer.fromJson(json));
    EXPECT_EQ(fromContainer.document(), stored);

    MasterScore* reloaded = compat::ScoreAccess::createMasterScoreWithBaseStyle(score->iocContext());
    reloaded->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>("practice-emphasis-roundtrip.mscz"));
    {
        muse::io::Buffer buffer(&mscz);
        MscReader::Params params;
        params.device = &buffer;
        params.filePath = "practice-emphasis-roundtrip.mscz";
        params.mode = MscIoMode::Zip;
        MscReader reader(params);
        ASSERT_TRUE(reader.open());
        MscLoader loader;
        const muse::Ret loaded = loader.loadMscz(reloaded, reader, nullptr, true);
        ASSERT_TRUE(loaded) << loaded.toString();
    }

    for (Score* each : reloaded->scoreList()) {
        each->doLayout();
    }
    reloaded->initAutomation();
    EXPECT_EQ(reloaded->practiceEmphasis().document(), stored);

    Note* reloadedUnison = findPitch(reloaded, 74);
    Note* reloadedAlto = findPitch(reloaded, 67);
    Note* reloadedTop = findPitch(reloaded, 76);
    Note* reloadedLower = findPitch(reloaded, 72);
    ASSERT_TRUE(reloadedUnison);
    ASSERT_TRUE(reloadedAlto);
    ASSERT_TRUE(reloadedTop);
    ASSERT_TRUE(reloadedLower);
    EXPECT_FALSE(reloaded->practiceEmphasis().isEmphasized(reloadedUnison));
    EXPECT_TRUE(reloaded->practiceEmphasis().isEmphasized(reloadedAlto));
    EXPECT_TRUE(reloaded->practiceEmphasis().isEmphasized(reloadedTop));
    EXPECT_FALSE(reloaded->practiceEmphasis().isEmphasized(reloadedLower));
    const PracticeEmphasisData::PartRule* reloadedS1 = findPart(reloaded->practiceEmphasis().document().parts, "S1");
    ASSERT_TRUE(reloadedS1);
    EXPECT_TRUE(std::find(reloadedS1->removed.begin(), reloadedS1->removed.end(),
                          reloadedUnison->eid().toStdString()) != reloadedS1->removed.end());
    EXPECT_TRUE(std::find(reloadedS1->added.begin(), reloadedS1->added.end(),
                          reloadedAlto->eid().toStdString()) != reloadedS1->added.end());
    EXPECT_EQ(reloaded->practiceEmphasis().document().version, PracticeEmphasisData::CURRENT_VERSION);

    reloaded->practiceEmphasis().setPlaybackActive(false);
    const dynamic_level_t writtenLevel = findMidi(heardNotes(loadPart(reloaded, 0)), 76)->level;
    reloaded->practiceEmphasis().setPlaybackActive(true);
    const dynamic_level_t loud = PracticeEmphasis::scaleDynamicLevel(writtenLevel, 8.f);
    const dynamic_level_t quiet = PracticeEmphasis::scaleDynamicLevel(writtenLevel, -18.f);
    const std::vector<HeardNote> women = heardNotes(loadPart(reloaded, 0));
    expectHeard(women, 76, PracticeEmphasis::EMPHASIS_STAFF_OFFSET, 0, loud);
    expectHeard(women, 74, 0, 0, quiet);
    expectHeard(women, 72, 0, 0, quiet);
    expectHeard(women, 67, PracticeEmphasis::EMPHASIS_STAFF_OFFSET, 1, loud);

    delete reloaded;
    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, ChildInheritsParentNotesAndKeepsItsOwnRemovals)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    Note* e5 = findPitch(score, 76);
    Note* c5 = findPitch(score, 72);
    Note* d5 = findPitch(score, 74);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(c5);
    ASSERT_TRUE(d5);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setEditingPartId(score, "S1");
    const std::string childId = emphasis.addCustomPart(score, "S1 extra", "S1");
    emphasis.setPartSelected(score, childId, true);
    emphasis.setPartSelected(score, "S1", false);

    EXPECT_TRUE(emphasis.partContains(e5, childId));
    EXPECT_TRUE(emphasis.partContains(d5, childId));
    EXPECT_FALSE(emphasis.partContains(c5, childId));
    EXPECT_TRUE(emphasis.isEmphasized(e5));
    EXPECT_FALSE(emphasis.isEmphasized(c5));

    emphasis.setEditingPartId(score, childId);
    emphasis.editNote(e5);
    EXPECT_FALSE(emphasis.partContains(e5, childId));
    EXPECT_TRUE(emphasis.partContains(e5, "S1"));
    EXPECT_FALSE(emphasis.isEmphasized(e5));

    emphasis.setEditingPartId(score, "S1");
    emphasis.editNote(c5);
    EXPECT_TRUE(emphasis.partContains(c5, "S1"));
    EXPECT_TRUE(emphasis.partContains(c5, childId));
    EXPECT_TRUE(emphasis.isEmphasized(c5));
    EXPECT_FALSE(emphasis.partContains(e5, childId));

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, PerPartEditsDoNotLeakAndUnionIsAcrossCheckedParts)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    Note* e5 = findPitch(score, 76);
    Note* c5 = findPitch(score, 72);
    Note* d5 = findPitch(score, 74);
    Note* alto = findPitch(score, 67);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(c5);
    ASSERT_TRUE(d5);
    ASSERT_TRUE(alto);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setPartSelected(score, "S2", true);
    emphasis.setEditingPartId(score, "S1");
    emphasis.editNote(d5);
    emphasis.editNote(alto);

    EXPECT_FALSE(emphasis.partContains(d5, "S1"));
    EXPECT_TRUE(emphasis.partContains(d5, "S2"));
    EXPECT_TRUE(emphasis.isEmphasized(d5));
    EXPECT_TRUE(emphasis.partContains(alto, "S1"));
    EXPECT_FALSE(emphasis.partContains(alto, "S2"));

    emphasis.setPartSelected(score, "S2", false);
    EXPECT_FALSE(emphasis.isEmphasized(d5));
    EXPECT_TRUE(emphasis.isEmphasized(e5));
    EXPECT_TRUE(emphasis.isEmphasized(alto));

    const std::string soloId = emphasis.addCustomPart(score, "Solo", {});
    EXPECT_TRUE(soloId.size() > 0);
    EXPECT_FALSE(emphasis.partContains(e5, soloId));
    emphasis.setEditingPartId(score, soloId);
    emphasis.editNote(c5);
    EXPECT_TRUE(emphasis.partContains(c5, soloId));
    EXPECT_FALSE(emphasis.partContains(c5, "S1"));
    EXPECT_FALSE(emphasis.isEmphasized(c5));

    emphasis.setPartSelected(score, soloId, true);
    EXPECT_TRUE(emphasis.isEmphasized(c5));
    EXPECT_TRUE(emphasis.isEmphasized(e5));

    emphasis.resetPartEdits(soloId);
    EXPECT_FALSE(emphasis.partContains(c5, soloId));
    EXPECT_FALSE(emphasis.isEmphasized(c5));
    EXPECT_TRUE(emphasis.partContains(alto, "S1"));

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, ClickSwitchesEditingPartByStaffAndManualPartsStayPut)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    Note* e5 = findPitch(score, 76);
    Note* alto = findPitch(score, 67);
    Note* tenor = findPitch(score, 60);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(alto);
    ASSERT_TRUE(tenor);
    EXPECT_EQ(static_cast<int>(e5->staffIdx()), 0);
    EXPECT_EQ(static_cast<int>(alto->staffIdx()), 0);
    EXPECT_EQ(static_cast<int>(tenor->staffIdx()), 1);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setEditingPartId(score, "S1");
    emphasis.setEmphasisMode(true);

    emphasis.editNote(alto);
    EXPECT_EQ(emphasis.document().editingPartId, "S1");
    EXPECT_TRUE(emphasis.partContains(alto, "S1"));
    EXPECT_EQ(emphasis.noteHighlight(e5), PracticeEmphasisData::Highlight::Edited);
    EXPECT_EQ(emphasis.noteHighlight(alto), PracticeEmphasisData::Highlight::EditedAdded);

    emphasis.editNote(tenor);
    EXPECT_EQ(emphasis.document().editingPartId, "T1");
    EXPECT_FALSE(emphasis.partContains(tenor, "S1"));
    EXPECT_FALSE(emphasis.partContains(tenor, "T1"));
    EXPECT_EQ(emphasis.noteHighlight(e5), PracticeEmphasisData::Highlight::OtherLoud);
    EXPECT_EQ(emphasis.noteHighlight(tenor), PracticeEmphasisData::Highlight::None);

    const std::string manualId = emphasis.addCustomPart(score, "Picked notes", {});
    emphasis.editNote(tenor);
    EXPECT_EQ(emphasis.document().editingPartId, manualId);
    EXPECT_TRUE(emphasis.partContains(tenor, manualId));
    EXPECT_FALSE(emphasis.partContains(tenor, "T1"));

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, CustomPartNameAndParentSurviveEditingSwitch)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setEditingPartId(score, "S1");
    const std::string customId = emphasis.addCustomPart(score, "sops", "S1");
    ASSERT_FALSE(customId.empty());
    EXPECT_EQ(emphasis.document().editingPartId, customId);

    const PracticeEmphasisData::PartRule* custom = findPart(emphasis.document().parts, customId);
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->name, "sops");
    EXPECT_EQ(custom->parentId, "S1");
    EXPECT_FALSE(custom->builtin);

    emphasis.setEditingPartId(score, "S1");
    EXPECT_EQ(emphasis.document().editingPartId, "S1");
    emphasis.setPartName("S1", "renamed builtin");
    emphasis.setPartParent(score, "S1", "S2");

    const PracticeEmphasisData::PartRule* soprano = findPart(emphasis.document().parts, "S1");
    ASSERT_TRUE(soprano);
    EXPECT_TRUE(soprano->builtin);
    EXPECT_EQ(soprano->name, "1st soprano");
    EXPECT_TRUE(soprano->parentId.empty());

    custom = findPart(emphasis.document().parts, customId);
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->name, "sops");
    EXPECT_EQ(custom->parentId, "S1");

    emphasis.setEditingPartId(score, customId);
    custom = findPart(emphasis.document().parts, customId);
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->name, "sops");
    EXPECT_EQ(custom->parentId, "S1");

    emphasis.setPartName(customId, "sops");
    emphasis.setPartParent(score, customId, "A1");
    custom = findPart(emphasis.document().parts, customId);
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->name, "sops");
    EXPECT_EQ(custom->parentId, "A1");

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, SoloUsesOnlyEditingPartWithoutChangingChecks)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    Note* e5 = findPitch(score, 76);
    Note* alto = findPitch(score, 67);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(alto);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPlaybackActive(true);
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setPartSelected(score, "A1", true);
    emphasis.setEditingPartId(score, "S1");

    EXPECT_TRUE(emphasis.partSelected("S1"));
    EXPECT_TRUE(emphasis.partSelected("A1"));
    EXPECT_TRUE(emphasis.isEmphasized(e5));
    EXPECT_TRUE(emphasis.isEmphasized(alto));
    EXPECT_TRUE(emphasis.playbackEngaged());

    emphasis.setSolo(true);
    EXPECT_TRUE(emphasis.document().solo);
    EXPECT_TRUE(emphasis.partSelected("S1"));
    EXPECT_TRUE(emphasis.partSelected("A1"));
    EXPECT_TRUE(emphasis.isEmphasized(e5));
    EXPECT_FALSE(emphasis.isEmphasized(alto));
    EXPECT_TRUE(emphasis.playbackEngaged());

    emphasis.setEditingPartId(score, "A1");
    EXPECT_FALSE(emphasis.isEmphasized(e5));
    EXPECT_TRUE(emphasis.isEmphasized(alto));

    emphasis.setSolo(false);
    EXPECT_FALSE(emphasis.document().solo);
    EXPECT_TRUE(emphasis.partSelected("S1"));
    EXPECT_TRUE(emphasis.partSelected("A1"));
    EXPECT_TRUE(emphasis.isEmphasized(e5));
    EXPECT_TRUE(emphasis.isEmphasized(alto));

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, SoloMutesNonEditingNotesDuringPlayback)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);
    score->initAutomation();

    Note* e5 = findPitch(score, 76);
    Note* alto = findPitch(score, 67);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(alto);

    PlaybackData writtenWomen = loadPart(score, 0);
    const std::vector<HeardNote> written = heardNotes(writtenWomen);
    const dynamic_level_t writtenSoprano = findMidi(written, 76)->level;
    const dynamic_level_t writtenAlto = findMidi(written, 67)->level;
    const dynamic_level_t loud = PracticeEmphasis::scaleDynamicLevel(writtenSoprano,
                                                                     PracticeEmphasisData::DEFAULT_EMPHASIZED_DB);
    const dynamic_level_t quiet = PracticeEmphasis::scaleDynamicLevel(writtenAlto,
                                                                      PracticeEmphasisData::DEFAULT_OTHER_DB);
    const dynamic_level_t muted = PracticeEmphasis::scaleDynamicLevel(writtenAlto,
                                                                      PracticeEmphasisData::SOLO_MUTE_DB);
    const staff_layer_idx_t sopranoCompanion = PracticeEmphasis::EMPHASIS_STAFF_OFFSET;

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setPartSelected(score, "A1", true);
    emphasis.setEditingPartId(score, "S1");
    EXPECT_FALSE(emphasis.document().playbackActive);
    EXPECT_FALSE(emphasis.playbackEngaged());

    {
        const std::vector<HeardNote> women = heardNotes(loadPart(score, 0));
        expectHeard(women, 76, 0, 0, writtenSoprano);
        expectHeard(women, 67, 0, 1, writtenAlto);
    }

    emphasis.setSolo(true);
    EXPECT_TRUE(emphasis.playbackEngaged());
    EXPECT_TRUE(emphasis.isEmphasized(e5));
    EXPECT_FALSE(emphasis.isEmphasized(alto));
    EXPECT_EQ(emphasis.otherGainDb(), PracticeEmphasisData::SOLO_MUTE_DB);
    EXPECT_LT(muted, quiet);

    {
        const std::vector<HeardNote> women = heardNotes(loadPart(score, 0));
        expectHeard(women, 76, sopranoCompanion, 0, loud);
        expectHeard(women, 67, 0, 1, muted);
    }

    emphasis.setPlaybackActive(true);
    emphasis.setSolo(false);
    EXPECT_TRUE(emphasis.playbackEngaged());
    EXPECT_EQ(emphasis.otherGainDb(), PracticeEmphasisData::DEFAULT_OTHER_DB);

    {
        const std::vector<HeardNote> women = heardNotes(loadPart(score, 0));
        expectHeard(women, 76, sopranoCompanion, 0, loud);
        expectHeard(women, 67, 0, 1, quiet);
    }

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, AddedNotesHighlightAsEditedAdded)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    Note* e5 = findPitch(score, 76);
    Note* c5 = findPitch(score, 72);
    Note* alto = findPitch(score, 67);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(c5);
    ASSERT_TRUE(alto);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setEmphasisMode(true);
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setPartSelected(score, "S2", true);
    emphasis.setEditingPartId(score, "S1");

    EXPECT_EQ(emphasis.noteHighlight(e5), PracticeEmphasisData::Highlight::Edited);
    EXPECT_EQ(emphasis.noteHighlight(c5), PracticeEmphasisData::Highlight::OtherLoud);
    EXPECT_EQ(emphasis.noteHighlight(alto), PracticeEmphasisData::Highlight::None);

    emphasis.editNote(alto);
    EXPECT_TRUE(emphasis.partContains(alto, "S1"));
    EXPECT_EQ(emphasis.noteHighlight(alto), PracticeEmphasisData::Highlight::EditedAdded);
    EXPECT_EQ(emphasis.noteHighlight(e5), PracticeEmphasisData::Highlight::Edited);

    emphasis.editNote(e5);
    EXPECT_FALSE(emphasis.partContains(e5, "S1"));
    EXPECT_EQ(emphasis.noteHighlight(e5), PracticeEmphasisData::Highlight::None);
    EXPECT_EQ(emphasis.noteHighlight(alto), PracticeEmphasisData::Highlight::EditedAdded);

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, SidecarRoundTripsSoloAndOlderFilesDefaultOff)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPlaybackActive(true);
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setEditingPartId(score, "S1");
    emphasis.setSolo(true);

    PracticeEmphasisData parsed;
    EXPECT_TRUE(parsed.fromJson(emphasis.toJson()));
    EXPECT_EQ(parsed.document().version, PracticeEmphasisData::CURRENT_VERSION);
    EXPECT_TRUE(parsed.document().solo);
    EXPECT_EQ(parsed.document(), emphasis.document());

    ByteArray mscz;
    ASSERT_TRUE(writeMscz(score, mscz));
    ByteArray json;
    ASSERT_TRUE(containerHasEmphasis(mscz, &json));
    PracticeEmphasisData fromContainer;
    EXPECT_TRUE(fromContainer.fromJson(json));
    EXPECT_TRUE(fromContainer.document().solo);
    EXPECT_EQ(fromContainer.document(), emphasis.document());

    PracticeEmphasisData legacy;
    const muse::ByteArray v2(
        "{"
        "\"version\":2,"
        "\"playbackActive\":true,"
        "\"editingPartId\":\"S1\","
        "\"parts\":[{\"id\":\"S1\",\"name\":\"1st soprano\",\"staffIndex\":0,\"voice\":0,\"role\":\"top\",\"selected\":true}]"
        "}");
    EXPECT_TRUE(legacy.fromJson(v2));
    EXPECT_FALSE(legacy.document().solo);
    EXPECT_EQ(legacy.document().version, PracticeEmphasisData::CURRENT_VERSION);

    delete score;
}

TEST(Engraving_PracticeEmphasis, VersionOneOverridesAreIgnored)
{
    PracticeEmphasisData parsed;
    const muse::ByteArray json(
        "{"
        "\"version\":1,"
        "\"playbackActive\":true,"
        "\"parts\":[{\"id\":\"S1\",\"name\":\"1st soprano\",\"staffIndex\":0,\"voice\":0,\"role\":\"top\",\"selected\":true}],"
        "\"overrides\":{\"not-a-real-eid\":\"forceOn\"}"
        "}");
    EXPECT_TRUE(parsed.fromJson(json));
    EXPECT_EQ(parsed.document().version, PracticeEmphasisData::CURRENT_VERSION);
    ASSERT_EQ(parsed.document().parts.size(), 1u);
    EXPECT_TRUE(parsed.document().parts.front().builtin);
    EXPECT_TRUE(parsed.document().parts.front().added.empty());
    EXPECT_TRUE(parsed.document().parts.front().removed.empty());
    EXPECT_TRUE(parsed.document().parts.front().selected);

    const muse::ByteArray written = parsed.toJson();
    const std::string text(written.constChar(), written.size());
    EXPECT_EQ(text.find("overrides"), std::string::npos);

    PracticeEmphasisData roundtrip;
    EXPECT_TRUE(roundtrip.fromJson(written));
    EXPECT_EQ(roundtrip.document().version, PracticeEmphasisData::CURRENT_VERSION);
    EXPECT_TRUE(roundtrip.document().parts.front().added.empty());
    EXPECT_TRUE(roundtrip.document().parts.front().removed.empty());
}

TEST_F(Engraving_PracticeEmphasisTests, UndoRedoClicksResetAndDropsRedoTail)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    Note* e5 = findPitch(score, 76);
    Note* c5 = findPitch(score, 72);
    Note* d5 = findPitch(score, 74);
    Note* alto = findPitch(score, 67);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(c5);
    ASSERT_TRUE(d5);
    ASSERT_TRUE(alto);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    EXPECT_FALSE(emphasis.canUndo());
    EXPECT_FALSE(emphasis.canRedo());

    emphasis.setPartSelected(score, "S1", true);
    emphasis.setEditingPartId(score, "S1");
    EXPECT_FALSE(emphasis.canUndo());
    EXPECT_FALSE(emphasis.canRedo());

    emphasis.editNote(d5);
    EXPECT_FALSE(emphasis.partContains(d5, "S1"));
    emphasis.editNote(c5);
    EXPECT_TRUE(emphasis.partContains(c5, "S1"));

    emphasis.undo();
    EXPECT_FALSE(emphasis.partContains(c5, "S1"));
    EXPECT_FALSE(emphasis.partContains(d5, "S1"));
    EXPECT_TRUE(emphasis.canRedo());

    emphasis.redo();
    EXPECT_TRUE(emphasis.partContains(c5, "S1"));
    EXPECT_FALSE(emphasis.partContains(d5, "S1"));

    emphasis.resetPartEdits("S1");
    EXPECT_TRUE(emphasis.partContains(d5, "S1"));
    EXPECT_FALSE(emphasis.partContains(c5, "S1"));

    emphasis.undo();
    EXPECT_TRUE(emphasis.partContains(c5, "S1"));
    EXPECT_FALSE(emphasis.partContains(d5, "S1"));

    emphasis.redo();
    EXPECT_TRUE(emphasis.partContains(d5, "S1"));
    EXPECT_FALSE(emphasis.partContains(c5, "S1"));

    emphasis.undo();
    EXPECT_TRUE(emphasis.partContains(c5, "S1"));
    emphasis.editNote(alto);
    EXPECT_TRUE(emphasis.partContains(alto, "S1"));
    EXPECT_TRUE(emphasis.partContains(c5, "S1"));
    EXPECT_FALSE(emphasis.canRedo());

    emphasis.undo();
    EXPECT_FALSE(emphasis.partContains(alto, "S1"));
    EXPECT_TRUE(emphasis.partContains(c5, "S1"));
    emphasis.redo();
    EXPECT_TRUE(emphasis.partContains(alto, "S1"));
    EXPECT_FALSE(emphasis.partContains(d5, "S1"));

    const std::string customId = emphasis.addCustomPart(score, "scratch", "S1");
    ASSERT_FALSE(customId.empty());
    EXPECT_EQ(emphasis.document().editingPartId, customId);
    EXPECT_TRUE(emphasis.canUndo());

    emphasis.setPartName(customId, "A");
    emphasis.setPartName(customId, "AB");
    emphasis.setPartName(customId, "ABC");
    const PracticeEmphasisData::PartRule* custom = findPart(emphasis.document().parts, customId);
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->name, "ABC");
    emphasis.undo();
    custom = findPart(emphasis.document().parts, customId);
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->name, "ABC");
    EXPECT_EQ(custom->parentId, "S1");
    EXPECT_FALSE(emphasis.partContains(alto, "S1"));

    EXPECT_TRUE(emphasis.deletePart(customId));
    EXPECT_EQ(findPart(emphasis.document().parts, customId), nullptr);
    emphasis.undo();
    EXPECT_EQ(findPart(emphasis.document().parts, customId), nullptr);

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, NameAndParentReloadAndParentDropdownIndex)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    Note* e5 = findPitch(score, 76);
    Note* alto = findPitch(score, 67);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(alto);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    const std::string customId = emphasis.addCustomPart(score, "sops", {});
    ASSERT_FALSE(customId.empty());
    EXPECT_EQ(emphasis.parentOptionIndex(score, customId, {}), 0);
    EXPECT_FALSE(emphasis.partContains(e5, customId));

    // Parent S1 is not yet in document.parts; ensurePart grows the vector.
    emphasis.setPartParent(score, customId, "S1");
    const PracticeEmphasisData::PartRule* custom = findPart(emphasis.document().parts, customId);
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->name, "sops");
    EXPECT_EQ(custom->parentId, "S1");
    EXPECT_TRUE(emphasis.partContains(e5, customId));
    EXPECT_FALSE(emphasis.partContains(alto, customId));
    const int s1Index = emphasis.parentOptionIndex(score, customId, "S1");
    EXPECT_GT(s1Index, 0);
    EXPECT_EQ(emphasis.parentOptionIds(score, customId).at(static_cast<size_t>(s1Index)), "S1");

    // Choosing A1 (not the first parent option) must not map back to index 0 / "(none)".
    emphasis.setPartParent(score, customId, "A1");
    custom = findPart(emphasis.document().parts, customId);
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->parentId, "A1");
    EXPECT_FALSE(emphasis.partContains(e5, customId));
    EXPECT_TRUE(emphasis.partContains(alto, customId));
    const int a1Index = emphasis.parentOptionIndex(score, customId, "A1");
    EXPECT_GT(a1Index, 0);
    EXPECT_NE(a1Index, s1Index);
    EXPECT_EQ(emphasis.parentOptionIds(score, customId).at(static_cast<size_t>(a1Index)), "A1");

    PracticeEmphasisData parsed;
    EXPECT_TRUE(parsed.fromJson(emphasis.toJson()));
    const PracticeEmphasisData::PartRule* parsedCustom = findPart(parsed.document().parts, customId);
    ASSERT_TRUE(parsedCustom);
    EXPECT_EQ(parsed.document().editingPartId, customId);
    EXPECT_EQ(parsedCustom->name, "sops");
    EXPECT_EQ(parsedCustom->parentId, "A1");
    EXPECT_EQ(parsed.parentOptionIndex(score, customId, parsedCustom->parentId), a1Index);

    ByteArray mscz;
    ASSERT_TRUE(writeMscz(score, mscz));
    ByteArray json;
    ASSERT_TRUE(containerHasEmphasis(mscz, &json));
    PracticeEmphasisData fromContainer;
    EXPECT_TRUE(fromContainer.fromJson(json));
    const PracticeEmphasisData::PartRule* containerCustom = findPart(fromContainer.document().parts, customId);
    ASSERT_TRUE(containerCustom);
    EXPECT_EQ(containerCustom->name, "sops");
    EXPECT_EQ(containerCustom->parentId, "A1");

    MasterScore* reloaded = compat::ScoreAccess::createMasterScoreWithBaseStyle(score->iocContext());
    reloaded->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>("practice-emphasis-roundtrip.mscz"));
    {
        muse::io::Buffer buffer(&mscz);
        MscReader::Params params;
        params.device = &buffer;
        params.filePath = "practice-emphasis-roundtrip.mscz";
        params.mode = MscIoMode::Zip;
        MscReader reader(params);
        ASSERT_TRUE(reader.open());
        MscLoader loader;
        const muse::Ret loaded = loader.loadMscz(reloaded, reader, nullptr, true);
        ASSERT_TRUE(loaded) << loaded.toString();
    }
    for (Score* each : reloaded->scoreList()) {
        each->doLayout();
    }
    reloaded->initAutomation();

    PracticeEmphasisData& reloadedEmphasis = reloaded->practiceEmphasis();
    EXPECT_EQ(reloadedEmphasis.document().editingPartId, customId);
    const PracticeEmphasisData::PartRule* reloadedCustom = findPart(reloadedEmphasis.document().parts, customId);
    ASSERT_TRUE(reloadedCustom);
    EXPECT_EQ(reloadedCustom->name, "sops");
    EXPECT_EQ(reloadedCustom->parentId, "A1");
    const int reloadedIndex = reloadedEmphasis.parentOptionIndex(reloaded, customId, reloadedCustom->parentId);
    EXPECT_GT(reloadedIndex, 0);
    EXPECT_EQ(reloadedEmphasis.parentOptionIds(reloaded, customId).at(static_cast<size_t>(reloadedIndex)), "A1");

    Note* reloadedE5 = findPitch(reloaded, 76);
    Note* reloadedAlto = findPitch(reloaded, 67);
    ASSERT_TRUE(reloadedE5);
    ASSERT_TRUE(reloadedAlto);
    EXPECT_FALSE(reloadedEmphasis.partContains(reloadedE5, customId));
    EXPECT_TRUE(reloadedEmphasis.partContains(reloadedAlto, customId));

    delete reloaded;
    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, SidecarJsonLoadsCustomNameAndParentKeys)
{
    PracticeEmphasisData parsed;
    const muse::ByteArray json(
        "{"
        "\"version\":3,"
        "\"editingPartId\":\"custom-1\","
        "\"parts\":["
        "{\"id\":\"S1\",\"name\":\"1st soprano\",\"staffIndex\":0,\"voice\":0,\"role\":\"top\",\"builtin\":true,\"selected\":true},"
        "{\"id\":\"custom-1\",\"name\":\"sops\",\"staffIndex\":0,\"voice\":0,\"role\":\"top\",\"builtin\":false,\"parentId\":\"A1\"}"
        "]"
        "}");
    EXPECT_TRUE(parsed.fromJson(json));
    EXPECT_EQ(parsed.document().editingPartId, "custom-1");
    const PracticeEmphasisData::PartRule* custom = findPart(parsed.document().parts, "custom-1");
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->name, "sops");
    EXPECT_EQ(custom->parentId, "A1");
    EXPECT_FALSE(custom->builtin);

    const muse::ByteArray written = parsed.toJson();
    const std::string text(written.constChar(), written.size());
    EXPECT_NE(text.find("sops"), std::string::npos);
    EXPECT_NE(text.find("parentId"), std::string::npos);
    EXPECT_NE(text.find("A1"), std::string::npos);
}

TEST_F(Engraving_PracticeEmphasisTests, UndoParentKeepsNameAndSettings)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    const std::string customId = emphasis.addCustomPart(score, "sops", "S1");
    ASSERT_FALSE(customId.empty());
    EXPECT_FALSE(emphasis.canUndo());

    emphasis.setSolo(true);
    emphasis.setPartName(customId, "renamed");
    emphasis.setPartSelected(score, customId, true);
    emphasis.setRange(1, 2);
    EXPECT_FALSE(emphasis.canUndo());

    emphasis.setPartParent(score, customId, "A1");
    EXPECT_TRUE(emphasis.canUndo());
    const PracticeEmphasisData::PartRule* custom = findPart(emphasis.document().parts, customId);
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->parentId, "A1");
    EXPECT_EQ(custom->name, "renamed");
    EXPECT_GT(emphasis.parentOptionIndex(score, customId, "A1"), 0);

    emphasis.undo();
    custom = findPart(emphasis.document().parts, customId);
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->parentId, "S1");
    EXPECT_EQ(custom->name, "renamed");
    EXPECT_TRUE(emphasis.document().solo);
    EXPECT_TRUE(emphasis.partSelected(customId));
    EXPECT_EQ(emphasis.document().fromMeasure, 1);
    EXPECT_EQ(emphasis.document().toMeasure, 2);
    EXPECT_GT(emphasis.parentOptionIndex(score, customId, "S1"), 0);

    emphasis.redo();
    custom = findPart(emphasis.document().parts, customId);
    ASSERT_TRUE(custom);
    EXPECT_EQ(custom->parentId, "A1");
    EXPECT_EQ(custom->name, "renamed");
    EXPECT_TRUE(emphasis.document().solo);

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, CopyFromPartIsOneUndoableStep)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    Note* e5 = findPitch(score, 76);
    Note* c5 = findPitch(score, 72);
    Note* d5 = findPitch(score, 74);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(c5);
    ASSERT_TRUE(d5);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setEditingPartId(score, "S1");
    const std::string childId = emphasis.addCustomPart(score, "from S2", "S1");
    EXPECT_TRUE(emphasis.partContains(e5, childId));
    EXPECT_TRUE(emphasis.partContains(d5, childId));
    EXPECT_FALSE(emphasis.partContains(c5, childId));
    EXPECT_FALSE(emphasis.copyFromPart(score, "S1", "S2"));

    EXPECT_TRUE(emphasis.copyFromPart(score, childId, "S2"));
    EXPECT_FALSE(emphasis.partContains(e5, childId));
    EXPECT_TRUE(emphasis.partContains(d5, childId));
    EXPECT_TRUE(emphasis.partContains(c5, childId));
    EXPECT_FALSE(emphasis.copyFromPart(score, childId, "S2"));

    emphasis.undo();
    EXPECT_TRUE(emphasis.partContains(e5, childId));
    EXPECT_TRUE(emphasis.partContains(d5, childId));
    EXPECT_FALSE(emphasis.partContains(c5, childId));
    EXPECT_TRUE(emphasis.canRedo());

    emphasis.redo();
    EXPECT_TRUE(emphasis.partContains(c5, childId));
    EXPECT_FALSE(emphasis.partContains(e5, childId));

    const std::string emptyId = emphasis.addCustomPart(score, "picked", {});
    EXPECT_FALSE(emphasis.partContains(e5, emptyId));
    EXPECT_TRUE(emphasis.copyFromPart(score, emptyId, "S1"));
    EXPECT_TRUE(emphasis.partContains(e5, emptyId));
    EXPECT_TRUE(emphasis.partContains(d5, emptyId));
    EXPECT_FALSE(emphasis.partContains(c5, emptyId));
    emphasis.undo();
    EXPECT_FALSE(emphasis.partContains(e5, emptyId));
    EXPECT_FALSE(emphasis.partContains(c5, emptyId));
    EXPECT_TRUE(emphasis.canRedo());

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, EmphasisUndoIsIndependentOfScoreUndo)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);
    ASSERT_TRUE(score->undoStack());
    EXPECT_FALSE(score->undoStack()->canUndo());

    Note* e5 = findPitch(score, 76);
    Note* d5 = findPitch(score, 74);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(d5);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setEditingPartId(score, "S1");
    emphasis.editNote(d5);
    EXPECT_FALSE(emphasis.partContains(d5, "S1"));
    EXPECT_FALSE(score->undoStack()->canUndo());

    const PracticeEmphasisData::Document afterEdit = emphasis.document();
    score->startCmd(TranslatableString::untranslatable("Practice emphasis tests"));
    e5->undoChangeProperty(Pid::VISIBLE, false);
    score->endCmd();
    EXPECT_TRUE(score->undoStack()->canUndo());
    EXPECT_FALSE(e5->visible());
    EXPECT_EQ(emphasis.document(), afterEdit);
    EXPECT_FALSE(emphasis.partContains(d5, "S1"));

    score->undoRedo(true, nullptr);
    EXPECT_TRUE(e5->visible());
    EXPECT_EQ(emphasis.document(), afterEdit);
    EXPECT_FALSE(emphasis.partContains(d5, "S1"));

    emphasis.undo();
    EXPECT_TRUE(emphasis.partContains(d5, "S1"));
    EXPECT_TRUE(e5->visible());
    EXPECT_FALSE(score->undoStack()->canUndo());

    delete score;
}

TEST_F(Engraving_PracticeEmphasisTests, MissingNoteEditsStayDormantUntilSameEidReturns)
{
    MasterScore* score = ScoreRW::readScore(SCORE_FILE);
    ASSERT_TRUE(score);

    Note* e5 = findPitch(score, 76);
    Note* alto = findPitch(score, 67);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(alto);

    PracticeEmphasisData& emphasis = score->practiceEmphasis();
    emphasis.setEmphasisMode(true);
    emphasis.setPartSelected(score, "S1", true);
    emphasis.setEditingPartId(score, "S1");
    emphasis.editNote(alto);
    ASSERT_TRUE(alto->eid().isValid());
    const std::string altoEid = alto->eid().toStdString();
    EXPECT_TRUE(emphasis.partContains(alto, "S1"));
    const PracticeEmphasisData::PartRule* soprano = findPart(emphasis.document().parts, "S1");
    ASSERT_TRUE(soprano);
    EXPECT_TRUE(std::find(soprano->added.begin(), soprano->added.end(), altoEid) != soprano->added.end());

    score->startCmd(TranslatableString::untranslatable("Practice emphasis tests"));
    score->select(alto);
    score->cmdDeleteSelection();
    score->endCmd();
    EXPECT_EQ(findPitch(score, 67), nullptr);

    soprano = findPart(emphasis.document().parts, "S1");
    ASSERT_TRUE(soprano);
    EXPECT_TRUE(std::find(soprano->added.begin(), soprano->added.end(), altoEid) != soprano->added.end());
    EXPECT_TRUE(emphasis.partContains(e5, "S1"));
    EXPECT_TRUE(emphasis.isEmphasized(e5));
    EXPECT_EQ(emphasis.noteHighlight(e5), PracticeEmphasisData::Highlight::Edited);

    score->undoRedo(true, nullptr);
    Note* restored = findPitch(score, 67);
    ASSERT_TRUE(restored);
    EXPECT_EQ(restored->eid().toStdString(), altoEid);
    EXPECT_TRUE(emphasis.partContains(restored, "S1"));
    EXPECT_EQ(emphasis.noteHighlight(restored), PracticeEmphasisData::Highlight::EditedAdded);

    score->undoRedo(false, nullptr);
    EXPECT_EQ(findPitch(score, 67), nullptr);
    soprano = findPart(emphasis.document().parts, "S1");
    ASSERT_TRUE(soprano);
    EXPECT_TRUE(std::find(soprano->added.begin(), soprano->added.end(), altoEid) != soprano->added.end());
    EXPECT_TRUE(emphasis.partContains(e5, "S1"));

    score->undoRedo(true, nullptr);
    restored = findPitch(score, 67);
    ASSERT_TRUE(restored);
    EXPECT_EQ(restored->eid().toStdString(), altoEid);
    EXPECT_TRUE(emphasis.partContains(restored, "S1"));

    delete score;
}
