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

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::PlaybackStatus& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::PlaybackStatus& value);

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioResourceType& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioResourceType& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioFxCategory& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioFxCategory& value);

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioResourceMeta& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioResourceMeta& value);

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioFxParams& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioFxParams& value);

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AuxSendParams& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AuxSendParams& value);

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioSourceParams& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioSourceParams& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioOutputParams& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioOutputParams& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioParams& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioParams& value);

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::SoundPreset& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::SoundPreset& value);

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::SoundTrackType& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::SoundTrackType& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::SoundTrackFormat& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::SoundTrackFormat& value);

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioSignalVal& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioSignalVal& value);

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::InputProcessingProgress::ChunkInfo& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::InputProcessingProgress::ChunkInfo& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::InputProcessingProgress::ProgressInfo& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::InputProcessingProgress::ProgressInfo& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::InputProcessingProgress::StatusInfo& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::InputProcessingProgress::StatusInfo& value);

// MPE
// PlaybackEvent
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArrangementContext& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArrangementContext& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::PitchContext& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::PitchContext& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArrangementPattern& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArrangementPattern& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::PitchPattern& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::PitchPattern& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ExpressionPattern& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ExpressionPattern& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArticulationPatternSegment& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArticulationPatternSegment& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArticulationType& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArticulationType& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArticulationMeta& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArticulationMeta& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArticulationAppliedData& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArticulationAppliedData& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ExpressionContext& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ExpressionContext& value);

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::NoteEvent& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::NoteEvent& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::RestEvent& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::RestEvent& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::TextArticulationEvent& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::TextArticulationEvent& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::SoundPresetChangeEvent& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::SoundPresetChangeEvent& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::SyllableEvent& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::SyllableEvent& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ControllerChangeEvent& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ControllerChangeEvent& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::PlaybackEvent& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::PlaybackEvent& value);

template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::SoundCategory& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::SoundCategory& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::PlaybackSetupData& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::PlaybackSetupData& value);
template<typename Data>
void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::PlaybackData& value);
template<typename Data>
void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::PlaybackData& value);

#include "global/serialization/msgpack.h"

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::PlaybackStatus& value)
{
    p.process(static_cast<int8_t>(value));
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::PlaybackStatus& value)
{
    int8_t val = 0;
    p.process(val);
    value = static_cast<muse::audio::PlaybackStatus>(val);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioResourceType& value)
{
    p.process(static_cast<int8_t>(value));
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioResourceType& value)
{
    int type = 0;
    p.process(type);
    value = static_cast<muse::audio::AudioResourceType>(type);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioFxCategory& value)
{
    p.process(static_cast<int>(value));
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioFxCategory& value)
{
    int cat = 0;
    p.process(cat);
    value = static_cast<muse::audio::AudioFxCategory>(cat);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioResourceMeta& value)
{
    p.process(value.id, value.type, value.vendor, value.attributes, value.hasNativeEditorSupport);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioResourceMeta& value)
{
    p.process(value.id, value.type, value.vendor, value.attributes, value.hasNativeEditorSupport);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioFxParams& value)
{
    p.process(value.categories, value.chainOrder, value.resourceMeta, value.configuration, value.active);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioFxParams& value)
{
    p.process(value.categories, value.chainOrder, value.resourceMeta, value.configuration, value.active);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AuxSendParams& value)
{
    p.process(value.signalAmount, value.active);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AuxSendParams& value)
{
    p.process(value.signalAmount, value.active);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioOutputParams& value)
{
    p.process(value.fxChain, value.volume, value.balance,  value.auxSends, value.solo, value.muted, value.forceMute);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioOutputParams& value)
{
    p.process(value.fxChain, value.volume, value.balance, value.auxSends, value.solo, value.muted, value.forceMute);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioParams& value)
{
    p.process(value.in, value.out);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioParams& value)
{
    p.process(value.in, value.out);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::SoundPreset& value)
{
    p.process(value.code, value.name, value.isDefault, value.attributes);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::SoundPreset& value)
{
    p.process(value.code, value.name, value.isDefault, value.attributes);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioSourceParams& value)
{
    p.process(value.resourceMeta, value.configuration);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioSourceParams& value)
{
    p.process(value.resourceMeta, value.configuration);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::SoundTrackType& value)
{
    p.process(static_cast<int>(value));
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::SoundTrackType& value)
{
    int type = 0;
    p.process(type);
    value = static_cast<muse::audio::SoundTrackType>(type);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::SoundTrackFormat& value)
{
    p.process(value.type, value.sampleRate, value.samplesPerChannel, value.audioChannelsNumber, value.bitRate);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::SoundTrackFormat& value)
{
    p.process(value.type, value.sampleRate, value.samplesPerChannel, value.audioChannelsNumber, value.bitRate);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::AudioSignalVal& value)
{
    p.process(value.amplitude, value.pressure);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::AudioSignalVal& value)
{
    p.process(value.amplitude, value.pressure);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::InputProcessingProgress::ChunkInfo& value)
{
    p.process(value.start, value.end);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::InputProcessingProgress::ChunkInfo& value)
{
    p.process(value.start, value.end);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::InputProcessingProgress::ProgressInfo& value)
{
    p.process(value.current, value.total);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::InputProcessingProgress::ProgressInfo& value)
{
    p.process(value.current, value.total);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::audio::InputProcessingProgress::StatusInfo& value)
{
    p.process(static_cast<uint8_t>(value.status), value.errcode);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::audio::InputProcessingProgress::StatusInfo& value)
{
    uint8_t status = 0;
    p.process(status, value.errcode);
    value.status = static_cast<muse::audio::InputProcessingProgress::Status>(status);
}

// MPE
template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArrangementContext& value)
{
    p.process(value.nominalTimestamp, value.actualTimestamp, value.nominalDuration, value.actualDuration,
              value.voiceLayerIndex, value.staffLayerIndex, value.bps);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArrangementContext& value)
{
    p.process(value.nominalTimestamp, value.actualTimestamp, value.nominalDuration, value.actualDuration,
              value.voiceLayerIndex, value.staffLayerIndex, value.bps);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::PitchContext& value)
{
    p.process(value.nominalPitchLevel, value.pitchCurve);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::PitchContext& value)
{
    p.process(value.nominalPitchLevel, value.pitchCurve);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArrangementPattern& value)
{
    p.process(value.durationFactor, value.timestampOffset);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArrangementPattern& value)
{
    p.process(value.durationFactor, value.timestampOffset);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::PitchPattern& value)
{
    p.process(value.pitchOffsetMap);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::PitchPattern& value)
{
    p.process(value.pitchOffsetMap);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ExpressionPattern& value)
{
    p.process(value.dynamicOffsetMap);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ExpressionPattern& value)
{
    p.process(value.dynamicOffsetMap);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArticulationPatternSegment& value)
{
    p.process(value.arrangementPattern, value.pitchPattern, value.expressionPattern);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArticulationPatternSegment& value)
{
    p.process(value.arrangementPattern, value.pitchPattern, value.expressionPattern);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArticulationType& value)
{
    p.process(static_cast<int8_t>(value));
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArticulationType& value)
{
    int8_t type = 0;
    p.process(type);
    value = static_cast<muse::mpe::ArticulationType>(type);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArticulationMeta& value)
{
    p.process(value.type, value.pattern, value.timestamp, value.overallDuration,
              value.overallPitchChangesRange, value.overallDynamicChangesRange);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArticulationMeta& value)
{
    p.process(value.type, value.pattern, value.timestamp, value.overallDuration,
              value.overallPitchChangesRange, value.overallDynamicChangesRange);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ArticulationAppliedData& value)
{
    p.process(value.meta, value.appliedPatternSegment, value.occupiedFrom, value.occupiedTo,
              value.occupiedPitchChangesRange, value.occupiedDynamicChangesRange);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ArticulationAppliedData& value)
{
    p.process(value.meta, value.appliedPatternSegment, value.occupiedFrom, value.occupiedTo,
              value.occupiedPitchChangesRange, value.occupiedDynamicChangesRange);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ExpressionContext& value)
{
    p.process(value.articulations, value.nominalDynamicLevel, value.expressionCurve, value.velocityOverride);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ExpressionContext& value)
{
    p.process(value.articulations, value.nominalDynamicLevel, value.expressionCurve, value.velocityOverride);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::NoteEvent& value)
{
    p.process(value.arrangementCtx(), value.pitchCtx(), value.expressionCtx());
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::NoteEvent& value)
{
    muse::mpe::ArrangementContext arrCtx;
    muse::mpe::PitchContext pitchCtx;
    muse::mpe::ExpressionContext exprCtx;
    p.process(arrCtx, pitchCtx, exprCtx);
    value = muse::mpe::NoteEvent(std::move(arrCtx), std::move(pitchCtx), std::move(exprCtx));
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::RestEvent& value)
{
    p.process(value.arrangementCtx());
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::RestEvent& value)
{
    muse::mpe::ArrangementContext arrCtx;
    p.process(arrCtx);
    value = muse::mpe::RestEvent(std::move(arrCtx));
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::TextArticulationEvent& value)
{
    p.process(value.text, value.layerIdx, value.flags);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::TextArticulationEvent& value)
{
    p.process(value.text, value.layerIdx, value.flags);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::SoundPresetChangeEvent& value)
{
    p.process(value.code, value.layerIdx);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::SoundPresetChangeEvent& value)
{
    p.process(value.code, value.layerIdx);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::SyllableEvent& value)
{
    p.process(value.text, value.layerIdx, value.flags);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::SyllableEvent& value)
{
    p.process(value.text, value.layerIdx, value.flags);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::ControllerChangeEvent& value)
{
    p.process(static_cast<int8_t>(value.type), value.val, value.layerIdx);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::ControllerChangeEvent& value)
{
    int8_t type = 0;
    p.process(type, value.val, value.layerIdx);
    value.type = static_cast<muse::mpe::ControllerChangeEvent::Type>(type);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::PlaybackEvent& value)
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

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::PlaybackEvent& value)
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

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::SoundCategory& value)
{
    p.process(static_cast<int8_t>(value));
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::SoundCategory& value)
{
    int8_t type = 0;
    p.process(type);
    value = static_cast<muse::mpe::SoundCategory>(type);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::PlaybackSetupData& value)
{
    p.process(value.id, value.category, value.subCategories,
              value.supportsSingleNoteDynamics, value.musicXmlSoundId, value.scoreId);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::PlaybackSetupData& value)
{
    p.process(value.id, value.category, value.subCategories,
              value.supportsSingleNoteDynamics, value.musicXmlSoundId, value.scoreId);
}

template<typename Data>
inline void pack_custom(muse::msgpack::DataPacker<Data>& p, const muse::mpe::PlaybackData& value)
{
    p.process(value.originEvents, value.setupData, value.dynamics);
}

template<typename Data>
inline void unpack_custom(muse::msgpack::DataUnPacker<Data>& p, muse::mpe::PlaybackData& value)
{
    p.process(value.originEvents, value.setupData, value.dynamics);
}

namespace muse::audio::rpc {
using Options = msgpack::Options;

template<typename T>
class RpcAllocator
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;

private:
    static inline std::vector<uint8_t> m_pool;
    static inline size_t m_currentPos = 0;
    static inline std::mutex m_mutex;

public:
    RpcAllocator() = default;

    pointer allocate(size_type n, const void* = static_cast<const void*>(0))
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        LOGDA() << "m_currentPos: " << m_currentPos;
        if (m_currentPos + n > m_pool.size()) {
            size_type newSize = std::max(m_pool.size() * 2, m_pool.size() + n);
            m_pool.resize(newSize);
        }

        pointer ptr = reinterpret_cast<pointer>(&m_pool[m_currentPos]);
        m_currentPos += n;
        LOGDA() << "m_currentPos += n: " << m_currentPos;
        return ptr;
    }

    void deallocate(pointer, size_type n)
    {
        LOGDA() << "m_currentPos: " << m_currentPos;
        m_currentPos -= n;
        LOGDA() << "m_currentPos -= n: " << m_currentPos;
    }
};

class RpcPacker
{
public:
    RpcPacker() = default;

    template<class ... Types>
    static ByteArray pack(const Options& opt, const Types&... args)
    {
        RpcAllocator<uint8_t> allocator;
        std::vector<uint8_t, RpcAllocator<uint8_t>> data(allocator);
        //std::vector<uint8_t> data;
        //data.reserve(opt.rezerveSize);
        msgpack::pack_to_data(data, args ...);
        // return ByteArray::fromRawData(&data[0], data.size());
        return ByteArray(&data[0], data.size());

        //return msgpack::pack(opt, args ...);
    }

    template<class ... Types>
    static ByteArray pack(const Types&... args)
    {
        return RpcPacker::pack(Options {}, args ...);
    }

    template<class ... Types>
    static bool unpack(const ByteArray& data, Types&... args)
    {
        return msgpack::unpack(data, args ...);
    }
};
}
