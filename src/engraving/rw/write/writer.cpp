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
#include "writer.h"

#include "../types/types.h"

#include "dom/score.h"
#include "dom/masterscore.h"
#include "dom/part.h"
#include "dom/excerpt.h"
#include "dom/staff.h"

#include "../xmlwriter.h"
#include "../inoutdata.h"

#include "twrite.h"
#include "staffwrite.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::engraving::write;

Writer::Writer(const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
}

bool Writer::writeScore(Score* score, io::IODevice* device, bool onlySelection, rw::WriteInOutData* inout)
{
    TRACEFUNC;

    XmlWriter xml(device);
    WriteContext ctx(score);
    if (inout) {
        ctx = inout->ctx;
    }

    xml.startDocument();

    xml.startElement("museScore", { { "version", Constants::MSC_VERSION_STR } });

    if (!MScore::testMode) {
        xml.tag("programVersion", application()->version().toString());
        xml.tag("programRevision", application()->revision());
    }

    compat::WriteScoreHook hook;
    write(score, xml, ctx, onlySelection, hook);

    xml.endElement();

    if (!onlySelection) {
        //update version values for i.e. plugin access
        score->m_mscoreVersion = application()->version().toString();
        score->m_mscoreRevision = application()->revision().toInt(nullptr, 16);
        score->m_mscVersion = Constants::MSC_VERSION;
    }

    if (inout) {
        inout->ctx = ctx;
    }

    return true;
}

void Writer::write(Score* score, XmlWriter& xml, WriteContext& ctx, bool selectionOnly, compat::WriteScoreHook& hook)
{
    TRACEFUNC;

    // if we have multi measure rests and some parts are hidden,
    // then some layout information is missing:
    // relayout with all parts set visible (but rollback at end)

    std::list<Part*> hiddenParts;
    bool unhide = false;
    if (score->style().styleB(Sid::createMultiMeasureRests)) {
        for (Part* part : score->m_parts) {
            if (!part->show()) {
                if (!unhide) {
                    score->startCmd(TranslatableString::untranslatable("Unhide instruments for save"));
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

    TWrite::writeItemEid(score, xml, ctx);

    if (Excerpt* e = score->excerpt()) {
        if (!e->name().empty()) {
            xml.tag("name", e->name());
        }

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

    if (score->isLayoutMode(LayoutMode::LINE)) {
        xml.tag("layoutMode", "line");
    }
    if (score->isLayoutMode(LayoutMode::SYSTEM)) {
        xml.tag("layoutMode", "system");
    }

    if (score->m_audio) {
        xml.tag("playMode", int(score->m_playMode));
        TWrite::write(score->m_audio, xml, ctx);
    }

    if (score->isMaster() && !MScore::testMode) {
        score->m_synthesizerState.write(xml);
    }

    if (score->pageNumberOffset()) {
        xml.tag("page-offset", score->pageNumberOffset());
    }
    xml.tag("Division", Constants::DIVISION);
    ctx.setCurTrack(muse::nidx);

    hook.onWriteStyle302(score, xml);

    xml.tag("showInvisible", score->m_showInvisible);
    xml.tag("showUnprintable", score->m_showUnprintable);
    xml.tag("showFrames", score->m_showFrames);
    xml.tag("showMargins", score->m_showPageborders);
    xml.tag("markIrregularMeasures", score->m_markIrregularMeasures, true);

    if (!score->m_showSoundFlags) { // true by default
        xml.tag("showSoundFlags", score->m_showSoundFlags);
    }

    if (score->m_isOpen) {
        xml.tag("open", score->m_isOpen);
    }

    for (const auto& t : score->m_metaTags) {
        // do not output "platform" and "creationDate" in test and save template mode
        if ((!MScore::testMode && !MScore::saveTemplateMode) || (t.first != "platform" && t.first != "creationDate")) {
            xml.tag("metaTag", { { "name", t.first.toXmlEscaped() } }, t.second);
        }
    }

    if (score->m_scoreOrder.isValid()) {
        ScoreOrder order = score->m_scoreOrder;
        order.updateInstruments(score);
        order.write(xml);
    }

    if (!score->m_systemObjectStaves.empty()) {
        bool saveSysObjStaves = false;
        for (Staff* s : score->m_systemObjectStaves) {
            IF_ASSERT_FAILED(s->idx() != muse::nidx) {
                continue;
            }
            saveSysObjStaves = true;
            break;
        }
        if (saveSysObjStaves) {
            xml.startElement("SystemObjects");
            for (Staff* s : score->m_systemObjectStaves) {
                IF_ASSERT_FAILED(s->idx() != muse::nidx) {
                    continue;
                }
                xml.tag("Instance", { { "staffId", s->idx() + 1 } });
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
        staffStart   = score->m_selection.staffStart();
        staffEnd     = score->m_selection.staffEnd();
        // make sure we select full parts
        Staff* sStaff = score->staff(staffStart);
        Part* sPart = sStaff->part();
        Staff* eStaff = score->staff(staffEnd - 1);
        Part* ePart = eStaff->part();
        staffStart = score->staffIdx(sPart);
        staffEnd = score->staffIdx(ePart) + ePart->nstaves();
        measureStart = score->m_selection.startSegment()->measure();
        if (measureStart->isMeasure() && toMeasure(measureStart)->isMMRest()) {
            measureStart = toMeasure(measureStart)->mmRestFirst();
        }
        if (score->m_selection.endSegment()) {
            measureEnd   = score->m_selection.endSegment()->measure()->next();
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
    for (const Part* part : score->m_parts) {
        if (!selectionOnly || ((score->staffIdx(part) >= staffStart) && (staffEnd >= score->staffIdx(part) + part->nstaves()))) {
            TWrite::write(part, xml, ctx);
        }
    }

    ctx.setCurTrack(0);
    ctx.setTrackDiff(-static_cast<int>(staffStart * VOICES));
    if (measureStart) {
        for (staff_idx_t staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            const Staff* st = score->staff(staffIdx);
            StaffWrite::writeStaff(st, xml, ctx, measureStart, measureEnd, staffStart, staffIdx, selectionOnly);
        }
    }
    ctx.setCurTrack(muse::nidx);

    hook.onWriteExcerpts302(score, xml, ctx, selectionOnly);

    TWrite::writeSystemLocks(score, xml);

    xml.endElement(); // score

    if (unhide) {
        score->endCmd(true);
    }
}

void Writer::writeSegments(XmlWriter& xml, SelectionFilter* filter, track_idx_t strack, track_idx_t etrack,
                           Segment* sseg, Segment* eseg, bool writeSystemElements, bool forceTimeSig, Fraction& curTick)
{
    WriteContext ctx(sseg->score());
    ctx.setClipboardmode(true);
    ctx.setFilter(*filter);
    ctx.setCurTrack(strack);
    ctx.setCurTick(curTick);
    TWrite::writeSegments(xml, ctx, strack, etrack, sseg, eseg, writeSystemElements, forceTimeSig);
    curTick = ctx.curTick();
}

void Writer::doWriteItem(const EngravingItem* item, XmlWriter& xml)
{
    WriteContext ctx(item->score());
    ctx.setClipboardmode(true);
    TWrite::writeItem(item, xml, ctx);
}
