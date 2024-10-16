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

#include "notation/view/playbackcursor.h"

#include "log.h"

#include <QPainter>

using namespace mu::iex::videoexport;
using namespace mu::project;
using namespace mu::notation;
using namespace muse::draw;
using namespace muse::midi;

std::vector<IProjectWriter::UnitType> VideoWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool VideoWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

muse::Ret VideoWriter::write(INotationProjectPtr, QIODevice&, const Options&)
{
    NOT_SUPPORTED;
    return make_ret(muse::Ret::Code::NotSupported);
}

muse::Ret VideoWriter::write(INotationProjectPtr project, const muse::io::path_t& filePath, const Options&)
{
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

    muse::Ret ret = generatePagedOriginalVideo(project, filePath, cfg);
    return ret;
}

muse::Ret VideoWriter::generatePagedOriginalVideo(INotationProjectPtr project, const muse::io::path_t& filePath, const Config& config)
{
    // --score-video -o ./simple5.mp4 ./simple5.mscz

    VideoEncoder encoder;
    if (!encoder.open(filePath, config.width, config.height, config.bitrate, config.fps / 2, config.fps)) {
        LOGE() << "failed open encoder";
        return make_ret(muse::Ret::Code::UnknownError);
    }

    IMasterNotationPtr masterNotation = project->masterNotation();

    engraving::MasterScore* score = masterNotation->notation()->elements()->msScore()->masterScore();

    // Setup Score view
    masterNotation->notation()->setViewMode(notation::ViewMode::PAGE);
    score->setShowFrames(false);
    score->setShowInstrumentNames(false);
    score->setShowInvisible(false);
    score->setShowPageborders(false);
    score->setShowUnprintable(false);
    score->setShowVBox(false);

    score->doLayout();

    PageList pages = masterNotation->notation()->elements()->pages();
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

    auto painting = masterNotation->notation()->painting();

    // Setup duration
    INotationPlaybackPtr playback = masterNotation->playback();
    float totalPlayTimeSec = playback->totalPlayTime();

    LOGI() << "totalPlayTime: " << totalPlayTimeSec << " sec";

    int frameCount = (totalPlayTimeSec + config.leadingSec + config.trailingSec) * config.fps;

    //! NOTE: After setting the score above, the number of pages may change - get them again
    pages = masterNotation->notation()->elements()->pages();

    auto pageByTick = [](const PageList& pages, tick_t tick) -> const Page* {
        for (const Page* p : pages) {
            if (tick <= static_cast<tick_t>(p->endTick().ticks())) {
                return p;
            }
        }
        return nullptr;
    };

    const Color CURSOR_COLOR = Color(0, 0, 255, 50);

    PlaybackCursor cursor(application()->iocContext());
    cursor.setNotation(masterNotation->notation());

    for (int f = 0; f < frameCount; f++) {
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
        opt.fromPage = page->no();
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

    return muse::make_ok();
}
