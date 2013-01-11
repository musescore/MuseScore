//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: clef.h 5343 2012-02-18 19:50:35Z miwarre $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __CLEF_H__
#define __CLEF_H__

/**
 \file
 Definition of classes Clef and ClefList.
*/

#include "element.h"
#include "mscore.h"

class Xml;
class MuseScoreView;
class Segment;
class QPainter;

static const int NO_CLEF = -1000;

//---------------------------------------------------------
//   ClefTypeList
//---------------------------------------------------------

struct ClefTypeList {
      ClefType _concertClef;
      ClefType _transposingClef;

      ClefTypeList() {}
      ClefTypeList(ClefType a, ClefType b) : _concertClef(a), _transposingClef(b) {}
      bool operator==(const ClefTypeList& t) const;
      bool operator!=(const ClefTypeList& t) const;
      };

//---------------------------------------------------------
//   ClefInfo
///   Info about a clef.
//---------------------------------------------------------

struct ClefInfo {
      const char* tag;        ///< comprehensive name for instruments.xml
      const char* sign;       ///< Name for musicXml.
      int line;               ///< Line for musicXml.
      int octChng;            ///< Octave change for musicXml.
      int pitchOffset;        ///< Pitch offset for line 0.
      char lines[14];
      const char* name;
      StaffGroup staffGroup;
      };

extern const ClefInfo clefTable[];

//---------------------------------------------------------
//   @@ Clef
///   Graphic representation of a clef.
//
//    @P showCourtesy bool
//    @P small        bool      r/o, set by layout
//---------------------------------------------------------

class Clef : public Element {
      Q_OBJECT
      Q_PROPERTY(bool showCourtesy READ showCourtesy WRITE undoSetShowCourtesy)
      Q_PROPERTY(bool small READ small)

      QList<Element*> elements;
      bool _showCourtesy;
      bool _showPreviousClef;       // show clef type at position tick-1
                                    // used for first clef on staff immediatly followed
                                    // by a different clef at same tick position
      bool _small;

      ClefTypeList _clefTypes;

      ClefType    curClefType;      // cached value of clef type (for re-laying out)
      int         curLines;         // cached value of staff nm. of lines  ( " )
      qreal       curLineDist;      // cached value of staff line distance ( " )
      void        layout1();        // lays the element out, using cached values

   public:
      Clef(Score*);
      Clef(const Clef&);
      virtual Clef* clone() const      { return new Clef(*this); }
      virtual ElementType type() const { return CLEF; }
      virtual void setSelected(bool f);

      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      virtual void layout();
      virtual void draw(QPainter*) const;
      virtual void read(XmlReader&);
      virtual void write(Xml&) const;

      virtual void addElement(Element* e, qreal x, qreal y);
      virtual Space space() const      { return Space(0.0, bbox().x() * 2.0 + width()); }

      bool small() const               { return _small; }
      void setSmall(bool val);

      int tick() const;

      bool showCourtesy() const        { return _showCourtesy; };
      void setShowCourtesy(bool v)     { _showCourtesy = v; }
      void undoSetShowCourtesy(bool v);

      static ClefType clefType(const QString& s);

      ClefType clefType() const;
      void setClefType(ClefType i);
      void setClefType(const QString& s);

      ClefTypeList clefTypeList() const     { return _clefTypes;                  }
      ClefType concertClef() const          { return _clefTypes._concertClef;     }
      ClefType transposingClef() const      { return _clefTypes._transposingClef; }
      void setConcertClef(ClefType val);
      void setTransposingClef(ClefType val);
      void setClefType(const ClefTypeList& ctl) { _clefTypes = ctl; }
      virtual void spatiumChanged(qreal oldValue, qreal newValue);

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;
      };

#endif

