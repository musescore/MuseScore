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

#include <QXmlStreamReader>

#include "log.h"

namespace mu::engraving {
//---------------------------------------------------------
//   xmlLocation
//---------------------------------------------------------

static QString xmlLocation(const QXmlStreamReader* const xmlreader)
{
    QString loc;
    if (xmlreader) {
        loc = QString(" at line %1 col %2").arg(xmlreader->lineNumber()).arg(xmlreader->columnNumber());
    }
    return loc;
}

//---------------------------------------------------------
//   logDebugTrace
//---------------------------------------------------------
static void to_xml_log(MxmlLogger::Level level, const QString& text, const QXmlStreamReader* const xmlreader)
{
    QString str;
    switch (level) {
    case MxmlLogger::Level::MXML_TRACE: str = "Trace";
        break;
    case MxmlLogger::Level::MXML_INFO: str = "Info";
        break;
    case MxmlLogger::Level::MXML_ERROR: str = "Error";
        break;
    default: str = "Unknown";
        break;
    }

    str += xmlLocation(xmlreader);
    str += ": ";
    str += text;

    LOGD("%s", qPrintable(str));
}

//---------------------------------------------------------
//   logDebugTrace
//---------------------------------------------------------

/**
 Log debug (function) trace.
 */

void MxmlLogger::logDebugTrace(const QString& trace, const QXmlStreamReader* const xmlreader)
{
    if (_level <= Level::MXML_TRACE) {
        to_xml_log(Level::MXML_TRACE, trace, xmlreader);
    }
}

//---------------------------------------------------------
//   logDebugInfo
//---------------------------------------------------------

/**
 Log debug \a info (non-fatal events relevant for debugging).
 */

void MxmlLogger::logDebugInfo(const QString& info, const QXmlStreamReader* const xmlreader)
{
    if (_level <= Level::MXML_INFO) {
        to_xml_log(Level::MXML_INFO, info, xmlreader);
    }
}

//---------------------------------------------------------
//   logError
//---------------------------------------------------------

/**
 Log \a error (possibly non-fatal but to be reported to the user anyway).
 */

void MxmlLogger::logError(const QString& error, const QXmlStreamReader* const xmlreader)
{
    if (_level <= Level::MXML_ERROR) {
        to_xml_log(Level::MXML_ERROR, error, xmlreader);
    }
}
}
