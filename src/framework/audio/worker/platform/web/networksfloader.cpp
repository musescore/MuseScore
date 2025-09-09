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

#include <fstream>

#include <emscripten/fetch.h>

#include "global/io/file.h"

using namespace muse;
using namespace muse::audio::worker;

static const char* SF_URL = "https://s3.us-east-1.amazonaws.com/new.musescore.org/MS_Basic.sf3";
static const io::path_t SF_PATH = "MS Basic.sf3";

async::Promise<RetVal<io::path_t> > NetworkSFLoader::load()
{
    return async::make_promise<RetVal<io::path_t> >([](auto resolve) {
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE;

        struct Holder
        {
            async::Promise<RetVal<io::path_t> >::Resolve resolve;

            void onSuccess(const char* data, uint64_t numBytes) {
                // ByteArray ba(data, numBytes);
                // io::File::writeFile(SF_PATH, ba);

                LOGDA() << "data size: " << numBytes;

                {
                    FILE* file = fopen(SF_PATH.c_str(), "wb");
                    IF_ASSERT_FAILED(file) {
                        (void)resolve(RetVal<io::path_t>::make_ret(10, "failed load sound font"));
                        return;
                    }
                    size_t wsize = fwrite(data, sizeof(char), numBytes, file);
                    IF_ASSERT_FAILED(wsize == numBytes) {
                        (void)resolve(RetVal<io::path_t>::make_ret(10, "failed load sound font"));
                        return;
                    }
                    fclose(file);
                }
                (void)resolve(RetVal<io::path_t>::make_ok(SF_PATH));
            }

            void onFailed(unsigned short status) {
                LOGE() << "failed load sound font, status: " << status;
                (void)resolve(RetVal<io::path_t>::make_ret(status, "failed load sound font"));
            }
        };

        Holder* h = new Holder();
        h->resolve = resolve;

        attr.userData = h;
        attr.onsuccess = [](emscripten_fetch_t* fetch) {
            LOGDA() << "success download sf: " << SF_URL;
            Holder* h = static_cast<Holder*>(fetch->userData);
            h->onSuccess(fetch->data, fetch->numBytes);
            emscripten_fetch_close(fetch);  // Free data associated with the fetch.
            delete h;
        };

        attr.onerror = [](emscripten_fetch_t* fetch) {
            LOGDA() << "failed download sf: " << SF_URL;
            Holder* h = static_cast<Holder*>(fetch->userData);
            h->onFailed(fetch->status);
            emscripten_fetch_close(fetch);  // Also free data on failure.
            delete h;
        };

        emscripten_fetch(&attr, SF_URL);
        LOGDA() << "start download sf: " << SF_URL;

        return async::Promise<RetVal<io::path_t> >::dummy_result();
    }, async::PromiseType::AsyncByBody);
}
