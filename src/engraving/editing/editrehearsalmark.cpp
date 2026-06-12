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

#include "editrehearsalmark.h"

#include "../dom/measure.h"
#include "../dom/rehearsalmark.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"

using namespace mu;
using namespace mu::engraving;

//---------------------------------------------------------
//   resequenceRehearsalMarks
///   resequences rehearsal marks within a range selection
///   or, if nothing is selected, the entire score
//---------------------------------------------------------

void EditRehearsalMark::resequenceRehearsalMarks(Transaction&, Score* score)
{
    bool noSelection = !score->selection().isRange();

    if (noSelection) {
        score->cmdSelectAll();
    } else if (!score->selection().isRange()) {
        return;
    }

    RehearsalMark* last = nullptr;
    for (Segment* s = score->selection().startSegment(); s && s != score->selection().endSegment(); s = s->next1()) {
        for (EngravingItem* e : s->annotations()) {
            if (!e->isRehearsalMark()) {
                continue;
            }
            RehearsalMark* rm = toRehearsalMark(e);
            if (!rm->isTopSystemObject()) {
                continue;
            }
            if (last) {
                String rmText = nextRehearsalMarkText(last, rm);
                for (EngravingObject* le : rm->linkList()) {
                    le->undoChangeProperty(Pid::TEXT, rmText);
                }
            }
            last = rm;
        }
    }

    if (noSelection) {
        score->deselectAll();
    }
}

//---------------------------------------------------------
//   createRehearsalMarkText
//---------------------------------------------------------

String EditRehearsalMark::createRehearsalMarkText(const Score* score, RehearsalMark* current)
{
    Fraction tick = current->segment()->tick();
    RehearsalMark* before = 0;
    RehearsalMark* after = 0;
    for (Segment* s = score->firstSegment(SegmentType::All); s; s = s->next1()) {
        for (EngravingItem* e : s->annotations()) {
            if (e && e->isRehearsalMark()) {
                if (s->tick() < tick) {
                    before = toRehearsalMark(e);
                } else if (s->tick() > tick) {
                    after = toRehearsalMark(e);
                    break;
                }
            }
        }
        if (after) {
            break;
        }
    }
    String s = u"A";
    String s1 = before ? before->xmlText() : u"";
    String s2 = after ? after->xmlText() : u"";
    if (s1.isEmpty()) {
        return s;
    }
    s = nextRehearsalMarkText(before, current);       // try to sequence
    if (s == current->xmlText()) {
        // no sequence detected (or current happens to be correct)
        return s;
    } else if (s == s2) {
        // next in sequence already present
        if (s1.at(0).isLetter()) {
            if (s1.size() == 2) {
                s = String(s1.at(0)) + Char::fromAscii(s1.at(1).toAscii() + 1);          // BB, BC, CC
            } else {
                s = s1 + u'1';                              // B, B1, C
            }
        } else {
            s = s1 + u'A';                                  // 2, 2A, 3
        }
    }
    return s;
}

//---------------------------------------------------------
//   nextRehearsalMarkText
//    finds next rehearsal in sequence established by previous
//     Alphabetic sequences:
//      A, B, ..., Y, Z, AA, BB, ..., YY, ZZ
//      a, b, ..., y, z, aa, bb, ..., yy, zz
//     Numeric sequences:
//      1, 2, 3, ...
//      If number of previous rehearsal mark matches measure number, assume use of measure numbers throughout
//---------------------------------------------------------

String EditRehearsalMark::nextRehearsalMarkText(const RehearsalMark* previous, const RehearsalMark* current)
{
    String previousText = previous->xmlText();
    String fallback = current ? current->xmlText() : previousText + u"'";

    if (previousText.size() == 1 && previousText.at(0).isLetter()) {
        // single letter sequence
        if (previousText == "Z") {
            return u"AA";
        } else if (previousText == "z") {
            return u"aa";
        } else {
            return String(Char::fromAscii(previousText.at(0).toAscii() + 1));
        }
    } else if (previousText.size() == 2 && previousText.at(0).isLetter() && previousText.at(1).isLetter()) {
        // double letter sequence
        if (previousText.at(0) == previousText.at(1)) {
            // repeated letter sequence
            if (previousText.toUpper() != "ZZ") {
                String c = Char::fromAscii(previousText.at(0).toAscii() + 1);
                return c + c;
            } else {
                return fallback;
            }
        } else {
            return fallback;
        }
    } else {
        // try to interpret as number
        bool ok;
        int n = previousText.toInt(&ok);
        if (!ok) {
            return fallback;
        } else if (current && n == previous->segment()->measure()->measureNumber() + 1) {
            // use measure number
            n = current->segment()->measure()->measureNumber() + 1;
            return String::number(n);
        } else {
            // use number sequence
            n = previousText.toInt() + 1;
            return String::number(n);
        }
    }
}
