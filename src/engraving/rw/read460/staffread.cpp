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
#include "staffread.h"

#include "rw/write/twrite.h"

#include "dom/factory.h"
#include "dom/box.h"
#include "dom/measure.h"
#include "dom/score.h"

#include "measureread.h"
#include "tread.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::read460;

void StaffRead::readStaff(Score* score, XmlReader& e, ReadContext& ctx)
{
    int staff = e.intAttribute("id", 1) - 1;
    int measureIdx = 0;
    ctx.setCurrentMeasureIndex(0);
    ctx.setTick(Fraction(0, 1));
    ctx.setTrack(staff * VOICES);

    if (staff == 0) {
        while (e.readNextStartElement()) {
            const AsciiStringView tag(e.name());

            if (tag == "Measure") {
                Measure* measure = Factory::createMeasure(ctx.dummy()->system());
                measure->setTick(ctx.tick());
                ctx.setCurrentMeasureIndex(measureIdx++);
                //
                // inherit timesig from previous measure
                //
                Measure* m = ctx.lastMeasure();             // measure->prevMeasure();
                Fraction f(ctx.timeSigForNextMeasure() != Fraction(0, 1) ? ctx.timeSigForNextMeasure()
                           : m ? m->timesig() : Fraction(4, 4));
                measure->setTicks(f);
                measure->setTimesig(f);
                ctx.setTimeSigForNextMeasure(Fraction(0, 1));

                MeasureRead::readMeasure(measure, e, ctx, staff);
                measure->checkMeasure(staff);
                if (!measure->isMMRest()) {
                    score->measures()->append(measure);
                    if (m && m->mmRest()) {
                        m->mmRest()->setNext(measure);
                    }
                    score->checkSpanner(ctx.tick(), ctx.tick() + measure->ticks(), /*removeOrphans*/ false);
                    ctx.setLastMeasure(measure);
                    ctx.setTick(measure->tick() + measure->ticks());
                } else {
                    // this is a multi measure rest
                    // always preceded by the first measure it replaces
                    Measure* m1 = ctx.lastMeasure();

                    if (m1) {
                        m1->setMMRest(measure);
                        measure->setTick(m1->tick());
                        measure->setPrev(m1->prev());
                    }
                }
            } else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                MeasureBase* mb = toMeasureBase(Factory::createItemByName(tag, ctx.dummy()));
                mb->setTick(ctx.tick());
                score->measures()->append(mb);
                // This default value needs initialising after being added to the score, as it depends on whether this is the title frame
                if (mb->score()->mscVersion() >= 440) {
                    mb->setSizeIsSpatiumDependent(mb->propertyDefault(Pid::SIZE_SPATIUM_DEPENDENT).toBool());
                }

                TRead::readItem(mb, e, ctx);
            } else if (tag == "tick") {
                ctx.setTick(Fraction::fromTicks(ctx.fileDivision(e.readInt())));
            } else {
                e.unknown();
            }
        }
    } else {
        Measure* measure = score->firstMeasure();
        while (e.readNextStartElement()) {
            const AsciiStringView tag(e.name());

            if (tag == "Measure") {
                if (measure == 0) {
                    LOGD("Score::readStaff(): missing measure!");
                    measure = Factory::createMeasure(ctx.dummy()->system());
                    measure->setTick(ctx.tick());
                    score->measures()->append(measure);
                }
                ctx.setTick(measure->tick());
                ctx.setCurrentMeasureIndex(measureIdx++);
                MeasureRead::readMeasure(measure, e, ctx, staff);
                measure->checkMeasure(staff);
                if (measure->isMMRest()) {
                    measure = ctx.lastMeasure()->nextMeasure();
                } else {
                    ctx.setLastMeasure(measure);
                    if (measure->mmRest()) {
                        measure = measure->mmRest();
                    } else {
                        measure = measure->nextMeasure();
                    }
                }
            } else if (tag == "tick") {
                ctx.setTick(Fraction::fromTicks(ctx.fileDivision(e.readInt())));
            } else {
                e.unknown();
            }
        }
    }
}
