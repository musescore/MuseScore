//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: hairpin.h 5242 2012-01-23 17:25:56Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __HAIRPIN_H__

#include "line.h"
#include "mscore.h"

class Score;
class Hairpin;
class QPainter;

//---------------------------------------------------------
//   HairpinSegment
//---------------------------------------------------------

class HairpinSegment : public LineSegment {
      Q_OBJECT

      QLineF l1, l2;

   protected:
   public:
      HairpinSegment(Score* s) : LineSegment(s) {}
      Hairpin* hairpin() const              { return (Hairpin*)spanner(); }
      virtual HairpinSegment* clone() const { return new HairpinSegment(*this); }
      virtual ElementType type() const      { return HAIRPIN_SEGMENT; }
      virtual void draw(QPainter*) const;
      virtual void layout();
      };

//---------------------------------------------------------
//   Hairpin
//
//    subtype: 0 = crescendo,  1 = decrescendo
//---------------------------------------------------------

class Hairpin : public SLine {
      Q_OBJECT

      int _subtype;
      int _veloChange;
      DynamicType _dynType;

      void* pSubtype()  { return &_subtype; }

   public:
      Hairpin(Score* s);
      virtual Hairpin* clone() const   { return new Hairpin(*this); }
      virtual ElementType type() const { return HAIRPIN;  }
      int subtype() const              { return _subtype; }
      void setSubtype(int val)         { _subtype = val;  }
      Segment* segment() const         { return (Segment*)parent(); }
      virtual void layout();
      virtual LineSegment* createLineSegment();
      int veloChange() const           { return _veloChange; }
      void setVeloChange(int v)        { _veloChange = v;    }
      DynamicType dynType() const      { return _dynType; }
      void setDynType(DynamicType t)   { _dynType = t;    }
      virtual void write(Xml&) const;
      virtual void read(const QDomElement&);

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual bool setProperty(const QString&, const QDomElement&);

      static Property<Hairpin> propertyList[];
      Property<Hairpin>* property(P_ID id) const;
      };

#define __HAIRPIN_H__

#endif

