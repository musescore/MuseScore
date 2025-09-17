/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "webbridge/webapi.h"

#include <emscripten.h>

using namespace mu::webbridge;

extern "C" {
EMSCRIPTEN_KEEPALIVE
void load(const void* source, unsigned int len) { WebApi::instance()->load(source, len); }

EMSCRIPTEN_KEEPALIVE
void addSoundFont(const char* uri)
{
    WebApi::instance()->addSoundFont(std::string(uri));
}

EMSCRIPTEN_KEEPALIVE
void startAudioProcessing() { WebApi::instance()->startAudioProcessing(); }
}
