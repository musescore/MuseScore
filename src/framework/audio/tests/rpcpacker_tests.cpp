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
