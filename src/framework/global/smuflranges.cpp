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
#include "smuflranges.h"

#include "io/file.h"
#include "serialization/json.h"

#include "log.h"

using namespace mu;
using namespace mu::io;

//---------------------------------------------------------
//   smuflRanges
//    read smufl ranges.json file
//---------------------------------------------------------

const std::map<String, StringList>& mu::smuflRanges()
{
    static std::map<String, StringList> ranges;
    StringList allSymbols;

    if (ranges.empty()) {
        File fi(":fonts/smufl/ranges.json");
        if (!fi.open(IODevice::ReadOnly)) {
            LOGE() << "failed open: " << fi.filePath();
        }
        std::string error;
        JsonObject o = JsonDocument::fromJson(fi.readAll(), &error).rootObject();
        if (!error.empty()) {
            LOGE() << "failed parse, err: " << error << ", file: " << fi.filePath();
        }

        for (auto s : o.keys()) {
            JsonObject range = o.value(s).toObject();
            String desc      = range.value("description").toString();
            JsonArray glyphs = range.value("glyphs").toArray();
            if (glyphs.size() > 0) {
                StringList glyphNames;
                for (size_t i = 0; i < glyphs.size(); ++i) {
                    glyphNames.append(glyphs.at(i).toString());
                }
                ranges.insert({ desc, glyphNames });
                allSymbols << glyphNames;
            }
        }
        ranges.insert({ String::fromAscii(SMUFL_ALL_SYMBOLS), allSymbols });     // TODO: make translatable as well as ranges.json
    }
    return ranges;
}
