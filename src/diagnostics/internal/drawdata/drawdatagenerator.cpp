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
#include "drawdatagenerator.h"

#include "global/io/file.h"
#include "global/io/dir.h"
#include "global/io/fileinfo.h"
#include "global/stringutils.h"

#include "draw/bufferedpaintprovider.h"
#include "draw/utils/drawdatarw.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "engraving/infrastructure/paint.h"
#include "engraving/rw/scorereader.h"
#include "engraving/libmscore/masterscore.h"
#include "engraving/libmscore/page.h"

#include "importexport/guitarpro/internal/guitarproreader.h"

#include "log.h"

using namespace mu;
using namespace mu::diagnostics;
using namespace mu::draw;
using namespace mu::engraving;
using namespace mu::iex::guitarpro;

static const int CANVAS_DPI = 300;
static const std::vector<std::string> FILES_FILTER = { "*.mscz", "*.gp", "*.gpx", "*.gp4", "*.gp5" };

Ret DrawDataGenerator::processDir(const io::path_t& scoreDir, const io::path_t& outDir, const io::path_t& ignoreFile)
{
    std::vector<std::string> ignore = loadIgnore(ignoreFile);

    PROFILER_CLEAR;

    RetVal<io::paths_t> scores = io::Dir::scanFiles(scoreDir, FILES_FILTER);
    for (size_t i = 0; i < scores.val.size(); ++i) {
//        if (i < 1919) {
//            continue;
//        }

//        if (i > 1005) {
//            break;
//        }

        LOGI() << "processFile: " << (i + 1) << "/" << scores.val.size() << " " << scores.val.at(i);

        bool skip = false;
        std::string scorePath = scores.val.at(i).toStdString();
        for (const std::string& path : ignore) {
            if (scorePath == "." || scorePath == "..") {
                skip = true;
                break;
            }
            if (scorePath.find(path) != std::string::npos) {
                LOGW() << "ignore: " << scores.val.at(i);
                skip = true;
                break;
            }
        }

        if (skip) {
            continue;
        }

        processFile(scores.val.at(i), outDir);
    }

    PROFILER_PRINT;

    return make_ok();
}

Ret DrawDataGenerator::processFile(const io::path_t& path, const io::path_t& outDir)
{
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle();
    if (!loadScore(score, path)) {
        LOGE() << "failed load score: " << path;
        return make_ret(Ret::Code::UnknownError);
    }

    {
        TRACEFUNC_C("Layout");
        score->doLayout();
    }

    std::shared_ptr<BufferedPaintProvider> pd = std::make_shared<BufferedPaintProvider>();
    {
        TRACEFUNC_C("Paint");
        Painter painter(pd, "DrawData");
        Paint::Options opt;
        opt.isSetViewport = true;
        opt.isMultiPage = false;
        opt.isPrinting = true;

        Paint::paintScore(&painter, score, opt);
    }

    DrawDataPtr drawData = genDrawData(path);
    io::path_t filePath = outDir + "/" + io::FileInfo(path).baseName() + ".json";
    DrawDataRW::writeData(filePath, drawData);

    delete score;

    return make_ok();
}

DrawDataPtr DrawDataGenerator::genDrawData(const io::path_t& scorePath) const
{
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle();
    if (!loadScore(score, scorePath)) {
        LOGE() << "failed load score: " << scorePath;
        return nullptr;
    }

    {
        TRACEFUNC_C("Layout");
        score->doLayout();
    }

    std::shared_ptr<BufferedPaintProvider> pd = std::make_shared<BufferedPaintProvider>();
    {
        TRACEFUNC_C("Paint");
        Painter painter(pd, "DrawData");
        Paint::Options opt;
        opt.fromPage = 0;
        opt.toPage = 0;
        opt.deviceDpi = CANVAS_DPI;
        opt.printPageBackground = true;
        opt.isSetViewport = true;
        opt.isMultiPage = false;
        opt.isPrinting = true;

        Paint::paintScore(&painter, score, opt);
    }

    DrawDataPtr drawData = pd->drawData();
    return drawData;
}

Pixmap DrawDataGenerator::genImage(const io::path_t& scorePath) const
{
    LOGD() << "try: " << scorePath;
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle();
    if (!loadScore(score, scorePath)) {
        LOGE() << "failed load score: " << scorePath;
        return Pixmap();
    }

    {
        TRACEFUNC_C("Layout");
        score->doLayout();
    }

    LOGD() << "success loaded: " << scorePath;

    const SizeF pageSizeInch = Paint::pageSizeInch(score);

    int width = std::lrint(pageSizeInch.width() * CANVAS_DPI);
    int height = std::lrint(pageSizeInch.height() * CANVAS_DPI);

    QPixmap qpx(width, height);
    {
        Painter painter(&qpx, "DrawData");

        painter.fillRect(RectF(0, 0, width, height), Color::WHITE);

        Paint::Options opt;
        opt.fromPage = 0;
        opt.toPage = 0;
        opt.deviceDpi = CANVAS_DPI;
        opt.printPageBackground = false;
        opt.isSetViewport = true;
        opt.isMultiPage = false;
        opt.isPrinting = true;

        Paint::paintScore(&painter, score, opt);
    }

    return Pixmap::fromQPixmap(qpx);
}

std::vector<std::string> DrawDataGenerator::loadIgnore(const io::path_t& ignoreFile) const
{
    io::File file(ignoreFile);
    if (!file.exists()) {
        return std::vector<std::string>();
    }

    if (!file.open(io::IODevice::ReadOnly)) {
        LOGE() << "failed open file: " << ignoreFile;
        return std::vector<std::string>();
    }

    ByteArray data = file.readAll();
    std::string str = data.constChar();
    std::vector<std::string> scores;
    mu::strings::split(str, scores, "\n");

    std::vector<std::string> ignore;
    for (std::string& score : scores) {
        if (score.empty()) {
            continue;
        }

        if (score.size() < 3) {
            continue;
        }

        mu::strings::trim(score);

        if (score.at(0) == '/' && score.at(1) == '/') {
            continue;
        }

        ignore.push_back(score);
    }

    return ignore;
}

bool DrawDataGenerator::loadScore(mu::engraving::MasterScore* score, const mu::io::path_t& path) const
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

        ScoreReader scoreReader;
        Ret ret = scoreReader.loadMscz(score, reader, true);
        if (!ret) {
            LOGE() << "failed read file: " << path;
            return false;
        }
    } else {
        // Import

        TRACEFUNC_C("Load gp");

        GuitarProReader reader;
        Ret ret = reader.read(score, path);
        if (!ret) {
            LOGE() << "failed read file: " << path;
            return false;
        }
    }

    return true;
}
