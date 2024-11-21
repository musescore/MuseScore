/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "musicxmlvoicedesc.h"

using namespace mu::engraving;
using namespace mu::iex::musicxml;
//---------------------------------------------------------
//   VoiceDesc
//---------------------------------------------------------

VoiceDesc::VoiceDesc()
    : m_staff(-1), m_voice(-1), m_overlaps(false)
{
    for (int i = 0; i < MAX_STAVES; ++i) {
        m_chordRests[i] =  0;
        m_staffAlloc[i] = -1;
        m_voices[i]     = -1;
    }
}

void VoiceDesc::incrChordRests(int s)
{
    if (0 <= s && s < MAX_STAVES) {
        m_chordRests[s]++;
    }
}

int VoiceDesc::numberChordRests() const
{
    int res = 0;
    for (int i = 0; i < MAX_STAVES; ++i) {
        res += m_chordRests[i];
    }
    return res;
}

int VoiceDesc::preferredStaff() const
{
    int max = 0;
    int res = 0;
    for (int i = 0; i < MAX_STAVES; ++i) {
        if (m_chordRests[i] > max) {
            max = m_chordRests[i];
            res = i;
        }
    }
    return res;
}

String VoiceDesc::toString() const
{
    String res = u"[";
    for (int i = 0; i < MAX_STAVES; ++i) {
        res += String(u" %1").arg(m_chordRests[i]);
    }
    res += String(u" ] overlaps %1").arg(m_overlaps);
    if (m_overlaps) {
        res += u" staffAlloc [";
        for (int i = 0; i < MAX_STAVES; ++i) {
            res += String(u" %1").arg(m_staffAlloc[i]);
        }
        res += u" ] voices [";
        for (int i = 0; i < MAX_STAVES; ++i) {
            res += String(u" %1").arg(m_voices[i]);
        }
        res += u" ]";
    } else {
        res += String(u" staff %1 voice %2").arg(m_staff + 1).arg(m_voice + 1);
    }
    return res;
}
