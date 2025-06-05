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
#include "linkedobjects.h"

#include "masterscore.h"
#include "measure.h"
#include "score.h"
#include "staff.h"

#include "log.h"

using namespace mu::engraving;

bool LinkedObjects::contains(const EngravingObject* o) const
{
    return std::find(this->begin(), this->end(), o) != this->end();
}

//---------------------------------------------------------
//   mainElement
//    Returns "main" linked element which is expected to
//    be written to the file prior to others.
//---------------------------------------------------------

EngravingObject* LinkedObjects::mainElement()
{
    if (empty()) {
        return nullptr;
    }
    MasterScore* ms = front()->score()->masterScore();
    const bool elements = front()->isEngravingItem();
    const bool staves = front()->isStaff();
    return *std::min_element(begin(), end(), [ms, elements, staves](EngravingObject* s1, EngravingObject* s2) {
        if (s1->score() == ms && s2->score() != ms) {
            return true;
        }
        if (s1->score() != s2->score()) {
            return false;
        }
        if (staves) {
            return toStaff(s1)->idx() < toStaff(s2)->idx();
        }
        if (elements) {
            // Now we compare either two elements from master score
            // or two elements from excerpt.
            EngravingItem* e1 = toEngravingItem(s1);
            EngravingItem* e2 = toEngravingItem(s2);
            const track_idx_t tr1 = e1->track();
            const track_idx_t tr2 = e2->track();
            if (tr1 == tr2) {
                const Fraction tick1 = e1->tick();
                const Fraction tick2 = e2->tick();
                if (tick1 == tick2) {
                    Measure* m1 = e1->findMeasure();
                    Measure* m2 = e2->findMeasure();
                    if (!m1 || !m2) {
                        return false;
                    }

                    // MM rests are written to MSCX in the following order:
                    // 1) first measure of MM rest (m->hasMMRest() == true);
                    // 2) MM rest itself (m->isMMRest() == true);
                    // 3) other measures of MM rest (m->hasMMRest() == false).
                    //
                    // As mainElement() must find the first element that
                    // is going to be written to a file, MM rest writing
                    // order should also be considered.

                    if (m1->isMMRest() == m2->isMMRest()) {
                        // no difference if both are MM rests or both are usual measures
                        return false;
                    }

                    // MM rests may be generated but not written (e.g. if
                    // saving a file right after disabling MM rests)
                    const bool mmRestsWritten = e1->style().styleB(Sid::createMultiMeasureRests);

                    if (m1->isMMRest()) {
                        // m1 is earlier if m2 is *not* the first MM rest measure
                        return mmRestsWritten && !m2->hasMMRest();
                    }
                    if (m2->isMMRest()) {
                        // m1 is earlier if it *is* the first MM rest measure
                        return !mmRestsWritten || m1->hasMMRest();
                    }
                    return false;
                }
                return tick1 < tick2;
            }
            return tr1 < tr2;
        }
        return false;
    });
}
