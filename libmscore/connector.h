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

#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "fraction.h"
#include "types.h"

#include <climits>

namespace Ms {

class Element;
class Score;
class ScoreElement;
class XmlReader;
class XmlWriter;

struct ConnectorPointInfo {
      int track         = INT_MIN;
      int measure       = INT_MIN;
      Fraction fpos     = INT_MIN;
      int graceIndex    = INT_MIN;
      int note          = 0;

      constexpr ConnectorPointInfo() = default;
      constexpr ConnectorPointInfo(int track, int measure, Fraction fpos, int graceIndex, int note) : track(track), measure(measure), fpos(fpos), graceIndex(graceIndex), note(note) {}
      };

bool operator==(const ConnectorPointInfo& cpi1, const ConnectorPointInfo& cpi2);
inline bool operator!=(const ConnectorPointInfo& cpi1, const ConnectorPointInfo& cpi2) { return !(cpi1 == cpi2); }

//---------------------------------------------------------
//   @@ ConnectorInfo
///    Stores a general information on various connecting
///    elements (spanners, beams, tuplets) including their
///    endpoints locations.
///    Base class of helper classes used to read and write
///    such elements.
//---------------------------------------------------------

class ConnectorInfo {
      const Element* _current    { 0      };
      bool _currentUpdated { false };

      bool finishedLeft() const;
      bool finishedRight() const;

   protected:
      ElementType _type       { ElementType::INVALID };
      ConnectorPointInfo _prevInfo;
      ConnectorPointInfo _nextInfo;
      ConnectorPointInfo _currentInfo;

      ConnectorInfo* _prev    { 0 };
      ConnectorInfo* _next    { 0 };

      void updatePointInfo(const Element* e, ConnectorPointInfo& i, bool clipboardmode);
      void updateCurrentInfo(bool clipboardmode);
      bool currentUpdated() const         { return _currentUpdated; }
      void setCurrentUpdated(bool v)      { _currentUpdated = v;    }

   public:
      ConnectorInfo(const Element* current, int track = -1, Fraction fpos = -1);
      ConnectorInfo(const ConnectorPointInfo& currentInfo);

      ConnectorInfo* prev() const   { return _prev; }
      ConnectorInfo* next() const   { return _next; }

      ElementType type() const { return _type; }
      const ConnectorPointInfo& info() const { return _currentInfo; }

      bool connect(ConnectorInfo* other);
      bool finished() const;

      bool hasPrevious() const      { return (_prevInfo.measure != INT_MIN); }
      bool hasNext() const          { return (_nextInfo.measure != INT_MIN); }
      bool isStart() const          { return (!hasPrevious() && hasNext()); }
      bool isMiddle() const         { return (hasPrevious() && hasNext());  }
      bool isEnd() const            { return (hasPrevious() && !hasNext()); }
      };

//---------------------------------------------------------
//   @@ ConnectorInfoReader
///    Helper class for reading beams, tuplets and spanners.
//---------------------------------------------------------

class ConnectorInfoReader final : public ConnectorInfo {
      XmlReader* _reader;
      Element* _connector;
      Element* _currentElement;
      ScoreElement* _connectorReceiver;

      void readDestinationInfo(XmlReader& e, ConnectorPointInfo& info);
      void convertRelToAbs(ConnectorPointInfo& info);

   public:
      ConnectorInfoReader(XmlReader& e, Element* current, int track = -1);
      ConnectorInfoReader(XmlReader& e, Score* current, int track = -1);

      ConnectorInfoReader* prev() const   { return static_cast<ConnectorInfoReader*>(_prev); }
      ConnectorInfoReader* next() const   { return static_cast<ConnectorInfoReader*>(_next); }

      Element* connector();
      const Element* connector() const;
      Element* releaseConnector(); // returns connector and "forgets" it by
                                   // setting an internal pointer to it to zero

      bool read();
      void update();
      void addToScore(bool pasteMode);

      static void readConnector(ConnectorInfoReader& info, XmlReader& e);

      bool operator==(const ConnectorInfoReader& other) const;
      };

//---------------------------------------------------------
//   @@ ConnectorInfoWriter
///    Helper class for writing beams, tuplets and spanners.
///    Subclasses should fill _prevInfo and _nextInfo with
///    the proper information on the connector's endpoints.
//---------------------------------------------------------

class ConnectorInfoWriter : public ConnectorInfo {
      XmlWriter* _xml;

      void writeDestinationInfo(XmlWriter& xml, const ConnectorPointInfo& info) const;

   protected:
      const Element* _connector;

      virtual const char* tagName() const = 0;

   public:
      ConnectorInfoWriter(XmlWriter& xml, const Element* current, const Element* connector, int track = -1, Fraction fpos = -1);

      ConnectorInfoWriter* prev() const   { return static_cast<ConnectorInfoWriter*>(_prev); }
      ConnectorInfoWriter* next() const   { return static_cast<ConnectorInfoWriter*>(_next); }

      const Element* connector() const    { return _connector; }

      void write();
      };

}     // namespace Ms
#endif
