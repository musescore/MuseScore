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
#include "sym.h"
#include "note.h"
#include "barline.h"

namespace Ms {

QString docName;

//---------------------------------------------------------
//   intAttribute
//---------------------------------------------------------

int XmlReader::intAttribute(const char* s, int _default) const
      {
      if (attributes().hasAttribute(s))
            // return attributes().value(s).toString().toInt();
            return attributes().value(s).toInt();
      else
            return _default;
      }

int XmlReader::intAttribute(const char* s) const
      {
      return attributes().value(s).toInt();
      }

//---------------------------------------------------------
//   doubleAttribute
//---------------------------------------------------------

double XmlReader::doubleAttribute(const char* s) const
      {
      return attributes().value(s).toDouble();
      }

double XmlReader::doubleAttribute(const char* s, double _default) const
      {
      if (attributes().hasAttribute(s))
            return attributes().value(s).toDouble();
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
      Q_ASSERT(tokenType() == XmlStreamReader::StartElement);
#ifndef NDEBUG
      if (!attributes().hasAttribute("x")) {
            XmlStreamAttributes map = attributes();
            qDebug("XmlReader::readPoint: x attribute missing: %s (%d)",
               name().toUtf8().data(), map.size());
            for (int i = 0; i < map.size(); ++i) {
                  const XmlStreamAttribute& a = map.at(i);
                  qDebug(" attr <%s> <%s>", a.name().toUtf8().data(), a.value().toUtf8().data());
                  }
            unknown();
            }
      if (!attributes().hasAttribute("y")) {
            qDebug("XmlReader::readPoint: y attribute missing: %s", name().toUtf8().data());
            unknown();
            }
#endif
      qreal x = doubleAttribute("x", 0.0);
      qreal y = doubleAttribute("y", 0.0);
      readNext();
      return QPointF(x, y);
      }

//---------------------------------------------------------
//   readColor
//---------------------------------------------------------

QColor XmlReader::readColor()
      {
      Q_ASSERT(tokenType() == XmlStreamReader::StartElement);
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
      Q_ASSERT(tokenType() == XmlStreamReader::StartElement);
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
      Q_ASSERT(tokenType() == XmlStreamReader::StartElement);
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
      Q_ASSERT(tokenType() == XmlStreamReader::StartElement);
      int z = attribute("z", "0").toInt();
      int n = attribute("n", "0").toInt();
      skipCurrentElement();
      return Fraction(z, n);
      }

//---------------------------------------------------------
//   unknown
//    unknown tag read
//---------------------------------------------------------

void XmlReader::unknown()
      {
      if (XmlStreamReader::error())
            qDebug("StreamReaderError: %s", qPrintable(errorString()));
      qDebug("%s: xml read error at line %lld col %lld: %s",
         qPrintable(docName), lineNumber(), columnNumber(),
         name().toUtf8().data());
      skipCurrentElement();
      }

//---------------------------------------------------------
//   addBeam
//---------------------------------------------------------

void XmlReader::addBeam(Beam* s)
      {
      _beams.insert(s->id(), s);
      }

//---------------------------------------------------------
//   addTuplet
//---------------------------------------------------------

void XmlReader::addTuplet(Tuplet* s)
      {
      _tuplets.insert(s->id(), s);
      }

//---------------------------------------------------------
//   readDouble
//---------------------------------------------------------

double XmlReader::readDouble(double min, double max)
      {
      double val = readElementText().toDouble();
      if (val < min)
            val = min;
      else if (val > max)
            val = max;
      return val;
      }

//---------------------------------------------------------
//   readBool
//---------------------------------------------------------

bool XmlReader::readBool()
      {
      bool val;
      XmlStreamReader::TokenType tt = readNext();
      if (tt == XmlStreamReader::Characters) {
            val = text().toInt() != 0;
            readNext();
            }
      else
            val = true;
      return val;
      }

//---------------------------------------------------------
//   checkTuplets
//---------------------------------------------------------

void XmlReader::checkTuplets()
      {
      for (Tuplet* tuplet : tuplets()) {
            if (tuplet->elements().empty()) {
                  // this should not happen and is a sign of input file corruption
                  qDebug("Measure:read(): empty tuplet id %d (%p), input file corrupted?",
                     tuplet->id(), tuplet);
                  delete tuplet;
                  }
            else {
                  //sort tuplet elements. Needed for nested tuplets #22537
                  tuplet->sortElements();
                  tuplet->sanitizeTuplet();
                  }
            }
      // This requires a separate pass in case of nested tuplets that required sanitizing
      for (Tuplet* tuplet : tuplets())
            tuplet->addMissingElements();
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
      setCodec("UTF-8");
      }

Xml::Xml(QIODevice* device)
   : QTextStream(device)
      {
      setCodec("UTF-8");
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
            return PlaceText::AUTO;
      if (s == "above" || s == "1")
            return PlaceText::ABOVE;
      if (s == "below" || s == "2")
            return PlaceText::BELOW;
      if (s == "left" || s == "3")
            return PlaceText::LEFT;
      qDebug("unknown placement value <%s>", qPrintable(s));
      return PlaceText::AUTO;
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
            case P_TYPE::BOOL:
            case P_TYPE::SUBTYPE:
            case P_TYPE::INT:
            case P_TYPE::SPATIUM:
            case P_TYPE::SP_REAL:
            case P_TYPE::REAL:
            case P_TYPE::SCALE:
            case P_TYPE::POINT:
            case P_TYPE::SIZE:
            case P_TYPE::COLOR:
                  tag(name, data);
                  break;
            case P_TYPE::ORNAMENT_STYLE:
                  switch (MScore::OrnamentStyle(data.toInt())) {
                        case MScore::OrnamentStyle::BAROQUE:
                              tag(name, QVariant("baroque"));
                              break;
                        default:
                             // tag(name, QVariant("default"));
                             break;
                             }
                  break;
            case P_TYPE::GLISSANDO_STYLE:
                  switch (MScore::GlissandoStyle(data.toInt())) {
                        case MScore::GlissandoStyle::BLACK_KEYS:
                              tag(name, QVariant("blackkeys"));
                              break;
                        case MScore::GlissandoStyle::WHITE_KEYS:
                              tag(name, QVariant("whitekeys"));
                              break;
                        case MScore::GlissandoStyle::DIATONIC:
                              tag(name, QVariant("diatonic"));
                              break;
                        default:
                             //tag(name, QVariant("Chromatic"));
                             break;
                             }
                  break;
            case P_TYPE::DIRECTION:
                  switch (MScore::Direction(data.toInt())) {
                        case MScore::Direction::UP:
                              tag(name, QVariant("up"));
                              break;
                        case MScore::Direction::DOWN:
                              tag(name, QVariant("down"));
                              break;
                        case MScore::Direction::AUTO:
                              break;
                        }
                  break;
            case P_TYPE::DIRECTION_H:
                  switch (MScore::DirectionH(data.toInt())) {
                        case MScore::DirectionH::LEFT:
                              tag(name, QVariant("left"));
                              break;
                        case MScore::DirectionH::RIGHT:
                              tag(name, QVariant("right"));
                              break;
                        case MScore::DirectionH::AUTO:
                              break;
                        }
                  break;
            case P_TYPE::LAYOUT_BREAK:
                  switch (LayoutBreak::Type(data.toInt())) {
                        case LayoutBreak::Type::LINE:
                              tag(name, QVariant("line"));
                              break;
                        case LayoutBreak::Type::PAGE:
                              tag(name, QVariant("page"));
                              break;
                        case LayoutBreak::Type::SECTION:
                              tag(name, QVariant("section"));
                              break;
                        }
                  break;
            case P_TYPE::VALUE_TYPE:
                  switch (Note::ValueType(data.toInt())) {
                        case Note::ValueType::OFFSET_VAL:
                              tag(name, QVariant("offset"));
                              break;
                        case Note::ValueType::USER_VAL:
                              tag(name, QVariant("user"));
                              break;
                        }
                  break;
            case P_TYPE::PLACEMENT:
                  switch (Element::Placement(data.toInt())) {
                        case Element::Placement::ABOVE:
                              tag(name, QVariant("above"));
                              break;
                        case Element::Placement::BELOW:
                              tag(name, QVariant("below"));
                              break;
                        }
                  break;
            case P_TYPE::SYMID:
                  tag(name, Sym::id2name(SymId(data.toInt())));
                  break;
            case P_TYPE::BARLINE_TYPE:
                  tag(name, BarLine::barLineTypeName(BarLineType(data.toInt())));
                  break;
            default:
                  Q_ASSERT(false);
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
            case QVariant::Size:
                  {
                  QSize p(data.value<QSize>());
                  *this << QString("<%1 w=\"%2\" h=\"%3\"/>\n").arg(name).arg(p.width()).arg(p.height());
                  }
                  break;
            default:
                  qDebug("Xml::tag: unsupported type %d", data.type());
                  // abort();
                  break;
            }
      }

void Xml::tag(const char* name, const QWidget* g)
      {
      tag(name, QRect(g->pos(), g->size()));
      }


//---------------------------------------------------------
//   xmlString
//---------------------------------------------------------

QString Xml::xmlString(ushort c)
      {
      switch(c) {
            case '<':
                  return QLatin1String("&lt;");
            case '>':
                  return QLatin1String("&gt;");
            case '&':
                  return QLatin1String("&amp;");
            case '\"':
                  return QLatin1String("&quot;");
            default:
                  // ignore invalid characters in xml 1.0
                  if ((c < 0x20 && c != 0x09 && c != 0x0A && c != 0x0D))
                        return QString();
                  return QString(QChar(c));
            }
      }

//---------------------------------------------------------
//   xmlString
//---------------------------------------------------------

QString Xml::xmlString(const QString& s)
      {
      QString escaped;
      escaped.reserve(s.size());
      for (int i = 0; i < s.size(); ++i) {
            ushort c = s.at(i).unicode();
            escaped += xmlString(c);
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

void XmlReader::htmlToString(int level, QString* s)
      {
      *s += QString("<%1").arg(name().toString());
      for (const XmlStreamAttribute& a : attributes())
            *s += QString(" %1=\"%2\"").arg(a.name().toString()).arg(a.value().toString());
      *s += ">";
      ++level;
      for (;;) {
            XmlStreamReader::TokenType t = readNext();
            switch(t) {
                  case XmlStreamReader::StartElement:
                        htmlToString(level, s);
                        break;
                  case XmlStreamReader::EndElement:
                        *s += QString("</%1>").arg(name().toString());
                        --level;
                        return;
                  case XmlStreamReader::Characters:
                        if (!isWhitespace())
                              *s += text().toString().toHtmlEscaped();
                        break;
                  case XmlStreamReader::Comment:
                        break;

                  default:
                        qDebug("htmlToString: read token: %s", qPrintable(tokenString()));
                        return;
                  }
            }
      }

//-------------------------------------------------------------------
//   readXml
//    read verbatim until end tag of current level is reached
//-------------------------------------------------------------------

QString XmlReader::readXml()
      {
      QString s;
      int level = 1;
      for (;;) {
            XmlStreamReader::TokenType t = readNext();
            switch(t) {
                  case XmlStreamReader::StartElement:
                        htmlToString(level, &s);
                        break;
                  case XmlStreamReader::EndElement:
                        return s;
                  case XmlStreamReader::Characters:
                        s += text().toString().toHtmlEscaped();
                        break;
                  case XmlStreamReader::Comment:
                        break;

                  default:
                        qDebug("htmlToString: read token: %s", qPrintable(tokenString()));
                        return s;
                  }
            }
      return s;
      }

//---------------------------------------------------------
//   writeXml
//    string s is already escaped (& -> "&amp;")
//---------------------------------------------------------

void Xml::writeXml(const QString& name, QString s)
      {
      QString ename(name.split(' ')[0]);
      putLevel();
      for (int i = 0; i < s.size(); ++i) {
            ushort c = s.at(i).unicode();
            if (c < 0x20 && c != 0x09 && c != 0x0A && c != 0x0D)
                  s[i] = '?';
            }
      *this << "<" << name << ">";
      *this << s;
      *this << "</" << ename << ">\n";
      }

//---------------------------------------------------------
//   spannerValues
//---------------------------------------------------------

const SpannerValues* XmlReader::spannerValues(int id) const
      {
      for (const SpannerValues& v : _spannerValues) {
            if (v.spannerId == id)
                  return &v;
            }
      return 0;
      }

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void XmlReader::addSpanner(int id, Spanner* s)
      {
      _spanner.append(std::pair<int, Spanner*>(id, s));
      }

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void XmlReader::removeSpanner(const Spanner* s)
      {
      for (auto i : _spanner) {
            if (i.second == s) {
                  _spanner.removeOne(i);
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   findSpanner
//---------------------------------------------------------

Spanner* XmlReader::findSpanner(int id)
      {
      for (auto i : _spanner) {
            if (i.first == id)
                  return i.second;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   spannerId
//---------------------------------------------------------

int XmlReader::spannerId(const Spanner* s)
      {
      for (auto i : _spanner) {
            if (i.second == s)
                  return i.first;
            }
      qDebug("XmlReader::spannerId not found");
      return -1;
      }

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

int Xml::addSpanner(const Spanner* s)
      {
      ++_spannerId;
      _spanner.append(std::pair<int, const Spanner*>(_spannerId, s));
      return _spannerId;
      }

//---------------------------------------------------------
//   findSpanner
//---------------------------------------------------------

const Spanner* Xml::findSpanner(int id)
      {
      for (auto i : _spanner) {
            if (i.first == id)
                  return i.second;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   spannerId
//---------------------------------------------------------

int Xml::spannerId(const Spanner* s)
      {
      for (auto i : _spanner) {
            if (i.second == s)
                  return i.first;
            }
      return addSpanner(s);
      }

//---------------------------------------------------------
//   clefs
//---------------------------------------------------------

QList<std::pair<int, ClefType>>& XmlReader::clefs(int idx)
      {
      while (idx >= _clefs.size())
            _clefs.append(QList<std::pair<int,ClefType>>());
      return _clefs[idx];
      }

//---------------------------------------------------------
//   canWrite
//---------------------------------------------------------

bool Xml::canWrite(const Element* e) const
      {
      if (!clipboardmode)
            return true;
      return _filter.canSelect(e);
      }

//---------------------------------------------------------
//   canWriteVoice
//---------------------------------------------------------

bool Xml::canWriteVoice(int track) const
      {
      if (!clipboardmode)
            return true;
      return _filter.canSelectVoice(track);
      }

}

