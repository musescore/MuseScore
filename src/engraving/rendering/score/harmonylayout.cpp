/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "harmonylayout.h"

#include <map>
#include <vector>

#include "realfn.h"
#include "containers.h"

#include "dom/fret.h"
#include "dom/harmony.h"
#include "dom/measurebase.h"
#include "dom/segment.h"
#include "dom/system.h"

#include "tlayout.h"
#include "autoplace.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

//---------------------------------------------------------
//  Harmony alignment compatibility
//  For compatibility with pre 4.6 maxChordShiftAbove/Below
//  we use the old algorithm to find chords which should not
//  be adjusted
//---------------------------------------------------------

// Help class.
// Contains harmonies/fretboard per segment.
class HarmonyList : public std::vector<EngravingItem*>
{
    OBJECT_ALLOCATOR(mu::engraving, HarmonyList);

    std::map<const Segment*, std::vector<EngravingItem*> > elements;
    std::vector<EngravingItem*> modified;

    EngravingItem* getReferenceElement(const Segment* s, bool above, bool visible) const;

public:
    HarmonyList()
    {
        elements.clear();
        modified.clear();
    }

    void append(const Segment* s, EngravingItem* e) { elements[s].push_back(e); }

    double getReferenceHeight(bool above) const;

    bool align(bool above, double reference, double maxShift);

    void addToSkyline(const System* system);
};

double HarmonyList::getReferenceHeight(bool above) const
{
    // The reference height is the height of
    //    the lowest element if placed above
    // or
    //    the highest element if placed below.
    bool first { true };
    double ref { 0.0 };
    for (auto s : muse::keys(elements)) {
        EngravingItem* e { getReferenceElement(s, above, true) };
        if (!e) {
            continue;
        }
        if (e->placeAbove() && above) {
            ref = first ? e->y() : std::min(ref, e->y());
            first = false;
        } else if (e->placeBelow() && !above) {
            ref = first ? e->y() : std::max(ref, e->y());
            first = false;
        }
    }
    return ref;
}

bool HarmonyList::align(bool above, double reference, double maxShift)
{
    // Align the elements. If a segment contains multiple elements,
    // only the reference elements is used in the algorithm. All other
    // elements will remain their original placement with respect to
    // the reference element.
    bool moved = false;
    if (muse::RealIsNull(reference)) {
        return moved;
    }

    for (auto segEls : muse::keys(elements)) {
        std::list<EngravingItem*> handled;
        EngravingItem* refEl = getReferenceElement(segEls, above, false);
        if (!refEl) {
            // If there are only invisible elements, we have to use an invisible
            // element for alignment reference.
            refEl = getReferenceElement(segEls, above, true);
        }

        if (!refEl) {
            continue;
        }

        const bool shouldAdjustAbove = (refEl->y() < (reference + maxShift));
        const bool shouldAdjustBelow = (refEl->y() > (reference - maxShift));
        const bool shouldAdjust = above ? shouldAdjustAbove : shouldAdjustBelow;

        if (shouldAdjust) {
            moved = true;
            refEl->setVerticalAlign(true);
            refEl->setPropertyFlags(Pid::VERTICAL_ALIGN, PropertyFlags::STYLED);
            refEl->triggerLayout();
            for (EngravingItem* e : elements[segEls]) {
                if (e == refEl || (above && e->placeBelow()) || (!above && e->placeAbove())) {
                    continue;
                }
                modified.push_back(e);
                handled.push_back(e);

                e->setVerticalAlign(true);
                e->setPropertyFlags(Pid::VERTICAL_ALIGN, PropertyFlags::STYLED);
                e->triggerLayout();
            }
            for (auto e : handled) {
                muse::remove(elements[segEls], e);
            }
        }
    }
    return moved;
}

EngravingItem* HarmonyList::getReferenceElement(const Segment* s, bool above, bool visible) const
{
    // Returns the reference element for aligning.
    // When a segments contains multiple harmonies/fretboard, the lowest placed
    // element (for placement above, otherwise the highest placed element) is
    // used for alignment.
    EngravingItem* element { nullptr };
    for (EngravingItem* e : elements.at(s)) {
        // Only chord symbols have styled offset, fretboards don't.
        if (!e->autoplace() || (e->isHarmony() && !e->isStyled(Pid::OFFSET)) || (visible && !e->visible())) {
            continue;
        }
        if (!element) {
            element = e;
        } else if ((e->placeAbove() && above && (element->y() < e->y()))
                   || (e->placeBelow() && !above && (element->y() > e->y()))) {
            element = e;
        }
    }
    return element;
}

bool HarmonyLayout::alignHarmonies(const std::vector<Segment*>& sl, bool harmony, const double maxShiftAbove,
                                   const double maxShiftBelow)
{
    if (muse::RealIsNull(maxShiftAbove) && muse::RealIsNull(maxShiftBelow)) {
        return false;
    }

    // Collect all fret diagrams and chord symbol and store them per staff.
    // In the same pass, the maximum height is collected.
    std::map<staff_idx_t, HarmonyList> staves;
    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if ((harmony && e->isHarmony()) || (!harmony && e->isFretDiagram())) {
                staves[e->staffIdx()].append(s, e);
            }
        }
    }

    bool moved = false;

    for (staff_idx_t idx : muse::keys(staves)) {
        // Align the objects.
        // Algorithm:
        //    - Find highest placed harmony/fretdiagram.
        //    - Exclude from alignment all harmony/fretdiagram objects placed outside height and height-maxShiftAbove.
        //    - Repeat for all harmony/fretdiagram objects below height-maxShiftAbove.
        bool movedStaff { true };
        int pass { 0 };
        while (movedStaff && (pass++ < 10)) {
            movedStaff = false;
            movedStaff |= staves[idx].align(true, staves[idx].getReferenceHeight(true), maxShiftAbove);
            movedStaff |= staves[idx].align(false, staves[idx].getReferenceHeight(false), maxShiftBelow);
        }

        moved |= movedStaff;
    }

    return moved;
}
