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
#include "networksfloader.h"

#include <emscripten/fetch.h>

using namespace muse;
using namespace muse::audio::synth;

async::Promise<RetVal<ByteArray> > NetworkSFLoader::load(const Uri& uri)
{
    return async::make_promise<RetVal<ByteArray> >([uri](auto resolve) {
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE;

        struct Holder
        {
            Uri uri;
            async::Promise<RetVal<ByteArray> >::Resolve resolve;

            void onSuccess(const char* data, uint64_t numBytes) {
                LOGDA() << "data size: " << numBytes;

                ByteArray ba(data, numBytes);
                (void)resolve(RetVal<ByteArray>::make_ok(ba));
            }

            void onFailed(unsigned short status) {
                LOGE() << "failed load sound font, status: " << status;
                (void)resolve(RetVal<ByteArray>::make_ret(status, "failed load sound font"));
            }
        };

        Holder* h = new Holder();
        h->uri = uri;
        h->resolve = resolve;

        attr.userData = h;
        attr.onsuccess = [](emscripten_fetch_t* fetch) {
            Holder* h = static_cast<Holder*>(fetch->userData);
            LOGI() << "success download sf: " << h->uri;
            h->onSuccess(fetch->data, fetch->numBytes);
            emscripten_fetch_close(fetch);  // Free data associated with the fetch.
            delete h;
        };

        attr.onerror = [](emscripten_fetch_t* fetch) {
            Holder* h = static_cast<Holder*>(fetch->userData);
            LOGE() << "failed download sf: " << h->uri;
            h->onFailed(fetch->status);
            emscripten_fetch_close(fetch);  // Also free data on failure.
            delete h;
        };

        std::string str = h->uri.toString();
        emscripten_fetch(&attr, str.c_str());
        LOGDA() << "start download sf: " << h->uri;

        return async::Promise<RetVal<ByteArray> >::dummy_result();
    }, async::PromiseType::AsyncByBody);
}
