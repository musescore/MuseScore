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

using namespace mu::iex::mei;
using namespace mu::engraving;

#define SCOREDEF_IDX -1

#define MEI_BASIC_VERSION "5.0.0-dev+basic"

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

/**
 * Read the Score from the file.
 */

bool MeiImporter::read(const QString& name)
{
    QFile fp(name);
    if (!fp.open(QIODevice::ReadOnly)) {
        LOGD("Cannot open file <%s>", qPrintable(name));
        return false;
    }
    QByteArray byteArray = fp.readAll();

    bool success = true;

    fp.close();

    return success;
}
