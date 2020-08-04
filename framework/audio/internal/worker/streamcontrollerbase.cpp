//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "streamcontrollerbase.h"

#include <soloud.h>
#include "log.h"

#include "iaudiosource.h"
#include "internal/loopsource.h"

using namespace mu::audio;
using namespace mu::audio::worker;

StreamControllerBase::StreamControllerBase()
{
}

StreamControllerBase::~StreamControllerBase()
{
}

void StreamControllerBase::createStream(const StreamID& id, const std::string& name)
{
    std::shared_ptr<Stream> s = std::make_shared<Stream>();
    s->id = id;
    s->name = name;
    s->source = makeSource(id, name);
    s->loopStream = std::make_shared<LoopSource>(s->source, name);

    m_streams.insert({ id, s });
}

void StreamControllerBase::destroyStream(const StreamID& id)
{
    m_streams.erase(id);
}

bool StreamControllerBase::hasStream(const StreamID& id) const
{
    if (m_streams.find(id) != m_streams.end()) {
        return true;
    }
    return false;
}

void StreamControllerBase::setSampleRate(const StreamID& id, float samplerate)
{
    std::shared_ptr<Stream> s = stream(id);
    IF_ASSERT_FAILED(s) {
        return;
    }

    IF_ASSERT_FAILED(s->source) {
        return;
    }

    s->source->setSampleRate(samplerate);
}

void StreamControllerBase::setLoopRegion(const StreamID& id, const LoopRegion& loop)
{
    std::shared_ptr<Stream> s = stream(id);
    IF_ASSERT_FAILED(s) {
        return;
    }

    s->loopStream->setLoopRegion(loop);
}

// Instance

void StreamControllerBase::streamInstance_create(const StreamID& id)
{
    std::shared_ptr<Stream> s = stream(id);
    IF_ASSERT_FAILED(s) {
        return;
    }

    if (s->instance) {
        return;
    }

    IF_ASSERT_FAILED(s->loopStream) {
        return;
    }

    SoLoud::AudioSource* sa = s->loopStream->source();
    IF_ASSERT_FAILED(sa) {
        return;
    }

    s->instance = sa->createInstance();
    IF_ASSERT_FAILED(s->instance) {
        return;
    }
}

void StreamControllerBase::streamInstance_destroy(const StreamID& id)
{
    std::shared_ptr<Stream> s = stream(id);
    if (!s) {
        LOGW() << "already destroied stream id: " << id;
        return;
    }

    s->inited = false;
    delete s->instance;
    s->instance = nullptr;
}

void StreamControllerBase::streamInstance_init(const StreamID& id, float samplerate, uint16_t chans,
                                               double streamTime, double streamPosition)
{
    std::shared_ptr<Stream> s = stream(id);
    IF_ASSERT_FAILED(s) {
        return;
    }

    IF_ASSERT_FAILED(s->instance) {
        return;
    }

//    LOGI() << "[" << id << "] samplerate: " << samplerate
//           << ", channels: " << channels
//           << ", streamTime: " << streamTime
//           << ", streamPosition: " << streamPosition;

    SoLoud::AudioSource* sa = s->loopStream->source();
    sa->mChannels = chans;
    sa->mBaseSamplerate = samplerate;

    s->instance->mBaseSamplerate = samplerate;
    s->instance->mSamplerate = samplerate;
    s->instance->mChannels = chans;
    s->instance->mStreamTime = streamTime;
    s->instance->mStreamPosition = streamPosition;

    s->instance->init(*sa, id);

    s->inited = true;
}

void StreamControllerBase::streamInstance_seek_frame(const StreamID& id, float sec)
{
    std::shared_ptr<Stream> s = stream(id);
    IF_ASSERT_FAILED(s) {
        return;
    }

    if (!s->instance) {
        return;
    }

    s->instance->seekFrame(sec);
}

void StreamControllerBase::getAudio(const StreamID& id, float* buf, uint32_t samples, uint32_t bufSize, Context* ctx)
{
    std::shared_ptr<Stream> s = stream(id);
    IF_ASSERT_FAILED(s) {
        return;
    }

    if (!s->instance) {
        for (size_t i = 0; i < bufSize; ++i) {
            buf[i] = 0.f;
        }
        return;
    }

    if (!s->inited) {
        return;
    }

    if (s->instance->hasEnded()) {
        for (uint32_t i = 0; i < bufSize; ++i) {
            buf[i] = 0.0f;
        }

        ctx->set<bool>(CtxKey::HasEnded, true);
        return;
    }

    double buffertime = (samples / static_cast<double>(s->instance->mSamplerate));
    s->instance->mStreamTime += buffertime;
    s->instance->mStreamPosition += buffertime;

    ctx->set<double>(CtxKey::Position, s->instance->mStreamPosition);

    s->instance->getAudio(buf, samples, bufSize);

    fillAudioContext(s, ctx);
}

void StreamControllerBase::fillAudioContext(const std::shared_ptr<Stream>&, Context*)
{
}

std::shared_ptr<StreamControllerBase::Stream> StreamControllerBase::stream(const StreamID& id) const
{
    auto it = m_streams.find(id);
    if (it != m_streams.end()) {
        return it->second;
    }
    return nullptr;
}
