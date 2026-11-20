/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

// Stream cipher for legacy Encore .enc files. The large TABLE_B substitution table lives in
// zbot_table.cpp (with its provenance note); this file holds only the cipher logic.

#include "zbot.h"

#include <cstdint>
#include <mutex>

namespace mu::iex::enc {
static const int kTableA[17] = {
    14, 2, 8, 9, 1, 0, 3, 7, 6, 5, 4, 11, 12, 15, 10, 13, 16
};

// The large nibble-packed TABLE_B (9776x4 values); defined in zbot_table.cpp.
extern const uint8_t kTablePacked[19552];

// The 9 bytes whose true value exceeds 9 (truncated in kTablePacked; see zbot_table.cpp).
static const struct {
    uint16_t idx;
    uint8_t val;
} kPatches[9] = {
    { 19684, 0xB2 },
    { 19686, 0xC6 },
    { 19687, 0xA1 },
    { 27390, 0x57 },
    { 32954, 0xE5 },
    { 32955, 0x21 },
    { 39100, 0x99 },
    { 39102, 0xCD },
    { 39103, 0xCC },
};

// Expands kTablePacked + kPatches into a flat 39104-byte table on first call.
static const uint8_t* tableFlat()
{
    static uint8_t tbl[39104];
    static std::once_flag flag;
    std::call_once(flag, []() {
        for (int i = 0; i < 19552; ++i) {
            tbl[i * 2]     = kTablePacked[i] >> 4;
            tbl[i * 2 + 1] = kTablePacked[i] & 0x0F;
        }
        for (const auto& p : kPatches) {
            tbl[p.idx] = p.val;
        }
    });
    return tbl;
}

bool isZbotMagic(const QByteArray& magic4)
{
    return magic4 == "ZBOT" || magic4 == "ZBOP" || magic4 == "ZBO6";
}

void zbotDecrypt(QByteArray& buf)
{
    const uint8_t* tbl = tableFlat();
    int pos = 0xAB, sub = 0, ctr = 0;
    for (int i = 0; i < buf.size(); ++i) {
        if (ctr == 17) {
            ctr = 0;
        }
        const int delta = kTableA[ctr++];
        const int idx   = (pos + delta) % 9776;
        buf[i] ^= static_cast<char>(tbl[idx * 4 + sub]);
        if (++sub == 4) {
            sub = 0;
            pos = (pos + 1) % 9776;
        }
    }
}
} // namespace mu::iex::enc
