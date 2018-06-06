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
#include "measure.h"
#include "mscore.h"
#include "score.h"
#include "scoreElement.h"
#include "xml.h"

namespace Ms {

static constexpr ConnectorPointInfo pointDefaults;
static constexpr ConnectorPointInfo writeDefaults(0, 0, 0, INT_MIN, 0);

//---------------------------------------------------------
//   ConnectorInfo
//---------------------------------------------------------

ConnectorInfo::ConnectorInfo(const Element* current, int track, Fraction fpos)
   : _current(current)
      {
      if (!current)
            qFatal("ConnectorInfo::ConnectorInfo(): invalid argument: %p", current);
      // It is not always possible to determine the track number correctly from
      // the current element (for example, in case of a Segment).
      // If the caller does not know the track number and passes -1
      // it may be corrected later.
      if (track >= 0)
            _currentInfo.track = track;
      if (fpos >= 0)
            _currentInfo.fpos = fpos;
      }

//---------------------------------------------------------
//   ConnectorInfo
//---------------------------------------------------------

ConnectorInfo::ConnectorInfo(const ConnectorPointInfo& currentInfo)
      {
      _currentInfo = currentInfo;
      }

//---------------------------------------------------------
//   ConnectorInfo::updatePointInfo
//---------------------------------------------------------

void ConnectorInfo::updatePointInfo(const Element* e, ConnectorPointInfo& i, bool clipboardmode)
      {
      if (!e) {
            qWarning("ConnectorInfo::updatePointInfo: element is nullptr");
            return;
      }
      if (i.track == pointDefaults.track)
            i.track = e->track();
      if (i.fpos == pointDefaults.fpos)
            i.fpos = clipboardmode ? e->absfpos() : e->fpos();
      if (i.measure == pointDefaults.measure) {
            if (clipboardmode)
                  i.measure = 0;
            else {
                  const Measure* m = toMeasure(e->findMeasure());
                  if (m)
                        i.measure = m->index();
                  else {
                        qWarning("ConnectorInfo:updatePointInfo: cannot find element's measure (%s)", e->name());
                        i.measure = 0;
                        }
                  }
            }

      if (e->isChord() || (e->parent() && e->parent()->isChord())) {
            const Chord* ch = e->isChord() ? toChord(e) : toChord(e->parent());
            if (ch->isGrace())
                  i.graceIndex = ch->graceIndex();
            }
      if (e->isNote()) {
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
                  i.note = noteIdx;
                  }
            }
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
//   ConnectorInfoReader
//---------------------------------------------------------

ConnectorInfoReader::ConnectorInfoReader(XmlReader& e, Element* current, int track)
   : ConnectorInfo(current, track), _reader(&e), _connector(nullptr), _currentElement(current), _connectorReceiver(current)
      {}

//---------------------------------------------------------
//   readPositionInfo
//---------------------------------------------------------

static ConnectorPointInfo readPositionInfo(const XmlReader& e, int track) {
      ConnectorPointInfo info;
      info.track = track;
      info.measure = e.pasteMode() ? 0 : e.currentMeasure()->index();
      info.fpos = e.pasteMode() ? Fraction::fromTicks(e.tick()) : Fraction::fromTicks(e.tick() - e.currentMeasure()->tick());
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

ConnectorInfoWriter::ConnectorInfoWriter(XmlWriter& xml, const Element* current, const Element* connector, int track, Fraction fpos)
   : ConnectorInfo(current, track, fpos), _xml(&xml), _connector(connector)
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

bool ConnectorInfoReader::read()
      {
      XmlReader& e = *_reader;
      const QString name(e.attribute("type"));
      _type = ScoreElement::name2type(&name);

      if (_currentInfo.track == pointDefaults.track)
            _currentInfo.track = e.track();
      if (_currentInfo.fpos == pointDefaults.fpos)
            _currentInfo.fpos = e.pasteMode() ? e.absfpos() : e.fpos();
      _currentInfo.measure = e.pasteMode() ? 0 : e.currentMeasure()->index();

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "prev")
                  readDestinationInfo(e, _prevInfo);
            else if (tag == "next")
                  readDestinationInfo(e, _nextInfo);
            else {
                  if (tag == name)
                        _connector = Element::name2Element(tag, _connectorReceiver->score());
                  else
                        qWarning("ConnectorInfoReader::read: element tag (%s) does not match connector type (%s). Is the file corrupted?", tag.toLatin1().constData(), name.toLatin1().constData());

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
            xml.comment(QString("current: measure=%1 fpos=%2/%3").arg(_currentInfo.measure).arg(_currentInfo.fpos.numerator()).arg(_currentInfo.fpos.denominator()));
            xml.comment(QString("dest: measure=%1 fpos=%2/%3").arg(info.measure).arg(info.fpos.numerator()).arg(info.fpos.denominator()));
            }
      const int currentStaff = _currentInfo.track / VOICES;
      const int currentVoice = _currentInfo.track % VOICES;
      const int destStaff = info.track / VOICES;
      const int destVoice = info.track % VOICES;

      static_assert(writeDefaults.track == 0, "Defaults for dstaff and dvoice correspond to writeDefaults.track == 0");
      xml.tag("dstaff", destStaff - currentStaff, 0);
      xml.tag("dvoice", destVoice - currentVoice, 0);
      xml.tag("dmeasure", info.measure - _currentInfo.measure, writeDefaults.measure);
      xml.tag("dpos", info.fpos - _currentInfo.fpos, writeDefaults.fpos);
      xml.tag("grace", info.graceIndex, writeDefaults.graceIndex);
      xml.tag("dnote", info.note - _currentInfo.note, writeDefaults.note);
      }

//---------------------------------------------------------
//   ConnectorInfoReader::readDestinationInfo
//---------------------------------------------------------

void ConnectorInfoReader::readDestinationInfo(XmlReader& e, ConnectorPointInfo& info)
      {
      info = writeDefaults;
      static_assert(writeDefaults.track == 0, "writeDefaults.track == 0 is assumed when reading the staff and voices info");

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "dstaff")
                  info.track += e.readInt() * VOICES;
            else if (tag == "dvoice")
                  info.track += e.readInt();
            else if (tag == "dmeasure")
                  info.measure = e.readInt();
            else if (tag == "dpos")
                  info.fpos = e.readFraction();
            else if (tag == "grace")
                  info.graceIndex = e.readInt();
            else if (tag == "dnote")
                  info.note = e.readInt();
            }
      }

//---------------------------------------------------------
//   ConnectorInfoReader::convertRelToAbs
//---------------------------------------------------------

void ConnectorInfoReader::convertRelToAbs(ConnectorPointInfo& info)
      {
      info.track += _currentInfo.track;
      info.measure += _currentInfo.measure;
      info.fpos += _currentInfo.fpos;
      info.note += _currentInfo.note;
      }

//---------------------------------------------------------
//   ConnectorInfoReader::update
//---------------------------------------------------------

void ConnectorInfoReader::update()
      {
      if (currentUpdated())
            return;
      updateCurrentInfo(_reader->pasteMode());
      if (hasPrevious())
            convertRelToAbs(_prevInfo);
      if (hasNext())
            convertRelToAbs(_nextInfo);
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

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool operator==(const ConnectorPointInfo& cpi1, const ConnectorPointInfo& cpi2) {
      return ((cpi1.fpos == cpi2.fpos)
             && (cpi1.measure == cpi2.measure)
             && (cpi1.track == cpi2.track)
             && (cpi1.graceIndex == cpi2.graceIndex)
             && (cpi1.note == cpi2.note)
             );
      }

}

