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
#include "musicxmlreader.h"

#include "io/path.h"
#include "engraving/engravingerrors.h"

namespace mu::engraving {
extern Err importMusicXML(MasterScore*, const muse::String&, bool forceMode);
extern Err importCompressedMusicXML(MasterScore*, const muse::String&, bool forceMode);
}

using namespace mu::iex::musicxml;
using namespace mu::engraving;

muse::Ret MusicXMLReader::read(MasterScore* score, const muse::io::path_t& path, const Options& options)
{
    Err err = Err::FileUnknownType;
    std::string suffix = muse::io::suffix(path);
    bool forceMode = false;
    if (options.find(INotationReader::OptionKey::ForceMode) != options.end()) {
        forceMode = options.at(INotationReader::OptionKey::ForceMode).toBool();
    }

    if (suffix == "xml" || suffix == "musicxml") {
        err = importMusicXML(score, path.toString(), forceMode);
    } else if (suffix == "mxl") {
        err = importCompressedMusicXML(score, path.toString(), forceMode);
    }
    return make_ret(err, path);
}
