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
#include "measure.h"
#include "spanner.h"
#include "beam.h"
#include "tuplet.h"
#include "sym.h"
#include "note.h"
#include "barline.h"
#include "style.h"

namespace Ms {

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
      Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
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
//    recognizes this two styles:
//    <move z="2" n="4"/>     (old style)
//    <move>2/4</move>        (new style)
//---------------------------------------------------------

Fraction XmlReader::readFraction()
      {
      Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
      int z = attribute("z", "0").toInt();
      int n = attribute("n", "1").toInt();
      const QString& s(readElementText());
      if (!s.isEmpty()) {
            int i = s.indexOf('/');
            if (i == -1)
                  qFatal("illegal fraction <%s>", qPrintable(s));
            else {
                  z = s.left(i).toInt();
                  n = s.mid(i+1).toInt();
                  }
            }
      return Fraction(z, n);
      }

//---------------------------------------------------------
//   unknown
//    unknown tag read
//---------------------------------------------------------

void XmlReader::unknown()
      {
      if (QXmlStreamReader::error())
            qDebug("%s ", qPrintable(errorString()));
      if (!docName.isEmpty())
            qDebug("tag in <%s> line %lld col %lld: %s",
               qPrintable(docName), lineNumber(), columnNumber(), name().toUtf8().data());
      else
            qDebug("line %lld col %lld: %s", lineNumber(), columnNumber(), name().toUtf8().data());
      skipCurrentElement();
      }

//---------------------------------------------------------
//   rfrac
//    return relative position in measure
//---------------------------------------------------------

Fraction XmlReader::rfrac() const
      {
      if (_currMeasure)
            return Fraction::fromTicks(tick() - _currMeasure->tick());
      return afrac();
      }

//---------------------------------------------------------
//   afrac
//    return absolute position
//---------------------------------------------------------

Fraction XmlReader::afrac() const
      {
      return Fraction::fromTicks(tick());
      }

//---------------------------------------------------------
//   location
//---------------------------------------------------------

Location XmlReader::location(bool forceAbsFrac) const
      {
      Location l = Location::absolute();
      fillLocation(l, forceAbsFrac);
      return l;
      }

//---------------------------------------------------------
//   fillLocation
//    fills location fields which have default values with
//    values relevant for the current reader's position.
//---------------------------------------------------------

void XmlReader::fillLocation(Location& l, bool forceAbsFrac) const
      {
      constexpr Location defaults = Location::absolute();
      const bool absFrac = (pasteMode() || forceAbsFrac);
      if (l.track() == defaults.track())
            l.setTrack(track());
      if (l.frac() == defaults.frac())
            l.setFrac(absFrac ? afrac() : rfrac());
      if (l.measure() == defaults.measure())
            l.setMeasure(absFrac ? 0 : currentMeasureIndex());
      }

//---------------------------------------------------------
//   setLocation
//    sets a new reading location, taking into account its
//    type (absolute or relative).
//---------------------------------------------------------

void XmlReader::setLocation(const Location& l)
      {
      if (l.isRelative()) {
            Location newLoc = l;
            newLoc.toAbsolute(location());
            setLocation(newLoc); // recursion
            return;
            }
      setTrack(l.track() - _trackOffset);
      int tick = l.frac().ticks() - _tickOffset;
      if (!pasteMode()) {
            Q_ASSERT(l.measure() == currentMeasureIndex());
            tick += currentMeasure()->tick();
            }
      initTick(tick);
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
      QXmlStreamReader::TokenType tt = readNext();
      if (tt == QXmlStreamReader::Characters) {
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
      }

//---------------------------------------------------------
//   htmlToString
//---------------------------------------------------------

void XmlReader::htmlToString(int level, QString* s)
      {
      *s += QString("<%1").arg(name().toString());
      for (const QXmlStreamAttribute& a : attributes())
            *s += QString(" %1=\"%2\"").arg(a.name().toString()).arg(a.value().toString());
      *s += ">";
      ++level;
      for (;;) {
            QXmlStreamReader::TokenType t = readNext();
            switch(t) {
                  case QXmlStreamReader::StartElement:
                        htmlToString(level, s);
                        break;
                  case QXmlStreamReader::EndElement:
                        *s += QString("</%1>").arg(name().toString());
                        --level;
                        return;
                  case QXmlStreamReader::Characters:
                        if (!isWhitespace())
                              *s += text().toString().toHtmlEscaped();
                        break;
                  case QXmlStreamReader::Comment:
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
      for (QXmlStreamReader::TokenType t = readNext(); t != QXmlStreamReader::EndElement; t = readNext()) {
            switch(t) {
                  case QXmlStreamReader::StartElement:
                        htmlToString(level, &s);
                        break;
                  case QXmlStreamReader::EndElement:
                        break;
                  case QXmlStreamReader::Characters:
                        s += text().toString().toHtmlEscaped();
                        break;
                  case QXmlStreamReader::Comment:
                        break;

                  default:
                        qDebug("htmlToString: read token: %s", qPrintable(tokenString()));
                        return s;
                  }
            }
      return s;
      }

//---------------------------------------------------------
//   compareProperty
//---------------------------------------------------------

template <class T> bool compareProperty(void* val, void* defaultVal)
      {
      return (defaultVal == 0) || (*(T*)val != *(T*)defaultVal);
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
//   addUserTextStyle
//    return false if mapping is not possible
//      (too many user text styles)
//---------------------------------------------------------

Tid XmlReader::addUserTextStyle(const QString& name)
      {
      qDebug("%s", qPrintable(name));
      Tid id = Tid::TEXT_STYLES;
      if (userTextStyles.size() == 0)
            id = Tid::USER1;
      else if (userTextStyles.size() == 1)
            id = Tid::USER2;
      else if (userTextStyles.size() == 2)
            id = Tid::USER3;
      else if (userTextStyles.size() == 3)
            id = Tid::USER4;
      else if (userTextStyles.size() == 4)
            id = Tid::USER5;
      else if (userTextStyles.size() == 5)
            id = Tid::USER6;
      else
            qDebug("too many user defined textstyles");
      if (id != Tid::TEXT_STYLES)
            userTextStyles.push_back({name, id});
      return id;
      }

//---------------------------------------------------------
//   lookupUserTextStyle
//---------------------------------------------------------

Tid XmlReader::lookupUserTextStyle(const QString& name)
      {
      for (const auto& i : userTextStyles) {
            if (i.name == name)
                  return i.ss;
            }
      return Tid::TEXT_STYLES;       // not found
      }
}


