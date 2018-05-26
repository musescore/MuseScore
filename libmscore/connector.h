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

#include "types.h"

namespace Ms {

class Element;
class XmlReader;
class XmlWriter;

struct ConnectorPointInfo {
      int track;
      int tick;
      int note;
      };

bool operator==(const ConnectorPointInfo& cpi1, const ConnectorPointInfo& cpi2);

//---------------------------------------------------------
//   @@ ConnectorInfo
///    Stores a general information on various connecting
///    elements (spanners, beams, tuplets) including their
///    endpoints locations.
///    Base class of helper classes used to read and write
///    such elements.
//---------------------------------------------------------

class ConnectorInfo {
      bool finishedLeft() const;
      bool finishedRight() const;

   protected:
      ElementType _type       { ElementType::INVALID };
      ConnectorPointInfo _prevInfo        { -1, -1, 0 };
      ConnectorPointInfo _nextInfo        { -1, -1, 0 };
      ConnectorPointInfo _currentInfo     { -1, -1, 0 };

      ConnectorInfo* _prev    { 0 };
      ConnectorInfo* _next    { 0 };

      void updatePointInfo(const Element* e, ConnectorPointInfo& i, bool needNote);

   public:
      ConnectorInfo(int track, const Element* current);

      ConnectorInfo* prev() const   { return _prev; }
      ConnectorInfo* next() const   { return _next; }

      ElementType type() const { return _type; }
      const ConnectorPointInfo& info() const { return _currentInfo; }

      bool connect(ConnectorInfo* other);
      bool finished() const;

      bool hasPrevious() const      { return (_prevInfo.tick != -1); }
      bool hasNext() const          { return (_nextInfo.tick != -1); }
      bool isStart() const          { return (!hasPrevious() && hasNext()); }
      bool isMiddle() const         { return (hasPrevious() && hasNext());  }
      bool isEnd() const            { return (hasPrevious() && !hasNext()); }
      };

//---------------------------------------------------------
//   @@ ConnectorInfoReader
///    Helper class for reading beams, tuplets and spanners.
//---------------------------------------------------------

class ConnectorInfoReader final : public ConnectorInfo {
      Element* _connector;
      Element* _current;

      void readDestinationInfo(XmlReader& e, ConnectorPointInfo& info);

   public:
      ConnectorInfoReader(int track, Element* current);

      ConnectorInfoReader* prev() const   { return static_cast<ConnectorInfoReader*>(_prev); }
      ConnectorInfoReader* next() const   { return static_cast<ConnectorInfoReader*>(_next); }

      Element* connector();
      const Element* connector() const;

      bool read(XmlReader&);
      void addToScore(bool pasteMode);

      bool operator==(const ConnectorInfoReader& other) const;
      };

//---------------------------------------------------------
//   @@ ConnectorInfoWriter
///    Helper class for writing beams, tuplets and spanners.
///    Subclasses should fill _prevInfo and _nextInfo with
///    the proper information on the connector's endpoints.
//---------------------------------------------------------

class ConnectorInfoWriter : public ConnectorInfo {
      void writeDestinationInfo(XmlWriter& xml, const ConnectorPointInfo& info) const;

   protected:
      const Element* _connector;
      const Element* _current;

      virtual const char* tagName() const = 0;

   public:
      ConnectorInfoWriter(int track, const Element* current, const Element* connector);

      ConnectorInfoWriter* prev() const   { return static_cast<ConnectorInfoWriter*>(_prev); }
      ConnectorInfoWriter* next() const   { return static_cast<ConnectorInfoWriter*>(_next); }

      const Element* connector() const    { return _connector; }

      void write(XmlWriter&) const;
      };

}     // namespace Ms
#endif
