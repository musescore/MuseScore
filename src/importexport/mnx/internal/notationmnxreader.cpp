/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
#include "notationmnxreader.h"
#include "import/mnximporter.h"

#include "engraving/dom/masterscore.h"
#include "engraving/engravingerrors.h"
#include "io/file.h"

#include "mnxdom.h"

using namespace mu::iex::mnxio;
using namespace mu::engraving;
using namespace muse;

static Ret importJson(MasterScore* score, ByteArray&& jsonData, const io::path_t& path, const NotationMnxReader::Options&)
{
    try {
        auto doc = mnx::Document::create(jsonData.constData(), jsonData.size());
        jsonData.clear();
        if (!mnx::validation::schemaValidate(doc)) {
            LOGE() << path << " is not a valid MNX document.";
            return make_ret(Ret::Code::NotSupported, TranslatableString("importexport/mnx", "File is not a valid MNX document.").str);
        }
        if (doc.global().measures().empty()) {
            LOGE() << path << " contains no measures.";
            return make_ret(Ret::Code::NotSupported, TranslatableString("importexport/mnx", "File contains no measures.").str);
        }
        MnxImporter importer(score, std::move(doc));
        importer.importMnx();
    } catch (const std::exception& ex) {
        LOGE() << String::fromStdString(ex.what());
        return make_ret(Ret::Code::InternalError);
    }

    return make_ok();
}

static Ret importMnx(MasterScore* score, ByteArray&& mnxData, const io::path_t& path, const NotationMnxReader::Options& options)
{
    /// @todo Eventually this will require unzipping the mnx archive and fishing out the json.
    /// Until the mnx archive format is specced out, we simply treat MNX as raw JSON.
    return importJson(score, std::move(mnxData), path, options);
}

Ret NotationMnxReader::read(MasterScore* score, const io::path_t& path, const Options& options)
{
    io::File mnxFile(path);
    if (!mnxFile.exists()) {
        return make_ret(Err::FileNotFound, path);
    }

    if (!mnxFile.open(io::IODevice::ReadOnly)) {
        LOGE() << "could not open MNX file: " << path.toString();
        return make_ret(Err::FileOpenError, path);
    }

    ByteArray data = mnxFile.readAll();
    mnxFile.close();

    std::string suffix = muse::io::suffix(path);
    if (suffix == "json") {
        return importJson(score, std::move(data), path, options);
    } else if (suffix == "mnx") {
        return importMnx(score, std::move(data), path, options);
    }

    return make_ret(Err::FileUnknownType, suffix);
}
