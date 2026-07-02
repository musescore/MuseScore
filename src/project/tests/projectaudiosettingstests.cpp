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
#include <gmock/gmock.h>

#include "project/internal/projectaudiosettings.h"

using namespace muse;
using namespace muse::audio;
using namespace mu::engraving;

namespace mu::project {
class Project_AudioSettingsTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_settings = std::shared_ptr<ProjectAudioSettings>(new ProjectAudioSettings(modularity::ContextPtr()));
        m_pianoOnPart1 = { muse::ID(static_cast<uint64_t>(1)), String(u"keyboard.piano") };
        m_pianoOnPart2 = { muse::ID(static_cast<uint64_t>(2)), String(u"keyboard.piano") };
    }

    static AudioResourceMeta makeVstResource(const AudioResourceId& id, const AudioResourceVendor& vendor)
    {
        AudioResourceMeta meta;
        meta.id = id;
        meta.vendor = vendor;
        meta.type = AudioResourceType::VstPlugin;
        meta.hasNativeEditorSupport = true;
        return meta;
    }

    static AudioInputParams makeVstInput(const AudioResourceId& id)
    {
        AudioInputParams input;
        input.resourceMeta = makeVstResource(id, AudioResourceVendor("TestVendor"));
        return input;
    }

    static AudioFxParams makeFxEffect(const AudioResourceId& id)
    {
        AudioFxParams fx;
        fx.resourceMeta = makeVstResource(id, AudioResourceVendor("TestVendor"));
        fx.active = true;
        return fx;
    }

    std::shared_ptr<ProjectAudioSettings> m_settings;
    InstrumentTrackId m_pianoOnPart1;
    InstrumentTrackId m_pianoOnPart2;
};
}

using namespace mu::project;

TEST_F(Project_AudioSettingsTests, DuplicateCopiesInputAndOutputParams)
{
    AudioInputParams sourceInput = makeVstInput(AudioResourceId("Steinway"));

    AudioOutputParams sourceOutput;
    sourceOutput.volume = -6.0f;
    sourceOutput.balance = 0.5f;
    sourceOutput.muted = true;
    sourceOutput.fxChain[0] = makeFxEffect(AudioResourceId("Reverb"));

    m_settings->setTrackInputParams(m_pianoOnPart1, sourceInput);
    m_settings->setTrackOutputParams(m_pianoOnPart1, sourceOutput);

    bool copySucceeded = m_settings->duplicateTrackParams(m_pianoOnPart1, m_pianoOnPart2);

    EXPECT_TRUE(copySucceeded);
    EXPECT_EQ(m_settings->trackInputParams(m_pianoOnPart2), sourceInput);
    EXPECT_TRUE(m_settings->trackHasExistingOutputParams(m_pianoOnPart2));
    EXPECT_EQ(m_settings->trackOutputParams(m_pianoOnPart2), sourceOutput);
}
