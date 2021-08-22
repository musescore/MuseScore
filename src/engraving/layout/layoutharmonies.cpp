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
#include "layoutharmonies.h"

#include "realfn.h"

#include "libmscore/fret.h"
#include "libmscore/harmony.h"
#include "libmscore/measurebase.h"
#include "libmscore/segment.h"
#include "libmscore/system.h"

using namespace mu::engraving;
using namespace Ms;

void LayoutHarmonies::layoutHarmonies(const std::vector<Segment*>& sl)
{
    for (const Segment* s : sl) {
        for (Element* e : s->annotations()) {
            if (e->isHarmony()) {
                Harmony* h = toHarmony(e);
                // For chord symbols that coincide with a chord or rest,
                // a partial layout can also happen (if needed) during ChordRest layout
                // in order to calculate a bbox and allocate its shape to the ChordRest.
                // But that layout (if it happens at all) does not do autoplace,
                // so we need the full layout here.
                h->layout();
                h->autoplaceSegmentElement();
            }
        }
    }
}

void LayoutHarmonies::alignHarmonies(const System* system, const std::vector<Segment*>& sl, bool harmony, const double maxShiftAbove,
                                     const double maxShiftBelow)
{
    // Help class.
    // Contains harmonies/fretboard per segment.
    class HarmonyList : public QList<Element*>
    {
        QMap<const Segment*, QList<Element*> > elements;
        QList<Element*> modified;

        Element* getReferenceElement(const Segment* s, bool above, bool visible) const
        {
            // Returns the reference element for aligning.
            // When a segments contains multiple harmonies/fretboard, the lowest placed
            // element (for placement above, otherwise the highest placed element) is
            // used for alignment.
            Element* element { nullptr };
            for (Element* e : elements[s]) {
                // Only chord symbols have styled offset, fretboards don't.
                if (!e->autoplace() || (e->isHarmony() && !e->isStyled(Pid::OFFSET)) || (visible && !e->visible())) {
                    continue;
                }
                if (!element) {
                    element = e;
                } else {
                    if ((e->placeAbove() && above && (element->y() < e->y()))
                        || (e->placeBelow() && !above && (element->y() > e->y()))) {
                        element = e;
                    }
                }
            }
            return element;
        }

    public:
        HarmonyList()
        {
            elements.clear();
            modified.clear();
        }

        void append(const Segment* s, Element* e)
        {
            elements[s].append(e);
        }

        qreal getReferenceHeight(bool above) const
        {
            // The reference height is the height of
            //    the lowest element if placed above
            // or
            //    the highest element if placed below.
            bool first { true };
            qreal ref { 0.0 };
            for (auto s : elements.keys()) {
                Element* e { getReferenceElement(s, above, true) };
                if (!e) {
                    continue;
                }
                if (e->placeAbove() && above) {
                    ref = first ? e->y() : qMin(ref, e->y());
                    first = false;
                } else if (e->placeBelow() && !above) {
                    ref = first ? e->y() : qMax(ref, e->y());
                    first = false;
                }
            }
            return ref;
        }

        bool align(bool above, qreal reference, qreal maxShift)
        {
            // Align the elements. If a segment contains multiple elements,
            // only the reference elements is used in the algorithm. All other
            // elements will remain their original placement with respect to
            // the reference element.
            bool moved { false };
            if (mu::RealIsNull(reference)) {
                return moved;
            }

            for (auto s : elements.keys()) {
                QList<Element*> handled;
                Element* be = getReferenceElement(s, above, false);
                if (!be) {
                    // If there are only invisible elements, we have to use an invisible
                    // element for alignment reference.
                    be = getReferenceElement(s, above, true);
                }
                if (be && ((above && (be->y() < (reference + maxShift))) || ((!above && (be->y() > (reference - maxShift)))))) {
                    qreal shift = be->rypos();
                    be->rypos() = reference - be->ryoffset();
                    shift -= be->rypos();
                    for (Element* e : elements[s]) {
                        if ((above && e->placeBelow()) || (!above && e->placeAbove())) {
                            continue;
                        }
                        modified.append(e);
                        handled.append(e);
                        moved = true;
                        if (e != be) {
                            e->rypos() -= shift;
                        }
                    }
                    for (auto e : handled) {
                        elements[s].removeOne(e);
                    }
                }
            }
            return moved;
        }

        void addToSkyline(const System* system)
        {
            for (Element* e : qAsConst(modified)) {
                const Segment* s = toSegment(e->parent());
                const MeasureBase* m = toMeasureBase(s->parent());
                system->staff(e->staffIdx())->skyline().add(e->shape().translated(e->pos() + s->pos() + m->pos()));
                if (e->isFretDiagram()) {
                    FretDiagram* fd = toFretDiagram(e);
                    Harmony* h = fd->harmony();
                    if (h) {
                        system->staff(e->staffIdx())->skyline().add(h->shape().translated(h->pos() + fd->pos() + s->pos() + m->pos()));
                    } else {
                        system->staff(e->staffIdx())->skyline().add(fd->shape().translated(fd->pos() + s->pos() + m->pos()));
                    }
                }
            }
        }
    };

    if (RealIsNull(maxShiftAbove) && RealIsNull(maxShiftBelow)) {
        return;
    }

    // Collect all fret diagrams and chord symbol and store them per staff.
    // In the same pass, the maximum height is collected.
    QMap<int, HarmonyList> staves;
    for (const Segment* s : sl) {
        for (Element* e : s->annotations()) {
            if ((harmony && e->isHarmony()) || (!harmony && e->isFretDiagram())) {
                staves[e->staffIdx()].append(s, e);
            }
        }
    }

    for (int idx: staves.keys()) {
        // Align the objects.
        // Algorithm:
        //    - Find highest placed harmony/fretdiagram.
        //    - Align all harmony/fretdiagram objects placed between height and height-maxShiftAbove.
        //    - Repeat for all harmony/fretdiagram objects below heigt-maxShiftAbove.
        bool moved { true };
        int pass { 0 };
        while (moved && (pass++ < 10)) {
            moved = false;
            moved |= staves[idx].align(true, staves[idx].getReferenceHeight(true), maxShiftAbove);
            moved |= staves[idx].align(false, staves[idx].getReferenceHeight(false), maxShiftBelow);
        }

        // Add all aligned objects to the sky line.
        staves[idx].addToSkyline(system);
    }
}
