//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "xmlreader.h"

//---------------------------------------------------------
//   XmlReader
//---------------------------------------------------------

XmlReader::XmlReader(QFile* d)
   : QXmlStreamReader(d)
      {
      docName = d->fileName();
      }

//---------------------------------------------------------
//   intAttribute
//---------------------------------------------------------

int XmlReader::intAttribute(const char* s, int _default, int base) const
      {
      bool ok;
      if (attributes().hasAttribute(s))
            return attributes().value(s).toString().toInt(&ok, base);
      else
            return _default;
      }

//---------------------------------------------------------
//   doubleAttribute
//---------------------------------------------------------

double XmlReader::doubleAttribute(const char* s) const
      {
      return attributes().value(s).toString().toDouble();
      }

double XmlReader::doubleAttribute(const char* s, double _default) const
      {
      if (attributes().hasAttribute(s))
            return attributes().value(s).toUtf8().toDouble();
      else
            return _default;
      }

//---------------------------------------------------------
//   attribute
//---------------------------------------------------------

QString XmlReader::attribute(const char* s, const QString& _default) const
      {
      if (attributes().hasAttribute(s))
            return attributes().value(s).toString();
      else
            return _default;
      }

//---------------------------------------------------------
//   hasAttribute
//---------------------------------------------------------

bool XmlReader::hasAttribute(const char* s) const
      {
      return attributes().hasAttribute(s);
      }

//---------------------------------------------------------
//   readPoint
//---------------------------------------------------------

QPointF XmlReader::readPoint()
      {
      Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
      QPointF p;
      p.setX(doubleAttribute("x", 0.0));
      p.setY(doubleAttribute("y", 0.0));
      readNext();
      return p;
      }

//---------------------------------------------------------
//   readSize
//---------------------------------------------------------

QSizeF XmlReader::readSize()
      {
      Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
      QSizeF p;
      p.setWidth(doubleAttribute("w", 0.0));
      p.setHeight(doubleAttribute("h", 0.0));
      skipCurrentElement();
      return p;
      }

//---------------------------------------------------------
//   readRect
//---------------------------------------------------------

QRectF XmlReader::readRect()
      {
      Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
      QRectF p;
      p.setX(doubleAttribute("x", 0.0));
      p.setY(doubleAttribute("y", 0.0));
      p.setWidth(doubleAttribute("w", 0.0));
      p.setHeight(doubleAttribute("h", 0.0));
      skipCurrentElement();
      return p;
      }

//---------------------------------------------------------
//   unknown
//    unknown tag read
//---------------------------------------------------------

void XmlReader::unknown() const
      {
      if (QXmlStreamReader::error())
            qDebug("StreamReaderError: %s", qPrintable(errorString()));
      qDebug("%s: xml read error at line %lld col %lld: %s",
         qPrintable(docName), lineNumber(), columnNumber(),
         name().toUtf8().data());
      }

//---------------------------------------------------------
//   error
//---------------------------------------------------------

void XmlReader::error(const QString& s) const
      {
      if (QXmlStreamReader::error())
            qDebug("StreamReaderError: %s", qPrintable(errorString()));
      qDebug("%s: %s at line %lld col %lld: %s",
         qPrintable(docName), qPrintable(s), lineNumber(), columnNumber(),
         name().toUtf8().data());
      }

