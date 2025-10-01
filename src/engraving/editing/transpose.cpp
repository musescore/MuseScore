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

#include "transpose.h"

#include "../dom/harmony.h"
#include "../dom/part.h"
#include "../dom/segment.h"
#include "../dom/staff.h"
#include "../dom/utils.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   TransposeHarmony
//---------------------------------------------------------

TransposeHarmony::TransposeHarmony(Harmony* h, Interval interval, bool doubleSharpsFlats)
{
    m_harmony = h;
    m_interval = interval;
    m_useDoubleSharpsFlats = doubleSharpsFlats;
}

void TransposeHarmony::flip(EditData*)
{
    m_harmony->realizedHarmony().setDirty(true);   // harmony should be re-realized after transposition

    for (HarmonyInfo* info : m_harmony->chords()) {
        info->setRootTpc(transposeTpc(info->rootTpc(), m_interval, m_useDoubleSharpsFlats));
        info->setBassTpc(transposeTpc(info->bassTpc(), m_interval, m_useDoubleSharpsFlats));
    }

    m_harmony->setXmlText(m_harmony->harmonyName());
    m_harmony->triggerLayout();
    m_interval.flip();
}

//---------------------------------------------------------
//   TransposeHarmonyDiatonic
//---------------------------------------------------------

TransposeHarmonyDiatonic::TransposeHarmonyDiatonic(Harmony* h, int interval, bool useDoubleSharpsFlats, bool transposeKeys)
{
    m_harmony = h;
    m_interval = interval;
    m_useDoubleSharpsFlats = useDoubleSharpsFlats;
    m_transposeKeys = transposeKeys;
}

void TransposeHarmonyDiatonic::flip(EditData*)
{
    m_harmony->realizedHarmony().setDirty(true);   // harmony should be re-realized after transposition

    Fraction tick = Fraction(0, 1);
    Segment* seg = toSegment(m_harmony->findAncestor(ElementType::SEGMENT));
    if (seg) {
        tick = seg->tick();
    }
    Key key = !m_harmony->staff() ? Key::C : m_harmony->staff()->key(tick);

    for (HarmonyInfo* info : m_harmony->chords()) {
        info->setRootTpc(transposeTpcDiatonicByKey(info->rootTpc(), m_interval, key, m_transposeKeys, m_useDoubleSharpsFlats));
        info->setBassTpc(transposeTpcDiatonicByKey(info->bassTpc(), m_interval, key, m_transposeKeys, m_useDoubleSharpsFlats));
    }

    m_harmony->setXmlText(m_harmony->harmonyName());
    m_harmony->triggerLayout();

    m_interval *= -1;
}
