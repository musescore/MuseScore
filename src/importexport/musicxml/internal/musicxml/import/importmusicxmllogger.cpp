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

#include "importmusicxmllogger.h"

#include "global/serialization/xmlstreamreader.h"

#include "log.h"

using namespace muse;

namespace mu::iex::musicxml {
//---------------------------------------------------------
//   xmlLocation
//---------------------------------------------------------

static String xmlLocation(const muse::XmlStreamReader* xmlreader)
{
    String loc;
    if (xmlreader) {
        loc = String(u" at line %1 col %2")
              .arg(size_t(xmlreader->lineNumber()))
              .arg(size_t(xmlreader->columnNumber()));
    }
    return loc;
}

//---------------------------------------------------------
//   logDebugTrace
//---------------------------------------------------------
static void to_xml_log(MusicXmlLogger::Level level, const String& text, const XmlStreamReader* xmlreader)
{
    String str;
    switch (level) {
    case MusicXmlLogger::Level::MXML_TRACE: str = u"Trace";
        break;
    case MusicXmlLogger::Level::MXML_INFO: str = u"Info";
        break;
    case MusicXmlLogger::Level::MXML_ERROR: str = u"Error";
        break;
    default: str = u"Unknown";
        break;
    }

    str += xmlLocation(xmlreader);
    str += u": ";
    str += text;

    LOGD() << str;
}

//---------------------------------------------------------
//   logDebugTrace
//---------------------------------------------------------

/**
 Log debug (function) trace.
 */

void MusicXmlLogger::logDebugTrace(const String& trace, const muse::XmlStreamReader* xmlreader)
{
    if (m_level <= Level::MXML_TRACE) {
        to_xml_log(Level::MXML_TRACE, trace, xmlreader);
    }
}

//---------------------------------------------------------
//   logDebugInfo
//---------------------------------------------------------

/**
 Log debug \a info (non-fatal events relevant for debugging).
 */

void MusicXmlLogger::logDebugInfo(const String& info, const XmlStreamReader* xmlreader)
{
    if (m_level <= Level::MXML_INFO) {
        to_xml_log(Level::MXML_INFO, info, xmlreader);
    }
}

//---------------------------------------------------------
//   logError
//---------------------------------------------------------

/**
 Log \a error (possibly non-fatal but to be reported to the user anyway).
 */

void MusicXmlLogger::logError(const String& error, const XmlStreamReader* xmlreader)
{
    if (m_level <= Level::MXML_ERROR) {
        to_xml_log(Level::MXML_ERROR, error, xmlreader);
    }
}
}
