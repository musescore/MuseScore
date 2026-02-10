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
#include "videowriter.h"

#include "videoencoder.h"

#include "engraving/dom/page.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/masterscore.h"

#include "notation/imasternotation.h"
#include "notation/notationtypes.h"

#include "notationscene/qml/MuseScore/NotationScene/playbackcursor.h"

#include "defer.h"
#include "log.h"

#include <QPainter>

using namespace mu::iex::videoexport;
using namespace mu::project;
using namespace mu::notation;
using namespace muse::draw;
using namespace muse::midi;

std::vector<INotationWriter::UnitType> VideoWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool VideoWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

muse::Ret VideoWriter::write(INotationPtr notation, muse::io::IODevice& device, const Options&)
{
    std::string filePath = device.meta("file_path");
    IF_ASSERT_FAILED(!filePath.empty()) {
        return make_ret(muse::Ret::Code::InternalError);
    }

    Config cfg;

    cfg.fps = configuration()->fps();

    std::string resolution = configuration()->resolution();
    if (resolution == "2160p") {
        cfg.width = 3840;
        cfg.height = 2160;
    } else if (resolution == "1440p") {
        cfg.width = 2560;
        cfg.height = 1440;
    } else if (resolution == "1080p") {
        cfg.width = 1920;
        cfg.height = 1080;
    } else if (resolution == "720p") {
        cfg.width = 1280;
        cfg.height = 720;
    } else if (resolution == "480p") {
        cfg.width = 854;
        cfg.height = 480;
    } else if (resolution == "360p") {
        cfg.width = 640;
        cfg.height = 360;
    } else {
        cfg.width = 1920;
        cfg.height = 1080;
    }

    // compute bitrate according to Google recommended settings
    // https://support.google.com/youtube/answer/1722171?hl=en
    float br = 8;
    if (cfg.height == 2160) {
        br = cfg.fps < 35 ? 40 : 60;
    } else if (cfg.height == 1440) {
        br = cfg.fps < 35 ? 16 : 24;
    } else if (cfg.height == 1080) {
        br = cfg.fps < 35 ? 10 : 15;
    } else if (cfg.height == 720) {
        br = cfg.fps < 35 ? 5 : 7.5;
    } else if (cfg.height == 480) {
        br = cfg.fps < 35 ? 2.5 : 4;
    } else if (cfg.height == 360) {
        br = cfg.fps < 35 ? 1 : 1.5;
    }
    cfg.bitrate = int(br * 1000000);

    cfg.leadingSec = configuration()->leadingSec();
    cfg.trailingSec = configuration()->trailingSec();

    return generatePagedOriginalVideo(notation, muse::io::path_t(filePath), cfg);
}

muse::Ret VideoWriter::writeList(const INotationPtrList&, muse::io::IODevice&, const Options&)
{
    NOT_SUPPORTED;
    return make_ret(muse::Ret::Code::NotSupported);
}

muse::Progress* VideoWriter::progress()
{
    return &m_progress;
}

void VideoWriter::abort()
{
    m_abort = true;
}

muse::Ret VideoWriter::generatePagedOriginalVideo(INotationPtr notation, const muse::io::path_t& filePath, const Config& config)
{
    VideoEncoder encoder;
    if (!encoder.open(filePath, config.width, config.height, config.bitrate, config.fps / 2, config.fps)) {
        LOGE() << "failed open encoder";
        return make_ret(muse::Ret::Code::UnknownError);
    }

    engraving::MasterScore* score = notation->elements()->msScore()->masterScore();

    notation::ViewMode originalViewMode = notation->viewMode();
    engraving::MStyle originalStyle = score->style();

    DEFER {
        score->style() = originalStyle;
        score->setLayoutAll();
        score->update();
        notation->setViewMode(originalViewMode);
    };

    notation->setViewMode(notation::ViewMode::PAGE);
    score->setShowFrames(false);
    score->setShowInstrumentNames(false);
    score->setShowInvisible(false);
    score->setShowPageborders(false);
    score->setShowUnprintable(false);
    score->setShowVBox(false);

    score->doLayout();

    PageList pages = notation->elements()->pages();
    if (pages.empty()) {
        LOGE() << "No pages";
        return make_ret(muse::Ret::Code::UnknownError);
    }

    double CANVAS_DPI = 300;

    const Page* page = pages.front();
    if (score->staves().size() > 3) {
        //! NOTE: Calculate the dpi to display all page elements
        muse::RectF ttbox = page->tbbox();
        double margin = 100.0;
        double ttboxHeight = ttbox.height() + margin * 2;
        double scale = config.height / ttboxHeight;
        CANVAS_DPI = scale * engraving::DPI;
    }

    score->style().set(engraving::Sid::pageHeight, config.height / CANVAS_DPI);
    score->style().set(engraving::Sid::pageWidth, config.width / CANVAS_DPI);
    score->style().set(engraving::Sid::pagePrintableWidth, score->style().styleD(engraving::Sid::pageWidth)
                       - score->style().styleD(engraving::Sid::pageOddLeftMargin)
                       - score->style().styleD(engraving::Sid::pageEvenLeftMargin));

    score->style().set(engraving::Sid::pageEvenTopMargin, 0.0);
    score->style().set(engraving::Sid::pageEvenBottomMargin, 0.0);
    score->style().set(engraving::Sid::pageOddTopMargin, 0.0);
    score->style().set(engraving::Sid::pageOddBottomMargin, 0.0);
    score->style().set(engraving::Sid::pageTwosided, false);
    score->style().set(engraving::Sid::showHeader, false);
    score->style().set(engraving::Sid::showFooter, false);

    score->style().set(engraving::Sid::minSystemDistance, engraving::Spatium(10));
    score->style().set(engraving::Sid::maxSystemDistance, engraving::Spatium(10));
    score->style().set(engraving::Sid::staffLowerBorder, engraving::Spatium(5));
    score->style().set(engraving::Sid::staffUpperBorder, engraving::Spatium(7));

    score->setLayoutAll();
    score->update();

    // Setup painting
    QImage frame(config.width, config.height, QImage::Format_RGB32);
    frame.setDotsPerMeterX(std::lrint((CANVAS_DPI * 1000) / engraving::INCH));
    frame.setDotsPerMeterY(std::lrint((CANVAS_DPI * 1000) / engraving::INCH));

    QPainter qp(&frame);
    qp.setRenderHint(QPainter::Antialiasing, true);
    qp.setRenderHint(QPainter::TextAntialiasing, true);
    muse::RectF frameRect = muse::RectF::fromQRectF(QRectF(frame.rect()));

    Painter painter(&qp, "video_writer");

    auto painting = notation->painting();

    // Setup duration
    INotationPlaybackPtr playback = notation->masterNotation()->playback();
    float totalPlayTimeSec = playback->totalPlayTime();

    int frameCount = (totalPlayTimeSec + config.leadingSec + config.trailingSec) * config.fps;
    LOGI() << "totalPlayTime: " << totalPlayTimeSec << " sec" << " frame count " << frameCount;

    //! NOTE: After setting the score above, the number of pages may change - get them again
    pages = notation->elements()->pages();

    auto pageByTick = [](const PageList& pages, tick_t tick) -> const Page* {
        for (const Page* p : pages) {
            if (tick <= static_cast<tick_t>(p->endTick().ticks())) {
                return p;
            }
        }
        return nullptr;
    };

    const Color CURSOR_COLOR = Color(0, 0, 255, 50);

    PlaybackCursor cursor(iocContext());
    cursor.setNotation(notation);

    m_abort = false;
    m_progress.start();

    for (int f = 0; f < frameCount; f++) {
        if (m_abort) {
            encoder.close();
            m_progress.finish(make_ret(muse::Ret::Code::Cancel));
            return make_ret(muse::Ret::Code::Cancel);
        }

        m_progress.progress(f, frameCount);

        float currentTimeSec = (qreal)f / config.fps;
        currentTimeSec -= config.leadingSec;
        if (currentTimeSec <= 0) {
            currentTimeSec = 0;
        }
        if (currentTimeSec > totalPlayTimeSec) {
            currentTimeSec = totalPlayTimeSec;
        }

        tick_t tick = playback->secToTick(currentTimeSec);

        const Page* page = pageByTick(pages, tick);
        if (!page) {
            break;
        }

        INotationPainting::Options opt;
        opt.fromPage = page->pageNumber();
        opt.toPage = opt.fromPage;
        opt.deviceDpi = CANVAS_DPI;

        painter.fillRect(frameRect, Color::WHITE);

        painting->paintPrint(&painter, opt);

        cursor.move(tick);

        muse::RectF cursorRect = cursor.rect();
        muse::PointF pagePos = page->pos();
        muse::RectF cursorAbsRect = cursorRect.translated(-pagePos);

        painter.fillRect(cursorAbsRect, CURSOR_COLOR);

        encoder.encodeImage(frame);
    }

    encoder.close();

    m_progress.finish(muse::make_ok());

    return muse::make_ok();
}
