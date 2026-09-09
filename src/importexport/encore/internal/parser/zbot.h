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

#ifndef MU_IMPORTEXPORT_ENC_PARSER_ZBOT_H
#define MU_IMPORTEXPORT_ENC_PARSER_ZBOT_H

#include <QByteArray>

namespace mu::iex::enc {
// Returns true if magic4 carries a ZBOT-family file signature (ZBOT, ZBOP, ZBO6).
bool isZbotMagic(const QByteArray& magic4);

// Decrypt a legacy Encore ZBOT file buffer in place using the stream cipher.
// After this call the buffer contains a SCOW-format (v0xC2/v0xC4) file.
void zbotDecrypt(QByteArray& buf);
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_PARSER_ZBOT_H
