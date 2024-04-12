/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_AUDIO_H
#define MU_ENGRAVING_AUDIO_H

#include "global/allocator.h"
#include "types/bytearray.h"
#include "types/string.h"

namespace mu::engraving {
//---------------------------------------------------------
//   Audio
//---------------------------------------------------------

class Audio
{
    OBJECT_ALLOCATOR(engraving, Audio)

public:
    Audio();
    const muse::String& path() const { return m_path; }
    void setPath(const muse::String& s) { m_path = s; }
    const muse::ByteArray& data() const { return m_data; }
    muse::ByteArray data() { return m_data; }
    void setData(const muse::ByteArray& ba) { m_data = ba; }

private:
    muse::String m_path;
    muse::ByteArray m_data;
};
} // namespace mu::engraving
#endif // MU_ENGRAVING_AUDIO_H
