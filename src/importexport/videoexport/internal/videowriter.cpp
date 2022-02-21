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
#include "videowriter.h"

#include "videoencoder.h"

#include "engraving/libmscore/page.h"
#include "engraving/libmscore/system.h"
#include "engraving/paint/paint.h"

#include "log.h"

static const int DEFAULT_BITRATE = 8000000;

using namespace mu::iex::videoexport;
using namespace mu::project;

std::vector<INotationWriter::UnitType> VideoWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool VideoWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

mu::Ret VideoWriter::write(notation::INotationPtr notation, io::Device& device, const Options& options)
{
    NOT_IMPLEMENTED;
    return Ret(Ret::Code::NotImplemented);
}

mu::Ret VideoWriter::writeList(const notation::INotationPtrList& notations, io::Device& device, const Options& options)
{
    UNUSED(notations);
    UNUSED(device);
    UNUSED(options);
    NOT_IMPLEMENTED;
    return Ret(Ret::Code::NotImplemented);
}

void VideoWriter::abort()
{
    NOT_IMPLEMENTED;
}

mu::framework::ProgressChannel VideoWriter::progress() const
{
    static mu::framework::ProgressChannel progressCh;
    return progressCh;
}

void VideoWriter::generatePagedOriginalVideo(notation::INotationPtr notation, io::Device& device, const Config& config)
{
    Ms::MasterScore* score = notation->elements()->msScore()->masterScore();

    // The image on which we draw the frames
    QImage frame(config.width, config.height, QImage::Format_RGB32);       // Only RGB32 is supported

    // A painter to help us draw
    QPainter qp(&frame);
    qp.setBrush(Qt::red);
    qp.setPen(Qt::white);
    qp.setRenderHint(QPainter::Antialiasing, true);
    qp.setRenderHint(QPainter::TextAntialiasing, true);

    draw::Painter painter(&qp, "video_writer");

    RectF frameRect = RectF::fromQRectF(QRectF(frame.rect()));

    score->setLayoutMode(engraving::LayoutMode::PAGE);
    //replace all pagebreak by system break
    for (Ms::MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->pageBreak()) {
            mb->setPageBreak(false);
            mb->setLineBreak(true);
        }
    }

    qreal scale = config.width / (score->styleD(Ms::Sid::pageWidth) * Ms::DPI);
//    QTransform matrix = QTransform(scale, 0.0, 0.0, scale, 0.0, 0.0);
//    setupScale(frame, scale);

    // make everything invisible
    score->setShowFrames(false);
    score->setShowInstrumentNames(false);
    score->setShowInvisible(false);
    score->setShowPageborders(false);
    score->setShowUnprintable(false);
    score->setShowVBox(false);

    // change pageformat to fit the video size
//    Piano piano;
//    const int pianoHeight = pianoAreaHeight(piano);

//    score->setStyleValue(Ms::Sid::pageHeight, ((_height - (config.showPiano ? pianoHeight : 0)) / Ms::DPI) / scale);
    score->setStyleValue(Ms::Sid::pageHeight, ((config.height / Ms::DPI) / scale));
    score->setStyleValue(Ms::Sid::pageEvenTopMargin, 0.0 / Ms::DPI);
    score->setStyleValue(Ms::Sid::pageEvenBottomMargin, 0.0 / Ms::DPI);
    score->setStyleValue(Ms::Sid::pageOddTopMargin, 0.0 / Ms::DPI);
    score->setStyleValue(Ms::Sid::pageOddBottomMargin, 0.0 / Ms::DPI);
    score->setStyleValue(Ms::Sid::pageTwosided, false);
    score->setStyleValue(Ms::Sid::showHeader, false);
    score->setStyleValue(Ms::Sid::showFooter, false);

    score->setStyleValue(Ms::Sid::minSystemDistance, engraving::Spatium(10));
    score->setStyleValue(Ms::Sid::maxSystemDistance, engraving::Spatium(10));
    score->setStyleValue(Ms::Sid::staffLowerBorder, engraving::Spatium(5));
    score->setStyleValue(Ms::Sid::staffUpperBorder, engraving::Spatium(7));

    score->setLayoutAll();
    score->update();
    score->setExpandRepeats(true);

    // only show the piano if one part
//    if (config.showPiano && score->parts().size() > 1) {
//        qDebug() << "ignore showPiano option, score has more than 1 part";
//    }
//    bool showPiano = config.showPiano && (score->parts().size() == 1);

//    EventMap events;
//    score->renderMidi(&events, synthState);
    qreal duration = 0;//score->utick2utime(score->repeatList().ticks());
//    bool useSyncData = config.syncData.size() > 0;
//    if (useSyncData) { // use last available sync time
//        duration = config.syncData.rbegin()->first;
//    }

    // Create the encoder
    VideoEncoder encoder;
    io::path outFilename = io::path(device.property("path").toString());
    if (!encoder.open(outFilename, config.width, config.height, DEFAULT_BITRATE, config.fps / 2, config.fps)) {
        LOGE() << "failed open encoder";
    }

    // print each page in QImage
    // output image in video
    int size = 0;
    int maxframe = (duration + config.leadingSeconds + config.trailingSeconds) * config.fps;
    int cacheCurrentPage = -1;
    QList<const Ms::EngravingItem*> ell;
    for (int i = 0; i < maxframe; i++) {
        qreal currentTime = (qreal)i / config.fps;
        currentTime -= config.leadingSeconds;
        if (currentTime <= 0) {
            currentTime = 0;
        }
        if (currentTime > duration) {
            currentTime = duration;
        }
        int utick;
//        if (useSyncData) {
//            qreal syncTime = getSyncTimeFromTime(config.syncData, currentTime);
//            utick = score->utime2utick(syncTime);
//        } else {
        utick = score->utime2utick(currentTime);
//        }
        int tick = 0;//score->repeatList().utick2tick(utick);
        if (tick > 0) {
            tick--;
        }
        if (utick > 0) {
            utick--;
        }

        // find active notes, mark them in score, keep them for piano
//        QList<const Note*> markedNotes;
//        markActiveNotes(events, utick, &markedNotes);

        int currentPage = 0;
        for (const Ms::Page* page : score->pages()) {
            QList<Ms::System*> systems = page->systems();
            if (systems.isEmpty()) {
                continue;
            }
            Ms::System* ss = systems[0];
            Ms::System* es = systems[systems.size() - 1];
            Ms::Measure* sm = ss->firstMeasure();
            Ms::Measure* em = es->lastMeasure();
            if (sm && em && tick >= sm->tick().ticks() && tick <= em->endTick().ticks()) {
                break;
            }
            currentPage++;
        }
        if (currentPage >= score->pages().size()) {
            currentPage = score->pages().size() - 1;
        }

        // Clear the frame
        painter.fillRect(frameRect, draw::Color::white);

        //Draw page
//        painter.drawTiledPixmap(QRect(0, 0, _width, _height - (config.showPiano ? pianoHeight : 0)), QPixmap(":/images/paper.png"),
//                                QPoint(0, 0));

        // collect elements on current page
        if (cacheCurrentPage != currentPage) {
            cacheCurrentPage = currentPage;

            Ms::Page* page = score->pages()[currentPage];

            // Draw page elements
            painter.setClipping(true);
            painter.setClipRect(frameRect);
            QList<Ms::EngravingItem*> elements = page->items(frameRect);
            engraving::Paint::paintElements(painter, elements);
            painter.setClipping(false);
        }

//        QColor c(Qt::blue);
//        c.setAlpha(50);
//        painter.fillRect(getCursorRectangle(tick, score), c);
//        painter.restore();

        // show the piano
//        if (showPiano) {
//            QMultiMap<int, int> pitches;
//            for (const Note* note : markedNotes) {
//                pitches.insert(note->ppitch(), note->staffIdx());
//            }
//            piano.pressKeys(pitches);
//            int offset = _height - pianoHeight;
//            // TODO deal with PIANO TOP
//            //if (config.pianoOrientation == PianoOrientation::PIANO_TOP)
//            //      offset = 0;
//            painter.translate(0, offset);
//            piano.render(&painter, QRectF(), piano.rect(), Qt::KeepAspectRatio);
//            painter.translate(0, -offset);
//        }

        size = encoder.encodeImage(frame);                          // Fixed frame rate
        //printf("Encoded: %d\n",size);
    }

    encoder.close();
}
