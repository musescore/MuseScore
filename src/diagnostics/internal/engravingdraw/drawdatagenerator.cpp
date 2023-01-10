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
#include "draw/utils/drawjson.h"

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

void DrawDataGenerator::processFile(const io::path_t& path, const io::path_t& outDir)
{
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle();
    if (!loadScore(score, path)) {
        LOGE() << "failed load score: " << path;
        return;
    }

    {
        TRACEFUNC_C("Layout");
        score->doLayout();
    }

    static const int CANVAS_DPI = 300;

    // draw image for test
    if (0) {
        const SizeF pageSizeInch = Paint::pageSizeInch(score);

        int width = std::lrint(pageSizeInch.width() * CANVAS_DPI);
        int height = std::lrint(pageSizeInch.height() * CANVAS_DPI);

        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
        image.setDotsPerMeterX(std::lrint((CANVAS_DPI * 1000) / mu::engraving::INCH));
        image.setDotsPerMeterY(std::lrint((CANVAS_DPI * 1000) / mu::engraving::INCH));

        image.fill(Qt::white);

        Painter painter(&image, "DrawDataGenerator");

        Paint::Options opt;
        opt.fromPage = 0;
        opt.toPage = 0;
        opt.deviceDpi = CANVAS_DPI;
        opt.printPageBackground = false;
        opt.isSetViewport = true;
        opt.isMultiPage = false;
        opt.isPrinting = true;

        Paint::paintScore(&painter, score, opt);

        QString filePath = outDir.toQString() + "/" + io::FileInfo(path).baseName() + ".png";
        image.save(filePath, "png");

        return;
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

    const DrawData& drawData = pd->drawData();

    {
        ByteArray json = DrawDataJson::toJson(drawData);
        io::path_t filePath = outDir + "/" + io::FileInfo(path).baseName() + ".json";
        io::File::writeFile(filePath, json);
    }

    delete score;
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

bool DrawDataGenerator::loadScore(mu::engraving::MasterScore* score, const mu::io::path_t& path)
{
    TRACEFUNC;
    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));

    std::string suffix = io::suffix(path);
    if (isMuseScoreFile(suffix)) {
        // Load

        TRACEFUNC_C("Load mscz");

        MscReader::Params params;
        params.filePath = path;
        params.mode = MscIoMode::Zip;

        MscReader reader(params);
        if (!reader.open()) {
            return false;
        }

        ScoreReader scoreReader;
        Err err = scoreReader.loadMscz(score, reader, true);
        if (err != Err::NoError) {
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
