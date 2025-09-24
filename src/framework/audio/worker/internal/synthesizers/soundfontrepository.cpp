/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "soundfontrepository.h"

#include "audio/common/audiosanitizer.h"
#include "audio/common/rpc/rpcpacker.h"

#include "fluidsynth/fluidsoundfontparser.h"

#ifdef Q_OS_WASM
#include "audio/worker/platform/web/networksfloader.h"
#endif

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::synth;
using namespace muse::audio::rpc;
using namespace muse::async;

void SoundFontRepository::init()
{
    ONLY_AUDIO_WORKER_THREAD;

    channel()->onMethod(Method::LoadSoundFonts, [this](const Msg& msg) {
        std::vector<SoundFontUri> uris;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, uris)) {
            return;
        }

        this->loadSoundFonts(uris);
    });

    channel()->onMethod(Method::AddSoundFont, [this](const Msg& msg) {
        SoundFontUri uri;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, uri)) {
            return;
        }

        this->addSoundFont(uri);
    });

    channel()->onMethod(Method::AddSoundFontData, [this](const Msg& msg) {
        SoundFontUri uri;
        ByteArray data;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, uri, data)) {
            return;
        }

        this->addSoundFontData(uri, data);
    });
}

bool SoundFontRepository::isSoundFontLoaded(const std::string& name) const
{
    ONLY_AUDIO_WORKER_THREAD;
    for (const auto& p : m_soundFonts) {
        if (p.second.name == name) {
            return true;
        }
    }
    return false;
}

const SoundFontsMap& SoundFontRepository::soundFonts() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_soundFonts;
}

Notification SoundFontRepository::soundFontsChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_soundFontsChanged;
}

void SoundFontRepository::addSoundFont(const SoundFontUri& uri)
{
    ONLY_AUDIO_WORKER_THREAD;
    doAddSoundFont(uri, nullptr, [this]() {
        m_soundFontsChanged.notify();
    });
}

static io::path_t fileNameFromUri(const SoundFontUri& uri)
{
    io::path_t path = uri.toLocalFile();
    return io::filename(path);
}

void SoundFontRepository::addSoundFontData(const SoundFontUri& uri, const ByteArray& data)
{
    ONLY_AUDIO_WORKER_THREAD;

    const io::path_t fileName = fileNameFromUri(uri);

    {
        FILE* file = fopen(fileName.c_str(), "wb");
        size_t wsize = fwrite(data.constChar(), sizeof(char), data.size(), file);
        IF_ASSERT_FAILED(wsize == data.size()) {
            return;
        }
        fclose(file);
    }

    RetVal<SoundFontMeta> meta = FluidSoundFontParser::parseSoundFont(fileName);

    if (meta.ret) {
        m_soundFonts.insert_or_assign(uri, std::move(meta.val));
        LOGI() << "added sound font, uri: " << uri;
    } else {
        LOGE() << "Failed parse SoundFont presets for " << fileName << ": " << meta.ret.toString();
    }

    m_soundFontsChanged.notify();
}

void SoundFontRepository::doAddSoundFont(const SoundFontUri& uri, const SoundFontsMap* cache, std::function<void()> onFinished)
{
    if (cache) {
        auto it = cache->find(uri);
        if (it != cache->cend()) {
            m_soundFonts.insert(*it);
            LOGI() << "added sound font from cache, uri: " << uri;
            if (onFinished) {
                onFinished();
            }
            return;
        }
    }

    auto parseAndAdd = [this, onFinished](const SoundFontUri& uri, const SoundFontPath& path) {
        RetVal<SoundFontMeta> meta = FluidSoundFontParser::parseSoundFont(path);

        if (meta.ret) {
            m_soundFonts.insert_or_assign(uri, std::move(meta.val));
            LOGI() << "added sound font, uri: " << uri;
        } else {
            LOGE() << "Failed parse SoundFont presets for " << path << ": " << meta.ret.toString();
        }

        if (onFinished) {
            onFinished();
        }
    };

    if (uri.scheme() == "file") {
        SoundFontPath path = uri.toLocalFile();
        parseAndAdd(uri, path);
    } else {
#ifdef Q_OS_WASM
        auto promise = NetworkSFLoader::load(uri);
        promise.onResolve(this, [uri, parseAndAdd](const RetVal<ByteArray>& data) {
            if (data.ret) {
                const io::path_t fileName = fileNameFromUri(uri);

                {
                    FILE* file = fopen(fileName.c_str(), "wb");
                    size_t wsize = fwrite(data.val.constChar(), sizeof(char), data.val.size(), file);
                    IF_ASSERT_FAILED(wsize == data.val.size()) {
                        return;
                    }
                    fclose(file);
                }

                parseAndAdd(uri, fileName);
            }
        });
#else
        NOT_SUPPORTED;
        if (onFinished) {
            onFinished();
        }
#endif
    }
}

void SoundFontRepository::loadSoundFonts(const std::vector<SoundFontUri>& uris)
{
    ONLY_AUDIO_WORKER_THREAD;

    SoundFontsMap* cache = new SoundFontsMap();
    m_soundFonts.swap(*cache);

    size_t total = uris.size();
    size_t count = 0;
    for (const SoundFontUri& uri : uris) {
        doAddSoundFont(uri, cache, [this, &count, total, cache]() {
            ++count;
            if (count == total) {
                delete cache;
                m_soundFontsChanged.notify();
            }
        });
    }
}
