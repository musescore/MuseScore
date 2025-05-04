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
#include "internal/importenigmaxml.h"
#include "internal/importfinalescoremap.h"
#include "internal/importfinalelogger.h"

#include <zlib.h>
#include <exception>

#include <QFile>
#include <QFileInfo>

#include "musx/musx.h"

#include "global/io/file.h"
#include "global/serialization/zipreader.h"

#include "types/string.h"

#include "engraving/dom/box.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordlist.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/utils.h"
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

Err importEnigmaXmlfromBuffer(Score* score, ByteArray&& data)
{
    auto logger = std::make_shared<FinaleLogger>();
    logger->setLoggingLevel(FinaleLogger::Level::MUSX_TRACE); // for now

    try {
        auto doc = musx::factory::DocumentFactory::create<musx::xml::qt::Document>(data.constChar(), data.size());

        data.clear(); // free up data now that it isn't needed

        EnigmaXmlImporter importer(score, doc, logger);
        importer.import();

        score->setUpTempoMap(); //??
        return engraving::Err::NoError;
    } catch (const std::exception& ex) {
        logger->logError(String::fromUtf8(ex.what()));
    }
    return engraving::Err::FileBadFormat;
}

static bool extractScoreFile(const String& name, ByteArray& data)
{
    // Extract EnigmaXml from compressed musx file \a name, return true if OK and false on error.
    ZipReader zip(name);
    data.clear();

    ByteArray gzipData = zip.fileData("score.dat");
    if (gzipData.empty()) {
        LOGE() << "no EngimaXML found: " << name;
        return false;
    }

    constexpr static uint32_t INITIAL_STATE = 0x28006D45; // arbitrary initial value for algorithm
    constexpr static uint32_t RESET_LIMIT = 0x20000; // reset value corresponding (probably) to an internal Finale buffer size

    uint32_t state = INITIAL_STATE;
    for (size_t i = 0; i < gzipData.size(); i++) {
        if (i % RESET_LIMIT == 0) {
            state = INITIAL_STATE;
        }
        // this algorithm is BSD rand()!
        state = state * 0x41c64e6d + 0x3039;
        uint16_t upper = state >> 16;
        uint8_t c = uint8_t(upper + upper / 255);
        gzipData[i] ^= c;
    }

    if (!gunzipBuffer(gzipData, data)) {
        LOGE() << "unable to extract Enigmaxml from file: " << name;
        return false;
    }

    return true;
}

Err importMusx(MasterScore* score, const QString& name)
{

    if (!io::File::exists(name)) {
        return Err::FileNotFound;
    }

    // extract the root file
    ByteArray data;
    if (!extractScoreFile(name, data)) {
        return Err::FileBadFormat;      // appropriate error message has been printed by extractScoreFile
    }

    return importEnigmaXmlfromBuffer(score, std::move(data));
}

Err importEnigmaXml(MasterScore* score, const QString& name)
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

    return importEnigmaXmlfromBuffer(score, std::move(data));
}

}
