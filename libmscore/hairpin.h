//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
      virtual QVariant getProperty(P_ID id) const;
      virtual bool setProperty(P_ID id, const QVariant& v);
      virtual QVariant propertyDefault(P_ID id) const;
      };

//---------------------------------------------------------
//   @@ Hairpin
//   @P hairpinType enum HairpinType CRESCENDO, DECRESCENDO
//   @P veloChange int
//   @P dynRange   enum Element::DynamicRange
//---------------------------------------------------------

class Hairpin : public SLine {
      Q_OBJECT
      Q_ENUMS(HairpinType)

   public:
      enum HairpinType { CRESCENDO, DECRESCENDO };

   private:
      Q_PROPERTY(HairpinType hairpinType READ hairpinType WRITE undoSetHairpinType)
      Q_PROPERTY(int         veloChange  READ veloChange  WRITE undoSetVeloChange)
      Q_PROPERTY(Element::DynamicRange   dynRange READ dynRange WRITE undoSetDynRange)

      HairpinType _hairpinType;
      int _veloChange;
      DynamicRange _dynRange;

   public:
      Hairpin(Score* s);
      virtual Hairpin* clone() const   { return new Hairpin(*this); }
      virtual ElementType type() const { return HAIRPIN;  }

      HairpinType hairpinType() const      { return _hairpinType; }
      void setHairpinType(HairpinType val) { _hairpinType = val;  }
      void undoSetHairpinType(HairpinType);

      Segment* segment() const         { return (Segment*)parent(); }
      virtual void layout();
      virtual LineSegment* createLineSegment();

      int veloChange() const           { return _veloChange; }
      void setVeloChange(int v)        { _veloChange = v;    }
      void undoSetVeloChange(int v);

      DynamicRange dynRange() const       { return _dynRange; }
      void setDynRange(DynamicRange t)    { _dynRange = t;    }
      void undoSetDynRange(DynamicRange t);

      virtual void write(Xml&) const;
      virtual void read(XmlReader&);

      virtual QVariant getProperty(P_ID id) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID id) const;

      virtual void setYoff(qreal);
      };

#define __HAIRPIN_H__

#endif

