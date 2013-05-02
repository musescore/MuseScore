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

#ifndef __TRILL_H__
#define __TRILL_H__

#include "line.h"

class Trill;
class Accidental;
class QPainter;

//---------------------------------------------------------
//   @@ TrillSegment
//---------------------------------------------------------

class TrillSegment : public LineSegment {
      Q_OBJECT

   protected:
   public:
      TrillSegment(Score* s) : LineSegment(s) {}
      Trill* trill() const                { return (Trill*)spanner(); }
      virtual ElementType type() const    { return TRILL_SEGMENT; }
      virtual TrillSegment* clone() const { return new TrillSegment(*this); }
      virtual void draw(QPainter*) const;
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      virtual void layout();
      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID) const;
      };

//---------------------------------------------------------
//   @@ Trill
//   @P trillType   enum TrillType TRILL_LINE, UPPRALL_LINE, DOWNPRALL_LINE, PRALLPRALL_LINE, PURE_LINE
//---------------------------------------------------------

class Trill : public SLine {
      Q_OBJECT
      Q_ENUMS(TrillType)

   public:
      enum TrillType {
            TRILL_LINE, UPPRALL_LINE, DOWNPRALL_LINE, PRALLPRALL_LINE, PURE_LINE
            };

   private:
      Q_PROPERTY(TrillType trillType READ trillType WRITE undoSetTrillType)
      TrillType _trillType;
      ElementList _el;        // accidentals etc.

   public:
      Trill(Score* s);
      virtual Trill* clone() const     { return new Trill(*this); }
      virtual ElementType type() const { return TRILL; }

      virtual void layout();
      virtual LineSegment* createLineSegment();
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);

      void setTrillType(const QString& s);
      void undoSetTrillType(TrillType val);
      void setTrillType(TrillType tt)     { _trillType = tt; }
      TrillType trillType() const         { return _trillType; }
      QString trillTypeName() const;

      Segment* segment() const          { return (Segment*)parent(); }
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID) const;
      virtual void setYoff(qreal);
      };

Q_DECLARE_METATYPE(Trill::TrillType)
#endif

