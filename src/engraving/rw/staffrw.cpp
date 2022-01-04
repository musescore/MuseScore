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
#include "staffrw.h"

#include "rw/xml.h"

#include "libmscore/factory.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/staff.h"

#include "measurerw.h"

using namespace mu::engraving::rw;
using namespace Ms;

void StaffRW::readStaff(Ms::Score* score, Ms::XmlReader& e, ReadContext& ctx)
{
    int staff = e.intAttribute("id", 1) - 1;
    int measureIdx = 0;
    e.setCurrentMeasureIndex(0);
    e.setTick(Fraction(0, 1));
    e.setTrack(staff * VOICES);

    if (staff == 0) {
        while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "Measure") {
                Measure* measure = Factory::createMeasure(ctx.dummy()->system());
                measure->setTick(e.tick());
                e.setCurrentMeasureIndex(measureIdx++);
                //
                // inherit timesig from previous measure
                //
                Measure* m = e.lastMeasure();             // measure->prevMeasure();
                Fraction f(m ? m->timesig() : Fraction(4, 4));
                measure->setTicks(f);
                measure->setTimesig(f);

                MeasureRW::readMeasure(measure, e, ctx, staff);
                measure->checkMeasure(staff);
                if (!measure->isMMRest()) {
                    score->measures()->add(measure);
                    e.setLastMeasure(measure);
                    e.setTick(measure->tick() + measure->ticks());
                } else {
                    // this is a multi measure rest
                    // always preceded by the first measure it replaces
                    Measure* m1 = e.lastMeasure();

                    if (m1) {
                        m1->setMMRest(measure);
                        measure->setTick(m1->tick());
                    }
                }
            } else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                MeasureBase* mb = toMeasureBase(Factory::createItemByName(tag, ctx.dummy()));
                mb->read(e);
                mb->setTick(e.tick());
                score->measures()->add(mb);
            } else if (tag == "tick") {
                e.setTick(Fraction::fromTicks(ctx.fileDivision(e.readInt())));
            } else {
                e.unknown();
            }
        }
    } else {
        Measure* measure = score->firstMeasure();
        while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "Measure") {
                if (measure == 0) {
                    qDebug("Score::readStaff(): missing measure!");
                    measure = Factory::createMeasure(ctx.dummy()->system());
                    measure->setTick(e.tick());
                    score->measures()->add(measure);
                }
                e.setTick(measure->tick());
                e.setCurrentMeasureIndex(measureIdx++);
                MeasureRW::readMeasure(measure, e, ctx, staff);
                measure->checkMeasure(staff);
                if (measure->isMMRest()) {
                    measure = e.lastMeasure()->nextMeasure();
                } else {
                    e.setLastMeasure(measure);
                    if (measure->mmRest()) {
                        measure = measure->mmRest();
                    } else {
                        measure = measure->nextMeasure();
                    }
                }
            } else if (tag == "tick") {
                e.setTick(Fraction::fromTicks(ctx.fileDivision(e.readInt())));
            } else {
                e.unknown();
            }
        }
    }
}

static void writeMeasure(XmlWriter& xml, MeasureBase* m, int staffIdx, bool writeSystemElements, bool forceTimeSig)
{
    //
    // special case multi measure rest
    //
    if (m->isMeasure() || staffIdx == 0) {
        m->write(xml, staffIdx, writeSystemElements, forceTimeSig);
    }

    if (m->score()->styleB(Sid::createMultiMeasureRests) && m->isMeasure() && toMeasure(m)->mmRest()) {
        toMeasure(m)->mmRest()->write(xml, staffIdx, writeSystemElements, forceTimeSig);
    }

    xml.setCurTick(m->endTick());
}

void StaffRW::writeStaff(const Ms::Staff* staff, Ms::XmlWriter& xml,
                         Ms::MeasureBase* measureStart, Ms::MeasureBase* measureEnd,
                         int staffStart, int staffIdx,
                         bool selectionOnly)
{
    xml.startObject(staff, QString("id=\"%1\"").arg(staffIdx + 1 - staffStart));

    xml.setCurTick(measureStart->tick());
    xml.setTickDiff(xml.curTick());
    xml.setCurTrack(staffIdx * VOICES);
    bool writeSystemElements = (staffIdx == staffStart);
    bool firstMeasureWritten = false;
    bool forceTimeSig = false;
    for (MeasureBase* m = measureStart; m != measureEnd; m = m->next()) {
        // force timesig if first measure and selectionOnly
        if (selectionOnly && m->isMeasure()) {
            if (!firstMeasureWritten) {
                forceTimeSig = true;
                firstMeasureWritten = true;
            } else {
                forceTimeSig = false;
            }
        }
        writeMeasure(xml, m, staffIdx, writeSystemElements, forceTimeSig);
    }

    xml.endObject();
}
