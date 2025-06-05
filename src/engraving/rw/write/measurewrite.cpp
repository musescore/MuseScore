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
#include "measurewrite.h"

#include "dom/measure.h"
#include "dom/measurenumber.h"
#include "dom/mmrestrange.h"
#include "dom/spacer.h"
#include "dom/textlinebase.h"

#include "twrite.h"

using namespace mu::engraving;
using namespace mu::engraving::write;

void MeasureWrite::writeMeasure(const Measure* measure, XmlWriter& xml, WriteContext& ctx,
                                staff_idx_t staff,
                                bool writeSystemElements,
                                bool forceTimeSig)
{
    if (MScore::debugMode) {
        const int mno = measure->no() + 1;
        xml.comment(String(u"Measure %1").arg(mno));
    }
    if (measure->m_len != measure->m_timesig) {
        // this is an irregular measure
        xml.startElement(measure, { { "len", measure->m_len.toString() } });
    } else {
        xml.startElement(measure);
    }

    ctx.setCurTick(measure->tick());
    ctx.setCurTrack(staff * VOICES);

    if (measure->m_mmRestCount > 0) {
        xml.tag("multiMeasureRest", measure->m_mmRestCount);
    }
    if (writeSystemElements) {
        TWrite::writeItemEid(measure, xml, ctx);
        if (measure->repeatStart()) {
            xml.tag("startRepeat");
        }
        if (measure->repeatEnd()) {
            xml.tag("endRepeat", measure->m_repeatCount);
        }
        TWrite::writeProperty(measure, xml, Pid::IRREGULAR);
        TWrite::writeProperty(measure, xml, Pid::BREAK_MMR);
        TWrite::writeProperty(measure, xml, Pid::USER_STRETCH);
        TWrite::writeProperty(measure, xml, Pid::NO_OFFSET);
        TWrite::writeProperty(measure, xml, Pid::MEASURE_NUMBER_MODE);
    }
    MStaff* mstaff = measure->m_mstaves[staff];
    if (mstaff->noText() && !mstaff->noText()->generated()) {
        TWrite::write(mstaff->noText(), xml, ctx);
    }

    if (mstaff->mmRangeText() && !mstaff->mmRangeText()->generated()) {
        TWrite::write(mstaff->mmRangeText(), xml, ctx);
    }

    if (mstaff->vspacerUp()) {
        xml.tag("vspacerUp", mstaff->vspacerUp()->gap().val());
    }
    if (mstaff->vspacerDown()) {
        if (mstaff->vspacerDown()->spacerType() == SpacerType::FIXED) {
            xml.tag("vspacerFixed", mstaff->vspacerDown()->gap().val());
        } else {
            xml.tag("vspacerDown", mstaff->vspacerDown()->gap().val());
        }
    }
    if (!mstaff->visible()) {
        xml.tag("visible", mstaff->visible());
    }
    if (mstaff->stemless()) {
        xml.tag("slashStyle", mstaff->stemless());     // for backwards compatibility
        xml.tag("stemless", mstaff->stemless());
    }
    if (mstaff->measureRepeatCount()) {
        xml.tag("measureRepeatCount", mstaff->measureRepeatCount());
    }

    track_idx_t strack = staff * VOICES;
    track_idx_t etrack = strack + VOICES;
    for (const EngravingItem* e : measure->el()) {
        if (e->generated()) {
            continue;
        }

        bool writeSystem = writeSystemElements;
        if (e->systemFlag()) {
            ElementType et = e->type();
            if ((et == ElementType::REHEARSAL_MARK)
                || (et == ElementType::SYSTEM_TEXT)
                || (et == ElementType::TRIPLET_FEEL)
                || (et == ElementType::PLAYTECH_ANNOTATION)
                || (et == ElementType::JUMP)
                || (et == ElementType::MARKER)
                || (et == ElementType::TEMPO_TEXT)
                || isSystemTextLine(e)) {
                writeSystem = (e->staffIdx() == staff); // always show these on appropriate staves
            }
        }

        if (e->staffIdx() != staff) {
            if (!e->systemFlag() || (e->systemFlag() && !writeSystem)) {
                continue;
            }
        }

        TWrite::writeItem(e, xml, ctx);
    }
    assert(measure->first());
    assert(measure->last());
    if (measure->first() && measure->last()) {
        TWrite::writeSegments(xml, ctx, strack, etrack, measure->first(), measure->last()->next1(), writeSystemElements,
                              forceTimeSig);
    }

    xml.endElement();
}
