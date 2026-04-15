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

#include "mmc.h"

#include "midi/midievent.h"
#include "log.h"

using namespace muse::midi;
using namespace muse::midiremote;

static bool extractSysExBytes(const Event& event, std::vector<uint8_t>& out)
{
    const size_t wordCount = event.midi20WordCount();
    if (wordCount == 0) {
        return false;
    }

    const uint32_t* words = event.midi20Words();
    const uint32_t w0 = words[0];
    const uint8_t count = (w0 >> 16) & 0xF; // number of valid bytes (0-6)

    if (count == 0) {
        return true;
    }

    out.reserve(out.size() + count);

    // Word 0: [header | byte0 | byte1]
    // Word 1: [byte2 | byte3 | byte4 | byte5]

    // Extract first 2 bytes from word 0
    if (count >= 1) {
        out.push_back((w0 >> 8) & 0xFF);
    }
    if (count >= 2) {
        out.push_back((w0 >> 0) & 0xFF);
    }

    // Extract remaining bytes from word 1 (if present)
    if (wordCount > 1) {
        const uint32_t w1 = words[1];

        // Bytes are stored MSB -> LSB
        // i = 0 -> highest byte, i = 3 -> lowest byte
        for (int i = 0; i < 4; ++i) {
            if (out.size() >= count) {
                break;
            }

            out.push_back((w1 >> (24 - i * 8)) & 0xFF);
        }
    }

    return true;
}

static bool isMMC(const std::vector<uint8_t>& sysex)
{
    return sysex.size() >= 4
           && sysex[0] == 0x7F // Device
           && sysex[2] == 0x06; // Command
}

static MMCMessage parseMMC(const std::vector<uint8_t>& sysex)
{
    MMCMessage msg;
    msg.deviceId = sysex[1];
    msg.command = static_cast<MMCCommand>(sysex[3]);

    if (sysex.size() > 4) {
        msg.data.assign(sysex.begin() + 4, sysex.end());
    }

    return msg;
}

class MMCParser::SysExAssembler
{
public:
    bool process(const Event& event, std::vector<uint8_t>& result)
    {
        std::vector<uint8_t> bytes;
        if (!extractSysExBytes(event, bytes)) {
            return false;
        }

        const uint32_t* words = event.midi20Words();
        const Status status = static_cast<Status>((words[0] >> 20) & 0xF);

        return processChunk(bytes.data(), bytes.size(), status, result);
    }

    bool process(const uint8_t* data, size_t size, std::vector<uint8_t>& result)
    {
        const bool hasStartByte = (data[0] == 0xF0);
        const bool hasEndByte = (data[size - 1] == 0xF7);

        Status status;

        if (hasStartByte && hasEndByte) {
            status = Status::Complete;
        } else if (hasStartByte) {
            status = Status::Start;
        } else if (hasEndByte) {
            status = Status::End;
        } else {
            status = m_inProgress ? Status::Continue : Status::Complete;
        }

        // Skip start & end bytes if present
        const uint8_t* payload = data;
        size_t payloadSize = size;

        if (hasStartByte) {
            payload += 1;
            payloadSize -= 1;
        }

        if (payloadSize > 0 && hasEndByte) {
            payloadSize -= 1;
        }

        return processChunk(payload, payloadSize, status, result);
    }

private:
    enum class Status : uint8_t {
        Complete = 0,
        Start = 1,
        Continue = 2,
        End = 3,
    };

    bool processChunk(const uint8_t* data, size_t size, Status status, std::vector<uint8_t>& result)
    {
        switch (status) {
        case Status::Complete: {
            m_buffer.clear();
            m_inProgress = false;
            result.assign(data, data + size);
            return true;
        }
        case Status::Start: {
            m_buffer.assign(data, data + size);
            m_inProgress = true;
        } break;
        case Status::Continue: {
            if (!m_inProgress) {
                return false;
            }
            m_buffer.insert(m_buffer.end(), data, data + size);
        } break;
        case Status::End: {
            if (!m_inProgress) {
                return false;
            }
            m_buffer.insert(m_buffer.end(), data, data + size);
            result = m_buffer;
            m_buffer.clear();
            m_inProgress = false;
            return true;
        }
        }

        return false;
    }

    std::vector<uint8_t> m_buffer;
    bool m_inProgress = false;
};

MMCParser::MMCParser()
    : m_assembler(new SysExAssembler())
{
}

MMCParser::~MMCParser()
{
    delete m_assembler;
}

std::optional<MMCMessage> MMCParser::process(const Event& event)
{
    if (event.messageType() != Event::MessageType::SystemExclusiveData) {
        return std::nullopt;
    }

    std::vector<uint8_t> sysex;
    if (!m_assembler->process(event, sysex)) {
        return std::nullopt;
    }

    if (!isMMC(sysex)) {
        return std::nullopt;
    }

    return parseMMC(sysex);
}

std::optional<MMCMessage> MMCParser::process(const uint8_t* data, size_t size)
{
    if (!data || size == 0) {
        return std::nullopt;
    }

    std::vector<uint8_t> sysex;
    if (!m_assembler->process(data, size, sysex)) {
        return std::nullopt;
    }

    if (!isMMC(sysex)) {
        return std::nullopt;
    }

    return parseMMC(sysex);
}

std::optional<double> MMCParser::locateToSeconds(const MMCMessage& msg)
{
    IF_ASSERT_FAILED(msg.command == MMCCommand::Locate) {
        return std::nullopt;
    }

    if (msg.data.size() < 6) {
        return std::nullopt;
    }

    const bool hasFormat = (msg.data.size() >= 7);
    const uint8_t format = hasFormat ? msg.data[1] : 0x01;

    // Only support SMPTE
    if (format != 0x01) {
        return std::nullopt;
    }

    const size_t offset = hasFormat ? 1 : 0;
    const uint8_t hr_byte = msg.data[1 + offset];
    const uint8_t min = msg.data[2 + offset];
    const uint8_t sec = msg.data[3 + offset];
    const uint8_t frame = msg.data[4 + offset];
    const uint8_t subf = msg.data[5 + offset];
    const uint8_t fpsCode = (hr_byte >> 5) & 0x03;
    const uint8_t hr = hr_byte & 0x1F;

    double fps = 25.0;
    switch (fpsCode) {
    case 0:
        fps = 24.0;
        break;
    case 1:
        fps = 25.0;
        break;
    case 2:
        fps = 29.97;
        break;
    case 3:
        fps = 30.0;
        break;
    }

    const double total
        = hr * 3600.0
          + min * 60.0
          + sec
          + (frame / fps)
          + (subf / (fps * 100.0));

    return total;
}
