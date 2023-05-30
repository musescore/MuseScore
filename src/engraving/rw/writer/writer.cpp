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
#include "writer.h"

#include "libmscore/score.h"
#include "libmscore/masterscore.h"
#include "libmscore/part.h"
#include "libmscore/excerpt.h"
#include "libmscore/staff.h"

#include "../xmlwriter.h"

#include "twrite.h"
#include "../400/staffrw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

bool Write400::writeScore(Score* score, io::IODevice* device, bool onlySelection, rw::WriteInOutData* out)
{
    XmlWriter xml(device);
    WriteContext ctx;
    xml.startDocument();

    xml.startElement("museScore", { { "version", MSC_VERSION } });

    if (!MScore::testMode) {
        xml.tag("programVersion", MUSESCORE_VERSION);
        xml.tag("programRevision", MUSESCORE_REVISION);
    }

    compat::WriteScoreHook hook;
    write(score, xml, ctx, onlySelection, hook);

    xml.endElement();

    if (!onlySelection) {
        //update version values for i.e. plugin access
        score->_mscoreVersion = String::fromAscii(MUSESCORE_VERSION);
        score->_mscoreRevision = AsciiStringView(MUSESCORE_REVISION).toInt(nullptr, 16);
        score->_mscVersion = MSCVERSION;
    }
    return true;
}

void Write400::write(Score* score, XmlWriter& xml, rw400::WriteContext& ctx, bool selectionOnly, compat::WriteScoreHook& hook)
{
    // if we have multi measure rests and some parts are hidden,
    // then some layout information is missing:
    // relayout with all parts set visible

    std::list<Part*> hiddenParts;
    bool unhide = false;
    if (score->styleB(Sid::createMultiMeasureRests)) {
        for (Part* part : score->_parts) {
            if (!part->show()) {
                if (!unhide) {
                    score->startCmd();
                    unhide = true;
                }
                part->undoChangeProperty(Pid::VISIBLE, true);
                hiddenParts.push_back(part);
            }
        }
    }
    if (unhide) {
        score->doLayout();
        for (Part* p : hiddenParts) {
            p->setShow(false);
        }
    }

    xml.startElement(score);

    if (score->excerpt()) {
        Excerpt* e = score->excerpt();
        const TracksMap& tracks = e->tracksMapping();
        if (!(tracks.size() == e->nstaves() * VOICES) && !tracks.empty()) {
            for (auto it = tracks.begin(); it != tracks.end(); ++it) {
                xml.tag("Tracklist", { { "sTrack", it->first }, { "dstTrack", it->second } });
            }
        }

        if (e->initialPartId().isValid()) {
            xml.tag("initialPartId", e->initialPartId().toUint64());
        }
    }

    if (score->lineMode()) {
        xml.tag("layoutMode", "line");
    }
    if (score->systemMode()) {
        xml.tag("layoutMode", "system");
    }

    if (score->_audio && ctx.isMsczMode()) {
        xml.tag("playMode", int(score->_playMode));
        rw400::TWrite::write(score->_audio, xml, ctx);
    }

    for (int i = 0; i < 32; ++i) {
        if (!score->_layerTags[i].isEmpty()) {
            xml.tag("LayerTag", { { "id", i }, { "tag", score->_layerTags[i] } }, score->_layerTagComments[i]);
        }
    }
    size_t n = score->_layer.size();
    for (size_t i = 1; i < n; ++i) {         // don’t save default variant
        const Layer& l = score->_layer.at(i);
        xml.tag("Layer",  { { "name", l.name }, { "mask", l.tags } });
    }
    xml.tag("currentLayer", score->_currentLayer);

    if (score->isMaster() && !MScore::testMode) {
        score->_synthesizerState.write(xml);
    }

    if (score->pageNumberOffset()) {
        xml.tag("page-offset", score->pageNumberOffset());
    }
    xml.tag("Division", Constants::division);
    ctx.setCurTrack(mu::nidx);

    hook.onWriteStyle302(score, xml);

    xml.tag("showInvisible", score->_showInvisible);
    xml.tag("showUnprintable", score->_showUnprintable);
    xml.tag("showFrames", score->_showFrames);
    xml.tag("showMargins", score->_showPageborders);
    xml.tag("markIrregularMeasures", score->_markIrregularMeasures, true);

    if (score->_isOpen) {
        xml.tag("open", score->_isOpen);
    }

    for (const auto& t : score->_metaTags) {
        // do not output "platform" and "creationDate" in test and save template mode
        if ((!MScore::testMode && !MScore::saveTemplateMode) || (t.first != "platform" && t.first != "creationDate")) {
            xml.tag("metaTag", { { "name", t.first.toXmlEscaped() } }, t.second);
        }
    }

    if (score->_scoreOrder.isValid()) {
        ScoreOrder order = score->_scoreOrder;
        order.updateInstruments(score);
        order.write(xml);
    }

    if (!score->m_systemObjectStaves.empty()) {
        bool saveSysObjStaves = false;
        for (Staff* s : score->m_systemObjectStaves) {
            IF_ASSERT_FAILED(s->idx() != mu::nidx) {
                continue;
            }
            saveSysObjStaves = true;
            break;
        }
        if (saveSysObjStaves) {
            // write which staves currently have system objects above them
            xml.startElement("SystemObjects");
            for (Staff* s : score->m_systemObjectStaves) {
                IF_ASSERT_FAILED(s->idx() != mu::nidx) {
                    continue;
                }
                // TODO: when we add more granularity to system object display, construct this string per staff
                String sysObjForStaff = u"barNumbers=\"false\"";
                // for now, everything except bar numbers is shown on system object staves
                // (also, the code to display bar numbers on system staves other than the first currently does not exist!)
                xml.tag("Instance", { { "staffId", s->idx() + 1 }, { "barNumbers", "false" } });
            }
            xml.endElement();
        }
    }

    ctx.setCurTrack(0);
    staff_idx_t staffStart;
    staff_idx_t staffEnd;
    MeasureBase* measureStart;
    MeasureBase* measureEnd;

    if (selectionOnly) {
        staffStart   = score->_selection.staffStart();
        staffEnd     = score->_selection.staffEnd();
        // make sure we select full parts
        Staff* sStaff = score->staff(staffStart);
        Part* sPart = sStaff->part();
        Staff* eStaff = score->staff(staffEnd - 1);
        Part* ePart = eStaff->part();
        staffStart = score->staffIdx(sPart);
        staffEnd = score->staffIdx(ePart) + ePart->nstaves();
        measureStart = score->_selection.startSegment()->measure();
        if (measureStart->isMeasure() && toMeasure(measureStart)->isMMRest()) {
            measureStart = toMeasure(measureStart)->mmRestFirst();
        }
        if (score->_selection.endSegment()) {
            measureEnd   = score->_selection.endSegment()->measure()->next();
        } else {
            measureEnd   = 0;
        }
    } else {
        staffStart   = 0;
        staffEnd     = score->nstaves();
        measureStart = score->first();
        measureEnd   = 0;
    }

    // Let's decide: write midi mapping to a file or not
    score->masterScore()->checkMidiMapping();
    for (const Part* part : score->_parts) {
        if (!selectionOnly || ((score->staffIdx(part) >= staffStart) && (staffEnd >= score->staffIdx(part) + part->nstaves()))) {
            rw400::TWrite::write(part, xml, ctx);
        }
    }

    ctx.setCurTrack(0);
    ctx.setTrackDiff(-static_cast<int>(staffStart * VOICES));
    if (measureStart) {
        for (staff_idx_t staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            const Staff* st = score->staff(staffIdx);
            rw400::StaffRW::writeStaff(st, xml, ctx, measureStart, measureEnd, staffStart, staffIdx, selectionOnly);
        }
    }
    ctx.setCurTrack(mu::nidx);

    hook.onWriteExcerpts302(score, xml, ctx, selectionOnly);

    xml.endElement();

    if (unhide) {
        score->endCmd(true);
    }
}
