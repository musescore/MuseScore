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
#include "measure.h"
#include "score.h"
#include "spanner.h"
#include "staff.h"
#include "beam.h"
#include "tuplet.h"

namespace Ms {

//---------------------------------------------------------
//   ~XmlReader
//---------------------------------------------------------

XmlReader::~XmlReader()
      {
      if (!_connectors.empty() || !_pendingConnectors.empty()) {
            qDebug("XmlReader::~XmlReader: there are unpaired connectors left");
            for (auto& c : _connectors) {
                  Element* conn = c->releaseConnector();
                  if (conn && !conn->isTuplet()) // tuplets are added to score even when not finished
                        delete conn;
                  }
            for (auto& c : _pendingConnectors)
                  delete c->releaseConnector();
            }
      }

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
            if (i == -1) {
                  return Fraction::fromTicks(s.toInt());
                  }
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
               qPrintable(docName), lineNumber() + _offsetLines, columnNumber(), name().toUtf8().data());
      else
            qDebug("line %lld col %lld: %s", lineNumber() + _offsetLines, columnNumber(), name().toUtf8().data());
      skipCurrentElement();
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
//    When in paste mode (or forceAbsFrac is true) absolute
//    fraction values are used and measure number is set to
//    zero.
//---------------------------------------------------------

void XmlReader::fillLocation(Location& l, bool forceAbsFrac) const
      {
      constexpr Location defaults = Location::absolute();
      const bool absFrac = (pasteMode() || forceAbsFrac);
      if (l.track() == defaults.track())
            l.setTrack(track());
      if (l.frac() == defaults.frac())
            l.setFrac(absFrac ? tick() : rtick());
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
            int intTicks = l.frac().ticks();
            if (_tick == Fraction::fromTicks(_intTick + intTicks)) {
                  _intTick += intTicks;
                  setTrack(newLoc.track() - _trackOffset);
                  return;
                  }
            setLocation(newLoc); // recursion
            return;
            }
      setTrack(l.track() - _trackOffset);
      setTick(l.frac() - _tickOffset);
      if (!pasteMode()) {
            Q_ASSERT(l.measure() == currentMeasureIndex());
            incTick(currentMeasure()->tick());
            }
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
      // This requires a separate pass in case of nested tuplets that required sanitizing
      for (Tuplet* tuplet : tuplets())
            tuplet->addMissingElements();
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
            switch (t) {
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
      else if (userTextStyles.size() == 6)
            id = Tid::USER7;
      else if (userTextStyles.size() == 7)
            id = Tid::USER8;
      else if (userTextStyles.size() == 8)
            id = Tid::USER9;
      else if (userTextStyles.size() == 9)
            id = Tid::USER10;
      else if (userTextStyles.size() == 10)
            id = Tid::USER11;
      else if (userTextStyles.size() == 11)
            id = Tid::USER12;
      else
            qDebug("too many user defined textstyles");
      if (id != Tid::TEXT_STYLES)
            userTextStyles.push_back({name, id});
      return id;
      }

//---------------------------------------------------------
//   lookupUserTextStyle
//---------------------------------------------------------

Tid XmlReader::lookupUserTextStyle(const QString& name) const
      {
      for (const auto& i : userTextStyles) {
            if (i.name == name)
                  return i.ss;
            }
      return Tid::TEXT_STYLES;       // not found
      }

//---------------------------------------------------------
//   addConnectorInfo
//---------------------------------------------------------

void XmlReader::addConnectorInfo(std::unique_ptr<ConnectorInfoReader> c)
      {
      _connectors.push_back(std::move(c));
      ConnectorInfoReader* c1 = _connectors.back().get();
      c1->update();
      for (std::unique_ptr<ConnectorInfoReader>& c2 : _connectors) {
            if (c2->connect(c1)) {
                  if (c2->finished()) {
                        c2->addToScore(pasteMode());
                        removeConnector(c2.get());
                        }
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   removeConnector
//---------------------------------------------------------

void XmlReader::removeConnector(const ConnectorInfoReader* c)
      {
      while (c->prev())
            c = c->prev();
      while (c) {
            ConnectorInfoReader* next = c->next();
            for (auto it = _connectors.begin(); it != _connectors.end(); ++it) {
                  if (it->get() == c) {
                        _connectors.erase(it);
                        break;
                        }
                  }
            c = next;
            }
      }

//---------------------------------------------------------
//   checkConnectors
//---------------------------------------------------------

void XmlReader::checkConnectors()
      {
      for (std::unique_ptr<ConnectorInfoReader>& c : _pendingConnectors)
            addConnectorInfo(std::move(c));
      _pendingConnectors.clear();
      }

//---------------------------------------------------------
//   distanceSort
//---------------------------------------------------------

static bool distanceSort(const QPair<int, QPair<ConnectorInfoReader*, ConnectorInfoReader*>>& p1, const QPair<int, QPair<ConnectorInfoReader*, ConnectorInfoReader*>>& p2)
      {
      return p1.first < p2.first;
      }

//---------------------------------------------------------
//   reconnectBrokenConnectors
//---------------------------------------------------------

void XmlReader::reconnectBrokenConnectors()
      {
      if (_connectors.empty())
            return;
      qDebug("Reconnecting broken connectors (%d nodes)", int(_connectors.size()));
      QList<QPair<int, QPair<ConnectorInfoReader*, ConnectorInfoReader*>>> brokenPairs;
      for (size_t i = 1; i < _connectors.size(); ++i) {
            for (size_t j = 0; j < i; ++j) {
                  ConnectorInfoReader* c1 = _connectors[i].get();
                  ConnectorInfoReader* c2 = _connectors[j].get();
                  int d = c1->connectionDistance(*c2);
                  if (d >= 0)
                        brokenPairs.append(qMakePair(d, qMakePair(c1, c2)));
                  else
                        brokenPairs.append(qMakePair(-d, qMakePair(c2, c1)));
                  }
            }
      std::sort(brokenPairs.begin(), brokenPairs.end(), distanceSort);
      for (auto& distPair : brokenPairs) {
            if (distPair.first == INT_MAX)
                  continue;
            auto& pair = distPair.second;
            if (pair.first->next() || pair.second->prev())
                  continue;
            pair.first->forceConnect(pair.second);
            }
      QSet<ConnectorInfoReader*> reconnected;
      for (auto& conn : _connectors) {
            ConnectorInfoReader* c = conn.get();
            if (c->finished())
                  reconnected.insert(static_cast<ConnectorInfoReader*>(c->start()));
            }
      for (ConnectorInfoReader* cptr : reconnected) {
            cptr->addToScore(pasteMode());
            removeConnector(cptr);
            }
      qDebug("reconnected %d broken connectors", reconnected.count());
      }

//---------------------------------------------------------
//   addLink
//---------------------------------------------------------

void XmlReader::addLink(Staff* s, LinkedElements* link)
      {
      int staff = s->idx();
      const bool masterScore = s->score()->isMaster();
      if (!masterScore)
            staff *= -1;

      QList<QPair<LinkedElements*, Location>>& staffLinks = _staffLinkedElements[staff];
      if (!masterScore) {
            if (!staffLinks.empty()
               && (link->mainElement()->score() != staffLinks.front().first->mainElement()->score())
               )
                  staffLinks.clear();
            }

      Location l = location(true);
      _linksIndexer.assignLocalIndex(l);
      staffLinks.push_back(qMakePair(link, l));
      }

//---------------------------------------------------------
//   getLink
//---------------------------------------------------------

LinkedElements* XmlReader::getLink(bool masterScore, const Location& l, int localIndexDiff)
      {
      int staff = l.staff();
      if (!masterScore)
            staff *= -1;
      const int localIndex = _linksIndexer.assignLocalIndex(l) + localIndexDiff;
      QList<QPair<LinkedElements*, Location>>& staffLinks = _staffLinkedElements[staff];

      if (!staffLinks.isEmpty() && staffLinks.constLast().second == l) {
            // This element potentially affects local index for "main"
            // elements that may go afterwards at the same tick, so
            // append it to staffLinks as well.
            staffLinks.push_back(staffLinks.constLast()); // nothing should reference exactly this local index, so it shouldn't matter what to append
            }

      for (int i = 0; i < staffLinks.size(); ++i) {
            if (staffLinks[i].second == l) {
                  if (localIndex == 0)
                        return staffLinks[i].first;
                  i += localIndex;
                  if ((i < 0) || (i >= staffLinks.size()))
                        return nullptr;
                  if (staffLinks[i].second == l)
                        return staffLinks[i].first;
                  return nullptr;
                  }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   assignLocalIndex
//---------------------------------------------------------

int LinksIndexer::assignLocalIndex(const Location& mainElementLocation)
      {
      if (_lastLinkedElementLoc == mainElementLocation)
            return (++_lastLocalIndex);
      _lastLocalIndex = 0;
      _lastLinkedElementLoc = mainElementLocation;
      return 0;
      }

//---------------------------------------------------------
//   rtick
//    return relative position in measure
//---------------------------------------------------------

Fraction XmlReader::rtick() const
      {
      return _curMeasure ? _tick - _curMeasure->tick() : _tick;
      }

//---------------------------------------------------------
//   setTick
//---------------------------------------------------------

void XmlReader::setTick(const Fraction& f)
      {
      _tick = f.reduced();
      _intTick = _tick.ticks();
      }

//---------------------------------------------------------
//   incTick
//---------------------------------------------------------

void XmlReader::incTick(const Fraction& f)
      {
      _tick += f;
      _tick.reduce();
      _intTick += f.ticks();
      }
}


