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

#include "chord.h"
#include "element.h"
#include "mscore.h"
#include "scoreElement.h"
#include "xml.h"

namespace Ms {

static constexpr ConnectorPointInfo defaults { 0, 320, 0 };

//---------------------------------------------------------
//   ConnectorInfo
//---------------------------------------------------------

ConnectorInfo::ConnectorInfo(int track, const Element* current)
      {
      if (!current)
            qFatal("ConnectorInfo::ConnectorInfo(): invalid argument: %p", current);
      updatePointInfo(current, _currentInfo, current->isNote());
      // It is not always possible to determine the track number correctly from
      // the current element (for example, in case of a Segment).
      // If the caller does not know the track number and passes -1
      // it may be corrected later.
      _currentInfo.track = track;
      }

//---------------------------------------------------------
//   ConnectorInfoReader
//---------------------------------------------------------

ConnectorInfoReader::ConnectorInfoReader(int track, Element* current)
   : ConnectorInfo(track, current), _connector(nullptr), _current(current)
      {}

//---------------------------------------------------------
//   ConnectorInfoWriter
//---------------------------------------------------------

ConnectorInfoWriter::ConnectorInfoWriter(int track, const Element* current, const Element* connector)
   : ConnectorInfo(track, current), _connector(connector), _current(current)
      {
      if (!connector) {
            qFatal("ConnectorInfoWriter::ConnectorInfoWriter(): invalid arguments: %p, %p", connector, current);
            return;
            }
      _type = connector->type();
      }

//---------------------------------------------------------
//   ConnectorInfo::updatePointInfo
//---------------------------------------------------------

void ConnectorInfo::updatePointInfo(const Element* e, ConnectorPointInfo& i, bool needNote)
      {
      i.track = e->track();
      i.tick = e->tick();
      if (needNote) {
            if (!e->isNote()) {
                  qWarning("ConnectorInfo::updatePointInfo(): element is not a note");
                  return;
                  }
            const Note* n = toNote(e);
            const std::vector<Note*>& notes = n->chord()->notes();
            if (notes.size() == 1)
                  i.note = 0;
            else {
                  int noteIdx;
                  for (noteIdx = 0; noteIdx < int(notes.size()); ++noteIdx) {
                        if (n == notes.at(noteIdx))
                              break;
                        }
                  }
            }
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
//   ConnectorInfoWriter::write
//---------------------------------------------------------

void ConnectorInfoWriter::write(XmlWriter& xml) const
      {
      xml.stag(QString("%1 type=\"%2\"").arg(tagName()).arg(_connector->name()));
      if (isStart())
            _connector->write(xml);
      if (hasPrevious()) {
            xml.stag("prev");
            writeDestinationInfo(xml, _prevInfo);
            xml.etag();
            }
      if (hasNext()) {
            xml.stag("next");
            writeDestinationInfo(xml, _nextInfo);
            xml.etag();
            }
      xml.etag();
      }

//---------------------------------------------------------
//   ConnectorInfoReader::read
//---------------------------------------------------------

bool ConnectorInfoReader::read(XmlReader& e)
      {
      const QString name(e.attribute("type"));
      _type = ScoreElement::name2type(&name);

      // Correct the current point info if needed
      if (_currentInfo.track == -1)
            _currentInfo.track = e.track();
      _currentInfo.tick = e.tick();

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "prev")
                  readDestinationInfo(e, _prevInfo);
            else if (tag == "next")
                  readDestinationInfo(e, _nextInfo);
            else {
                  _connector = Element::name2Element(tag, _current->score());
                  if (!_connector) {
                        e.unknown();
                        return false;
                        }
                  _connector->setTrack(_currentInfo.track);
                  _connector->read(e);
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   ConnectorInfoWriter::writeDestinationInfo
//---------------------------------------------------------

void ConnectorInfoWriter::writeDestinationInfo(XmlWriter& xml, const ConnectorPointInfo& info) const
      {
      if (MScore::debugMode) {
            xml.comment(QString("tick: current=%1 dest=%2").arg(_currentInfo.tick).arg(info.tick));
            }
      xml.tag("dtrack", info.track - _currentInfo.track, defaults.track);
      xml.tag("dtick", info.tick - _currentInfo.tick, defaults.tick);
      xml.tag("dnote", info.note - _currentInfo.note, defaults.note);
      }

//---------------------------------------------------------
//   ConnectorInfoReader::readDestinationInfo
//---------------------------------------------------------

void ConnectorInfoReader::readDestinationInfo(XmlReader& e, ConnectorPointInfo& info)
      {
      info.track = _currentInfo.track + defaults.track;
      info.tick = _currentInfo.tick + defaults.tick;
      info.note = _currentInfo.note + defaults.note;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "dtrack")
                  info.track = _currentInfo.track + e.readInt();
            else if (tag == "dtick")
                  info.tick = _currentInfo.tick + e.readInt();
            else if (tag == "dnote")
                  info.note = _currentInfo.note + e.readInt();
            }
      }

//---------------------------------------------------------
//   ConnectorInfoReader::addToScore
//---------------------------------------------------------

void ConnectorInfoReader::addToScore(bool pasteMode)
      {
      ConnectorInfoReader* r = this;
      while (r->prev())
            r = prev();
      while (r) {
            r->_current->readAddConnector(r, pasteMode);
            r = r->next();
            }
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
//   ConnectorInfoReader::operator==
//---------------------------------------------------------

bool ConnectorInfoReader::operator==(const ConnectorInfoReader& other) const {
      if (this == &other)
            return true;
      if ((_type != other._type)
         || (_current != other._current)
         || (connector() != other.connector())
         )
            return false;
      return true;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool operator==(const ConnectorPointInfo& cpi1, const ConnectorPointInfo& cpi2) {
      return ((cpi1.tick == cpi2.tick)
             && (cpi1.track == cpi2.track)
             && (cpi1.note == cpi2.note)
             );
      }

}

