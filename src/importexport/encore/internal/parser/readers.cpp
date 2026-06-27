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

// Shared block-skip/clamp helper implementations and the EncFormatReader factory (version to reader).

#include "readers.h"
#include "readers-v0xc2.h"
#include "readers-v0xc4.h"

#include <algorithm>
#include <limits>

#include <QDataStream>
#include <QIODevice>

#include "log.h"

namespace mu::iex::enc {
bool skipBlock(QDataStream& ds, qint64 size)
{
    QIODevice* dev = ds.device();
    const qint64 remaining = dev->size() - dev->pos();
    if (size < 0 || size > remaining) {
        return false;
    }
    qint64 left = size;
    while (left > 0) {
        const int chunk = static_cast<int>(std::min<qint64>(left, std::numeric_limits<int>::max()));
        const int n = ds.skipRawData(chunk);
        if (n <= 0) {
            return false;
        }
        left -= n;
    }
    return true;
}

bool skipToBlockEnd(QDataStream& ds, qint64 blockStartPos, qint64 declaredLen)
{
    const qint64 toSkip = (blockStartPos + declaredLen) - ds.device()->pos();
    if (toSkip <= 0) {
        return false;
    }
    return skipBlock(ds, toSkip);
}

qint64 clampMeasureEnd(qint64 measStart, quint32 varsize, qint64 elemBlockOffset, qint64 deviceSize)
{
    const qint64 end = measStart + static_cast<qint64>(varsize) + elemBlockOffset;
    return std::min(end, deviceSize);
}

// Selects a format reader. SCO5 (macOS Encore 5) is matched by magic string because its chuMagio
// is not 0xC4 even though it shares the v0xC4 format; otherwise chuMagio picks the reader.
std::unique_ptr<EncFormatReader> EncFormatReader::create(quint8 chuMagio, const QString& magic)
{
    if (magic == "SCO5") {
        return makeFormatReader_SCO5();
    }
    switch (chuMagio) {
    case static_cast<quint8>(EncFormatVersion::V3_4_X):
        return makeFormatReader_V0xC2();
    case static_cast<quint8>(EncFormatVersion::V5_X):
        return makeFormatReader_V0xC4();
    default:
        LOGW() << QString("Encore: unsupported format version 0x%1 - import may fail")
            .arg(chuMagio, 2, 16, QChar('0'));
        return makeFormatReader_V0xC4();
    }
}
} // namespace mu::iex::enc
