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

#include "../dom/segment.h"
#include "../dom/score.h"
#include "../dom/system.h"

#include "editpagebreaks.h"

using namespace mu::engraving;

void EditPageBreaks::addRemovePageBreaks(Score* score, int interval, bool afterEachPage)
{
    Segment* startSegment = score->selection().startSegment();
    if (!startSegment) { // empty score?
        return;
    }
    Segment* endSegment = score->selection().endSegment();
    Measure* startMeasure = startSegment->measure();
    Measure* endMeasure = endSegment ? endSegment->measure() : score->lastMeasureMM();
    Measure* lastMeasure = score->lastMeasureMM();

    // loop through measures in selection
    // First system
    int sCount = 1;
    for (Measure* mm = startMeasure; mm; mm = mm->nextMeasureMM()) {
        // even though we are counting mmrests as a single measure,
        // we need to find last real measure within mmrest for the actual break
        Measure* m = mm->isMMRest() ? mm->mmRestLast() : mm;

        if (afterEachPage) {
            // skip last measure of score
            if (mm == lastMeasure) {
                break;
            }
            // skip if it already has a page break
            if (m->pageBreak()) {
                continue;
            }
            // add break if last measure of the last system of the page
            if (mm->system() && mm->system()->lastMeasure() == mm && mm->nextMeasureMM() && mm->nextMeasureMM()->system()
                && mm->system()->page() != mm->nextMeasureMM()->system()->page()) {
                m->undoSetPageBreak(true);
            }
        } else {
            if (interval == 0) {
                // remove page break if present
                if (m->pageBreak()) {
                    m->undoSetPageBreak(false);
                }
            } else {
                if (sCount == interval) {
                    // skip last measure of score and measures that aren't the last of the System
                    if (mm == lastMeasure) {
                        break;
                    }

                    // found place for break
                    if (mm->system()->lastMeasure() == mm) {
                        // add if not already one present
                        if (!m->pageBreak()) {
                            m->undoSetPageBreak(true);
                        }
                        // reset count
                        sCount = 1;
                    }
                } else if (m->pageBreak()) {
                    // remove page break if present in wrong place
                    m->undoSetPageBreak(false);
                }
                // count if the last Measure of the system
                if (!m->pageBreak() && mm->system() && mm->system()->lastMeasure() == mm) {
                    ++sCount;
                }
            }
        }

        if (mm == endMeasure) {
            break;
        }
    }
}
