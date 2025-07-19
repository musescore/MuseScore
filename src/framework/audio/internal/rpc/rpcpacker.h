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

void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioOutputParams& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioOutputParams& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundPreset& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundPreset& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::AudioSourceParams& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::AudioSourceParams& value);

void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundTrackType& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundTrackType& value);
void pack_custom(muse::msgpack::Packer& p, const muse::audio::SoundTrackFormat& value);
void unpack_custom(muse::msgpack::UnPacker& p, muse::audio::SoundTrackFormat& value);

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
