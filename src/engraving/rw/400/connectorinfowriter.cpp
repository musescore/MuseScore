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
#include "connectorinfowriter.h"

#include "../../libmscore/engravingitem.h"
#include "../../libmscore/spanner.h"
#include "../../libmscore/measurebase.h"
#include "../../libmscore/measure.h"
#include "../../libmscore/score.h"

#include "../xmlwriter.h"

#include "twrite.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

ConnectorInfoWriter::ConnectorInfoWriter(XmlWriter& xml, const EngravingItem* current, const EngravingItem* connector, int track,
                                         Fraction frac)
    : ConnectorInfo(current, track, frac), _xml(&xml), _connector(connector)
{
    IF_ASSERT_FAILED(current) {
        return;
    }
    _type = connector->type();
    updateCurrentInfo(xml.context()->clipboardmode());
}

void ConnectorInfoWriter::write()
{
    XmlWriter& xml = *_xml;
    WriteContext& ctx = *xml.context();
    if (!ctx.canWrite(_connector)) {
        return;
    }
    xml.startElement(tagName(), { { "type", _connector->typeName() } });
    if (isStart()) {
        rw400::TWrite::writeItem(_connector, xml, ctx);
    }
    if (hasPrevious()) {
        xml.startElement("prev");
        _prevLoc.toRelative(_currentLoc);
        rw400::TWrite::write(&_prevLoc, xml, ctx);
        xml.endElement();
    }
    if (hasNext()) {
        xml.startElement("next");
        _nextLoc.toRelative(_currentLoc);
        rw400::TWrite::write(&_nextLoc, xml, ctx);
        xml.endElement();
    }
    xml.endElement();
}

//---------------------------------------------------------
//   SpannerWriter::fillSpannerPosition
//---------------------------------------------------------

void SpannerWriter::fillSpannerPosition(Location& l, const MeasureBase* m, const Fraction& tick, bool clipboardmode)
{
    if (clipboardmode) {
        l.setMeasure(0);
        l.setFrac(tick);
    } else {
        if (!m) {
            LOGW("fillSpannerPosition: couldn't find spanner's endpoint's measure");
            l.setMeasure(0);
            l.setFrac(tick);
            return;
        }
        l.setMeasure(m->measureIndex());
        l.setFrac(tick - m->tick());
    }
}

//---------------------------------------------------------
//   SpannerWriter::SpannerWriter
//---------------------------------------------------------

SpannerWriter::SpannerWriter(XmlWriter& xml, const EngravingItem* current, const Spanner* sp, int track, Fraction frac, bool start)
    : ConnectorInfoWriter(xml, current, sp, track, frac)
{
    const bool clipboardmode = xml.context()->clipboardmode();
    if (!sp->startElement() || !sp->endElement()) {
        LOGW("SpannerWriter: spanner (%s) doesn't have an endpoint!", sp->typeName());
        return;
    }
    if (current->isMeasure() || current->isSegment() || (sp->startElement()->type() != current->type())) {
        // (The latter is the hairpins' case, for example, though they are
        // covered by the other checks too.)
        // We cannot determine position of the spanner from its start/end
        // elements and will try to obtain this info from the spanner itself.
        if (!start) {
            _prevLoc.setTrack(static_cast<int>(sp->track()));
            Measure* m = sp->score()->tick2measure(sp->tick());
            fillSpannerPosition(_prevLoc, m, sp->tick(), clipboardmode);
        } else {
            const track_idx_t track2 = (sp->track2() != mu::nidx) ? sp->track2() : sp->track();
            _nextLoc.setTrack(static_cast<int>(track2));
            Measure* m = sp->score()->tick2measure(sp->tick2());
            fillSpannerPosition(_nextLoc, m, sp->tick2(), clipboardmode);
        }
    } else {
        // We can obtain the spanner position info from its start/end
        // elements and will prefer this source of information.
        // Reason: some spanners contain no or wrong information (e.g. Ties).
        if (!start) {
            updateLocation(sp->startElement(), _prevLoc, clipboardmode);
        } else {
            updateLocation(sp->endElement(), _nextLoc, clipboardmode);
        }
    }
}
