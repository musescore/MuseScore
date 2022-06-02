/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <QString>
#include "types/bytearray.h"

namespace mu::engraving {
class XmlWriter;
class XmlReader;

//---------------------------------------------------------
//   Audio
//---------------------------------------------------------

class Audio
{
    QString _path;
    mu::ByteArray _data;

public:
    Audio();
    const QString& path() const { return _path; }
    void setPath(const QString& s) { _path = s; }
    const mu::ByteArray& data() const { return _data; }
    mu::ByteArray data() { return _data; }
    void setData(const mu::ByteArray& ba) { _data = ba; }

    void read(XmlReader&);
    void write(XmlWriter&) const;
};
}     // namespace Ms
#endif
