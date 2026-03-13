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

#include "mmc.h"

#include "midievent.h"
#include "log.h"

using namespace muse::midi;

inline std::optional<MMCMessage> fastParseMMC(const Event& event)
{
    const uint32_t* words = event.midi20Words();
    const uint32_t w0 = words[0];

    // Extract UMP SysEx header fields
    const uint8_t status = (w0 >> 20) & 0xF; // 0 = complete packet
    const uint8_t count  = (w0 >> 16) & 0xF; // number of valid bytes (0-6)

    // Fast path only handles complete, short messages
    // Long messages, such as Locate, are ignored here
    if (status != 0 || count < 4) {
        return std::nullopt;
    }

    // SysEx packs first 2 bytes in word 0, next bytes in word 1
    const uint8_t b0 = (w0 >> 8) & 0xFF;
    const uint8_t b1 = (w0 >> 0) & 0xFF;
    const uint32_t w1 = words[1];
    const uint8_t b2 = (w1 >> 24) & 0xFF;
    const uint8_t b3 = (w1 >> 16) & 0xFF;

    // MMC structure: 7F <device> 06 <command>
    if (!(b0 == 0x7F && b2 == 0x06)) {
        return std::nullopt;
    }

    MMCMessage msg;
    msg.deviceId = b1;
    msg.command = static_cast<MMCCommand>(b3);

    return msg;
}

static bool extractSysExBytes(const Event& event, std::vector<uint8_t>& out)
{
    const uint32_t* words = event.midi20Words();
    const size_t wordCount = event.midi20WordCount();
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
        const uint32_t* words = event.midi20Words();
        const uint8_t status = (words[0] >> 20) & 0xF;

        std::vector<uint8_t> bytes;
        extractSysExBytes(event, bytes);

        switch (status) {
        case 0: // COMPLETE
            result = bytes;
            return true;
        case 1: // START
            m_buffer = bytes;
            break;
        case 2: // CONTINUE
            m_buffer.insert(m_buffer.end(), bytes.begin(), bytes.end());
            break;
        case 3: // END
            m_buffer.insert(m_buffer.end(), bytes.begin(), bytes.end());
            result = m_buffer;
            m_buffer.clear();
            return true;
        }

        return false;
    }

private:
    std::vector<uint8_t> m_buffer;
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

    if (std::optional<MMCMessage> msg = fastParseMMC(event)) {
        return msg;
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
