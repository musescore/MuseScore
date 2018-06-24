//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "connector.h"

#include "element.h"
#include "score.h"
#include "scoreElement.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   ConnectorInfo
//---------------------------------------------------------

ConnectorInfo::ConnectorInfo(const Element* current, int track, Fraction frac)
   : _current(current), _currentInfo(PointInfo::absolute())
      {
      if (!current)
            qFatal("ConnectorInfo::ConnectorInfo(): invalid argument: %p", current);
      // It is not always possible to determine the track number correctly from
      // the current element (for example, in case of a Segment).
      // If the caller does not know the track number and passes -1
      // it may be corrected later.
      if (track >= 0)
            _currentInfo.setTrack(track);
      if (frac >= 0)
            _currentInfo.setFrac(frac);
      }

//---------------------------------------------------------
//   ConnectorInfo
//---------------------------------------------------------

ConnectorInfo::ConnectorInfo(const PointInfo& currentInfo)
   : _currentInfo(currentInfo)
      {}

//---------------------------------------------------------
//   ConnectorInfo::updatePointInfo
//---------------------------------------------------------

void ConnectorInfo::updatePointInfo(const Element* e, PointInfo& i, bool clipboardmode)
      {
      i.fillForElement(e, clipboardmode);
      }

//---------------------------------------------------------
//   ConnectorInfo::updateCurrentInfo
//---------------------------------------------------------

void ConnectorInfo::updateCurrentInfo(bool clipboardmode) {
      if (!currentUpdated() && _current)
            updatePointInfo(_current, _currentInfo, clipboardmode);
      setCurrentUpdated(true);
}

//---------------------------------------------------------
//   ConnectorInfo::connect
//---------------------------------------------------------

bool ConnectorInfo::connect(ConnectorInfo* other)
      {
      if (!other || (this == other))
            return false;
      if (_type != other->_type)
            return false;
      if (!_prev && hasPrevious() && !other->_next && other->hasNext()) {
            if ((_prevInfo == other->_currentInfo)
               && (_currentInfo == other->_nextInfo)
               ) {
                  _prev = other;
                  other->_next = this;
                  return true;
                  }
            }
      if (!_next && hasNext() && !other->_prev && other->hasPrevious()) {
            if ((_nextInfo == other->_currentInfo)
               && (_currentInfo == other->_prevInfo)
               ) {
                  _next = other;
                  other->_prev = this;
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   ConnectorInfo::forceConnect
//---------------------------------------------------------

void ConnectorInfo::forceConnect(ConnectorInfo* other)
      {
      if (!other || (this == other))
            return;
      _next = other;
      other->_prev = this;
      }

//---------------------------------------------------------
//   distance
//---------------------------------------------------------

static int distance(const PointInfo& p1, const PointInfo& p2)
      {
      constexpr int commonDenominator = 1000;
      Fraction dfrac = (p2.frac() - p1.frac()).absValue();
      int dpos = dfrac.numerator() * commonDenominator / dfrac.denominator();
      dpos += 10000 * qAbs(p2.measure() - p1.measure());
      return 1000 * dpos + 100 * qAbs(p2.track() - p1.track()) + 10 * qAbs(p2.note() - p1.note()) + qAbs(p2.graceIndex() - p1.graceIndex());
      }

//---------------------------------------------------------
//   ConnectorInfo::orderedConnectionDistance
//---------------------------------------------------------

int ConnectorInfo::orderedConnectionDistance(const ConnectorInfo& c1, const ConnectorInfo& c2)
      {
      PointInfo c1Next = c1._nextInfo;
      c1Next.toRelative(c1._currentInfo);
      PointInfo c2Prev = c2._currentInfo; // inversed order to get equal signs
      c2Prev.toRelative(c2._prevInfo);
      if (c1Next == c2Prev)
            return distance(c1._nextInfo, c2._currentInfo);
      return INT_MAX;
      }

//---------------------------------------------------------
//   ConnectorInfo::connectionDistance
//    Returns a "distance" representing a likelihood of
//    that the checked connectors should be connected.
//    Returns 0 if can be readily connected via connect(),
//    < 0 if other is likely to be the first,
//    INT_MAX if cannot be connected
//---------------------------------------------------------

int ConnectorInfo::connectionDistance(const ConnectorInfo& other) const
      {
      if (_type != other._type)
            return INT_MAX;
      int distThisOther = INT_MAX;
      int distOtherThis = INT_MAX;
      if (!_next && hasNext() && !other._prev && other.hasPrevious())
            distThisOther = orderedConnectionDistance(*this, other);
      if (!_prev && hasPrevious() && !other._next && other.hasNext())
            distOtherThis = orderedConnectionDistance(other, *this);
      if (distOtherThis < distThisOther)
            return -distOtherThis;
      return distThisOther;
      }

//---------------------------------------------------------
//   ConnectorInfo::finished
//---------------------------------------------------------

bool ConnectorInfo::finished() const
      {
      return (finishedLeft() && finishedRight());
      }

//---------------------------------------------------------
//   ConnectorInfo::finishedLeft
//---------------------------------------------------------

bool ConnectorInfo::finishedLeft() const
      {
      const ConnectorInfo* i = this;
      while (i->_prev)
            i = i->_prev;
      return (!i->hasPrevious());
      }

//---------------------------------------------------------
//   ConnectorInfo::finishedRight
//---------------------------------------------------------

bool ConnectorInfo::finishedRight() const
      {
      const ConnectorInfo* i = this;
      while (i->_next)
            i = i->_next;
      return (!i->hasNext());
      }

//---------------------------------------------------------
//   ConnectorInfo::start
//---------------------------------------------------------

ConnectorInfo* ConnectorInfo::start()
      {
      ConnectorInfo* i = this;
      while (i->_prev)
            i = i->_prev;
      if (i->hasPrevious())
            return nullptr;
      return i;
      }

//---------------------------------------------------------
//   ConnectorInfo::end
//---------------------------------------------------------

ConnectorInfo* ConnectorInfo::end()
      {
      ConnectorInfo* i = this;
      while (i->_next)
            i = i->_next;
      if (i->hasNext())
            return nullptr;
      return i;
      }

//---------------------------------------------------------
//   ConnectorInfoReader
//---------------------------------------------------------

ConnectorInfoReader::ConnectorInfoReader(XmlReader& e, Element* current, int track)
   : ConnectorInfo(current, track), _reader(&e), _connector(nullptr), _currentElement(current), _connectorReceiver(current)
      {}

//---------------------------------------------------------
//   readPositionInfo
//---------------------------------------------------------

static PointInfo readPositionInfo(const XmlReader& e, int track) {
      PointInfo info = e.point();
      info.setTrack(track);
      return info;
      }

//---------------------------------------------------------
//   ConnectorInfoReader
//---------------------------------------------------------

ConnectorInfoReader::ConnectorInfoReader(XmlReader& e, Score* current, int track)
   : ConnectorInfo(readPositionInfo(e, track)), _reader(&e), _connector(nullptr), _currentElement(nullptr), _connectorReceiver(current)
      {
      setCurrentUpdated(true);
      }

//---------------------------------------------------------
//   ConnectorInfoWriter
//---------------------------------------------------------

ConnectorInfoWriter::ConnectorInfoWriter(XmlWriter& xml, const Element* current, const Element* connector, int track, Fraction frac)
   : ConnectorInfo(current, track, frac), _xml(&xml), _connector(connector)
      {
      if (!connector) {
            qFatal("ConnectorInfoWriter::ConnectorInfoWriter(): invalid arguments: %p, %p", connector, current);
            return;
            }
      _type = connector->type();
      updateCurrentInfo(xml.clipboardmode());
      }

//---------------------------------------------------------
//   ConnectorInfoWriter::write
//---------------------------------------------------------

void ConnectorInfoWriter::write()
      {
      XmlWriter& xml = *_xml;
      xml.stag(QString("%1 type=\"%2\"").arg(tagName()).arg(_connector->name()));
      if (isStart())
            _connector->write(xml);
      if (hasPrevious()) {
            xml.stag("prev");
            _prevInfo.toRelative(_currentInfo);
            _prevInfo.write(xml);
            xml.etag();
            }
      if (hasNext()) {
            xml.stag("next");
            _nextInfo.toRelative(_currentInfo);
            _nextInfo.write(xml);
            xml.etag();
            }
      xml.etag();
      }

//---------------------------------------------------------
//   ConnectorInfoReader::read
//---------------------------------------------------------

bool ConnectorInfoReader::read()
      {
      XmlReader& e = *_reader;
      const QString name(e.attribute("type"));
      _type = ScoreElement::name2type(&name);

      e.fillPoint(_currentInfo);

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "prev")
                  readDestinationInfo(_prevInfo);
            else if (tag == "next")
                  readDestinationInfo(_nextInfo);
            else {
                  if (tag == name)
                        _connector = Element::name2Element(tag, _connectorReceiver->score());
                  else
                        qWarning("ConnectorInfoReader::read: element tag (%s) does not match connector type (%s). Is the file corrupted?", tag.toLatin1().constData(), name.toLatin1().constData());

                  if (!_connector) {
                        e.unknown();
                        return false;
                        }
                  _connector->setTrack(_currentInfo.track());
                  _connector->read(e);
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   ConnectorInfoReader::readDestinationInfo
//---------------------------------------------------------

void ConnectorInfoReader::readDestinationInfo(PointInfo& info)
      {
      XmlReader& e = *_reader;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "move") {
                  info = PointInfo::relative();
                  info.read(e);
                  }
            else if (tag == "move_abs") {
                  info = PointInfo::absolute();
                  info.read(e);
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   ConnectorInfoReader::update
//---------------------------------------------------------

void ConnectorInfoReader::update()
      {
      if (!currentUpdated())
            updateCurrentInfo(_reader->pasteMode());
      if (hasPrevious())
            _prevInfo.toAbsolute(_currentInfo);
      if (hasNext())
            _nextInfo.toAbsolute(_currentInfo);
      }

//---------------------------------------------------------
//   ConnectorInfoReader::addToScore
//---------------------------------------------------------

void ConnectorInfoReader::addToScore(bool pasteMode)
      {
      ConnectorInfoReader* r = this;
      while (r->prev())
            r = r->prev();
      while (r) {
            r->_connectorReceiver->readAddConnector(r, pasteMode);
            r = r->next();
            }
      }

//---------------------------------------------------------
//   ConnectorInfoReader::readConnector
//---------------------------------------------------------

void ConnectorInfoReader::readConnector(ConnectorInfoReader& info, XmlReader& e)
      {
      if (!info.read()) {
            e.skipCurrentElement();
            return;
            }
      e.addConnectorInfoLater(info);
      }

//---------------------------------------------------------
//   ConnectorInfoReader::connector
//---------------------------------------------------------

Element* ConnectorInfoReader::connector()
      {
      if (_connector)
            return _connector;
      if (prev())
            return prev()->connector();
      return nullptr;
      }

//---------------------------------------------------------
//   ConnectorInfoReader::connector
//---------------------------------------------------------

const Element* ConnectorInfoReader::connector() const
      {
      return const_cast<ConnectorInfoReader*>(this)->connector();
      }

//---------------------------------------------------------
//   ConnectorInfoReader::releaseConnector
//---------------------------------------------------------

Element* ConnectorInfoReader::releaseConnector()
      {
      Element* c = _connector;
      _connector = nullptr;
      if (prev())
            return prev()->releaseConnector();
      return c;
      }

//---------------------------------------------------------
//   ConnectorInfoReader::operator==
//---------------------------------------------------------

bool ConnectorInfoReader::operator==(const ConnectorInfoReader& other) const {
      if (this == &other)
            return true;
      if ((_type != other._type)
         || (_connectorReceiver != other._connectorReceiver)
         || (connector() != other.connector())
         || (_currentInfo != other._currentInfo)
         )
            return false;
      return true;
      }

}

