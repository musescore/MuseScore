/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "audio/internal/rpc/rpcpacker.h"

using namespace muse;
using namespace muse::audio;

class Audio_RpcPackerTests : public ::testing::Test
{
public:
};

TEST_F(Audio_RpcPackerTests, AudioResourceMeta)
{
    AudioResourceMeta origin;
    origin.id = "1234";
    origin.type = AudioResourceType::MusePlugin;
    origin.vendor = "muse";
    origin.attributes.insert({ u"key", u"val" });
    origin.hasNativeEditorSupport = true;

    ByteArray data = rpc::RpcPacker::pack(origin);

    AudioResourceMeta unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin.id == unpacked.id);
    EXPECT_TRUE(origin.type == unpacked.type);
    EXPECT_TRUE(origin.vendor == unpacked.vendor);
    EXPECT_TRUE(origin.attributes == unpacked.attributes);
    EXPECT_TRUE(origin.hasNativeEditorSupport == unpacked.hasNativeEditorSupport);
}

TEST_F(Audio_RpcPackerTests, AudioFxParams)
{
    AudioFxParams origin;
    origin.categories.insert(AudioFxCategory::FxEqualizer);
    origin.chainOrder = 3;
    origin.resourceMeta.id = "1234";
    origin.configuration.insert({ "key", "val" });
    origin.active = false;

    ByteArray data = rpc::RpcPacker::pack(origin);

    AudioFxParams unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, AuxSendParams)
{
    AuxSendParams origin;
    origin.signalAmount = 0.42;
    origin.active = true;

    ByteArray data = rpc::RpcPacker::pack(origin);

    AuxSendParams unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, AudioOutputParams)
{
    AudioOutputParams origin;
    origin.fxChain.insert({ 3, {} });
    origin.volume = 0.6;
    origin.balance = 0.5;
    origin.auxSends.push_back({});
    origin.forceMute = true;
    origin.muted = true;
    origin.solo = true;

    ByteArray data = rpc::RpcPacker::pack(origin);

    AudioOutputParams unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, SoundPreset)
{
    SoundPreset origin;
    origin.code = u"code";
    origin.name = u"name";
    origin.isDefault = true;
    origin.attributes.insert({ u"key", u"val" });

    ByteArray data = rpc::RpcPacker::pack(origin);

    SoundPreset unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, AudioSourceParams)
{
    AudioSourceParams origin;
    origin.resourceMeta.id = "1234";
    origin.configuration.insert({ "key", "val" });

    ByteArray data = rpc::RpcPacker::pack(origin);

    AudioSourceParams unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

// MPE
static mpe::ArrangementContext makeArrangementContext()
{
    mpe::ArrangementContext origin;
    origin.nominalTimestamp = 12;
    origin.actualTimestamp = 13;
    origin.nominalDuration = 14;
    origin.actualDuration = 15;
    origin.voiceLayerIndex = 16;
    origin.staffLayerIndex = 17;
    origin.bps = 18.19;
    return origin;
}

TEST_F(Audio_RpcPackerTests, MPE_ArrangementContext)
{
    mpe::ArrangementContext origin = makeArrangementContext();

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::ArrangementContext unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

static mpe::PitchContext makePitchContext()
{
    mpe::PitchContext origin;
    origin.nominalPitchLevel = 2;
    origin.pitchCurve.insert({ 12, 14 });
    return origin;
}

TEST_F(Audio_RpcPackerTests, MPE_PitchContext)
{
    mpe::PitchContext origin = makePitchContext();

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::PitchContext unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, MPE_ArrangementPattern)
{
    mpe::ArrangementPattern origin;
    origin.durationFactor = 2;
    origin.timestampOffset = 3;

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::ArrangementPattern unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, MPE_PitchPattern)
{
    mpe::PitchPattern origin;
    origin.pitchOffsetMap.insert({ 12, 14 });

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::PitchPattern unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, MPE_ExpressionPattern)
{
    mpe::ExpressionPattern origin;
    origin.dynamicOffsetMap.insert({ 12, 14 });

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::ExpressionPattern unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

static mpe::ArticulationPatternSegment makeArticulationPatternSegment()
{
    mpe::ArticulationPatternSegment origin;
    origin.arrangementPattern.durationFactor = 6;
    origin.arrangementPattern.timestampOffset = 2;
    origin.pitchPattern.pitchOffsetMap.insert({ 2, 4 });
    origin.expressionPattern.dynamicOffsetMap.insert({ 15, 34 });
    return origin;
}

TEST_F(Audio_RpcPackerTests, MPE_ArticulationPatternSegment)
{
    mpe::ArticulationPatternSegment origin = makeArticulationPatternSegment();

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::ArticulationPatternSegment unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

static mpe::ArticulationMeta makeArticulationMeta()
{
    mpe::ArticulationMeta origin;
    origin.type = mpe::ArticulationType::Standard;
    origin.pattern.insert({ 5, makeArticulationPatternSegment() });
    origin.timestamp = 7;
    origin.overallDuration = 8;
    origin.overallPitchChangesRange = 9;
    origin.overallDynamicChangesRange = 10;
    return origin;
}

TEST_F(Audio_RpcPackerTests, MPE_ArticulationMeta)
{
    mpe::ArticulationMeta origin = makeArticulationMeta();

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::ArticulationMeta unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

static mpe::ArticulationAppliedData makeArticulationAppliedData()
{
    mpe::ArticulationAppliedData origin;
    origin.meta = makeArticulationMeta();
    origin.appliedPatternSegment = makeArticulationPatternSegment();
    origin.occupiedFrom = 4;
    origin.occupiedTo = mpe::HUNDRED_PERCENT;
    origin.occupiedPitchChangesRange = 34;
    origin.occupiedDynamicChangesRange = 35;
    return origin;
}

TEST_F(Audio_RpcPackerTests, MPE_ArticulationAppliedData)
{
    mpe::ArticulationAppliedData origin = makeArticulationAppliedData();

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::ArticulationAppliedData unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

static mpe::ExpressionContext makeExpressionContext()
{
    mpe::ExpressionContext origin;
    origin.articulations.insert({ mpe::ArticulationType::Accent, makeArticulationAppliedData() });
    origin.nominalDynamicLevel = 2;
    origin.expressionCurve.insert({ 56, 68 });
    origin.velocityOverride = std::make_optional<float>(0.6);
    return origin;
}

TEST_F(Audio_RpcPackerTests, MPE_ExpressionContext)
{
    mpe::ExpressionContext origin = makeExpressionContext();

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::ExpressionContext unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

static mpe::NoteEvent makeNoteEvent()
{
    muse::mpe::ArrangementContext arrCtx = makeArrangementContext();
    muse::mpe::PitchContext pitchCtx = makePitchContext();
    muse::mpe::ExpressionContext exprCtx = makeExpressionContext();
    mpe::NoteEvent origin = muse::mpe::NoteEvent(std::move(arrCtx), std::move(pitchCtx), std::move(exprCtx));
    return origin;
}

TEST_F(Audio_RpcPackerTests, MPE_NoteEvent)
{
    mpe::NoteEvent origin = makeNoteEvent();

    ByteArray data = rpc::RpcPacker::pack(origin);

    muse::mpe::ArrangementContext nullarrCtx;
    muse::mpe::PitchContext nullpitchCtx;
    muse::mpe::ExpressionContext nullexprCtx;
    mpe::NoteEvent unpacked = muse::mpe::NoteEvent(std::move(nullarrCtx), std::move(nullpitchCtx), std::move(nullexprCtx));
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

static mpe::RestEvent makeRestEvent()
{
    muse::mpe::ArrangementContext arrCtx = makeArrangementContext();
    mpe::RestEvent origin = muse::mpe::RestEvent(std::move(arrCtx));
    return origin;
}

TEST_F(Audio_RpcPackerTests, MPE_RestEvent)
{
    mpe::RestEvent origin = makeRestEvent();

    ByteArray data = rpc::RpcPacker::pack(origin);

    muse::mpe::ArrangementContext nullarrCtx;
    mpe::RestEvent unpacked = muse::mpe::RestEvent(std::move(nullarrCtx));
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, MPE_PlaybackEvent)
{
    // NoteEvent
    {
        mpe::PlaybackEvent origin = makeNoteEvent();

        ByteArray data = rpc::RpcPacker::pack(origin);

        muse::mpe::ArrangementContext nullarrCtx;
        muse::mpe::PitchContext nullpitchCtx;
        muse::mpe::ExpressionContext nullexprCtx;
        mpe::NoteEvent null = muse::mpe::NoteEvent(std::move(nullarrCtx), std::move(nullpitchCtx), std::move(nullexprCtx));
        mpe::PlaybackEvent unpacked = null;
        bool ok = rpc::RpcPacker::unpack(data, unpacked);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(origin == unpacked);
    }

    // RestEvent
    {
        mpe::PlaybackEvent origin = makeRestEvent();

        ByteArray data = rpc::RpcPacker::pack(origin);

        muse::mpe::ArrangementContext nullarrCtx;
        mpe::RestEvent null = muse::mpe::RestEvent(std::move(nullarrCtx));
        mpe::PlaybackEvent unpacked = null;
        bool ok = rpc::RpcPacker::unpack(data, unpacked);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(origin == unpacked);
    }
}

TEST_F(Audio_RpcPackerTests, MPE_PlaybackSetupData)
{
    mpe::PlaybackSetupData origin;
    origin.id = u"123";
    origin.category = mpe::SoundCategory::Keyboards;
    origin.subCategories = { u"ggg" };
    origin.supportsSingleNoteDynamics = true;
    origin.musicXmlSoundId = "567";

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::PlaybackSetupData unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, MPE_PlaybackData)
{
    mpe::PlaybackData origin;

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::PlaybackData unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}
