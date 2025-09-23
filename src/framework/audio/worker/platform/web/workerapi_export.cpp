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

#include "webworkerapi.h"

#include "log.h"

using namespace muse::web::worker;

int main()
{
    // noop
    return 0;
}

extern "C" {
void Init(unsigned int val)
{
    LOGI() << val;
    WebWorkerApi::instance()->init();
}

void process(uintptr_t ptr, unsigned samplesPerChannel)
{
    float* stream = reinterpret_cast<float*>(ptr);
    std::memset(stream, 0.0f, samplesPerChannel * 2);
    WebWorkerApi::instance()->process(stream, samplesPerChannel);
}
}
