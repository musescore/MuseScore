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

#include <cstdint>
#include <vector>
#include <optional>

namespace muse::midi {
struct Event;
}

namespace muse::midiremote {
enum class MMCCommand : uint8_t
{
    Stop               = 0x01,
    Play               = 0x02,
    DeferredPlay       = 0x03,
    FastForward        = 0x04,
    Rewind             = 0x05,
    RecordStrobe       = 0x06,
    RecordExit         = 0x07,
    RecordPause        = 0x08,
    Pause              = 0x09,
    Eject              = 0x0A,
    Chase              = 0x0B,
    CommandErrorReset  = 0x0D,
    MMCReset           = 0x0F,
    Locate             = 0x44,
    Shuttle            = 0x47,
    Unknown            = 0xFF
};

struct MMCMessage
{
    uint8_t deviceId = 0;
    MMCCommand command = MMCCommand::Unknown;
    std::vector<uint8_t> data;
};

class MMCParser
{
public:
    MMCParser() = default;
    ~MMCParser();

    MMCParser(const MMCParser&) = delete;
    MMCParser& operator=(const MMCParser&) = delete;

    std::optional<MMCMessage> process(const midi::Event& event);

    static std::optional<double> locateToSeconds(const MMCMessage& msg);

private:
    class SysExAssembler;
    SysExAssembler* m_assembler = nullptr;
};
}
