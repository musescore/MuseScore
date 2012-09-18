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

#include "element.h"
#include "line.h"
#include "mscore.h"

class Score;
class Hairpin;
class QPainter;

//---------------------------------------------------------
//   @@ HairpinSegment
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
//   @@ Hairpin
//   @P subtype enum HairpinType CRESCENDO, DECRESCENDO
//---------------------------------------------------------

class Hairpin : public SLine {
      Q_OBJECT
      Q_ENUMS(HairpinType)

   public:
      enum HairpinType { CRESCENDO, DECRESCENDO };

   private:
      Q_PROPERTY(HairpinType subtype     READ subtype     WRITE undoSetSubtype)
      Q_PROPERTY(int         veloChange  READ veloChange  WRITE undoSetVeloChange)
      Q_PROPERTY(Element::DynamicType dynType READ dynType WRITE undoSetDynType)

      HairpinType _subtype;
      int _veloChange;
      DynamicType _dynType;

   public:
      Hairpin(Score* s);
      virtual Hairpin* clone() const   { return new Hairpin(*this); }
      virtual ElementType type() const { return HAIRPIN;  }

      HairpinType subtype() const      { return _subtype; }
      void setSubtype(HairpinType val) { _subtype = val;  }
      void undoSetSubtype(HairpinType);

      Segment* segment() const         { return (Segment*)parent(); }
      virtual void layout();
      virtual LineSegment* createLineSegment();

      int veloChange() const           { return _veloChange; }
      void setVeloChange(int v)        { _veloChange = v;    }
      void undoSetVeloChange(int v);

      DynamicType dynType() const      { return _dynType; }
      void setDynType(DynamicType t)   { _dynType = t;    }
      void undoSetDynType(DynamicType t);

      virtual void write(Xml&) const;
      virtual void read(const QDomElement&);

      virtual QVariant getProperty(P_ID id) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      };

#define __HAIRPIN_H__

#endif

