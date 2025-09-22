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
#pragma once

#include <vector>
#include <functional>

namespace muse::audio::worker {
//! NOTE This is an RPC channel for
//! communication between the driver and the worker
//! and transfer audio data
class WebAudioChannel
{
public:
    WebAudioChannel() = default;

    using Callback = std::function<void (float* stream, size_t samples)>;

    void open(Callback callback);
    void close();

private:

    Callback m_callback = nullptr;

    std::vector<float> m_buffer;
};
}
