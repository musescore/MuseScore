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
#include <QDir>
#include <QBuffer>
#include <QRegularExpression>

#include "style/defaultstyle.h"

#include "compat/writescorehook.h"
#include "rw/xml.h"
#include "rw/writecontext.h"
#include "rw/staffrw.h"

#include "score.h"
#include "engravingitem.h"
#include "measure.h"
#include "segment.h"
#include "slur.h"
#include "chordrest.h"
#include "chord.h"
#include "tuplet.h"
#include "beam.h"
#include "revisions.h"
#include "page.h"
#include "part.h"
#include "staff.h"
#include "system.h"
#include "keysig.h"
#include "clef.h"
#include "text.h"
#include "ottava.h"
#include "volta.h"
#include "excerpt.h"
#include "mscore.h"
#include "stafftype.h"
#include "scoreorder.h"
#include "utils.h"
#include "sig.h"
#include "undo.h"
#include "imageStore.h"
#include "audio.h"
#include "barline.h"
#include "masterscore.h"
#include "factory.h"

#include "log.h"
#include "config.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rw;

namespace Ms {
//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Score::write(XmlWriter& xml, bool selectionOnly, compat::WriteScoreHook& hook)
{
    // if we have multi measure rests and some parts are hidden,
    // then some layout information is missing:
    // relayout with all parts set visible

    QList<Part*> hiddenParts;
    bool unhide = false;
    if (styleB(Sid::createMultiMeasureRests)) {
        for (Part* part : qAsConst(_parts)) {
            if (!part->show()) {
                if (!unhide) {
                    startCmd();
                    unhide = true;
                }
                part->undoChangeProperty(Pid::VISIBLE, true);
                hiddenParts.append(part);
            }
        }
    }
    if (unhide) {
        doLayout();
        for (Part* p : hiddenParts) {
            p->setShow(false);
        }
    }

    xml.startObject(this);

    if (excerpt()) {
        Excerpt* e = excerpt();
        QMultiMap<int, int> trackList = e->tracksMapping();
        QMapIterator<int, int> i(trackList);
        if (!(trackList.size() == e->nstaves() * VOICES) && !trackList.isEmpty()) {
            while (i.hasNext()) {
                i.next();
                xml.tagE(QString("Tracklist sTrack=\"%1\" dstTrack=\"%2\"").arg(i.key()).arg(i.value()));
            }
        }
    }

    if (lineMode()) {
        xml.tag("layoutMode", "line");
    }
    if (systemMode()) {
        xml.tag("layoutMode", "system");
    }

    if (_audio && xml.isMsczMode()) {
        xml.tag("playMode", int(_playMode));
        _audio->write(xml);
    }

    for (int i = 0; i < 32; ++i) {
        if (!_layerTags[i].isEmpty()) {
            xml.tag(QString("LayerTag id=\"%1\" tag=\"%2\"").arg(i).arg(_layerTags[i]),
                    _layerTagComments[i]);
        }
    }
    int n = _layer.size();
    for (int i = 1; i < n; ++i) {         // don’t save default variant
        const Layer& l = _layer[i];
        xml.tagE(QString("Layer name=\"%1\" mask=\"%2\"").arg(l.name).arg(l.tags));
    }
    xml.tag("currentLayer", _currentLayer);

    if (isMaster() && !MScore::testMode) {
        _synthesizerState.write(xml);
    }

    if (pageNumberOffset()) {
        xml.tag("page-offset", pageNumberOffset());
    }
    xml.tag("Division", Constant::division);
    xml.setCurTrack(-1);

    hook.onWriteStyle302(this, xml);

    xml.tag("showInvisible",   _showInvisible);
    xml.tag("showUnprintable", _showUnprintable);
    xml.tag("showFrames",      _showFrames);
    xml.tag("showMargins",     _showPageborders);
    xml.tag("markIrregularMeasures", _markIrregularMeasures, true);

    if (!_isOpen) {
        xml.tag("open", _isOpen);
    }

    QMapIterator<QString, QString> i(_metaTags);
    while (i.hasNext()) {
        i.next();
        // do not output "platform" and "creationDate" in test and save template mode
        if ((!MScore::testMode && !MScore::saveTemplateMode) || (i.key() != "platform" && i.key() != "creationDate")) {
            xml.tag(QString("metaTag name=\"%1\"").arg(i.key().toHtmlEscaped()), i.value());
        }
    }

    if (_scoreOrder.isValid()) {
        ScoreOrder order = _scoreOrder;
        order.updateInstruments(this);
        order.write(xml);
    }

    if (!systemObjectStaves.isEmpty()) {
        // write which staves currently have system objects above them
        xml.startObject("SystemObjects");
        for (Staff* s : systemObjectStaves) {
            // TODO: when we add more granularity to system object display, construct this string per staff
            QString sysObjForStaff = "barNumbers=\"false\"";
            // for now, everything except bar numbers is shown on system object staves
            // (also, the code to display bar numbers on system staves other than the first currently does not exist!)
            xml.tagE(QString("Instance staffId=\"%1\" %2").arg(s->idx() + 1).arg(sysObjForStaff));
        }
        xml.endObject();
    }

    xml.setCurTrack(0);
    int staffStart;
    int staffEnd;
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
    for (const Part* part : qAsConst(_parts)) {
        if (!selectionOnly || ((staffIdx(part) >= staffStart) && (staffEnd >= staffIdx(part) + part->nstaves()))) {
            part->write(xml);
        }
    }

    xml.setCurTrack(0);
    xml.setTrackDiff(-staffStart * VOICES);
    if (measureStart) {
        for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            const Staff* st = staff(staffIdx);
            StaffRW::writeStaff(st, xml, measureStart, measureEnd, staffStart, staffIdx, selectionOnly);
        }
    }
    xml.setCurTrack(-1);

    hook.onWriteExcerpts302(this, xml, selectionOnly);

    xml.endObject();

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
            qDebug("Measures in MasterScore and Score are not in sync.");
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
    LayoutMode mode = layoutMode();
    setLayoutMode(LayoutMode::PAGE);
    doLayout();

    Page* page = pages().at(0);
    RectF fr = page->abbox();
    qreal mag = 256.0 / qMax(fr.width(), fr.height());
    int w = int(fr.width() * mag);
    int h = int(fr.height() * mag);

    int dpm = lrint(DPMM * 1000.0);

    auto pixmap = imageProvider()->createPixmap(w, h, dpm, mu::draw::Color::white);

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

bool Score::loadStyle(const QString& fn, bool ign, const bool overlap)
{
    QFile f(fn);
    if (f.open(QIODevice::ReadOnly)) {
        MStyle st = style();
        if (st.read(&f, ign)) {
            undo(new ChangeStyle(this, st, overlap));
            return true;
        } else {
            MScore::lastError = QObject::tr("The style file is not compatible with this version of MuseScore.");
            return false;
        }
    }
    MScore::lastError = strerror(errno);
    return false;
}

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

bool Score::saveStyle(const QString& name)
{
    QString ext(".mss");
    QFileInfo info(name);

    if (info.suffix().isEmpty()) {
        info.setFile(info.filePath() + ext);
    }
    QFile f(info.filePath());
    if (!f.open(QIODevice::WriteOnly)) {
        MScore::lastError = tr("Open Style File %1 failed: %2").arg(info.filePath(), f.errorString());
        return false;
    }

    bool ok = style().write(&f);
    if (!ok) {
        MScore::lastError = QObject::tr("Write Style failed: %1").arg(f.errorString());
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

//! FIXME
//extern QString revision;
static QString revision;

bool Score::writeScore(QIODevice* f, bool msczFormat, bool onlySelection, mu::engraving::compat::WriteScoreHook& hook)
{
    WriteContext ctx;
    return writeScore(f, msczFormat, onlySelection, hook, ctx);
}

bool Score::writeScore(QIODevice* f, bool msczFormat, bool onlySelection, compat::WriteScoreHook& hook, WriteContext& ctx)
{
    XmlWriter xml(this, f);
    xml.setIsMsczMode(msczFormat);
    xml.setContext(&ctx);
    xml.writeHeader();

    xml.startObject("museScore version=\"" MSC_VERSION "\"");

    if (!MScore::testMode) {
        xml.tag("programVersion", VERSION);
        xml.tag("programRevision", revision);
    }
    write(xml, onlySelection, hook);

    xml.endObject();

    if (isMaster()) {
        masterScore()->revisions()->write(xml);
    }
    if (!onlySelection) {
        //update version values for i.e. plugin access
        _mscoreVersion = VERSION;
        _mscoreRevision = revision.toInt(0, 16);
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

    QList<EngravingItem*> ell = page->items(fr);
    std::stable_sort(ell.begin(), ell.end(), elementLessThan);
    for (const EngravingItem* e : qAsConst(ell)) {
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

static bool writeVoiceMove(XmlWriter& xml, Segment* seg, const Fraction& startTick, int track, int* lastTrackWrittenPtr)
{
    bool voiceTagWritten = false;
    int& lastTrackWritten = *lastTrackWrittenPtr;
    if ((lastTrackWritten < track) && !xml.clipboardmode()) {
        while (lastTrackWritten < (track - 1)) {
            xml.tagE("voice");
            ++lastTrackWritten;
        }
        xml.startObject("voice");
        xml.setCurTick(startTick);
        xml.setCurTrack(track);
        ++lastTrackWritten;
        voiceTagWritten = true;
    }

    if ((xml.curTick() != seg->tick()) || (track != xml.curTrack())) {
        Location curr = Location::absolute();
        Location dest = Location::absolute();
        curr.setFrac(xml.curTick());
        dest.setFrac(seg->tick());
        curr.setTrack(xml.curTrack());
        dest.setTrack(track);

        dest.toRelative(curr);
        dest.write(xml);

        xml.setCurTick(seg->tick());
        xml.setCurTrack(track);
    }

    return voiceTagWritten;
}

//---------------------------------------------------------
//   writeSegments
//    ls  - write upto this segment (excluding)
//          can be zero
//---------------------------------------------------------

void Score::writeSegments(XmlWriter& xml, int strack, int etrack,
                          Segment* sseg, Segment* eseg, bool writeSystemElements, bool forceTimeSig)
{
    Fraction startTick = xml.curTick();
    Fraction endTick   = eseg ? eseg->tick() : lastMeasure()->endTick();
    bool clip          = xml.clipboardmode();

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
                qDebug("writeSegments: no measure for end segment in mmrest");
            }
        }
        if (fm && fm->isMMRest()) {
            fm = fm->mmRestFirst();
            if (fm) {
                sseg = fm->first();
            }
        }
    }

    QList<Spanner*> spanners;
    auto sl = spannerMap().findOverlapping(sseg->tick().ticks(), endTick.ticks());
    for (auto i : sl) {
        Spanner* s = i.value;
        if (s->generated() || !xml.canWrite(s)) {
            continue;
        }
        // don't write voltas to clipboard
        if (clip && s->isVolta() && s->systemFlag()) {
            continue;
        }
        spanners.push_back(s);
    }

    int lastTrackWritten = strack - 1;   // for counting necessary <voice> tags
    for (int track = strack; track < etrack; ++track) {
        if (!xml.canWriteVoice(track)) {
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
            bool needMove = (segment->tick() != xml.curTick() || (track > lastTrackWritten));
            if ((segment->isEndBarLineType()) && !e && writeSystemElements && ((track % VOICES) == 0)) {
                // search barline:
                for (int idx = track - VOICES; idx >= 0; idx -= VOICES) {
                    if (segment->element(idx)) {
                        int oDiff = xml.trackDiff();
                        xml.setTrackDiff(idx);                      // staffIdx should be zero
                        segment->element(idx)->write(xml);
                        xml.setTrackDiff(oDiff);
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
                        || (et == ElementType::PLAYTECH_ANNOTATION)
                        || (et == ElementType::JUMP)
                        || (et == ElementType::MARKER)
                        || (et == ElementType::TEMPO_TEXT)
                        || (et == ElementType::VOLTA)
                        || (et == ElementType::TEXTLINE)) {
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
                e1->write(xml);
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

            if (!e || !xml.canWrite(e)) {
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
                    ks->write(xml);
                    delete ks;
                    keySigWritten = true;
                }
                // we will miss a time sig!
                Fraction tsf = sigmap()->timesig(segment->tick()).timesig();
                TimeSig* ts = Factory::createTimeSig(this->dummy()->segment());
                ts->setSig(tsf);
                ts->write(xml);
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
            e->write(xml);

            if (e->isChordRest()) {
                ChordRest* cr = toChordRest(e);
                cr->writeTupletEnd(xml);
            }

            if (!(e->isRest() && toRest(e)->isGap())) {
                segment->write(xml);            // write only once
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
                    && (s->track2() == track || (s->track2() == -1 && s->track() == track))
                    && (!clip || s->tick() >= sseg->tick())
                    ) {
                    s->writeSpannerEnd(xml, lastMeasure(), track, endTick);
                }
            }
        }

        if (voiceTagWritten) {
            xml.endObject();       // </voice>
        }
    }
}
}
