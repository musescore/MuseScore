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

#include "meiimporter.h"

#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/bracket.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/dynamic.h"
#include "libmscore/factory.h"
#include "libmscore/key.h"
#include "libmscore/keysig.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/lyrics.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/part.h"
#include "libmscore/rest.h"
#include "libmscore/segment.h"
#include "libmscore/sig.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/text.h"
#include "libmscore/timesig.h"
#include "libmscore/timesig.h"
#include "libmscore/tuplet.h"

#include "io/file.h"

#include "thirdparty/libmei/cmn.h"
#include "thirdparty/libmei/shared.h"

#include "thirdparty/pugixml.hpp"

using namespace mu;
using namespace mu::iex::mei;
using namespace mu::engraving;

#define SCOREDEF_IDX -1

#define MEI_BASIC_VERSION "5.0.0-dev+basic"

/**
 * Read the Score from the file.
 * Return false on error.
 */

bool MeiImporter::read(const io::path_t& path)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(
        path.toStdString().c_str(), (pugi::parse_comments | pugi::parse_default) & ~pugi::parse_eol);

    if (!result) {
        LOGD("Cannot open file <%s>", qPrintable(path.toString()));
        return false;
    }

    pugi::xml_node root = doc.first_child();

    pugi::xml_attribute meiVersion = root.attribute("meiversion");
    if (!meiVersion || String(meiVersion.value()) != String(MEI_BASIC_VERSION)) {
        // Temporary commented
        // Convert::logs.push_back(String("The MEI file does not seem to be a MEI basic version '%1' file").arg(String(MEI_BASIC_VERSION)));
    }

    bool success = true;

    // TODO: actual import

    return success;
}
