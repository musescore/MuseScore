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

#include "location.h"
#include "types.h"

namespace Ms {

class Element;
class Score;
class ScoreElement;
class XmlReader;
class XmlWriter;

//---------------------------------------------------------
//   @@ ConnectorInfo
///    Stores a general information on various connecting
///    elements (currently only spanners) including their
///    endpoints locations.
///    Base class of helper classes used to read and write
///    such elements.
//---------------------------------------------------------

class ConnectorInfo {
      const Element* _current    { 0      };
      bool _currentUpdated       { false  };

      bool finishedLeft() const;
      bool finishedRight() const;

      static int orderedConnectionDistance(const ConnectorInfo& c1, const ConnectorInfo& c2);

   protected:
      ElementType _type       { ElementType::INVALID };
      Location _currentLoc;
      Location _prevLoc       { Location::absolute() };
      Location _nextLoc       { Location::absolute() };

      ConnectorInfo* _prev    { 0 };
      ConnectorInfo* _next    { 0 };

      void updateLocation(const Element* e, Location& i, bool clipboardmode);
      void updateCurrentInfo(bool clipboardmode);
      bool currentUpdated() const         { return _currentUpdated; }
      void setCurrentUpdated(bool v)      { _currentUpdated = v;    }

   public:
      ConnectorInfo(const Element* current, int track = -1, Fraction frac = -1);
      ConnectorInfo(const Location& currentLocation);

      ConnectorInfo* prev() const   { return _prev; }
      ConnectorInfo* next() const   { return _next; }
      ConnectorInfo* start();
      ConnectorInfo* end();

      ElementType type() const { return _type; }
      const Location& location() const { return _currentLoc; }

      bool connect(ConnectorInfo* other);
      bool finished() const;

      // for reconnection of broken connectors
      int connectionDistance(const ConnectorInfo& c2) const;
      void forceConnect(ConnectorInfo* c2);

      bool hasPrevious() const      { return (_prevLoc.measure() != INT_MIN); }
      bool hasNext() const          { return (_nextLoc.measure() != INT_MIN); }
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

      void readEndpointLocation(Location& l);

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
///    Helper class for writing connecting elements.
///    Subclasses should fill _prevInfo and _nextInfo with
///    the proper information on the connector's endpoints.
//---------------------------------------------------------

class ConnectorInfoWriter : public ConnectorInfo {
      XmlWriter* _xml;

   protected:
      const Element* _connector;

      virtual const char* tagName() const = 0;

   public:
      ConnectorInfoWriter(XmlWriter& xml, const Element* current, const Element* connector, int track = -1, Fraction frac = -1);

      ConnectorInfoWriter* prev() const   { return static_cast<ConnectorInfoWriter*>(_prev); }
      ConnectorInfoWriter* next() const   { return static_cast<ConnectorInfoWriter*>(_next); }

      const Element* connector() const    { return _connector; }

      void write();
      };

}     // namespace Ms
#endif
