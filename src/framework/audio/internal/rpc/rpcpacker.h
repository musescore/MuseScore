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
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackEvent& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackEvent& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::SoundCategory& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::SoundCategory& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackSetupData& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackSetupData& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackParam& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackParam& value);
void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackData& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackData& value);

#include "global/serialization/msgpack.h"

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioResourceType& value)
{
    p(static_cast<int>(value));
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioResourceType& value)
{
    int type = 0;
    p(type);
    value = static_cast<muse::audio::AudioResourceType>(type);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioFxCategory& value)
{
    p(static_cast<int>(value));
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioFxCategory& value)
{
    int cat = 0;
    p(cat);
    value = static_cast<muse::audio::AudioFxCategory>(cat);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioResourceMeta& value)
{
    p(value.id, value.type, value.vendor, value.attributes, value.hasNativeEditorSupport);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioResourceMeta& value)
{
    p(value.id, value.type, value.vendor, value.attributes, value.hasNativeEditorSupport);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioFxParams& value)
{
    p(value.categories, value.chainOrder, value.resourceMeta, value.configuration, value.active);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioFxParams& value)
{
    p(value.categories, value.chainOrder, value.resourceMeta, value.configuration, value.active);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AuxSendParams& value)
{
    p(value.signalAmount, value.active);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AuxSendParams& value)
{
    p(value.signalAmount, value.active);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioOutputParams& value)
{
    p(value.fxChain, value.volume, value.balance,  value.auxSends, value.solo, value.muted, value.forceMute);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioOutputParams& value)
{
    p(value.fxChain, value.volume, value.balance, value.auxSends, value.solo, value.muted, value.forceMute);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioParams& value)
{
    p(value.in, value.out);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioParams& value)
{
    p(value.in, value.out);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundPreset& value)
{
    p(value.code, value.name, value.isDefault, value.attributes);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundPreset& value)
{
    p(value.code, value.name, value.isDefault, value.attributes);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioSourceParams& value)
{
    p(value.resourceMeta, value.configuration);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioSourceParams& value)
{
    p(value.resourceMeta, value.configuration);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundTrackType& value)
{
    p(static_cast<int>(value));
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundTrackType& value)
{
    int type = 0;
    p(type);
    value = static_cast<muse::audio::SoundTrackType>(type);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundTrackFormat& value)
{
    p(value.type, value.sampleRate, value.samplesPerChannel, value.audioChannelsNumber, value.bitRate);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundTrackFormat& value)
{
    p(value.type, value.sampleRate, value.samplesPerChannel, value.audioChannelsNumber, value.bitRate);
}

// MPE
inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArrangementContext& value)
{
    p(value.nominalTimestamp, value.actualTimestamp, value.nominalDuration, value.actualDuration,
      value.voiceLayerIndex, value.staffLayerIndex, value.bps);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArrangementContext& value)
{
    p(value.nominalTimestamp, value.actualTimestamp, value.nominalDuration, value.actualDuration,
      value.voiceLayerIndex, value.staffLayerIndex, value.bps);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PitchContext& value)
{
    p(value.nominalPitchLevel, value.pitchCurve);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PitchContext& value)
{
    p(value.nominalPitchLevel, value.pitchCurve);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArrangementPattern& value)
{
    p(value.durationFactor, value.timestampOffset);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArrangementPattern& value)
{
    p(value.durationFactor, value.timestampOffset);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PitchPattern& value)
{
    p(value.pitchOffsetMap);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PitchPattern& value)
{
    p(value.pitchOffsetMap);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ExpressionPattern& value)
{
    p(value.dynamicOffsetMap);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ExpressionPattern& value)
{
    p(value.dynamicOffsetMap);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationPatternSegment& value)
{
    p(value.arrangementPattern, value.pitchPattern, value.expressionPattern);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationPatternSegment& value)
{
    p(value.arrangementPattern, value.pitchPattern, value.expressionPattern);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationType& value)
{
    p(static_cast<int8_t>(value));
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationType& value)
{
    int8_t type = 0;
    p(type);
    value = static_cast<muse::mpe::ArticulationType>(type);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationMeta& value)
{
    p(value.type, value.pattern, value.timestamp, value.overallDuration,
      value.overallPitchChangesRange, value.overallDynamicChangesRange);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationMeta& value)
{
    p(value.type, value.pattern, value.timestamp, value.overallDuration,
      value.overallPitchChangesRange, value.overallDynamicChangesRange);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ArticulationAppliedData& value)
{
    p(value.meta, value.appliedPatternSegment, value.occupiedFrom, value.occupiedTo,
      value.occupiedPitchChangesRange, value.occupiedDynamicChangesRange);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ArticulationAppliedData& value)
{
    p(value.meta, value.appliedPatternSegment, value.occupiedFrom, value.occupiedTo,
      value.occupiedPitchChangesRange, value.occupiedDynamicChangesRange);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::ExpressionContext& value)
{
    p(value.articulations, value.nominalDynamicLevel, value.expressionCurve, value.velocityOverride);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::ExpressionContext& value)
{
    p(value.articulations, value.nominalDynamicLevel, value.expressionCurve, value.velocityOverride);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::NoteEvent& value)
{
    p(value.arrangementCtx(), value.pitchCtx(), value.expressionCtx());
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::NoteEvent& value)
{
    muse::mpe::ArrangementContext arrCtx;
    muse::mpe::PitchContext pitchCtx;
    muse::mpe::ExpressionContext exprCtx;
    p(arrCtx, pitchCtx, exprCtx);
    value = muse::mpe::NoteEvent(std::move(arrCtx), std::move(pitchCtx), std::move(exprCtx));
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::RestEvent& value)
{
    p(value.arrangementCtx());
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::RestEvent& value)
{
    muse::mpe::ArrangementContext arrCtx;
    p(arrCtx);
    value = muse::mpe::RestEvent(std::move(arrCtx));
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackEvent& value)
{
    uint8_t idx = static_cast<uint8_t>(value.index());
    p(idx);

    if (idx == 0) {
        // no data
    } else if (idx == 1) {
        const muse::mpe::NoteEvent& noteEvent = std::get<muse::mpe::NoteEvent>(value);
        p(noteEvent);
    } else if (idx == 2) {
        const muse::mpe::RestEvent& restEvent = std::get<muse::mpe::RestEvent>(value);
        p(restEvent);
    } else {
        assert(false && "not supported PlaybackEvent variant index");
    }
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackEvent& value)
{
    uint8_t idx = 0;
    p(idx);

    if (idx == 0) {
        value = {};
    } else if (idx == 1) {
        muse::mpe::ArrangementContext nullarrCtx;
        muse::mpe::PitchContext nullpitchCtx;
        muse::mpe::ExpressionContext nullexprCtx;
        muse::mpe::NoteEvent noteEvent = muse::mpe::NoteEvent(std::move(nullarrCtx), std::move(nullpitchCtx), std::move(nullexprCtx));
        p(noteEvent);
        value = noteEvent;
    } else if (idx == 2) {
        muse::mpe::ArrangementContext nullarrCtx;
        muse::mpe::RestEvent restEvent = muse::mpe::RestEvent(std::move(nullarrCtx));
        p(restEvent);
        value = restEvent;
    } else {
        assert(false && "not supported PlaybackEvent variant index");
    }
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::SoundCategory& value)
{
    p(static_cast<int8_t>(value));
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::SoundCategory& value)
{
    int8_t type = 0;
    p(type);
    value = static_cast<muse::mpe::SoundCategory>(type);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackSetupData& value)
{
    p(value.id, value.category, value.subCategories,
      value.supportsSingleNoteDynamics, value.musicXmlSoundId);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackSetupData& value)
{
    p(value.id, value.category, value.subCategories,
      value.supportsSingleNoteDynamics, value.musicXmlSoundId);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackParam& value)
{
    p(static_cast<int8_t>(value.type), value.val, value.flags);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackParam& value)
{
    int8_t type = 0;
    p(type, value.val, value.flags);
    value.type = static_cast<muse::mpe::PlaybackParam::Type>(type);
}

inline void pack_custom(muse::msgpack::Packer& p, const muse::mpe::PlaybackData& value)
{
    p(value.originEvents, value.setupData, value.dynamics, value.params);
}

inline void unpack_custom(muse::msgpack::UnPacker& p, muse::mpe::PlaybackData& value)
{
    p(value.originEvents, value.setupData, value.dynamics, value.params);
}

namespace muse::audio::rpc {
class RpcPacker
{
public:
    RpcPacker() = default;

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
