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
#include "staffwrite.h"

#include "dom/score.h"
#include "dom/staff.h"
#include "dom/measure.h"

#include "twrite.h"
#include "measurewrite.h"

using namespace mu::engraving;
using namespace mu::engraving::write;

static void writeMeasure(XmlWriter& xml, WriteContext& ctx, MeasureBase* m,
                         staff_idx_t staffIdx,
                         bool writeSystemElements,
                         bool forceTimeSig)
{
    //
    // special case multi measure rest
    //
    if (m->isMeasure() || staffIdx == 0) {
        if (Measure::classof(m)) {
            MeasureWrite::writeMeasure(static_cast<const Measure*>(m), xml, ctx, staffIdx, writeSystemElements, forceTimeSig);
        } else {
            TWrite::writeItem(m, xml, ctx);
        }
    }

    if (m->score()->style().styleB(Sid::createMultiMeasureRests) && m->isMeasure() && toMeasure(m)->mmRest()) {
        MeasureWrite::writeMeasure(toMeasure(m)->mmRest(), xml, ctx, staffIdx, writeSystemElements, forceTimeSig);
    }

    ctx.setCurTick(m->endTick());
}

void StaffWrite::writeStaff(const Staff* staff, XmlWriter& xml, write::WriteContext& ctx,
                            MeasureBase* measureStart, MeasureBase* measureEnd,
                            staff_idx_t staffStart, staff_idx_t staffIdx,
                            bool selectionOnly)
{
    xml.startElement(staff, { { "id", static_cast<int>(staffIdx + 1 - staffStart) } });

    ctx.setCurTick(measureStart->tick());
    ctx.setTickDiff(ctx.curTick());
    ctx.setCurTrack(staffIdx * VOICES);
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
        writeMeasure(xml, ctx, m, staffIdx, writeSystemElements, forceTimeSig);
    }

    xml.endElement();
}
