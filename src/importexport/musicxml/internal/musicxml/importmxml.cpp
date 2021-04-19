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

#include "libmscore/box.h"
#include "libmscore/measure.h"
#include "libmscore/page.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/sym.h"
#include "libmscore/symbol.h"

#include "importmxml.h"
#include "importmxmllogger.h"
#include "importmxmlpass1.h"
#include "importmxmlpass2.h"

namespace Ms {
Score::FileError importMusicXMLfromBuffer(Score* score, const QString& /*name*/, QIODevice* dev)
{
    //qDebug("importMusicXMLfromBuffer(score %p, name '%s', dev %p)",
    //       score, qPrintable(name), dev);

    MxmlLogger logger;
    logger.setLoggingLevel(MxmlLogger::Level::MXML_ERROR);   // errors only
    //logger.setLoggingLevel(MxmlLogger::Level::MXML_INFO);
    //logger.setLoggingLevel(MxmlLogger::Level::MXML_TRACE); // also include tracing

    // pass 1
    dev->seek(0);
    MusicXMLParserPass1 pass1(score, &logger);
    Score::FileError res = pass1.parse(dev);
    if (res != Score::FileError::FILE_NO_ERROR) {
        return res;
    }

    // pass 2
    dev->seek(0);
    MusicXMLParserPass2 pass2(score, pass1, &logger);
    return pass2.parse(dev);
}
} // namespace Ms
