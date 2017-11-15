//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "importmxmllogger.h"

namespace Ms {

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

static void log(MxmlLogger::Level level, const QString& text, const QXmlStreamReader* const xmlreader)
      {
      QString str;
      switch (level) {
            case MxmlLogger::Level::MXML_TRACE: str = "Trace"; break;
            case MxmlLogger::Level::MXML_INFO: str = "Info"; break;
            case MxmlLogger::Level::MXML_ERROR: str = "Error"; break;
            default: str = "Unknown"; break;
            }

      str += xmlLocation(xmlreader);
      str += ": ";
      str += text;

      qDebug("%s", qPrintable(str));
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
            log(Level::MXML_TRACE, trace, xmlreader);
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
            log(Level::MXML_INFO, info, xmlreader);
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
            log(Level::MXML_ERROR, error, xmlreader);
            }
      }

}
