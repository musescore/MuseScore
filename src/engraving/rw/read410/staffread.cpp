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
#include "staffread.h"

#include "rw/write/twrite.h"

#include "libmscore/factory.h"
#include "libmscore/measure.h"
#include "libmscore/score.h"

#include "measureread.h"
#include "tread.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::read410;

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
                Fraction f(m ? m->timesig() : Fraction(4, 4));
                measure->setTicks(f);
                measure->setTimesig(f);

                MeasureRead::readMeasure(measure, e, ctx, staff);
                measure->checkMeasure(staff);
                if (!measure->isMMRest()) {
                    score->measures()->add(measure);
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
                    }
                }
            } else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                MeasureBase* mb = toMeasureBase(Factory::createItemByName(tag, ctx.dummy()));
                TRead::readItem(mb, e, ctx);
                mb->setTick(ctx.tick());
                score->measures()->add(mb);
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
                    score->measures()->add(measure);
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
