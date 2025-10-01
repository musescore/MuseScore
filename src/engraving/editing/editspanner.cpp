/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "editspanner.h"

#include "../dom/chord.h"
#include "../dom/engravingitem.h"
#include "../dom/note.h"
#include "../dom/spanner.h"
#include "../dom/tie.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   ChangeSpannerElements
//---------------------------------------------------------

void ChangeSpannerElements::flip(EditData*)
{
    EngravingItem* oldStartElement   = spanner->startElement();
    EngravingItem* oldEndElement     = spanner->endElement();
    bool isPartialSpanner = spanner->isPartialTie() || spanner->isLaissezVib();
    if (spanner->anchor() == Spanner::Anchor::NOTE) {
        // be sure new spanner elements are of the right type
        if (!isPartialSpanner && (!startElement || !startElement->isNote() || !endElement || !endElement->isNote())) {
            return;
        }
        Note* oldStartNote = toNote(oldStartElement);
        Note* oldEndNote = toNote(oldEndElement);
        Note* newStartNote = toNote(startElement);
        Note* newEndNote = toNote(endElement);
        // update spanner's start and end notes
        if ((newStartNote && newEndNote) || (isPartialSpanner && (newStartNote || newEndNote))) {
            spanner->setNoteSpan(newStartNote, newEndNote);
            if (spanner->isTie()) {
                Tie* tie = toTie(spanner);
                if (oldStartNote && newStartNote) {
                    oldStartNote->setTieFor(nullptr);
                    newStartNote->setTieFor(tie);
                }
                if (oldEndNote && newEndNote) {
                    oldEndNote->setTieBack(nullptr);
                    newEndNote->setTieBack(tie);
                }
            } else {
                oldStartNote->removeSpannerFor(spanner);
                oldEndNote->removeSpannerBack(spanner);
                newStartNote->addSpannerFor(spanner);
                newEndNote->addSpannerBack(spanner);
                if (spanner->isGlissando()) {
                    oldEndNote->chord()->updateEndsNoteAnchoredLine();
                }
            }
        }
    } else {
        spanner->setStartElement(startElement);
        spanner->setEndElement(endElement);
    }
    startElement = oldStartElement;
    endElement   = oldEndElement;
    spanner->triggerLayout();
}

//---------------------------------------------------------
//   ChangeStartEndSpanner
//---------------------------------------------------------

void ChangeStartEndSpanner::flip(EditData*)
{
    EngravingItem* s = spanner->startElement();
    EngravingItem* e = spanner->endElement();
    spanner->setStartElement(start);
    spanner->setEndElement(end);
    start = s;
    end   = e;
}
