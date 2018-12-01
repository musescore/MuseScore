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
   : _current(current), _currentLoc(Location::absolute())
      {
      if (!current)
            qFatal("ConnectorInfo::ConnectorInfo(): invalid argument: %p", current);
      // It is not always possible to determine the track number correctly from
      // the current element (for example, in case of a Segment).
      // If the caller does not know the track number and passes -1
      // it may be corrected later.
      if (track >= 0)
            _currentLoc.setTrack(track);
      if (frac >= 0)
            _currentLoc.setFrac(frac);
      }

//---------------------------------------------------------
//   ConnectorInfo
//---------------------------------------------------------

ConnectorInfo::ConnectorInfo(const Location& currentLocation)
   : _currentLoc(currentLocation)
      {}

//---------------------------------------------------------
//   ConnectorInfo::updateLocation
//---------------------------------------------------------

void ConnectorInfo::updateLocation(const Element* e, Location& l, bool clipboardmode)
      {
      l.fillForElement(e, clipboardmode);
      }

//---------------------------------------------------------
//   ConnectorInfo::updateCurrentInfo
//---------------------------------------------------------

void ConnectorInfo::updateCurrentInfo(bool clipboardmode) {
      if (!currentUpdated() && _current)
            updateLocation(_current, _currentLoc, clipboardmode);
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
      if (hasPrevious() && _prev == nullptr
         && other->hasNext() && other->_next == nullptr
         ) {
            if ((_prevLoc == other->_currentLoc)
               && (_currentLoc == other->_nextLoc)
               ) {
                  _prev = other;
                  other->_next = this;
                  return true;
                  }
            }
      if (hasNext() && _next == nullptr
         && other->hasPrevious() && other->_prev == nullptr
         ) {
            if ((_nextLoc == other->_currentLoc)
               && (_currentLoc == other->_prevLoc)
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

static int distance(const Location& l1, const Location& l2)
      {
      constexpr int commonDenominator = 1000;
      Fraction dfrac = (l2.frac() - l1.frac()).absValue();
      int dpos = dfrac.numerator() * commonDenominator / dfrac.denominator();
      dpos += 10000 * qAbs(l2.measure() - l1.measure());
      return 1000 * dpos + 100 * qAbs(l2.track() - l1.track()) + 10 * qAbs(l2.note() - l1.note()) + qAbs(l2.graceIndex() - l1.graceIndex());
      }

//---------------------------------------------------------
//   ConnectorInfo::orderedConnectionDistance
//---------------------------------------------------------

int ConnectorInfo::orderedConnectionDistance(const ConnectorInfo& c1, const ConnectorInfo& c2)
      {
      Location c1Next = c1._nextLoc;
      c1Next.toRelative(c1._currentLoc);
      Location c2Prev = c2._currentLoc; // inversed order to get equal signs
      c2Prev.toRelative(c2._prevLoc);
      if (c1Next == c2Prev)
            return distance(c1._nextLoc, c2._currentLoc);
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
      if (hasNext() && _next == nullptr
         && other.hasPrevious() && other._prev == nullptr)
            distThisOther = orderedConnectionDistance(*this, other);
      if (hasPrevious() && _prev == nullptr
         && other.hasNext() && other._next == nullptr)
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

static Location readPositionInfo(const XmlReader& e, int track) {
      Location info = e.location();
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
      if (!xml.canWrite(_connector))
            return;
      xml.stag(QString("%1 type=\"%2\"").arg(tagName()).arg(_connector->name()));
      if (isStart())
            _connector->write(xml);
      if (hasPrevious()) {
            xml.stag("prev");
            _prevLoc.toRelative(_currentLoc);
            _prevLoc.write(xml);
            xml.etag();
            }
      if (hasNext()) {
            xml.stag("next");
            _nextLoc.toRelative(_currentLoc);
            _nextLoc.write(xml);
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

      e.fillLocation(_currentLoc);

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "prev")
                  readEndpointLocation(_prevLoc);
            else if (tag == "next")
                  readEndpointLocation(_nextLoc);
            else {
                  if (tag == name)
                        _connector = Element::name2Element(tag, _connectorReceiver->score());
                  else
                        qWarning("ConnectorInfoReader::read: element tag (%s) does not match connector type (%s). Is the file corrupted?", tag.toLatin1().constData(), name.toLatin1().constData());

                  if (!_connector) {
                        e.unknown();
                        return false;
                        }
                  _connector->setTrack(_currentLoc.track());
                  _connector->read(e);
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   ConnectorInfoReader::readEndpointLocation
//---------------------------------------------------------

void ConnectorInfoReader::readEndpointLocation(Location& l)
      {
      XmlReader& e = *_reader;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "location") {
                  l = Location::relative();
                  l.read(e);
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
            _prevLoc.toAbsolute(_currentLoc);
      if (hasNext())
            _nextLoc.toAbsolute(_currentLoc);
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

void ConnectorInfoReader::readConnector(std::unique_ptr<ConnectorInfoReader> info, XmlReader& e)
      {
      if (!info->read()) {
            e.skipCurrentElement();
            return;
            }
      e.addConnectorInfoLater(std::move(info));
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

}

