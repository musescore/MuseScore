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

#ifndef __IMPORTMXMLLOGGER_H__
#define __IMPORTMXMLLOGGER_H__

class QXmlStreamReader;

namespace Ms {

class MxmlLogger {
public:
      enum class Level : char {
            MXML_TRACE, MXML_INFO, MXML_ERROR
            };
      MxmlLogger() {}
      void logDebugTrace(const QString& trace, const QXmlStreamReader* const xmlreader = 0);
      void logDebugInfo(const QString& info, const QXmlStreamReader* const xmlreader = 0);
      void logError(const QString& error, const QXmlStreamReader* const xmlreader = 0);
      void setLoggingLevel(const Level level) { _level = level; }
private:
      Level _level = Level::MXML_INFO;
      };

} // namespace Ms

#endif
