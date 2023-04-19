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

#include <cmath>

#include "io/file.h"
#include "io/fileinfo.h"

#include "style/style.h"

#include "compat/writescorehook.h"
#include "rw/xml.h"
#include "rw/400/twrite.h"
#include "rw/400/writecontext.h"
#include "rw/400/staffrw.h"

#include "audio.h"
#include "chordrest.h"
#include "engravingitem.h"
#include "excerpt.h"
#include "factory.h"
#include "keysig.h"
#include "masterscore.h"
#include "measure.h"
#include "mscore.h"
#include "page.h"
#include "part.h"
#include "rest.h"
#include "score.h"
#include "scoreorder.h"
#include "segment.h"
#include "sig.h"
#include "staff.h"
#include "timesig.h"
#include "undo.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Score::write(XmlWriter& xml, bool selectionOnly, compat::WriteScoreHook& hook)
{
    // if we have multi measure rests and some parts are hidden,
    // then some layout information is missing:
    // relayout with all parts set visible

    std::list<Part*> hiddenParts;
    bool unhide = false;
    if (styleB(Sid::createMultiMeasureRests)) {
        for (Part* part : _parts) {
            if (!part->show()) {
                if (!unhide) {
                    startCmd();
                    unhide = true;
                }
                part->undoChangeProperty(Pid::VISIBLE, true);
                hiddenParts.push_back(part);
            }
        }
    }
    if (unhide) {
        doLayout();
        for (Part* p : hiddenParts) {
            p->setShow(false);
        }
    }

    xml.startElement(this);

    if (excerpt()) {
        Excerpt* e = excerpt();
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

    if (lineMode()) {
        xml.tag("layoutMode", "line");
    }
    if (systemMode()) {
        xml.tag("layoutMode", "system");
    }

    if (_audio && xml.context()->isMsczMode()) {
        xml.tag("playMode", int(_playMode));
        _audio->write(xml);
    }

    for (int i = 0; i < 32; ++i) {
        if (!_layerTags[i].isEmpty()) {
            xml.tag("LayerTag", { { "id", i }, { "tag", _layerTags[i] } }, _layerTagComments[i]);
        }
    }
    size_t n = _layer.size();
    for (size_t i = 1; i < n; ++i) {         // don’t save default variant
        const Layer& l = _layer.at(i);
        xml.tag("Layer",  { { "name", l.name }, { "mask", l.tags } });
    }
    xml.tag("currentLayer", _currentLayer);

    if (isMaster() && !MScore::testMode) {
        _synthesizerState.write(xml);
    }

    if (pageNumberOffset()) {
        xml.tag("page-offset", pageNumberOffset());
    }
    xml.tag("Division", Constants::division);
    xml.context()->setCurTrack(mu::nidx);

    hook.onWriteStyle302(this, xml);

    xml.tag("showInvisible", _showInvisible);
    xml.tag("showUnprintable", _showUnprintable);
    xml.tag("showFrames", _showFrames);
    xml.tag("showMargins", _showPageborders);
    xml.tag("markIrregularMeasures", _markIrregularMeasures, true);

    if (_isOpen) {
        xml.tag("open", _isOpen);
    }

    for (const auto& t : _metaTags) {
        // do not output "platform" and "creationDate" in test and save template mode
        if ((!MScore::testMode && !MScore::saveTemplateMode) || (t.first != "platform" && t.first != "creationDate")) {
            xml.tag("metaTag", { { "name", t.first.toXmlEscaped() } }, t.second);
        }
    }

    if (_scoreOrder.isValid()) {
        ScoreOrder order = _scoreOrder;
        order.updateInstruments(this);
        order.write(xml);
    }

    if (!m_systemObjectStaves.empty()) {
        bool saveSysObjStaves = false;
        for (Staff* s : m_systemObjectStaves) {
            IF_ASSERT_FAILED(s->idx() != mu::nidx) {
                continue;
            }
            saveSysObjStaves = true;
            break;
        }
        if (saveSysObjStaves) {
            // write which staves currently have system objects above them
            xml.startElement("SystemObjects");
            for (Staff* s : m_systemObjectStaves) {
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

    xml.context()->setCurTrack(0);
    staff_idx_t staffStart;
    staff_idx_t staffEnd;
    MeasureBase* measureStart;
    MeasureBase* measureEnd;

    if (selectionOnly) {
        staffStart   = _selection.staffStart();
        staffEnd     = _selection.staffEnd();
        // make sure we select full parts
        Staff* sStaff = staff(staffStart);
        Part* sPart = sStaff->part();
        Staff* eStaff = staff(staffEnd - 1);
        Part* ePart = eStaff->part();
        staffStart = staffIdx(sPart);
        staffEnd = staffIdx(ePart) + ePart->nstaves();
        measureStart = _selection.startSegment()->measure();
        if (measureStart->isMeasure() && toMeasure(measureStart)->isMMRest()) {
            measureStart = toMeasure(measureStart)->mmRestFirst();
        }
        if (_selection.endSegment()) {
            measureEnd   = _selection.endSegment()->measure()->next();
        } else {
            measureEnd   = 0;
        }
    } else {
        staffStart   = 0;
        staffEnd     = nstaves();
        measureStart = first();
        measureEnd   = 0;
    }

    // Let's decide: write midi mapping to a file or not
    masterScore()->checkMidiMapping();
    for (const Part* part : _parts) {
        if (!selectionOnly || ((staffIdx(part) >= staffStart) && (staffEnd >= staffIdx(part) + part->nstaves()))) {
            rw400::TWrite::write(part, xml, *xml.context());
        }
    }

    xml.context()->setCurTrack(0);
    xml.context()->setTrackDiff(-static_cast<int>(staffStart * VOICES));
    if (measureStart) {
        for (staff_idx_t staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            const Staff* st = staff(staffIdx);
            rw400::StaffRW::writeStaff(st, xml, measureStart, measureEnd, staffStart, staffIdx, selectionOnly);
        }
    }
    xml.context()->setCurTrack(mu::nidx);

    hook.onWriteExcerpts302(this, xml, selectionOnly);

    xml.endElement();

    if (unhide) {
        endCmd(true);
    }
}

//---------------------------------------------------------
// linkMeasures
//---------------------------------------------------------

void Score::linkMeasures(Score* score)
{
    MeasureBase* mbMaster = score->first();
    for (MeasureBase* mb = first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        while (mbMaster && !mbMaster->isMeasure()) {
            mbMaster = mbMaster->next();
        }
        if (!mbMaster) {
            LOGD("Measures in MasterScore and Score are not in sync.");
            break;
        }
        mb->linkTo(mbMaster);
        mbMaster = mbMaster->next();
    }
}

//---------------------------------------------------------
//   createThumbnail
//---------------------------------------------------------

std::shared_ptr<mu::draw::Pixmap> Score::createThumbnail()
{
    TRACEFUNC;

    LayoutMode mode = layoutMode();
    switchToPageMode();

    Page* page = pages().at(0);
    RectF fr = page->abbox();
    double mag = 256.0 / std::max(fr.width(), fr.height());
    int w = int(fr.width() * mag);
    int h = int(fr.height() * mag);

    int dpm = lrint(DPMM * 1000.0);

    auto pixmap = imageProvider()->createPixmap(w, h, dpm, configuration()->thumbnailBackgroundColor());

    double pr = MScore::pixelRatio;
    MScore::pixelRatio = 1.0;

    auto painterProvider = imageProvider()->painterForImage(pixmap);
    mu::draw::Painter p(painterProvider, "thumbnail");

    p.setAntialiasing(true);
    p.scale(mag, mag);
    print(&p, 0);
    p.endDraw();

    MScore::pixelRatio = pr;

    if (layoutMode() != mode) {
        setLayoutMode(mode);
        doLayout();
    }
    return pixmap;
}

//---------------------------------------------------------
//   loadStyle
//---------------------------------------------------------

bool Score::loadStyle(const String& fn, bool ign, const bool overlap)
{
    TRACEFUNC;

    File f(fn);
    if (f.open(IODevice::ReadOnly)) {
        MStyle st = style();
        if (st.read(&f, ign)) {
            undo(new ChangeStyle(this, st, overlap));
            return true;
        } else {
            LOGE() << "The style file is not compatible with this version of MuseScore.";
            return false;
        }
    }

    return false;
}

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

bool Score::saveStyle(const String& name)
{
    String ext(u".mss");
    FileInfo info(name);

    if (info.suffix().isEmpty()) {
        info = FileInfo(info.filePath() + ext);
    }
    File f(info.filePath());
    if (!f.open(IODevice::WriteOnly)) {
        LOGE() << "Failed open style file: " << info.filePath();
        return false;
    }

    bool ok = style().write(&f);
    if (!ok) {
        LOGE() << "Failed write style file: " << info.filePath();
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

bool Score::writeScore(io::IODevice* f, bool msczFormat, bool onlySelection, compat::WriteScoreHook& hook)
{
    WriteContext ctx;
    return writeScore(f, msczFormat, onlySelection, hook, ctx);
}

bool Score::writeScore(io::IODevice* f, bool msczFormat, bool onlySelection, compat::WriteScoreHook& hook, WriteContext& ctx)
{
    XmlWriter xml(f);
    xml.context()->setIsMsczMode(msczFormat);
    xml.setContext(&ctx);
    xml.startDocument();

    xml.startElement("museScore", { { "version", MSC_VERSION } });

    if (!MScore::testMode) {
        xml.tag("programVersion", MUSESCORE_VERSION);
        xml.tag("programRevision", MUSESCORE_REVISION);
    }
    write(xml, onlySelection, hook);

    xml.endElement();

    if (!onlySelection) {
        //update version values for i.e. plugin access
        _mscoreVersion = String::fromAscii(MUSESCORE_VERSION);
        _mscoreRevision = AsciiStringView(MUSESCORE_REVISION).toInt(nullptr, 16);
        _mscVersion = MSCVERSION;
    }
    return true;
}

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void Score::print(mu::draw::Painter* painter, int pageNo)
{
    _printing  = true;
    MScore::pdfPrinting = true;
    Page* page = pages().at(pageNo);
    RectF fr  = page->abbox();

    std::vector<EngravingItem*> ell = page->items(fr);
    std::sort(ell.begin(), ell.end(), elementLessThan);
    for (const EngravingItem* e : ell) {
        if (!e->visible()) {
            continue;
        }
        painter->save();
        painter->translate(e->pagePos());
        e->draw(painter);
        painter->restore();
    }
    MScore::pdfPrinting = false;
    _printing = false;
}

//---------------------------------------------------------
//   writeVoiceMove
//    write <move> and starting <voice> tags to denote
//    change in position.
//    Returns true if <voice> tag was written.
//---------------------------------------------------------

static bool writeVoiceMove(XmlWriter& xml, Segment* seg, const Fraction& startTick, track_idx_t track, int* lastTrackWrittenPtr)
{
    bool voiceTagWritten = false;
    int& lastTrackWritten = *lastTrackWrittenPtr;
    if ((lastTrackWritten < static_cast<int>(track)) && !xml.context()->clipboardmode()) {
        while (lastTrackWritten < (static_cast < int > (track) - 1)) {
            xml.tag("voice");
            ++lastTrackWritten;
        }
        xml.startElement("voice");
        xml.context()->setCurTick(startTick);
        xml.context()->setCurTrack(track);
        ++lastTrackWritten;
        voiceTagWritten = true;
    }

    if ((xml.context()->curTick() != seg->tick()) || (track != xml.context()->curTrack())) {
        Location curr = Location::absolute();
        Location dest = Location::absolute();
        curr.setFrac(xml.context()->curTick());
        dest.setFrac(seg->tick());
        curr.setTrack(static_cast<int>(xml.context()->curTrack()));
        dest.setTrack(static_cast<int>(track));

        dest.toRelative(curr);
        dest.write(xml);

        xml.context()->setCurTick(seg->tick());
        xml.context()->setCurTrack(track);
    }

    return voiceTagWritten;
}

//---------------------------------------------------------
//   writeSegments
//    ls  - write upto this segment (excluding)
//          can be zero
//---------------------------------------------------------

void Score::writeSegments(XmlWriter& xml, track_idx_t strack, track_idx_t etrack,
                          Segment* sseg, Segment* eseg, bool writeSystemElements, bool forceTimeSig)
{
    WriteContext& ctx = *xml.context();
    Fraction startTick = ctx.curTick();
    Fraction endTick   = eseg ? eseg->tick() : lastMeasure()->endTick();
    bool clip          = ctx.clipboardmode();

    // in clipboard mode, ls might be in an mmrest
    // since we are traversing regular measures,
    // force them out of mmRest
    if (clip) {
        Measure* lm = eseg ? eseg->measure() : 0;
        Measure* fm = sseg ? sseg->measure() : 0;
        if (lm && lm->isMMRest()) {
            lm = lm->mmRestLast();
            if (lm) {
                eseg = lm->nextMeasure() ? lm->nextMeasure()->first() : nullptr;
            } else {
                LOGD("writeSegments: no measure for end segment in mmrest");
            }
        }
        if (fm && fm->isMMRest()) {
            fm = fm->mmRestFirst();
            if (fm) {
                sseg = fm->first();
            }
        }
    }

    std::list<Spanner*> spanners;
    auto sl = spannerMap().findOverlapping(sseg->tick().ticks(), endTick.ticks());
    for (auto i : sl) {
        Spanner* s = i.value;
        if (s->generated() || !ctx.canWrite(s)) {
            continue;
        }
        // don't write voltas to clipboard
        if (clip && s->isVolta() && s->systemFlag()) {
            continue;
        }
        spanners.push_back(s);
    }

    int lastTrackWritten = static_cast<int>(strack - 1);   // for counting necessary <voice> tags
    for (track_idx_t track = strack; track < etrack; ++track) {
        if (!xml.context()->canWriteVoice(track)) {
            continue;
        }

        bool voiceTagWritten = false;

        bool timeSigWritten = false;     // for forceTimeSig
        bool crWritten = false;          // for forceTimeSig
        bool keySigWritten = false;      // for forceTimeSig

        for (Segment* segment = sseg; segment && segment != eseg; segment = segment->next1()) {
            if (!segment->enabled()) {
                continue;
            }
            if (track == 0) {
                segment->setWritten(false);
            }
            EngravingItem* e = segment->element(track);

            //
            // special case: - barline span > 1
            //               - part (excerpt) staff starts after
            //                 barline element
            bool needMove = (segment->tick() != xml.context()->curTick() || (static_cast<int>(track) > lastTrackWritten));
            if ((segment->isEndBarLineType()) && !e && writeSystemElements && ((track % VOICES) == 0)) {
                // search barline:
                for (int idx = static_cast<int>(track - VOICES); idx >= 0; idx -= static_cast<int>(VOICES)) {
                    if (segment->element(idx)) {
                        int oDiff = xml.context()->trackDiff();
                        xml.context()->setTrackDiff(idx);                      // staffIdx should be zero
                        rw400::TWrite::writeItem(segment->element(idx), xml, ctx);
                        xml.context()->setTrackDiff(oDiff);
                        break;
                    }
                }
            }
            for (EngravingItem* e1 : segment->annotations()) {
                if (e1->generated()) {
                    continue;
                }
                bool writeSystem = writeSystemElements;
                if (!writeSystem) {
                    ElementType et = e1->type();
                    if ((et == ElementType::REHEARSAL_MARK)
                        || (et == ElementType::SYSTEM_TEXT)
                        || (et == ElementType::TRIPLET_FEEL)
                        || (et == ElementType::PLAYTECH_ANNOTATION)
                        || (et == ElementType::JUMP)
                        || (et == ElementType::MARKER)
                        || (et == ElementType::TEMPO_TEXT)
                        || (et == ElementType::VOLTA)
                        || (et == ElementType::GRADUAL_TEMPO_CHANGE)) {
                        writeSystem = (e1->track() == track); // always show these on appropriate staves
                    }
                }
                if (e1->track() != track || (e1->systemFlag() && !writeSystem)) {
                    continue;
                }
                if (needMove) {
                    voiceTagWritten |= writeVoiceMove(xml, segment, startTick, track, &lastTrackWritten);
                    needMove = false;
                }
                rw400::TWrite::writeItem(e1, xml, ctx);
            }
            Measure* m = segment->measure();
            // don't write spanners for multi measure rests

            if ((!(m && m->isMMRest())) && segment->isChordRestType()) {
                for (Spanner* s : spanners) {
                    if (s->track() == track) {
                        bool end = false;
                        if (s->anchor() == Spanner::Anchor::CHORD || s->anchor() == Spanner::Anchor::NOTE) {
                            end = s->tick2() < endTick;
                        } else {
                            end = s->tick2() <= endTick;
                        }
                        if (s->tick() == segment->tick() && (!clip || end) && !s->isSlur()) {
                            if (needMove) {
                                voiceTagWritten |= writeVoiceMove(xml, segment, startTick, track, &lastTrackWritten);
                                needMove = false;
                            }
                            s->writeSpannerStart(xml, segment, track);
                        }
                    }
                    if ((s->tick2() == segment->tick())
                        && !s->isSlur()
                        && (s->effectiveTrack2() == track)
                        && (!clip || s->tick() >= sseg->tick())
                        ) {
                        if (needMove) {
                            voiceTagWritten |= writeVoiceMove(xml, segment, startTick, track, &lastTrackWritten);
                            needMove = false;
                        }
                        s->writeSpannerEnd(xml, segment, track);
                    }
                }
            }

            if (!e || !xml.context()->canWrite(e)) {
                continue;
            }
            if (e->generated()) {
                continue;
            }
            if (forceTimeSig && track2voice(track) == 0 && segment->segmentType() == SegmentType::ChordRest && !timeSigWritten
                && !crWritten) {
                // Ensure that <voice> tag is open
                voiceTagWritten |= writeVoiceMove(xml, segment, startTick, track, &lastTrackWritten);
                // we will miss a key sig!
                if (!keySigWritten) {
                    Key k = score()->staff(track2staff(track))->key(segment->tick());
                    KeySig* ks = Factory::createKeySig(this->dummy()->segment());
                    ks->setKey(k);
                    rw400::TWrite::write(ks, xml, ctx);
                    delete ks;
                    keySigWritten = true;
                }
                // we will miss a time sig!
                Fraction tsf = sigmap()->timesig(segment->tick()).timesig();
                TimeSig* ts = Factory::createTimeSig(this->dummy()->segment());
                ts->setSig(tsf);
                rw400::TWrite::write(ts, xml, ctx);
                delete ts;
                timeSigWritten = true;
            }
            if (needMove) {
                voiceTagWritten |= writeVoiceMove(xml, segment, startTick, track, &lastTrackWritten);
                // needMove = false; //! NOTE Not necessary, because needMove is currently never read again.
            }
            if (e->isChordRest()) {
                ChordRest* cr = toChordRest(e);
                cr->writeTupletStart(xml);
            }
            rw400::TWrite::writeItem(e, xml, ctx);

            if (e->isChordRest()) {
                ChordRest* cr = toChordRest(e);
                cr->writeTupletEnd(xml);
            }

            if (!(e->isRest() && toRest(e)->isGap())) {
                rw400::TWrite::write(segment, xml, ctx); // write only once
            }
            if (forceTimeSig) {
                if (segment->segmentType() == SegmentType::KeySig) {
                    keySigWritten = true;
                }
                if (segment->segmentType() == SegmentType::TimeSig) {
                    timeSigWritten = true;
                }
                if (segment->segmentType() == SegmentType::ChordRest) {
                    crWritten = true;
                }
            }
        }

        //write spanner ending after the last segment, on the last tick
        if (clip || eseg == 0) {
            for (Spanner* s : spanners) {
                if ((s->tick2() == endTick)
                    && !s->isSlur()
                    && (s->track2() == track || (s->track2() == mu::nidx && s->track() == track))
                    && (!clip || s->tick() >= sseg->tick())
                    ) {
                    s->writeSpannerEnd(xml, lastMeasure(), track, endTick);
                }
            }
        }

        if (voiceTagWritten) {
            xml.endElement();       // </voice>
        }
    }
}
}
