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
#pragma once

#include <cassert>

#include "global/serialization/msgpack_forward.h"
#include "../../audiotypes.h"

void pack_custom(muse::msgpack::Packer& p, const muse::audio::PlaybackStatus& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::PlaybackStatus& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioResourceType& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioResourceType& value);
void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioFxCategory& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioFxCategory& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioResourceMeta& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioResourceMeta& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioFxParams& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioFxParams& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::AuxSendParams& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AuxSendParams& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioSourceParams& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioSourceParams& value);
void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioOutputParams& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioOutputParams& value);
void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioParams& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioParams& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundPreset& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundPreset& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundTrackType& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundTrackType& value);
void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundTrackFormat& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundTrackFormat& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioSignalVal& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioSignalVal& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::InputProcessingProgress::ChunkInfo& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::InputProcessingProgress::ChunkInfo& value);
void pack_custom(muse::msgpack::Packer& p, const muse::audio::InputProcessingProgress::ProgressInfo& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::InputProcessingProgress::ProgressInfo& value);
void pack_custom(muse::msgpack::Packer& p, const muse::audio::InputProcessingProgress::StatusInfo& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::InputProcessingProgress::StatusInfo& value);

// MPE
// PlaybackEvent
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArrangementContext& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArrangementContext& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PitchContext& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PitchContext& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArrangementPattern& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArrangementPattern& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PitchPattern& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PitchPattern& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ExpressionPattern& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ExpressionPattern& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationPatternSegment& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationPatternSegment& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationType& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationType& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationMeta& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationMeta& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationAppliedData& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationAppliedData& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ExpressionContext& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ExpressionContext& value);

void pack_custom(muse::msgpack::Packer& p, const muse::mpe::NoteEvent& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::NoteEvent& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::RestEvent& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::RestEvent& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::TextArticulationEvent& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::TextArticulationEvent& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::SoundPresetChangeEvent& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::SoundPresetChangeEvent& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::SyllableEvent& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::SyllableEvent& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ControllerChangeEvent& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ControllerChangeEvent& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackEvent& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackEvent& value);

void pack_custom(muse::msgpack::Packer& p, const muse::mpe::SoundCategory& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::SoundCategory& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackSetupData& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackSetupData& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackData& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackData& value);

#include "global/serialization/msgpack.h"

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::PlaybackStatus& value)
{
    p.process(static_cast<int8_t>(value));
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::PlaybackStatus& value)
{
    int8_t val = 0;
    p.process(val);
    value = static_cast<muse::audio::PlaybackStatus>(val);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioResourceType& value)
{
    p.process(static_cast<int8_t>(value));
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioResourceType& value)
{
    int type = 0;
    p.process(type);
    value = static_cast<muse::audio::AudioResourceType>(type);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioFxCategory& value)
{
    p.process(static_cast<int>(value));
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioFxCategory& value)
{
    int cat = 0;
    p.process(cat);
    value = static_cast<muse::audio::AudioFxCategory>(cat);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioResourceMeta& value)
{
    p.process(value.id, value.type, value.vendor, value.attributes, value.hasNativeEditorSupport);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioResourceMeta& value)
{
    p.process(value.id, value.type, value.vendor, value.attributes, value.hasNativeEditorSupport);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioFxParams& value)
{
    p.process(value.categories, value.chainOrder, value.resourceMeta, value.configuration, value.active);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioFxParams& value)
{
    p.process(value.categories, value.chainOrder, value.resourceMeta, value.configuration, value.active);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AuxSendParams& value)
{
    p.process(value.signalAmount, value.active);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AuxSendParams& value)
{
    p.process(value.signalAmount, value.active);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioOutputParams& value)
{
    p.process(value.fxChain, value.volume, value.balance,  value.auxSends, value.solo, value.muted, value.forceMute);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioOutputParams& value)
{
    p.process(value.fxChain, value.volume, value.balance, value.auxSends, value.solo, value.muted, value.forceMute);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioParams& value)
{
    p.process(value.in, value.out);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioParams& value)
{
    p.process(value.in, value.out);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundPreset& value)
{
    p.process(value.code, value.name, value.isDefault, value.attributes);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundPreset& value)
{
    p.process(value.code, value.name, value.isDefault, value.attributes);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioSourceParams& value)
{
    p.process(value.resourceMeta, value.configuration);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioSourceParams& value)
{
    p.process(value.resourceMeta, value.configuration);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundTrackType& value)
{
    p.process(static_cast<int>(value));
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundTrackType& value)
{
    int type = 0;
    p.process(type);
    value = static_cast<muse::audio::SoundTrackType>(type);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundTrackFormat& value)
{
    p.process(value.type, value.sampleRate, value.samplesPerChannel, value.audioChannelsNumber, value.bitRate);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundTrackFormat& value)
{
    p.process(value.type, value.sampleRate, value.samplesPerChannel, value.audioChannelsNumber, value.bitRate);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioSignalVal& value)
{
    p.process(value.amplitude, value.pressure);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioSignalVal& value)
{
    p.process(value.amplitude, value.pressure);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::InputProcessingProgress::ChunkInfo& value)
{
    p.process(value.start, value.end);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::InputProcessingProgress::ChunkInfo& value)
{
    p.process(value.start, value.end);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::InputProcessingProgress::ProgressInfo& value)
{
    p.process(value.current, value.total);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::InputProcessingProgress::ProgressInfo& value)
{
    p.process(value.current, value.total);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::InputProcessingProgress::StatusInfo& value)
{
    p.process(static_cast<uint8_t>(value.status), value.errcode);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::InputProcessingProgress::StatusInfo& value)
{
    uint8_t status = 0;
    p.process(status, value.errcode);
    value.status = static_cast<muse::audio::InputProcessingProgress::Status>(status);
}

// MPE
inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArrangementContext& value)
{
    p.process(value.nominalTimestamp, value.actualTimestamp, value.nominalDuration, value.actualDuration,
              value.voiceLayerIndex, value.staffLayerIndex, value.bps);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArrangementContext& value)
{
    p.process(value.nominalTimestamp, value.actualTimestamp, value.nominalDuration, value.actualDuration,
              value.voiceLayerIndex, value.staffLayerIndex, value.bps);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PitchContext& value)
{
    p.process(value.nominalPitchLevel, value.pitchCurve);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PitchContext& value)
{
    p.process(value.nominalPitchLevel, value.pitchCurve);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArrangementPattern& value)
{
    p.process(value.durationFactor, value.timestampOffset);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArrangementPattern& value)
{
    p.process(value.durationFactor, value.timestampOffset);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PitchPattern& value)
{
    p.process(value.pitchOffsetMap);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PitchPattern& value)
{
    p.process(value.pitchOffsetMap);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ExpressionPattern& value)
{
    p.process(value.dynamicOffsetMap);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ExpressionPattern& value)
{
    p.process(value.dynamicOffsetMap);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationPatternSegment& value)
{
    p.process(value.arrangementPattern, value.pitchPattern, value.expressionPattern);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationPatternSegment& value)
{
    p.process(value.arrangementPattern, value.pitchPattern, value.expressionPattern);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationType& value)
{
    p.process(static_cast<int8_t>(value));
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationType& value)
{
    int8_t type = 0;
    p.process(type);
    value = static_cast<muse::mpe::ArticulationType>(type);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationMeta& value)
{
    p.process(value.type, value.pattern, value.timestamp, value.overallDuration,
              value.overallPitchChangesRange, value.overallDynamicChangesRange);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationMeta& value)
{
    p.process(value.type, value.pattern, value.timestamp, value.overallDuration,
              value.overallPitchChangesRange, value.overallDynamicChangesRange);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationAppliedData& value)
{
    p.process(value.meta, value.appliedPatternSegment, value.occupiedFrom, value.occupiedTo,
              value.occupiedPitchChangesRange, value.occupiedDynamicChangesRange);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationAppliedData& value)
{
    p.process(value.meta, value.appliedPatternSegment, value.occupiedFrom, value.occupiedTo,
              value.occupiedPitchChangesRange, value.occupiedDynamicChangesRange);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ExpressionContext& value)
{
    p.process(value.articulations, value.nominalDynamicLevel, value.expressionCurve, value.velocityOverride);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ExpressionContext& value)
{
    p.process(value.articulations, value.nominalDynamicLevel, value.expressionCurve, value.velocityOverride);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::NoteEvent& value)
{
    p.process(value.arrangementCtx(), value.pitchCtx(), value.expressionCtx());
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::NoteEvent& value)
{
    muse::mpe::ArrangementContext arrCtx;
    muse::mpe::PitchContext pitchCtx;
    muse::mpe::ExpressionContext exprCtx;
    p.process(arrCtx, pitchCtx, exprCtx);
    value = muse::mpe::NoteEvent(std::move(arrCtx), std::move(pitchCtx), std::move(exprCtx));
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::RestEvent& value)
{
    p.process(value.arrangementCtx());
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::RestEvent& value)
{
    muse::mpe::ArrangementContext arrCtx;
    p.process(arrCtx);
    value = muse::mpe::RestEvent(std::move(arrCtx));
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::TextArticulationEvent& value)
{
    p.process(value.text, value.layerIdx, value.flags);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::TextArticulationEvent& value)
{
    p.process(value.text, value.layerIdx, value.flags);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::SoundPresetChangeEvent& value)
{
    p.process(value.code, value.layerIdx);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::SoundPresetChangeEvent& value)
{
    p.process(value.code, value.layerIdx);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::SyllableEvent& value)
{
    p.process(value.text, value.layerIdx, value.flags);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::SyllableEvent& value)
{
    p.process(value.text, value.layerIdx, value.flags);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ControllerChangeEvent& value)
{
    p.process(static_cast<int8_t>(value.type), value.val, value.layerIdx);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ControllerChangeEvent& value)
{
    int8_t type = 0;
    p.process(type, value.val, value.layerIdx);
    value.type = static_cast<muse::mpe::ControllerChangeEvent::Type>(type);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackEvent& value)
{
    uint8_t idx = static_cast<uint8_t>(value.index());
    p.process(idx);

    switch (idx) {
    case 0:
        // no data
        break;
    case 1: {
        const muse::mpe::NoteEvent& event = std::get<muse::mpe::NoteEvent>(value);
        p.process(event);
    } break;
    case 2: {
        const muse::mpe::RestEvent& event = std::get<muse::mpe::RestEvent>(value);
        p.process(event);
    } break;
    case 3: {
        const muse::mpe::TextArticulationEvent& event = std::get<muse::mpe::TextArticulationEvent>(value);
        p.process(event);
    } break;
    case 4: {
        const muse::mpe::SoundPresetChangeEvent& event = std::get<muse::mpe::SoundPresetChangeEvent>(value);
        p.process(event);
    } break;
    case 5: {
        const muse::mpe::SyllableEvent& event = std::get<muse::mpe::SyllableEvent>(value);
        p.process(event);
    } break;
    case 6: {
        const muse::mpe::ControllerChangeEvent& event = std::get<muse::mpe::ControllerChangeEvent>(value);
        p.process(event);
    } break;
    default: {
        assert(false && "unknown PlaybackEvent variant index");
    }
    }
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackEvent& value)
{
    uint8_t idx = 0;
    p.process(idx);

    switch (idx) {
    case 0:
        // no data
        break;
    case 1: {
        muse::mpe::NoteEvent event;
        p.process(event);
        value = event;
    } break;
    case 2: {
        muse::mpe::RestEvent event;
        p.process(event);
        value = event;
    } break;
    case 3: {
        muse::mpe::TextArticulationEvent event;
        p.process(event);
        value = event;
    } break;
    case 4: {
        muse::mpe::SoundPresetChangeEvent event;
        p.process(event);
        value = event;
    } break;
    case 5: {
        muse::mpe::SyllableEvent event;
        p.process(event);
        value = event;
    } break;
    case 6: {
        muse::mpe::ControllerChangeEvent event;
        p.process(event);
        value = event;
    } break;
    default: {
        assert(false && "unknown PlaybackEvent variant index");
    }
    }
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::SoundCategory& value)
{
    p.process(static_cast<int8_t>(value));
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::SoundCategory& value)
{
    int8_t type = 0;
    p.process(type);
    value = static_cast<muse::mpe::SoundCategory>(type);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackSetupData& value)
{
    p.process(value.id, value.category, value.subCategories,
              value.supportsSingleNoteDynamics, value.musicXmlSoundId, value.scoreId);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackSetupData& value)
{
    p.process(value.id, value.category, value.subCategories,
              value.supportsSingleNoteDynamics, value.musicXmlSoundId, value.scoreId);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackData& value)
{
    p.process(value.originEvents, value.setupData, value.dynamics);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackData& value)
{
    p.process(value.originEvents, value.setupData, value.dynamics);
}

namespace muse::audio::rpc {
using Options = msgpack::Options;
class RpcPacker
{
public:
    RpcPacker() = default;

    template<class ... Types>
    static ByteArray pack(const Options& opt, const Types&... args)
    {
        return msgpack::pack(opt, args ...);
    }

    template<class ... Types>
    static ByteArray pack(const Types&... args)
    {
        return msgpack::pack(args ...);
    }

    template<class ... Types>
    static bool unpack(const ByteArray& data, Types&... args)
    {
        return msgpack::unpack(data, args ...);
    }
};
}
