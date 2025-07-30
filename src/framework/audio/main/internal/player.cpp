/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "player.h"

#include "audio/common/rpc/rpcpacker.h"
#include "audio/common/audiosanitizer.h"

#include "log.h"

using namespace muse;
using namespace muse::async;
using namespace muse::audio;
using namespace muse::audio::rpc;

Player::Player(const TrackSequenceId sequenceId)
    : m_sequenceId(sequenceId)
{
}

void Player::init()
{
    ONLY_AUDIO_MAIN_THREAD;

    //! NOTE Subscribe and request initial state

    {
        m_playbackStatusChanged.onReceive(this, [this](PlaybackStatus st) {
            m_playbackStatus = st;
        });

        Msg msg = rpc::make_request(Method::GetPlaybackStatus, RpcPacker::pack(m_sequenceId));
        channel()->send(msg, [this](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            PlaybackStatus status = PlaybackStatus::Stopped;
            StreamId streamId = 0;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, status, streamId)) {
                return;
            }

            channel()->addReceiveStream(streamId, m_playbackStatusChanged);
            //! NOTE Send initial state
            m_playbackStatusChanged.send(status);
        });
    }

    {
        m_playbackPositionChanged.onReceive(this, [this](const secs_t newPos) {
            m_playbackPosition = newPos;
        });

        Msg msg = rpc::make_request(Method::GetPlaybackPosition, RpcPacker::pack(m_sequenceId));
        channel()->send(msg, [this](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            secs_t pos = 0.0;
            StreamId streamId = 0;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, pos, streamId)) {
                return;
            }

            channel()->addReceiveStream(streamId, m_playbackPositionChanged);
            //! NOTE Send initial state
            m_playbackPositionChanged.send(pos);
        });
    }
}

TrackSequenceId Player::sequenceId() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_sequenceId;
}

void Player::play(const secs_t delay)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::Play, RpcPacker::pack(m_sequenceId, delay));
    channel()->send(msg);
}

void Player::seek(const secs_t newPosition)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::Seek, RpcPacker::pack(m_sequenceId, newPosition));
    channel()->send(msg);
}

void Player::stop()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::Stop, RpcPacker::pack(m_sequenceId));
    channel()->send(msg);
}

void Player::pause()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::Pause, RpcPacker::pack(m_sequenceId));
    channel()->send(msg);
}

void Player::resume(const secs_t delay)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::Resume, RpcPacker::pack(m_sequenceId, delay));
    channel()->send(msg);
}

void Player::setDuration(const msecs_t durationMsec)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::SetDuration, RpcPacker::pack(m_sequenceId, durationMsec));
    channel()->send(msg);
}

async::Promise<bool> Player::setLoop(const msecs_t fromMsec, const msecs_t toMsec)
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<bool>([this, fromMsec, toMsec](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::SetLoop, RpcPacker::pack(m_sequenceId, fromMsec, toMsec));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            Ret ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }

            if (ret) {
                (void)resolve(true);
            } else {
                (void)reject(ret.code(), ret.text());
            }
        });
        return Promise<bool>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Player::resetLoop()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::ResetLoop, RpcPacker::pack(m_sequenceId));
    channel()->send(msg);
}

secs_t Player::playbackPosition() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_playbackPosition;
}

async::Channel<secs_t> Player::playbackPositionChanged() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_playbackPositionChanged;
}

PlaybackStatus Player::playbackStatus() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_playbackStatus;
}

async::Channel<PlaybackStatus> Player::playbackStatusChanged() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_playbackStatusChanged;
}
