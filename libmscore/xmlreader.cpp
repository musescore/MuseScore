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
#include "score.h"
#include "spanner.h"
#include "staff.h"
#include "beam.h"
#include "tuplet.h"
#include "sym.h"
#include "note.h"
#include "barline.h"
#include "style.h"

namespace Ms {

//---------------------------------------------------------
//   ~XmlReader
//---------------------------------------------------------

XmlReader::~XmlReader()
      {
      if (!_connectors.isEmpty() || !_pendingConnectors.isEmpty()) {
            qDebug("XmlReader::~XmlReader: there are unpaired connectors left");
            for (ConnectorInfoReader& c : _connectors) {
                  Element* conn = c.releaseConnector();
                  if (conn && !conn->isTuplet()) // tuplets are added to score even when not finished
                        delete conn;
                  }
            for (ConnectorInfoReader& c : _pendingConnectors)
                  delete c.releaseConnector();
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
//   point
//---------------------------------------------------------

PointInfo XmlReader::point(bool forceAbsFrac) const
      {
      PointInfo info = PointInfo::absolute();
      fillPoint(info, forceAbsFrac);
      return info;
      }

//---------------------------------------------------------
//   fillPoint
//    fills point fields which have default values with
//    values relevant for the current reader's position.
//    When in paste mode (or forceAbsFrac is true) absolute
//    fraction values are used and measure number is set to
//    zero.
//---------------------------------------------------------

void XmlReader::fillPoint(PointInfo& p, bool forceAbsFrac) const
      {
      constexpr PointInfo defaults = PointInfo::absolute();
      const bool absFrac = (pasteMode() || forceAbsFrac);
      if (p.track() == defaults.track())
            p.setTrack(track());
      if (p.frac() == defaults.frac())
            p.setFrac(absFrac ? afrac() : rfrac());
      if (p.measure() == defaults.measure())
            p.setMeasure(absFrac ? 0 : currentMeasureIndex());
      }

//---------------------------------------------------------
//   fillPoint
//    sets a new reading point, taking into account its
//    type (absolute or relative).
//---------------------------------------------------------

void XmlReader::setPoint(const PointInfo& p)
      {
      if (p.isRelative()) {
            PointInfo info = p;
            info.toAbsolute(point());
            setPoint(info); // recursion
            return;
            }
      setTrack(p.track() - _trackOffset);
      int tick = p.frac().ticks() - _tickOffset;
      if (!pasteMode()) {
            Q_ASSERT(p.measure() == currentMeasureIndex());
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
      for (;;) {
            QXmlStreamReader::TokenType t = readNext();
            switch(t) {
                  case QXmlStreamReader::StartElement:
                        htmlToString(level, &s);
                        break;
                  case QXmlStreamReader::EndElement:
                        return s;
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

//---------------------------------------------------------
//   addConnectorInfo
//---------------------------------------------------------

void XmlReader::addConnectorInfo(const ConnectorInfoReader& c)
      {
      _connectors.push_back(c);
      ConnectorInfoReader& c1 = _connectors.back();
      c1.update();
      for (ConnectorInfoReader& c2 : _connectors) {
            if (c2.connect(&c1)) {
                  if (c2.finished()) {
                        c2.addToScore(pasteMode());
                        removeConnector(c2);
                        }
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   removeConnectorInfo
//---------------------------------------------------------

void XmlReader::removeConnectorInfo(const ConnectorInfoReader& c)
      {
      _connectors.removeOne(c);
      }

//---------------------------------------------------------
//   removeConnector
//---------------------------------------------------------

void XmlReader::removeConnector(const ConnectorInfoReader& cref)
      {
      const ConnectorInfoReader* c = &cref;
      while (c->prev())
            c = c->prev();
      while (c) {
            removeConnectorInfo(*c);
            c = c->next();
            }
      }

//---------------------------------------------------------
//   addConnectorInfoLater
//---------------------------------------------------------

void XmlReader::addConnectorInfoLater(const ConnectorInfoReader& c)
      {
      _pendingConnectors.push_back(c);
      }

//---------------------------------------------------------
//   checkConnectors
//---------------------------------------------------------

void XmlReader::checkConnectors()
      {
      for (ConnectorInfoReader& c : _pendingConnectors) {
            addConnectorInfo(c);
            }
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
      if (_connectors.isEmpty())
            return;
      qDebug("Reconnecting broken connectors (%d nodes)", _connectors.size());
      QList<QPair<int, QPair<ConnectorInfoReader*, ConnectorInfoReader*>>> brokenPairs;
      for (int i = 1; i < _connectors.size(); ++i) {
            for (int j = 0; j < i; ++j) {
                  ConnectorInfoReader& c1 = _connectors[i];
                  ConnectorInfoReader& c2 = _connectors[j];
                  int d = c1.connectionDistance(c2);
                  if (d >= 0)
                        brokenPairs.append(qMakePair(d, qMakePair(&c1, &c2)));
                  else
                        brokenPairs.append(qMakePair(-d, qMakePair(&c2, &c1)));
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
      for (ConnectorInfoReader& c : _connectors) {
            if (c.finished())
                  reconnected.insert(static_cast<ConnectorInfoReader*>(c.start()));
            }
      for (ConnectorInfoReader* cptr : reconnected) {
            cptr->addToScore(pasteMode());
            removeConnector(*cptr);
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

      QList<QPair<LinkedElements*, PointInfo>>& staffLinks = _staffLinkedElements[staff];
      if (!masterScore) {
            if (!staffLinks.empty()
               && (link->mainElement()->score() != staffLinks.front().first->mainElement()->score())
               )
                  staffLinks.clear();
            }

      PointInfo p = point(true);
      _linksIndexer.assignLocalIndex(p);
      staffLinks.push_back(qMakePair(link, p));
      }

//---------------------------------------------------------
//   getLink
//---------------------------------------------------------

LinkedElements* XmlReader::getLink(bool masterScore, const PointInfo& p, int localIndexDiff)
      {
      int staff = p.staff();
      if (!masterScore)
            staff *= -1;
      const int localIndex = _linksIndexer.assignLocalIndex(p) + localIndexDiff;
      QList<QPair<LinkedElements*, PointInfo>>& staffLinks = _staffLinkedElements[staff];
      for (int i = 0; i < staffLinks.size(); ++i) {
            if (staffLinks[i].second == p) {
                  if (localIndex == 0)
                        return staffLinks[i].first;
                  i += localIndex;
                  if ((i < 0) || (i >= staffLinks.size()))
                        return nullptr;
                  if (staffLinks[i].second == p)
                        return staffLinks[i].first;
                  return nullptr;
                  }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   assignLocalIndex
//---------------------------------------------------------

int LinksIndexer::assignLocalIndex(const PointInfo& mainElementInfo)
      {
      if (_lastLinkedElementInfo == mainElementInfo)
            return (++_lastLocalIndex);
      _lastLocalIndex = 0;
      _lastLinkedElementInfo = mainElementInfo;
      return 0;
      }
}


