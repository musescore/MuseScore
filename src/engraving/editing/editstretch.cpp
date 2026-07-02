/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "editstretch.h"

#include "../dom/measure.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"

using namespace mu::engraving;

void EditStretch::addStretch(Transaction&, Score* score, double val)
{
    if (!score->selection().isRange()) {
        return;
    }

    Fraction startTick = score->selection().tickStart();
    Fraction endTick   = score->selection().tickEnd();
    for (Measure* m = score->firstMeasureMM(); m; m = m->nextMeasureMM()) {
        if (m->tick() < startTick) {
            continue;
        }
        if (m->tick() >= endTick) {
            break;
        }
        double stretch = m->userStretch();
        stretch += val;
        if (stretch < 0) {
            stretch = 0;
        }
        m->undoChangeProperty(Pid::USER_STRETCH, stretch);
    }
}

void EditStretch::resetUserStretch(Transaction&, Score* score)
{
    Measure* m1 = nullptr;
    Measure* m2 = nullptr;
    // retrieve span of selection
    Segment* s1 = score->selection().startSegment();
    Segment* s2 = score->selection().endSegment();
    // if either segment is not returned by the selection
    // (for instance, no selection) fall back to first/last measure
    if (!s1) {
        m1 = score->firstMeasureMM();
    } else {
        m1 = s1->measure();
    }
    if (!s2) {
        m2 = score->lastMeasureMM();
    } else {
        m2 = s2->measure();
    }
    if (!m1 || !m2) {               // should not happen!
        return;
    }

    for (Measure* m = m1; m; m = m->nextMeasureMM()) {
        m->undoChangeProperty(Pid::USER_STRETCH, 1.0);
        if (m == m2) {
            break;
        }
    }
}
