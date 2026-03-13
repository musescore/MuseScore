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
#include "internal/importfinale.h"
#include "internal/importfinaleparser.h"
#include "internal/importfinalelogger.h"

#include <zlib.h>
#include <exception>

#include "musx/musx.h"
#include "internal/musxxmldom.h"
#include "third_party/score_encoder.h"

#include "global/io/file.h"
#include "global/serialization/zipreader.h"

#include "types/string.h"

#include "engraving/dom/masterscore.h"
// #include "engraving/dom/mscore.h"
// #include "engraving/dom/utils.h"
#include "engraving/engravingerrors.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {
static size_t readGzipUncompressedSize(const ByteArray& gzDataInput)
{
    if (gzDataInput.size() < 18) { // Must be at least 18 bytes
        return 0;
    }

    const uint8_t* p = reinterpret_cast<const uint8_t*>(gzDataInput.constData());
    size_t n = gzDataInput.size();

    uint32_t isize = static_cast<uint32_t>(p[n - 4])
                     | (static_cast<uint32_t>(p[n - 3]) << 8)
                     | (static_cast<uint32_t>(p[n - 2]) << 16)
                     | (static_cast<uint32_t>(p[n - 1]) << 24);
    return static_cast<size_t>(isize);
}

static bool gunzipBuffer(const ByteArray& gzDataInput, ByteArray& output)
{
    constexpr size_t chunkSize = 262144; // 256 KB chunks
    output.clear();
    output.reserve(readGzipUncompressedSize(gzDataInput));

    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(gzDataInput.constData());
    stream.avail_in = static_cast<uInt>(gzDataInput.size());

    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        LOGE() << "inflateInit2 failed";
        return false;
    }

    int ret;
    do {
        size_t oldSize = output.size();
        output.resize(oldSize + chunkSize);
        stream.next_out = output.data() + oldSize;
        stream.avail_out = static_cast<uInt>(chunkSize);

        ret = inflate(&stream, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            LOGE() << "inflate failed";
            inflateEnd(&stream);
            return false;
        }
    } while (ret != Z_STREAM_END);

    output.resize(output.size() - stream.avail_out); // Trim extra
    inflateEnd(&stream);
    return true;
}

static Err importEnigmaXmlfromBuffer(Score* score, ByteArray&& data,
                                     musx::factory::DocumentFactory::CreateOptions&& createOptions,
                                     const muse::modularity::ContextPtr& iocCtx)
{
    FinaleLoggerPtr logger = std::make_shared<FinaleLogger>();
    logger->setLoggingLevel(FinaleLogger::Level::MUSX_TRACE); // for now

    try {
        auto doc = musx::factory::DocumentFactory::create<xml::Document>(data.constChar(), data.size(),
                                                                         std::move(createOptions));

        data.clear(); // free up data now that it isn't needed

        FinaleParser parser(score, doc, logger, iocCtx);
        parser.parse();

        /// @todo see which are needed
        score->connectTies();
        score->setUpTempoMap();
        score->setPlaylistDirty();
        score->setLayoutAll();
        return Err::NoError;
    } catch (const std::exception& ex) {
        logger->logError(String::fromUtf8(ex.what()));
    }
    return Err::FileBadFormat;
}

static bool extractFilesFromMusx(const String& name, ByteArray& data,
                                 musx::factory::DocumentFactory::CreateOptions& createOptions)
{
    // Extract EnigmaXml from compressed musx file \a name, return true if OK and false on error.
    ZipReader zip(name);
    data.clear();

    ByteArray gzipData = zip.fileData("score.dat");
    if (gzipData.empty()) {
        LOGE() << "no EngimaXML found: " << name;
        return false;
    }

    musx::encoder::ScoreFileEncoder::recodeBuffer(gzipData);

    if (!gunzipBuffer(gzipData, data)) {
        LOGE() << "unable to extract Enigmaxml from file: " << name;
        return false;
    }

    std::vector<char> notationMetadata;
    musx::factory::DocumentFactory::CreateOptions::EmbeddedGraphicFiles embeddedGraphics;
    for (const auto& fileInfo : zip.fileInfoList()) {
        if (!fileInfo.isFile) {
            continue;
        }

        if (fileInfo.filePath.toString() == u"NotationMetadata.xml") {
            ByteArray metadataBytes = zip.fileData(fileInfo.filePath.toStdString());
            notationMetadata.assign(metadataBytes.constChar(), metadataBytes.constChar() + metadataBytes.size());
            continue;
        }

        if (muse::io::dirpath(fileInfo.filePath).toString() == u"graphics") {
            ByteArray graphicData = zip.fileData(fileInfo.filePath.toStdString());
            musx::factory::DocumentFactory::CreateOptions::EmbeddedGraphicFile graphicFile;
            graphicFile.filename = muse::io::filename(fileInfo.filePath).toStdString();
            graphicFile.bytes.assign(reinterpret_cast<const uint8_t*>(graphicData.constData()),
                                     reinterpret_cast<const uint8_t*>(graphicData.constData()) + graphicData.size());
            embeddedGraphics.push_back(std::move(graphicFile));
        }
    }
    createOptions = musx::factory::DocumentFactory::CreateOptions(std::filesystem::path(name.toStdU16String()),
                                                                  std::move(notationMetadata),
                                                                  std::move(embeddedGraphics));

    return true;
}

Err importMusx(MasterScore* score, const String& name, const muse::modularity::ContextPtr& iocCtx)
{
    if (!io::File::exists(name)) {
        return Err::FileNotFound;
    }

    // extract the root file
    ByteArray data;
    musx::factory::DocumentFactory::CreateOptions createOptions;
    if (!extractFilesFromMusx(name, data, createOptions)) {
        return Err::FileBadFormat;      // appropriate error message has been printed by extractScoreFile
    }

    return importEnigmaXmlfromBuffer(score, std::move(data), std::move(createOptions), iocCtx);
}

Err importEnigmaXml(MasterScore* score, const String& name, const muse::modularity::ContextPtr& iocCtx)
{
    io::File xmlFile(name);
    if (!xmlFile.exists()) {
        return Err::FileNotFound;
    }

    if (!xmlFile.open(io::IODevice::ReadOnly)) {
        LOGE() << "could not open EngimaXML file: " << name;
        return Err::FileOpenError;
    }

    ByteArray data = xmlFile.readAll();
    xmlFile.close();

    return importEnigmaXmlfromBuffer(score, std::move(data), {}, iocCtx);
}
}
