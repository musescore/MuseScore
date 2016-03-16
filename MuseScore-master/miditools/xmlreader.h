//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2004-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __XML_H__
#define __XML_H__

#include <QFile>
#include <QByteArray>
#include <QString>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QXmlStreamReader>

//---------------------------------------------------------
//   XmlReader
//---------------------------------------------------------

class XmlReader : public QXmlStreamReader {
      QString docName;  // used for error reporting

   public:
      XmlReader(QFile*);
      XmlReader(const QByteArray& d) : QXmlStreamReader(d) {}
      XmlReader(QIODevice* d)        : QXmlStreamReader(d) {}
      XmlReader(const QString& d)    : QXmlStreamReader(d) {}

      void unknown() const;
      void error(const QString& s) const;

      void error(int, int);

      // attribute helper routines:
      QString attribute(const char* s) const { return attributes().value(s).toString(); }
      QString attribute(const char* s, const QString&) const;
      int intAttribute(const char* s, int _default = 0, int base = 10) const;
      double doubleAttribute(const char* s) const;
      double doubleAttribute(const char* s, double _default) const;
      bool hasAttribute(const char* s) const;

      int readInt()         { return readElementText().toInt();    }
      int readInt(bool* ok) { return readElementText().toInt(ok);  }
      double readDouble()   { return readElementText().toDouble(); }
      QPointF readPoint();
      QSizeF readSize();
      QRectF readRect();

      void setDocName(const QString& s) { docName = s; }
      };

#endif

