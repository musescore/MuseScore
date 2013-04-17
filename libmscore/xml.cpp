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

#include "xml.h"
#include "layoutbreak.h"
#include "spanner.h"
#include "beam.h"
#include "tuplet.h"

QString docName;

//---------------------------------------------------------
//   XmlReader
//---------------------------------------------------------

XmlReader::XmlReader(QFile* d)
   : QXmlStreamReader(d)
      {
      docName = d->fileName();
      _tick  = 0;
      _track = 0;
      }

XmlReader::XmlReader(const QByteArray& d)
   : QXmlStreamReader(d)
      {
      _tick  = 0;
      _track = 0;
      }

XmlReader::XmlReader(QIODevice* d)
   : QXmlStreamReader(d)
      {
      _tick  = 0;
      _track = 0;
      }

XmlReader::XmlReader(const QString& d)
   : QXmlStreamReader(d)
      {
      _tick  = 0;
      _track = 0;
      }

//---------------------------------------------------------
//   intAttribute
//---------------------------------------------------------

int XmlReader::intAttribute(const char* s, int _default) const
      {
      if (attributes().hasAttribute(s))
            return attributes().value(s).toString().toInt();
      else
            return _default;
      }

int XmlReader::intAttribute(const char* s) const
      {
      return attributes().value(s).toString().toInt();
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
#ifndef NDEBUG
      if (!attributes().hasAttribute("x")) {
            QXmlStreamAttributes map = attributes();
            qDebug("XmlReader::readPoint: x attribute missing: %s (%d)",
               name().toUtf8().data(), map.size());
            for (int i = 0; i < map.size(); ++i) {
                  const QXmlStreamAttribute& a = map.at(i);
                  qDebug(" attr <%s> <%s>", a.name().toUtf8().data(), a.value().toUtf8().data());
                  }
            unknown();
            }
      if (!attributes().hasAttribute("y")) {
            qDebug("XmlReader::readPoint: y attribute missing: %s", name().toUtf8().data());
            unknown();
            }
#endif
      p.setX(doubleAttribute("x", 0.0));
      p.setY(doubleAttribute("y", 0.0));
      readNext();
      return p;
      }

//---------------------------------------------------------
//   readColor
//---------------------------------------------------------

QColor XmlReader::readColor()
      {
      Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
      QColor c;
      c.setRed(intAttribute("r"));
      c.setGreen(intAttribute("g"));
      c.setBlue(intAttribute("b"));
      c.setAlpha(intAttribute("a", 255));
      skipCurrentElement();
      return c;
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
//   readFraction
//---------------------------------------------------------

Fraction XmlReader::readFraction()
      {
      Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
      qreal z = attribute("z", "0.0").toDouble();
      qreal n = attribute("n", "0.0").toDouble();
      skipCurrentElement();
      return Fraction(z, n);
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
      abort();
      }

//---------------------------------------------------------
//   findSpanner
//---------------------------------------------------------

Spanner* XmlReader::findSpanner(int id) const
      {
      int n = _spanner.size();
      for (int i = 0; i < n; ++i) {
            if (_spanner.at(i)->id() == id)
                  return _spanner.at(i);
            }
      return 0;
      }

//---------------------------------------------------------
//   findBeam
//---------------------------------------------------------

Beam* XmlReader::findBeam(int id) const
      {
      int n = _beams.size();
      for (int i = 0; i < n; ++i) {
            if (_beams.at(i)->id() == id)
                  return _beams.at(i);
            }
      return 0;
      }

//---------------------------------------------------------
//   findTuplet
//---------------------------------------------------------

Tuplet* XmlReader::findTuplet(int id) const
      {
      int n = _tuplets.size();
      for (int i = 0; i < n; ++i) {
            if (_tuplets.at(i)->id() == id)
                  return _tuplets.at(i);
            }
      return 0;
      }

//---------------------------------------------------------
//   addTuplet
//---------------------------------------------------------

void XmlReader::addTuplet(Tuplet* s)
      {
      Tuplet* t = findTuplet(s->id());
      if (t) {
            qDebug("Tuplet %d already read", s->id());
            delete s;
            return;
            }
      _tuplets.append(s);
      }

//---------------------------------------------------------
//   compareProperty
//---------------------------------------------------------

template <class T>
bool compareProperty(void* val, void* defaultVal)
      {
      return (defaultVal == 0) || (*(T*)val != *(T*)defaultVal);
      }

//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

Xml::Xml()
      {
      stack.clear();
      curTick       = 0;
      curTrack      = -1;
      tickDiff      = 0;
      trackDiff     = 0;
      clipboardmode = false;
      excerptmode   = false;
      tupletId      = 1;
      beamId        = 1;
      spannerId     = 1;
      writeOmr      = true;
      }

Xml::Xml(QIODevice* device)
   : QTextStream(device)
      {
      setCodec("UTF-8");
      stack.clear();
      curTick       = 0;
      curTrack      = -1;
      tickDiff      = 0;
      trackDiff     = 0;
      clipboardmode = false;
      excerptmode   = false;
      tupletId      = 1;
      beamId        = 1;
      spannerId     = 1;
      writeOmr      = true;
      }

//---------------------------------------------------------
//   pTag
//---------------------------------------------------------

void Xml::pTag(const char* name, PlaceText place)
      {
      const char* tags[] = {
            "auto", "above", "below", "left"
            };
      tag(name, tags[int(place)]);
      }

//---------------------------------------------------------
//   readPlacement
//---------------------------------------------------------

PlaceText readPlacement(XmlReader& e)
      {
      const QString& s(e.readElementText());
      if (s == "auto" || s == "0")
            return PLACE_AUTO;
      if (s == "above" || s == "1")
            return PLACE_ABOVE;
      if (s == "below" || s == "2")
            return PLACE_BELOW;
      if (s == "left" || s == "3")
            return PLACE_LEFT;
      qDebug("unknown placement value <%s>\n", qPrintable(s));
      return PLACE_AUTO;
      }

//---------------------------------------------------------
//   fTag
//---------------------------------------------------------

void Xml::fTag(const char* name, const Fraction& f)
      {
      tagE(QString("%1 z=\"%2\" n=\"%3\"").arg(name).arg(f.numerator()).arg(f.denominator()));
      }

//---------------------------------------------------------
//   putLevel
//---------------------------------------------------------

void Xml::putLevel()
      {
      int level = stack.size();
      for (int i = 0; i < level * 2; ++i)
            *this << ' ';
      }

//---------------------------------------------------------
//   header
//---------------------------------------------------------

void Xml::header()
      {
      *this << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      }

//---------------------------------------------------------
//   stag
//    <mops attribute="value">
//---------------------------------------------------------

void Xml::stag(const QString& s)
      {
      putLevel();
      *this << '<' << s << '>' << endl;
      stack.append(s.split(' ')[0]);
      }

//---------------------------------------------------------
//   etag
//    </mops>
//---------------------------------------------------------

void Xml::etag()
      {
      putLevel();
      *this << "</" << stack.takeLast() << '>' << endl;
      }

//---------------------------------------------------------
//   tagE
//    <mops attribute="value"/>
//---------------------------------------------------------

void Xml::tagE(const char* format, ...)
      {
      va_list args;
      va_start(args, format);
      putLevel();
      *this << '<';
    	char buffer[BS];
      vsnprintf(buffer, BS, format, args);
    	*this << buffer;
      va_end(args);
      *this << "/>" << endl;
      }

//---------------------------------------------------------
//   tagE
//---------------------------------------------------------

void Xml::tagE(const QString& s)
      {
      putLevel();
      *this << '<' << s << "/>\n";
      }

//---------------------------------------------------------
//   ntag
//    <mops> without newline
//---------------------------------------------------------

void Xml::ntag(const char* name)
      {
      putLevel();
      *this << "<" << name << ">";
      }

//---------------------------------------------------------
//   netag
//    </mops>     without indentation
//---------------------------------------------------------

void Xml::netag(const char* s)
      {
      *this << "</" << s << '>' << endl;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Xml::tag(P_ID id, QVariant data, QVariant defaultData)
      {
      if (data == defaultData)
            return;
      const char* name = propertyName(id);
      if (name == 0)
            return;

      switch(propertyType(id)) {
            case T_BOOL:
            case T_SUBTYPE:
            case T_INT:
            case T_SPATIUM:
            case T_SP_REAL:
            case T_REAL:
            case T_SCALE:
            case T_POINT:
            case T_SIZE:
            case T_COLOR:
                  tag(name, data);
                  break;

            case T_DIRECTION:
                  switch(MScore::Direction(data.toInt())) {
                        case MScore::UP:
                              tag(name, QVariant("up"));
                              break;
                        case MScore::DOWN:
                              tag(name, QVariant("down"));
                              break;
                        case MScore::AUTO:
                              break;
                        }
                  break;
            case T_DIRECTION_H:
                  switch(MScore::DirectionH(data.toInt())) {
                        case MScore::DH_LEFT:
                              tag(name, QVariant("left"));
                              break;
                        case MScore::DH_RIGHT:
                              tag(name, QVariant("right"));
                              break;
                        case MScore::DH_AUTO:
                              break;
                        }
                  break;
            case T_LAYOUT_BREAK:
                  switch(LayoutBreakType(data.toInt())) {
                        case LAYOUT_BREAK_LINE:
                              tag(name, QVariant("line"));
                              break;
                        case LAYOUT_BREAK_PAGE:
                              tag(name, QVariant("page"));
                              break;
                        case LAYOUT_BREAK_SECTION:
                              tag(name, QVariant("section"));
                              break;
                        }
                  break;
            case T_VALUE_TYPE:
                  switch(MScore::ValueType(data.toInt())) {
                        case MScore::OFFSET_VAL:
                              tag(name, QVariant("offset"));
                              break;
                        case MScore::USER_VAL:
                              tag(name, QVariant("user"));
                              break;
                        }
                  break;
            case T_PLACEMENT:
                  switch(Element::Placement(data.toInt())) {
                        case Element::ABOVE:
                              tag(name, QVariant("above"));
                              break;
                        case Element::BELOW:
                              tag(name, QVariant("below"));
                              break;
                        }
                  break;
            default:
                  abort();
            }
      }

//---------------------------------------------------------
//   tag
//    <mops>value</mops>
//---------------------------------------------------------

void Xml::tag(const char* name, QVariant data, QVariant defaultData)
      {
      if (data != defaultData)
            tag(QString(name), data);
      }

void Xml::tag(const QString& name, QVariant data)
      {
      QString ename(name.split(' ')[0]);

      putLevel();
      switch(data.type()) {
            case QVariant::Bool:
            case QVariant::Char:
            case QVariant::Int:
            case QVariant::UInt:
                  *this << "<" << name << ">";
                  *this << data.toInt();
                  *this << "</" << ename << ">\n";
                  break;
            case QVariant::Double:
                  *this << "<" << name << ">";
                  *this << data.value<double>();
                  *this << "</" << ename << ">\n";
                  break;
            case QVariant::String:
                  *this << "<" << name << ">";
                  *this << xmlString(data.value<QString>());
                  *this << "</" << ename << ">\n";
                  break;
            case QVariant::Color:
                  {
                  QColor color(data.value<QColor>());
                  *this << QString("<%1 r=\"%2\" g=\"%3\" b=\"%4\" a=\"%5\"/>\n")
                     .arg(name).arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
                  }
                  break;
            case QVariant::Rect:
                  {
                  QRect r(data.value<QRect>());
                  *this << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg(name).arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
                  }
                  break;
            case QVariant::RectF:
                  {
                  QRectF r(data.value<QRectF>());
                  *this << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg(name).arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
                  }
                  break;
            case QVariant::PointF:
                  {
                  QPointF p(data.value<QPointF>());
                  *this << QString("<%1 x=\"%2\" y=\"%3\"/>\n").arg(name).arg(p.x()).arg(p.y());
                  }
                  break;
            case QVariant::SizeF:
                  {
                  QSizeF p(data.value<QSizeF>());
                  *this << QString("<%1 w=\"%2\" h=\"%3\"/>\n").arg(name).arg(p.width()).arg(p.height());
                  }
                  break;
            default:
                  qDebug("Xml::tag: unsupported type %d\n", data.type());
                  // abort();
                  break;
            }
      }

void Xml::tag(const char* name, const QWidget* g)
      {
      tag(name, QRect(g->pos(), g->size()));
      }

//---------------------------------------------------------
//   toHtml
//---------------------------------------------------------

QString Xml::xmlString(const QString& s)
      {
      QString escaped;
      escaped.reserve(s.size());
      for (int i = 0; i < s.size(); ++i) {
            QChar c = s.at(i);
            switch(c.unicode()) {
                  case '<':
                        escaped.append(QLatin1String("&lt;"));
                        break;
                  case '>':
                        escaped.append(QLatin1String("&gt;"));
                        break;
                  case '&':
                        escaped.append(QLatin1String("&amp;"));
                        break;
                  case '\"':
                        escaped.append(QLatin1String("&quot;"));
                        break;
                  case 0x00:
                  case 0x01:
                  case 0x02:
                  case 0x03:
                  case 0x04:
                  case 0x05:
                  case 0x06:
                  case 0x07:
                  case 0x08:
                  case 0x0B:
                  case 0x0C:
                  case 0x0E:
                  case 0x0F:
                  case 0x10:
                  case 0x11:
                  case 0x12:
                  case 0x13:
                  case 0x14:
                  case 0x15:
                  case 0x16:
                  case 0x17:
                  case 0x18:
                  case 0x19:
                  case 0x1A:
                  case 0x1B:
                  case 0x1C:
                  case 0x1D:
                  case 0x1E:
                  case 0x1F:
                        break;
                  default:
                        escaped += QChar(c);
                        break;
                  }
            }
      return escaped;
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Xml::dump(int len, const unsigned char* p)
      {
      putLevel();
      int col = 0;
      setFieldWidth(5);
      setNumberFlags(numberFlags() | QTextStream::ShowBase);
      setIntegerBase(16);
      for (int i = 0; i < len; ++i, ++col) {
            if (col >= 16) {
                  setFieldWidth(0);
                  *this << endl;
                  col = 0;
                  putLevel();
                  setFieldWidth(5);
                  }
            *this << (p[i] & 0xff);
            }
      if (col)
            *this << endl << dec;
      setFieldWidth(0);
      setIntegerBase(10);
      }

//---------------------------------------------------------
//   htmlToString
//---------------------------------------------------------

void Xml::htmlToString(XmlReader& e, int level, QString* s)
      {
// printf("  html <%s>\n", e.name().toUtf8().data());
      *s += QString("<%1").arg(e.name().toString());
      QXmlStreamAttributes map = e.attributes();
      int n = map.size();
      for (int i = 0; i < n; ++i) {
            const QXmlStreamAttribute& a = map.at(i);
            *s += QString(" %1=\"%2\"").arg(a.name().toString()).arg(a.value().toString());
            }
      *s += ">";
      ++level;
      for (;;) {
            QXmlStreamReader::TokenType t = e.readNext();
//            printf("    token %d\n", int(t));
            switch(t) {
                  case QXmlStreamReader::StartElement:
//                        printf("         start <%s>\n", e.name().toUtf8().data());
                        htmlToString(e, level, s);
                        break;
                  case QXmlStreamReader::EndElement:
//                        printf("         end <%s>\n", e.name().toUtf8().data());
                        *s += QString("</%1>").arg(e.name().toString());
                        --level;
                        return;
                  case QXmlStreamReader::Characters:
//                        printf("         char <%s>\n", e.text().toUtf8().data());
                        if (!e.isWhitespace())
                              *s += e.text().toString();
                        break;
                  case QXmlStreamReader::Comment:
                        break;
                  default:
//                        printf("      ?? %d\n", int(t));
                        return;
                  }
            }
      }

QString Xml::htmlToString(XmlReader& e)
      {
      QString s;
      if (e.readNextStartElement()) {
            htmlToString(e, 0, &s);
//            printf("====HTML<%s>====\n", qPrintable(s));
            e.skipCurrentElement();
            }
      return s;
      }

//---------------------------------------------------------
//   writeHtml
//---------------------------------------------------------

void Xml::writeHtml(const QString& s)
      {
      QStringList sl(s.split("\n"));
      //
      // remove first line from html (DOCTYPE...)
      //
      for (int i = 1; i < sl.size(); ++i)
            *this << sl[i] << "\n";
      }


