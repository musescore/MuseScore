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
#ifndef MU_PALETTE_MIMEDATAUTILS_H
#define MU_PALETTE_MIMEDATAUTILS_H

#include "io/buffer.h"

#include "engraving/rw/xmlreader.h"
#include "engraving/rw/xmlwriter.h"
#include "engraving/rw/400/readcontext.h"
#include "engraving/rw/400/writecontext.h"

namespace mu::palette {
template<class T>
QByteArray toMimeData(T* t)
{
    io::Buffer buffer;
    buffer.open(io::IODevice::WriteOnly);
    engraving::XmlWriter xml(&buffer);
    xml.context()->setClipboardmode(true);
    t->write(xml);
    buffer.close();
    return buffer.data().toQByteArray();
}

template<class T>
std::shared_ptr<T> fromMimeData(const QByteArray& data, const AsciiStringView& tagName)
{
    engraving::XmlReader e(data);
    e.context()->setPasteMode(true);
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == tagName) {
            std::shared_ptr<T> t(new T);
            if (!t->read(e)) {
                return nullptr;
            }
            return t;
        } else {
            return nullptr;
        }
    }
    return nullptr;
}
}

#endif // MU_PALETTE_MIMEDATAUTILS_H
