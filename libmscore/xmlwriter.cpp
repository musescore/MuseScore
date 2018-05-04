//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
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
//   compareProperty
//---------------------------------------------------------

template <class T> bool compareProperty(void* val, void* defaultVal)
      {
      return (defaultVal == 0) || (*(T*)val != *(T*)defaultVal);
      }

//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

XmlWriter::XmlWriter(Score* s)
      {
      _score = s;
      setCodec("UTF-8");
      }

XmlWriter::XmlWriter(Score* s, QIODevice* device)
   : QTextStream(device)
      {
      _score = s;
      setCodec("UTF-8");
      }

//---------------------------------------------------------
//   pTag
//---------------------------------------------------------

void XmlWriter::pTag(const char* name, PlaceText place)
      {
      const char* tags[] = {
            "auto", "above", "below", "left"
            };
      tag(name, tags[int(place)]);
      }

//---------------------------------------------------------
//   putLevel
//---------------------------------------------------------

void XmlWriter::putLevel()
      {
      int level = stack.size();
      for (int i = 0; i < level * 2; ++i)
            *this << ' ';
      }

//---------------------------------------------------------
//   header
//---------------------------------------------------------

void XmlWriter::header()
      {
      *this << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      }

//---------------------------------------------------------
//   stag
//    <mops attribute="value">
//---------------------------------------------------------

void XmlWriter::stag(const QString& s)
      {
      putLevel();
      *this << '<' << s << '>' << endl;
      stack.append(s.split(' ')[0]);
      }

//---------------------------------------------------------
//   etag
//    </mops>
//---------------------------------------------------------

void XmlWriter::etag()
      {
      putLevel();
      *this << "</" << stack.takeLast() << '>' << endl;
      }

//---------------------------------------------------------
//   tagE
//    <mops attribute="value"/>
//---------------------------------------------------------

void XmlWriter::tagE(const char* format, ...)
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

void XmlWriter::tagE(const QString& s)
      {
      putLevel();
      *this << '<' << s << "/>\n";
      }

//---------------------------------------------------------
//   ntag
//    <mops> without newline
//---------------------------------------------------------

void XmlWriter::ntag(const char* name)
      {
      putLevel();
      *this << "<" << name << ">";
      }

//---------------------------------------------------------
//   netag
//    </mops>     without indentation
//---------------------------------------------------------

void XmlWriter::netag(const char* s)
      {
      *this << "</" << s << '>' << endl;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void XmlWriter::tag(P_ID id, QVariant data, QVariant defaultData)
      {
      if (data == defaultData)
            return;
      const char* name = propertyName(id);
      if (name == 0)
            return;

      switch (propertyType(id)) {
            case P_TYPE::BOOL:
            case P_TYPE::SUBTYPE:
            case P_TYPE::INT:
            case P_TYPE::ZERO_INT:
            case P_TYPE::SPATIUM:
            case P_TYPE::SP_REAL:
            case P_TYPE::REAL:
            case P_TYPE::SCALE:
            case P_TYPE::POINT:
            case P_TYPE::SIZE:
            case P_TYPE::COLOR:
            case P_TYPE::DIRECTION:
            case P_TYPE::STRING:
            case P_TYPE::FONT:
            case P_TYPE::ALIGN:
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
                  switch (GlissandoStyle(data.toInt())) {
                        case GlissandoStyle::BLACK_KEYS:
                              tag(name, QVariant("blackkeys"));
                              break;
                        case GlissandoStyle::WHITE_KEYS:
                              tag(name, QVariant("whitekeys"));
                              break;
                        case GlissandoStyle::DIATONIC:
                              tag(name, QVariant("diatonic"));
                              break;
                        default:
                             //tag(name, QVariant("Chromatic"));
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
                        case LayoutBreak::LINE:
                              tag(name, QVariant("line"));
                              break;
                        case LayoutBreak::PAGE:
                              tag(name, QVariant("page"));
                              break;
                        case LayoutBreak::SECTION:
                              tag(name, QVariant("section"));
                              break;
                        case LayoutBreak::NOBREAK:
                              tag(name, QVariant("nobreak"));
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
                  switch (Placement(data.toInt())) {
                        case Placement::ABOVE:
                              tag(name, QVariant("above"));
                              break;
                        case Placement::BELOW:
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
            case P_TYPE::HEAD_GROUP:
                  tag(name, NoteHead::group2name(NoteHead::Group(data.toInt())));
                  break;
            case P_TYPE::HEAD_TYPE:
                  tag(name, NoteHead::type2name(NoteHead::Type(data.toInt())));
                  break;
            case P_TYPE::SUB_STYLE:
                  tag(name, subStyleName(SubStyle(data.toInt())));
                  break;
            default:
                  Q_ASSERT(false);
            }
      }

//---------------------------------------------------------
//   tag
//    <mops>value</mops>
//---------------------------------------------------------

void XmlWriter::tag(const char* name, QVariant data, QVariant defaultData)
      {
      if (data != defaultData)
            tag(QString(name), data);
      }

void XmlWriter::tag(const QString& name, QVariant data)
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
                  const QRect& r(data.value<QRect>());
                  *this << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg(name).arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
                  }
                  break;
            case QVariant::RectF:
                  {
                  const QRectF& r(data.value<QRectF>());
                  *this << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg(name).arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
                  }
                  break;
            case QVariant::PointF:
                  {
                  const QPointF& p(data.value<QPointF>());
                  *this << QString("<%1 x=\"%2\" y=\"%3\"/>\n").arg(name).arg(p.x()).arg(p.y());
                  }
                  break;
            case QVariant::SizeF:
                  {
                  const QSizeF& p(data.value<QSizeF>());
                  *this << QString("<%1 w=\"%2\" h=\"%3\"/>\n").arg(name).arg(p.width()).arg(p.height());
                  }
                  break;
            default: {
                  const char* type = data.typeName();
                  if (strcmp(type, "Ms::Spatium") == 0) {
                        *this << "<" << name << ">";
                        *this << data.value<Spatium>().val();
                        *this << "</" << ename << ">\n";
                        }
                  else if (strcmp(type, "Ms::Fraction") == 0) {
                        const Fraction& f = data.value<Fraction>();
                        *this << QString("<%1>%2/%3</%1>\n").arg(name).arg(f.numerator()).arg(f.denominator());
                        }
                  else if (strcmp(type, "Ms::Direction") == 0)
                        *this << QString("<%1>%2</%1>\n").arg(name).arg(toString(data.value<Direction>()));
                  else if (strcmp(type, "Ms::Align") == 0) {
                        Align a = Align(data.toInt());
                        const char* h;
                        if (a & Align::HCENTER)
                              h = "center";
                        else if (a & Align::RIGHT)
                              h = "right";
                        else
                              h = "left";
                        const char* v;
                        if (a & Align::BOTTOM)
                              v = "bottom";
                        else if (a & Align::VCENTER)
                              v = "center";
                        else if (a & Align::BASELINE)
                              v = "baseline";
                        else
                              v = "top";
                        *this << QString("<%1>%2,%3</%1>\n").arg(name).arg(h).arg(v);
                        }
                  else {
                        qFatal("XmlWriter::tag: unsupported type %d %s", data.type(), type);
                        }
                  }
                  break;
            }
      }

void XmlWriter::tag(const char* name, const QWidget* g)
      {
      tag(name, QRect(g->pos(), g->size()));
      }

//---------------------------------------------------------
//   xmlString
//---------------------------------------------------------

QString XmlWriter::xmlString(ushort c)
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

QString XmlWriter::xmlString(const QString& s)
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

void XmlWriter::dump(int len, const unsigned char* p)
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
//   writeXml
//    string s is already escaped (& -> "&amp;")
//---------------------------------------------------------

void XmlWriter::writeXml(const QString& name, QString s)
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
//   addSpanner
//---------------------------------------------------------

int XmlWriter::addSpanner(const Spanner* s)
      {
      ++_spannerId;
      _spanner.append(std::pair<int, const Spanner*>(_spannerId, s));
      return _spannerId;
      }

//---------------------------------------------------------
//   findSpanner
//---------------------------------------------------------

const Spanner* XmlWriter::findSpanner(int id)
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

int XmlWriter::spannerId(const Spanner* s)
      {
      for (auto i : _spanner) {
            if (i.second == s)
                  return i.first;
            }
      return addSpanner(s);
      }

//---------------------------------------------------------
//   canWrite
//---------------------------------------------------------

bool XmlWriter::canWrite(const Element* e) const
      {
      if (!_clipboardmode)
            return true;
      return _filter.canSelect(e);
      }

//---------------------------------------------------------
//   canWriteVoice
//---------------------------------------------------------

bool XmlWriter::canWriteVoice(int track) const
      {
      if (!_clipboardmode)
            return true;
      return _filter.canSelectVoice(track);
      }

}


