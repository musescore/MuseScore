/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "dom/segment.h"

#include "global/io/buffer.h"

#include "../xmlwriter.h"
#include "../inoutdata.h"

#include "twrite.h"
#include "staffwrite.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::engraving::write;

Writer::Writer()
{
}

bool Writer::writeScore(Score* score, io::IODevice* device, rw::WriteInOutData* inout)
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
    write(score, xml, ctx, hook);

    xml.endElement();
    xml.flush();

    if (!inout || !inout->ctx.shouldWriteRange()) {
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

void Writer::write(Score* score, XmlWriter& xml, WriteContext& ctx, compat::WriteScoreHook& hook)
{
    TRACEFUNC;

    // if we have multi measure rests and some parts are hidden,
    // then some layout information is missing:
    // relayout with all parts set visible (but rollback at end)

    std::vector<Part*> hiddenParts;
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

    staff_idx_t staffStart = 0;
    staff_idx_t staffEnd = 0;
    MeasureBase* measureStart = nullptr;
    MeasureBase* measureEnd = nullptr;

    if (ctx.shouldWriteRange()) {
        const WriteRange& r = ctx.range().value();
        staffStart = r.startStaffIdx;
        staffEnd = r.endStaffIdx;
        measureStart = r.startMeasure;
        measureEnd = r.endMeasure;
    } else {
        staffEnd     = score->nstaves();
        measureStart = score->first();
    }

    if (!score->m_systemObjectStaves.empty()) {
        bool saveSysObjStaves = false;
        for (const Staff* s : score->m_systemObjectStaves) {
            IF_ASSERT_FAILED(s->idx() != muse::nidx) {
                continue;
            }
            saveSysObjStaves = true;
            break;
        }

        if (saveSysObjStaves) {
            xml.startElement("SystemObjects");
            for (const Staff* s : score->m_systemObjectStaves) {
                const staff_idx_t idx = s->idx();
                IF_ASSERT_FAILED(idx != muse::nidx) {
                    continue;
                }

                if (ctx.shouldWriteRange()) {
                    if (idx < staffStart || idx >= staffEnd) {
                        continue;
                    }
                }

                xml.tag("Instance", { { "staffId", idx + 1 } });
            }
            xml.endElement();
        }
    }

    ctx.setCurTrack(0);

    if (score->isMaster()) {
        // Let's decide: write midi mapping to a file or not
        score->masterScore()->checkMidiMapping();
    }

    auto shouldWritePart = [&ctx, score, staffStart, staffEnd](const Part* part) {
        if (!ctx.shouldWriteRange()) {
            return true;
        }

        const staff_idx_t firstStaffIdx = score->staffIdx(part);
        const staff_idx_t lastStaffIdx = firstStaffIdx + part->nstaves() - 1;

        return (firstStaffIdx >= staffStart && firstStaffIdx < staffEnd)
               || (lastStaffIdx >= staffStart && lastStaffIdx < staffEnd);
    };

    for (const Part* part : score->m_parts) {
        if (shouldWritePart(part)) {
            TWrite::write(part, xml, ctx);
        }
    }

    ctx.setCurTrack(0);
    ctx.setTrackDiff(-static_cast<int>(staffStart * VOICES));
    if (measureStart) {
        for (staff_idx_t staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            const Staff* st = score->staff(staffIdx);
            StaffWrite::writeStaff(st, xml, ctx, measureStart, measureEnd, staffStart, staffIdx);
        }
    }
    ctx.setCurTrack(muse::nidx);

    TWrite::writeScoreSpanners(score, xml, ctx);

    hook.onWriteExcerpts302(score, xml, ctx);

    TWrite::writePageLocks(score, xml);
    TWrite::writeSystemLocks(score, xml);
    TWrite::writeSystemDividers(score, xml, ctx);

    xml.endElement(); // score

    if (unhide) {
        score->endCmd(true);
    }
}

void Writer::writeSegments(XmlWriter& xml, WriteContext& ctx, track_idx_t strack, track_idx_t etrack,
                           Segment* sseg, Segment* eseg, bool writeSystemElements, bool forceTimeSig)
{
    ctx.setCurTrack(strack);
    TWrite::writeSegments(xml, ctx, strack, etrack, sseg, eseg, writeSystemElements, forceTimeSig);
}

void Writer::doWriteItem(const EngravingItem* item, XmlWriter& xml)
{
    WriteContext ctx(item->score(), /* clipboardMode = */ true);
    TWrite::writeItem(item, xml, ctx);
}

muse::ByteArray Writer::writeStaffSelection(Score* score, const SelectionFilter& filter, staff_idx_t staffStart, staff_idx_t staffEnd,
                                            const Fraction& tickStart, const Fraction& tickEnd, Segment* startSegment,
                                            Segment* endSegment)
{
    auto buffer = io::Buffer::opened(io::IODevice::WriteOnly);
    XmlWriter xml(&buffer);

    xml.startDocument();

    Fraction ticks = tickEnd - tickStart;
    int staves = static_cast<int>(staffEnd - staffStart);

    XmlWriter::Attributes staffListAttributes = {
        { "version", (MScore::testMode ? "2.00" : Constants::MSC_VERSION_STR) },
        { "tick", tickStart.toString() },
        { "len", ticks.toString() },
        { "staff", staffStart },
        { "staves", staves },
    };

    // Note: canCopy() ensures that the whole selection has a single time stretch ratio.
    Fraction timeStretch = score->staff(staffStart)->timeStretch(tickStart);
    if (timeStretch != Fraction(1, 1)) {
        staffListAttributes.push_back({ "timeStretch", timeStretch.toString() });
    }

    xml.startElement("StaffList", staffListAttributes);

    WriteContext ctx(score, /* clipboardMode = */ true);
    ctx.setFilter(filter);
    ctx.setFirstClipboardTick(tickStart);

    for (staff_idx_t staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
        track_idx_t startTrack = staffIdx * VOICES;
        track_idx_t endTrack   = startTrack + VOICES;

        xml.startElement("Staff", { { "id", staffIdx } });

        Staff* staff = score->staff(staffIdx);
        Part* part = staff->part();
        Interval interval = part->instrument(startSegment->tick())->transpose();
        if (interval.chromatic) {
            xml.tag("transposeChromatic", interval.chromatic);
        }
        if (interval.diatonic) {
            xml.tag("transposeDiatonic", interval.diatonic);
        }
        writeSegments(xml, ctx, startTrack, endTrack, startSegment, endSegment, false, false);
        xml.endElement();
    }

    TWrite::writeScoreSpanners(score, staff2track(staffStart), staff2track(staffEnd), startSegment, endSegment, xml, ctx);

    xml.endElement();
    xml.flush();
    return buffer.data();
}

muse::ByteArray Writer::writeSymbolListSelection(track_idx_t fromTrack, track_idx_t toTrack,
                                                 const std::vector<SelectedSymbol>& symbols)
{
    auto buffer = io::Buffer::opened(io::IODevice::WriteOnly);
    XmlWriter xml(&buffer);

    xml.startDocument();

    xml.startElement("SymbolList", { { "version", Constants::MSC_VERSION_STR },
                         { "fromtrack", fromTrack },
                         { "totrack", toTrack } });

    track_idx_t currTrack = muse::nidx;
    for (const SelectedSymbol& symbol : symbols) {
        if (currTrack != symbol.track) {
            xml.tag("trackOffset", static_cast<int>(symbol.track - fromTrack));
            currTrack = symbol.track;
        }
        xml.tag("tickOffset", symbol.tickOffset);
        xml.tag("segDelta", symbol.segDelta);
        doWriteItem(symbol.element, xml);
    }

    xml.endElement();
    xml.flush();
    buffer.close();
    return buffer.data();
}
