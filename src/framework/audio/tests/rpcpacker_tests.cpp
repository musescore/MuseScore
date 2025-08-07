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

#pragma pack(push, 1)
#include "audio/common/audiotypes.h"
#pragma pack(pop)

#include "audio/common/rpc/rpcpacker.h"

using namespace muse;
using namespace muse::audio;

class Audio_RpcPackerTests : public ::testing::Test
{
public:
};

template<typename ... Fields>
static constexpr size_t sum_sizeof()
{
    size_t s = (0 + ... + sizeof(Fields));
    return s;
}

template<typename T, typename ... Fields>
static constexpr void KNOWN_FIELDS(const T&, const Fields&...)
{
    //! NOTE If the asserted, it means the size of the type or structure has changed
    //! and you need to improve the packing and unpacking functions
    //! and write the new size in the test.
    static_assert(sizeof(T) == sum_sizeof<Fields...>());
}

TEST_F(Audio_RpcPackerTests, AudioResourceMeta)
{
    AudioResourceMeta origin;
    origin.id = "1234";
    origin.type = AudioResourceType::MusePlugin;
    origin.vendor = "muse";
    origin.attributes.insert({ u"key", u"val" });
    origin.hasNativeEditorSupport = true;

    KNOWN_FIELDS(origin,
                 origin.id,
                 origin.type,
                 origin.vendor,
                 origin.attributes,
                 origin.hasNativeEditorSupport);

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

TEST_F(Audio_RpcPackerTests, AudioResourceMetaList)
{
    int count = 1000;
    AudioResourceMetaList origin;
    origin.reserve(count);

    for (int i = 0; i < count; ++i) {
        AudioResourceMeta meta;
        meta.id = std::to_string(1234567);
        meta.type = AudioResourceType::MuseSamplerSoundPack;
        meta.vendor = "MuseSounds";
        meta.attributes = {
            { u"playbackSetupData", u"instrumentSoundId" },
            { u"museCategory", u"internalCategory" },
            { u"musePack", u"instrumentPackName" },
            { u"museVendorName", u"vendorName" },
            { u"museName", u"internalName" },
            { u"museUID", String::fromStdString(std::to_string(1234567)) },
        };

        meta.attributes.insert(std::make_pair(u"isOnline", String::number(1)));

        origin.push_back(meta);
    }

    BEGIN_STEP_TIME("pack_meta_list");
    ByteArray data = rpc::RpcPacker::pack(rpc::Options { origin.size() * 300 }, origin);
    STEP_TIME("pack_meta_list", "after pack");
    LOGDA() << "data.size: " << data.size();
}

TEST_F(Audio_RpcPackerTests, AudioFxParams)
{
    AudioFxParams origin;
    origin.categories.insert(AudioFxCategory::FxEqualizer);
    origin.chainOrder = 3;
    origin.resourceMeta.id = "1234";
    origin.configuration.insert({ "key", "val" });
    origin.active = false;

    KNOWN_FIELDS(origin,
                 origin.categories,
                 origin.chainOrder,
                 origin.resourceMeta,
                 origin.configuration,
                 origin.active);

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

    KNOWN_FIELDS(origin,
                 origin.signalAmount,
                 origin.active);

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

    KNOWN_FIELDS(origin,
                 origin.fxChain,
                 origin.volume,
                 origin.balance,
                 origin.auxSends,
                 origin.forceMute,
                 origin.muted,
                 origin.solo);

    ByteArray data = rpc::RpcPacker::pack(origin);

    AudioOutputParams unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, AudioParams)
{
    AudioParams origin;

    KNOWN_FIELDS(origin,
                 origin.in,
                 origin.out);

    ByteArray data = rpc::RpcPacker::pack(origin);

    AudioParams unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin.in == unpacked.in);
    EXPECT_TRUE(origin.out == unpacked.out);
}

TEST_F(Audio_RpcPackerTests, SoundPreset)
{
    SoundPreset origin;
    origin.code = u"code";
    origin.name = u"name";
    origin.isDefault = true;
    origin.attributes.insert({ u"key", u"val" });

    KNOWN_FIELDS(origin,
                 origin.code,
                 origin.name,
                 origin.isDefault,
                 origin.attributes);

    ByteArray data = rpc::RpcPacker::pack(origin);

    SoundPreset unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, SoundTrackFormat)
{
    SoundTrackFormat origin;
    origin.type = SoundTrackType::OGG;
    origin.sampleRate = 44000;
    origin.samplesPerChannel = 256;
    origin.audioChannelsNumber = 2;
    origin.bitRate = 196;

    KNOWN_FIELDS(origin,
                 origin.type,
                 origin.sampleRate,
                 origin.samplesPerChannel,
                 origin.audioChannelsNumber,
                 origin.bitRate);

    ByteArray data = rpc::RpcPacker::pack(origin);

    SoundTrackFormat unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, AudioSourceParams)
{
    AudioSourceParams origin;
    origin.resourceMeta.id = "1234";
    origin.configuration.insert({ "key", "val" });

    KNOWN_FIELDS(origin,
                 origin.resourceMeta,
                 origin.configuration);

    ByteArray data = rpc::RpcPacker::pack(origin);

    AudioSourceParams unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, AudioSignalVal)
{
    AudioSignalVal origin;
    origin.amplitude = 0.6;
    origin.pressure = 0.5;

    KNOWN_FIELDS(origin,
                 origin.amplitude,
                 origin.pressure);

    ByteArray data = rpc::RpcPacker::pack(origin);

    AudioSignalVal unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

TEST_F(Audio_RpcPackerTests, InputProcessingProgress)
{
    // InputProcessingProgress
    {
        InputProcessingProgress origin;

        KNOWN_FIELDS(origin,
                     origin.isStarted,
                     origin.processedChannel);
    }

    // ChunkInfo
    {
        InputProcessingProgress::ChunkInfo origin;
        origin.start = 1.6;
        origin.end = 2.5;

        KNOWN_FIELDS(origin,
                     origin.start,
                     origin.end);

        ByteArray data = rpc::RpcPacker::pack(origin);

        InputProcessingProgress::ChunkInfo unpacked;
        bool ok = rpc::RpcPacker::unpack(data, unpacked);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(origin == unpacked);
    }

    // ProgressInfo
    {
        InputProcessingProgress::ProgressInfo origin;
        origin.current = 16;
        origin.total = 25;

        KNOWN_FIELDS(origin,
                     origin.current,
                     origin.total);

        ByteArray data = rpc::RpcPacker::pack(origin);

        InputProcessingProgress::ProgressInfo unpacked;
        bool ok = rpc::RpcPacker::unpack(data, unpacked);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(origin.current == unpacked.current);
        EXPECT_TRUE(origin.total == unpacked.total);
    }

    // ProgressInfo
    {
        InputProcessingProgress::StatusInfo origin;
        origin.status = InputProcessingProgress::Status::Started;
        origin.errcode = 73;

        KNOWN_FIELDS(origin,
                     origin.status,
                     origin.errcode);

        ByteArray data = rpc::RpcPacker::pack(origin);

        InputProcessingProgress::StatusInfo unpacked;
        bool ok = rpc::RpcPacker::unpack(data, unpacked);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(origin.status == unpacked.status);
        EXPECT_TRUE(origin.errcode == unpacked.errcode);
    }
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

    KNOWN_FIELDS(origin,
                 origin.nominalTimestamp,
                 origin.actualTimestamp,
                 origin.nominalDuration,
                 origin.actualDuration,
                 origin.voiceLayerIndex,
                 origin.staffLayerIndex,
                 origin.bps);

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

    KNOWN_FIELDS(origin,
                 origin.nominalPitchLevel,
                 origin.pitchCurve);

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

    KNOWN_FIELDS(origin,
                 origin.durationFactor,
                 origin.timestampOffset);

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

    KNOWN_FIELDS(origin,
                 origin.pitchOffsetMap);

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

    KNOWN_FIELDS(origin,
                 origin.dynamicOffsetMap);

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

    KNOWN_FIELDS(origin,
                 origin.arrangementPattern,
                 origin.pitchPattern,
                 origin.expressionPattern);

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

    KNOWN_FIELDS(origin,
                 origin.type,
                 origin.pattern,
                 origin.timestamp,
                 origin.overallDuration,
                 origin.overallPitchChangesRange,
                 origin.overallDynamicChangesRange);

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

    KNOWN_FIELDS(origin,
                 origin.meta,
                 origin.appliedPatternSegment,
                 origin.occupiedFrom,
                 origin.occupiedTo,
                 origin.occupiedPitchChangesRange,
                 origin.occupiedDynamicChangesRange);

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

    KNOWN_FIELDS(origin,
                 origin.articulations,
                 origin.nominalDynamicLevel,
                 origin.expressionCurve,
                 origin.velocityOverride);

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::ExpressionContext unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

// void pack_custom(muse::msgpack::Packer& p, const muse::mpe::SyllableEvent& value);
// void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::SyllableEvent& value);
// void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ControllerChangeEvent& value);
// void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ControllerChangeEvent& value);
// void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackEvent& value);
// void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackEvent& value);

TEST_F(Audio_RpcPackerTests, MPE_PlaybackEvent)
{
    // NoteEvent
    {
        muse::mpe::ArrangementContext arrCtx = makeArrangementContext();
        muse::mpe::PitchContext pitchCtx = makePitchContext();
        muse::mpe::ExpressionContext exprCtx = makeExpressionContext();
        mpe::NoteEvent event = muse::mpe::NoteEvent(std::move(arrCtx), std::move(pitchCtx), std::move(exprCtx));

        KNOWN_FIELDS(event,
                     event.arrangementCtx(),
                     event.pitchCtx(),
                     event.expressionCtx());

        mpe::PlaybackEvent origin = event;

        ByteArray data = rpc::RpcPacker::pack(origin);

        mpe::PlaybackEvent unpacked;
        bool ok = rpc::RpcPacker::unpack(data, unpacked);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(origin == unpacked);
    }

    // RestEvent
    {
        muse::mpe::ArrangementContext arrCtx = makeArrangementContext();
        mpe::RestEvent event = muse::mpe::RestEvent(std::move(arrCtx));

        KNOWN_FIELDS(event,
                     event.arrangementCtx());

        mpe::PlaybackEvent origin = event;

        ByteArray data = rpc::RpcPacker::pack(origin);

        mpe::PlaybackEvent unpacked;
        bool ok = rpc::RpcPacker::unpack(data, unpacked);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(origin == unpacked);
    }

    // TextArticulationEvent
    {
        mpe::TextArticulationEvent event;
        event.text = "ggg";
        event.layerIdx = 3;
        event.flags = mpe::TextArticulationEvent::FlagType::StartsAtPlaybackPosition;

        KNOWN_FIELDS(event,
                     event.text,
                     event.layerIdx,
                     event.flags);

        mpe::PlaybackEvent origin = event;

        ByteArray data = rpc::RpcPacker::pack(origin);

        mpe::PlaybackEvent unpacked;
        bool ok = rpc::RpcPacker::unpack(data, unpacked);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(origin == unpacked);
    }

    // SoundPresetChangeEvent
    {
        mpe::SoundPresetChangeEvent event;
        event.code = "ggg";
        event.layerIdx = 3;

        KNOWN_FIELDS(event,
                     event.code,
                     event.layerIdx);

        mpe::PlaybackEvent origin = event;

        ByteArray data = rpc::RpcPacker::pack(origin);

        mpe::PlaybackEvent unpacked;
        bool ok = rpc::RpcPacker::unpack(data, unpacked);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(origin == unpacked);
    }

    // SyllableEvent
    {
        mpe::SyllableEvent event;
        event.text = "ggg";
        event.layerIdx = 3;
        event.flags = mpe::SyllableEvent::FlagType::StartsAtPlaybackPosition;

        KNOWN_FIELDS(event,
                     event.text,
                     event.layerIdx,
                     event.flags);

        mpe::PlaybackEvent origin = event;

        ByteArray data = rpc::RpcPacker::pack(origin);

        mpe::PlaybackEvent unpacked;
        bool ok = rpc::RpcPacker::unpack(data, unpacked);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(origin == unpacked);
    }

    // ControllerChangeEvent
    {
        mpe::ControllerChangeEvent event;
        event.type = mpe::ControllerChangeEvent::Type::Modulation;
        event.val = 0.4;
        event.layerIdx = 2;

        KNOWN_FIELDS(event,
                     event.type,
                     event.val,
                     event.layerIdx);

        mpe::PlaybackEvent origin = event;

        ByteArray data = rpc::RpcPacker::pack(origin);

        mpe::PlaybackEvent unpacked;
        bool ok = rpc::RpcPacker::unpack(data, unpacked);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(origin == unpacked);
    }

    {
        using KnownPlaybackEvent = std::variant<std::monostate,
                                                mpe::NoteEvent,
                                                mpe::RestEvent,
                                                mpe::TextArticulationEvent,
                                                mpe::SoundPresetChangeEvent,
                                                mpe::SyllableEvent,
                                                mpe::ControllerChangeEvent>;

        static_assert(std::is_same<mpe::PlaybackEvent, KnownPlaybackEvent>::value);
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
    origin.scoreId = "678";

    KNOWN_FIELDS(origin,
                 origin.id,
                 origin.category,
                 origin.subCategories,
                 origin.supportsSingleNoteDynamics,
                 origin.musicXmlSoundId,
                 origin.scoreId);

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::PlaybackSetupData unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
    EXPECT_TRUE(origin.scoreId == unpacked.scoreId);
    EXPECT_TRUE(origin.musicXmlSoundId == unpacked.musicXmlSoundId);
}

TEST_F(Audio_RpcPackerTests, MPE_PlaybackData)
{
    mpe::PlaybackData origin;

    KNOWN_FIELDS(origin,
                 origin.originEvents,
                 origin.setupData,
                 origin.dynamics,
                 origin.mainStream,
                 origin.offStream);

    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::PlaybackData unpacked;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}

template<typename ... Types>
static void unpack_stream(ByteArray data, async::Channel<Types...> ch)
{
    std::tuple<Types...> values;
    bool success = std::apply([data](auto&... args) {
        return rpc::RpcPacker::unpack(data, args ...);
    }, values);

    if (success) {
        std::apply([&ch](const auto&... args) {
            ch.send(args ...);
        }, values);
    }
}

TEST_F(Audio_RpcPackerTests, StreamUnpack)
{
    int v1_1 = 42;
    double v1_2 = 73.0;
    std::string v1_3 = "gg";

    ByteArray data = rpc::RpcPacker::pack(v1_1, v1_2, v1_3);

    async::Channel<int, double, std::string> ch;

    ch.onReceive(nullptr, [v1_1, v1_2, v1_3](int v2_1, double v2_2, std::string v2_3) {
        EXPECT_EQ(v1_1, v2_1);
        EXPECT_DOUBLE_EQ(v1_2, v2_2);
        EXPECT_EQ(v1_3, v2_3);
    });

    unpack_stream(data, ch);
}

TEST_F(Audio_RpcPackerTests, Duration)
{
    mpe::duration_t origin = 9223372036854775807;
    ByteArray data = rpc::RpcPacker::pack(origin);

    mpe::duration_t unpacked = 0;
    bool ok = rpc::RpcPacker::unpack(data, unpacked);

    EXPECT_TRUE(ok);
    EXPECT_TRUE(origin == unpacked);
}
