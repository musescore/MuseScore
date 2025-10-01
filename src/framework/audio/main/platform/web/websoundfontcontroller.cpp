/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "websoundfontcontroller.h"

#include "audio/common/rpc/rpcpacker.h"

#include "audio/engine/platform/web/networksfloader.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::rpc;

void WebSoundFontController::loadSoundFonts()
{
    // noop
}

void WebSoundFontController::addSoundFont(const synth::SoundFontUri& uri)
{
    async::Promise<RetVal<ByteArray> > promise = synth::NetworkSFLoader::load(uri);
    promise.onResolve(this, [this, uri](const RetVal<ByteArray>& rv) {
        if (!rv.ret) {
            LOGE() << rv.ret.toString();
            return;
        }

        channel()->send(rpc::make_request(Method::AddSoundFontData, RpcPacker::pack(uri, rv.val)));
    });
}
