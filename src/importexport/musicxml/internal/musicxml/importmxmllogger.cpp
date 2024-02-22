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

#include "importmxmllogger.h"

#include "global/serialization/xmlstreamreader.h"

#include "log.h"

namespace mu::engraving {
//---------------------------------------------------------
//   xmlLocation
//---------------------------------------------------------

static String xmlLocation(const XmlStreamReader* xmlreader)
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
static void to_xml_log(MxmlLogger::Level level, const String& text, const XmlStreamReader* xmlreader)
{
    String str;
    switch (level) {
    case MxmlLogger::Level::MXML_TRACE: str = u"Trace";
        break;
    case MxmlLogger::Level::MXML_INFO: str = u"Info";
        break;
    case MxmlLogger::Level::MXML_ERROR: str = u"Error";
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

void MxmlLogger::logDebugTrace(const String& trace, const XmlStreamReader* xmlreader)
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

void MxmlLogger::logDebugInfo(const String& info, const XmlStreamReader* xmlreader)
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

void MxmlLogger::logError(const String& error, const XmlStreamReader* xmlreader)
{
    if (m_level <= Level::MXML_ERROR) {
        to_xml_log(Level::MXML_ERROR, error, xmlreader);
    }
}
}
