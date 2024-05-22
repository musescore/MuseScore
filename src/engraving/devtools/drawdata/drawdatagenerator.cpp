/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "drawdatagenerator.h"

#include "global/io/dir.h"
#include "global/io/fileinfo.h"

#include "draw/bufferedpaintprovider.h"
#include "draw/utils/drawdatarw.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "engraving/rw/mscloader.h"
#include "engraving/dom/masterscore.h"

// #ifdef MUE_BUILD_IMPORTEXPORT_MODULE
// #include "importexport/guitarpro/internal/guitarproreader.h"
// #endif

#include "log.h"

using namespace muse;
using namespace muse::draw;
using namespace mu::engraving;

//! TODO
//! 1. Make saving by relative paths
//!    scoreDir = path/scores
//!    outDir = path/json
//!    scorePath = scoreDir/v3/score.mscz
//! -> jsonPath = outDir/v3/score.json (now just outDir/score.json)

static const std::vector<std::string> FILES_FILTER = { "*.mscz", "*.mscx", "*.gp", "*.gpx", "*.gp4", "*.gp5" };

DrawDataGenerator::DrawDataGenerator(const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
}

Ret DrawDataGenerator::processDir(const muse::io::path_t& scoreDir, const muse::io::path_t& outDir, const GenOpt& opt)
{
    io::Dir::mkpath(outDir);

    //PROFILER_CLEAR;

    RetVal<io::paths_t> scores = io::Dir::scanFiles(scoreDir, FILES_FILTER);
    for (size_t i = 0; i < scores.val.size(); ++i) {
//        if (i < 1919) {
//            continue;
//        }

//        if (i > 4) {
//            break;
//        }

        LOGI() << "processFile: " << (i + 1) << "/" << scores.val.size() << " " << scores.val.at(i);

        bool skip = false;
        std::string scorePath = scores.val.at(i).toStdString();

        if (scorePath.find("disabled") != std::string::npos || scorePath.find("DISABLED") != std::string::npos) {
            LOGW() << "disabled: " << scores.val.at(i);
            skip = true;
        }

        if (skip) {
            continue;
        }

        muse::io::path_t scoreFile = scores.val.at(i);
        muse::io::path_t outFile = outDir + "/" + io::FileInfo(scoreFile).completeBaseName() + ".json";
        processFile(scoreFile, outFile, opt);
    }

    //PROFILER_PRINT;

    return muse::make_ok();
}

Ret DrawDataGenerator::processFile(const muse::io::path_t& scoreFile, const muse::io::path_t& outFile, const GenOpt& opt)
{
    DrawDataPtr drawData = genDrawData(scoreFile, opt);
    DrawDataRW::writeData(outFile, drawData);

    return muse::make_ok();
}

DrawDataPtr DrawDataGenerator::genDrawData(const muse::io::path_t& scorePath, const GenOpt& opt) const
{
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr);
    if (!loadScore(score, scorePath)) {
        LOGE() << "failed load score: " << scorePath;
        return nullptr;
    }

    applyOptions(score, opt);

    {
        TRACEFUNC_C("Layout");
        score->doLayout();
    }

    std::shared_ptr<BufferedPaintProvider> pd = std::make_shared<BufferedPaintProvider>();
    {
        TRACEFUNC_C("Paint");
        Painter painter(pd, "DrawData");
        rendering::IScoreRenderer::PaintOptions option;
        //option.fromPage = 0;
        //option.toPage = 0;
        option.isMultiPage = true;
        option.deviceDpi = DrawData::CANVAS_DPI;
        option.printPageBackground = true;
        option.isSetViewport = true;
        option.isPrinting = true;

        scoreRenderer()->paintScore(&painter, score, option);
    }

    delete score;

    DrawDataPtr drawData = pd->drawData();
    return drawData;
}

Pixmap DrawDataGenerator::genImage(const muse::io::path_t& scorePath) const
{
    LOGD() << "try: " << scorePath;
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr);
    if (!loadScore(score, scorePath)) {
        LOGE() << "failed load score: " << scorePath;
        return Pixmap();
    }

    {
        TRACEFUNC_C("Layout");
        score->doLayout();
    }

    LOGD() << "success loaded: " << scorePath;

    const SizeF pageSizeInch = scoreRenderer()->pageSizeInch(score);

    int width = std::lrint(pageSizeInch.width() * DrawData::CANVAS_DPI);
    int height = std::lrint(pageSizeInch.height() * DrawData::CANVAS_DPI);

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.setDotsPerMeterX(std::lrint((DrawData::CANVAS_DPI * 1000) / mu::engraving::INCH));
    image.setDotsPerMeterY(std::lrint((DrawData::CANVAS_DPI * 1000) / mu::engraving::INCH));

    image.fill(Qt::white);
    {
        Painter painter(&image, "DrawData");

        rendering::IScoreRenderer::PaintOptions opt;
        opt.fromPage = 0;
        opt.toPage = 0;
        opt.deviceDpi = DrawData::CANVAS_DPI;
        opt.printPageBackground = false;
        opt.isSetViewport = true;
        opt.isMultiPage = false;
        opt.isPrinting = true;

        scoreRenderer()->paintScore(&painter, score, opt);
    }

    return Pixmap::fromQImage(image);
}

bool DrawDataGenerator::loadScore(mu::engraving::MasterScore* score, const muse::io::path_t& path) const
{
    TRACEFUNC;
    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));

    std::string suffix = io::suffix(path);
    if (isMuseScoreFile(suffix)) {
        // Load

        TRACEFUNC_C("Load mscz");

        MscReader::Params params;
        params.filePath = path;
        params.mode = mscIoModeBySuffix(suffix);

        MscReader reader(params);
        if (!reader.open()) {
            return false;
        }

        MscLoader scoreReader;
        SettingsCompat settingsCompat;
        Ret ret = scoreReader.loadMscz(score, reader, settingsCompat, true);
        if (!ret) {
            LOGE() << "failed read file: " << path;
            return false;
        }
    } else {
        // Import

//         TRACEFUNC_C("Load gp");
// #ifdef MUE_BUILD_IMPORTEXPORT_MODULE
//         mu::iex::guitarpro::GuitarProReader reader;
//         Ret ret = reader.read(score, path);
//         if (!ret) {
//             LOGE() << "failed read file: " << path;
//             return false;
//         }
// #else
        NOT_SUPPORTED;
        return false;
//#endif
    }

    return true;
}

void DrawDataGenerator::applyOptions(engraving::MasterScore* score, const GenOpt& opt) const
{
    if (!opt.pageSize.isNull()) {
        PageSizeSetAccessor pageSize = score->pageSize();
        pageSize.setHeight(opt.pageSize.height() / engraving::INCH);
        pageSize.setWidth(opt.pageSize.width() / engraving::INCH);
        pageSize.setPrintableWidth(pageSize.width() - pageSize.evenLeftMargin() - pageSize.oddLeftMargin());
    }
}
