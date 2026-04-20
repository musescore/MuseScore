/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <cstddef>
#include <cstdint>
#include <optional>
#include <memory>

#include "mmctypes.h"

namespace muse::midi {
struct Event;
}

namespace muse::midiremote {
class IMMCDecoder
{
public:
    virtual ~IMMCDecoder() = default;

    virtual std::optional<MMCMessage> decode(const midi::Event& event) = 0;
    virtual std::optional<MMCMessage> decode(const uint8_t* data, size_t size) = 0;

    virtual std::optional<double> locateToSeconds(const MMCMessage& msg) const = 0;
};

using IMMCDecoderPtr = std::shared_ptr<IMMCDecoder>;
}
