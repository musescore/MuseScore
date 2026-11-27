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

#include <gmock/gmock.h>

#include <QJsonDocument>
#include <QJsonObject>

#include "io/buffer.h"

#include "project/internal/projectaudiosettings.h"

#include "playback/tests/mocks/playbackconfigurationmock.h"

using ::testing::NiceMock;
using ::testing::ReturnRef;

using namespace muse;
using namespace mu::engraving;
using namespace mu::project;

namespace mu::project {
class ProjectAudioSettingsTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_playbackConfiguration = std::make_shared<NiceMock<playback::PlaybackConfigurationMock> >();
        ON_CALL(*m_playbackConfiguration, compatMuseSoundsProfileName()).WillByDefault(ReturnRef(m_compatProfileName));
        ON_CALL(*m_playbackConfiguration, museSoundsProfileName()).WillByDefault(ReturnRef(m_museSoundsProfileName));

        modularity::globalIoc()->registerExport<playback::IPlaybackConfiguration>("utests", m_playbackConfiguration);
    }

    void TearDown() override
    {
        modularity::globalIoc()->unregister<playback::IPlaybackConfiguration>("utests");
    }

    static std::shared_ptr<ProjectAudioSettings> makeSettings()
    {
        std::shared_ptr<ProjectAudioSettings> settings(new ProjectAudioSettings(modularity::globalCtx()));
        settings->setActiveSoundProfile(u"Test profile");
        return settings;
    }

    //! Writes the settings into an in-memory .mscz file and returns its audiosettings.json
    static ByteArray writeToJson(ProjectAudioSettings& settings)
    {
        ByteArray msczData;
        {
            io::Buffer buf(&msczData);
            MscWriter::Params params;
            params.device = &buf;
            params.filePath = "vca.mscz";
            params.mode = MscIoMode::Zip;

            MscWriter writer(params);
            EXPECT_TRUE(writer.open());
            EXPECT_TRUE(settings.write(writer, nullptr));
        }

        io::Buffer buf(&msczData);
        MscReader::Params params;
        params.device = &buf;
        params.filePath = "vca.mscz";
        params.mode = MscIoMode::Zip;

        MscReader reader(params);
        EXPECT_TRUE(reader.open());
        return reader.readAudioSettingsJsonFile();
    }

    //! Reads the given audiosettings.json into new settings, as opening a score does
    static std::shared_ptr<ProjectAudioSettings> readFromJson(const ByteArray& json)
    {
        ByteArray msczData;
        {
            io::Buffer buf(&msczData);
            MscWriter::Params params;
            params.device = &buf;
            params.filePath = "vca.mscz";
            params.mode = MscIoMode::Zip;

            MscWriter writer(params);
            EXPECT_TRUE(writer.open());
            writer.writeAudioSettingsJsonFile(json);
        }

        io::Buffer buf(&msczData);
        MscReader::Params params;
        params.device = &buf;
        params.filePath = "vca.mscz";
        params.mode = MscIoMode::Zip;

        MscReader reader(params);
        EXPECT_TRUE(reader.open());

        std::shared_ptr<ProjectAudioSettings> settings = makeSettings();
        EXPECT_TRUE(settings->read(reader));
        return settings;
    }

    static InstrumentTrackId track(uint64_t partId, const char16_t* instrumentId)
    {
        InstrumentTrackId result;
        result.partId = partId;
        result.instrumentId = instrumentId;
        return result;
    }

    static VcaGroupList rhythmAndHornsGroups()
    {
        VcaGroup rhythm;
        rhythm.name = u"Rhythm";
        rhythm.volume = -6.5f;
        rhythm.balance = -0.25f;
        rhythm.muted = true;
        rhythm.members = { track(1, u"piano"), track(2, u"bass"), track(3, u"drumset") };

        VcaGroup horns;
        horns.name = u"Horns";
        horns.volume = 3.f;
        horns.balance = 0.5f;
        horns.solo = true;
        horns.members = { track(4, u"trumpet") };

        return { rhythm, horns };
    }

    std::shared_ptr<NiceMock<playback::PlaybackConfigurationMock> > m_playbackConfiguration;
    playback::SoundProfileName m_compatProfileName = u"Compat profile";
    playback::SoundProfileName m_museSoundsProfileName = u"MuseSounds profile";
};
}

TEST_F(ProjectAudioSettingsTests, VcaGroups_SaveAndOpen)
{
    //! GIVEN Settings with two VCA groups
    std::shared_ptr<ProjectAudioSettings> settings = makeSettings();
    const VcaGroupList groups = rhythmAndHornsGroups();
    settings->setVcaGroups(groups);

    //! WHEN The settings are saved and opened again
    std::shared_ptr<ProjectAudioSettings> reopened = readFromJson(writeToJson(*settings));

    //! THEN Every group comes back with its name, volume, pan, mute, solo and members
    EXPECT_EQ(reopened->vcaGroups(), groups);
}

TEST_F(ProjectAudioSettingsTests, VcaGroups_NotWrittenWhenThereAreNone)
{
    //! GIVEN Settings without VCA groups
    std::shared_ptr<ProjectAudioSettings> settings = makeSettings();

    //! WHEN The settings are saved
    const ByteArray json = writeToJson(*settings);

    //! THEN The file is the same as one saved by a version without VCA groups
    const QJsonObject rootObj = QJsonDocument::fromJson(json.toQByteArrayNoCopy()).object();
    EXPECT_FALSE(rootObj.contains("vcaGroups"));

    //! AND Opening it gives no groups
    EXPECT_TRUE(readFromJson(json)->vcaGroups().empty());
}

TEST_F(ProjectAudioSettingsTests, VcaGroups_InvalidMembersAreSkipped)
{
    //! GIVEN A file with a VCA group member that has no part id
    const ByteArray json(
        R"({
        "master": {},
        "aux": [],
        "tracks": [],
        "vcaGroups": [{
            "name": "Rhythm",
            "volumeDb": -3,
            "members": [
                { "partId": "1", "instrumentId": "piano" },
                { "partId": "", "instrumentId": "bass" }
            ]
        }],
        "activeSoundProfile": "Test profile"
    })");

    //! WHEN It is opened
    std::shared_ptr<ProjectAudioSettings> settings = readFromJson(json);

    //! THEN Only the valid member is kept, and missing keys take their defaults
    ASSERT_EQ(settings->vcaGroups().size(), 1);

    const VcaGroup& group = settings->vcaGroups().front();
    EXPECT_EQ(group.name, u"Rhythm");
    EXPECT_FLOAT_EQ(group.volume.raw(), -3.f);
    EXPECT_FLOAT_EQ(group.balance.raw(), 0.f);
    EXPECT_FALSE(group.muted);
    EXPECT_FALSE(group.solo);
    const std::vector<InstrumentTrackId> expectedMembers = { track(1, u"piano") };
    EXPECT_EQ(group.members, expectedMembers);
}

TEST_F(ProjectAudioSettingsTests, VcaGroups_RemovedTrackLeavesItsGroup)
{
    //! GIVEN Settings with two VCA groups
    std::shared_ptr<ProjectAudioSettings> settings = makeSettings();
    settings->setVcaGroups(rhythmAndHornsGroups());

    //! WHEN The bass track is removed from the score
    settings->removeTrackParams(track(2, u"bass"));

    //! THEN The bass is no longer a member of the Rhythm group, and the other members stay
    const VcaGroupList& groups = settings->vcaGroups();
    ASSERT_EQ(groups.size(), 2);
    const std::vector<InstrumentTrackId> expectedRhythmMembers = { track(1, u"piano"), track(3, u"drumset") };
    const std::vector<InstrumentTrackId> expectedHornsMembers = { track(4, u"trumpet") };
    EXPECT_EQ(groups.at(0).members, expectedRhythmMembers);
    EXPECT_EQ(groups.at(1).members, expectedHornsMembers);
}
