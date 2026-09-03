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

// Implements string decoding: probes the first bytes to pick UTF-16 LE or Latin-1, then reads.

#include "parsers-encoding.h"

namespace mu::iex::enc {
bool probeUtf16LE(quint8 b0, quint8 b1)
{
    return b0 >= 0x20 && b0 < 0x7F && b1 == 0x00;
}

QString readEncodedStringRemaining(QDataStream& ds, int& remaining)
{
    if (remaining <= 0) {
        return {};
    }

    // Peek at the first bytes to detect encoding; peek leaves the cursor in place and works
    // on non-seekable devices.
    quint8 b0 = 0, b1 = 0;
    {
        char buf[2] = { 0, 0 };
        const qint64 want = (remaining >= 2) ? 2 : 1;
        const qint64 n = ds.device()->peek(buf, want);
        if (n >= 1) {
            b0 = static_cast<quint8>(buf[0]);
        }
        if (n >= 2) {
            b1 = static_cast<quint8>(buf[1]);
        }
    }
    const bool utf16 = probeUtf16LE(b0, b1);

    QString result;
    while (remaining > 0) {
        if (utf16) {
            if (remaining < 2) {
                break;
            }
            quint8 lo, hi;
            ds >> lo >> hi;
            remaining -= 2;
            const QChar ch(char16_t((hi << 8) | lo));
            if (ch == u'\0') {
                break;
            }
            result.append(ch);
        } else {
            quint8 b;
            ds >> b;
            remaining -= 1;
            if (b == 0) {
                break;
            }
            result.append(QChar(char16_t(b)));
        }
    }
    return result;
}

QString readEncodedStringFixed(QDataStream& ds, int fixedLen)
{
    if (fixedLen <= 0) {
        return {};
    }
    const QByteArray buf = ds.device()->read(fixedLen);
    const int n = buf.size();
    if (n < 2) {
        if (n == 1 && buf[0] != '\0') {
            return QString(QChar(char16_t(static_cast<quint8>(buf[0]))));
        }
        return {};
    }
    const quint8 b0 = static_cast<quint8>(buf[0]);
    const quint8 b1 = static_cast<quint8>(buf[1]);
    const bool utf16 = probeUtf16LE(b0, b1);

    QString result;
    int i = 0;
    while (i < n) {
        if (utf16) {
            if (i + 1 >= n) {
                break;
            }
            const quint8 lo = static_cast<quint8>(buf[i]);
            const quint8 hi = static_cast<quint8>(buf[i + 1]);
            const QChar ch(char16_t((hi << 8) | lo));
            i += 2;
            if (ch == u'\0') {
                break;
            }
            result.append(ch);
        } else {
            const quint8 b = static_cast<quint8>(buf[i]);
            ++i;
            if (b == 0) {
                break;
            }
            result.append(QChar(char16_t(b)));
        }
    }
    return result;
}
} // namespace mu::iex::enc
