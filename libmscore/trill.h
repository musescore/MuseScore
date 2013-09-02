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

class QPainter;

namespace Ms {

class Trill;
class Accidental;

//---------------------------------------------------------
//   @@ TrillSegment
//---------------------------------------------------------

class TrillSegment : public LineSegment {
      Q_OBJECT

   protected:
   public:
      TrillSegment(Score* s) : LineSegment(s) {}
      Trill* trill() const                { return (Trill*)spanner(); }
      virtual ElementType type() const override    { return TRILL_SEGMENT; }
      virtual TrillSegment* clone() const override { return new TrillSegment(*this); }
      virtual void draw(QPainter*) const override;
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const override;
      virtual Element* drop(const DropData&) override;
      virtual void layout() override;
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all) override;
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
      Accidental* _accidental;

   public:
      Trill(Score* s);
      virtual ~Trill();
      virtual Trill* clone() const override     { return new Trill(*this); }
      virtual ElementType type() const override { return TRILL; }

      virtual void layout() override;
      virtual LineSegment* createLineSegment() override;
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void write(Xml&) const override;
      virtual void read(XmlReader&) override;

      void setTrillType(const QString& s);
      void undoSetTrillType(TrillType val);
      void setTrillType(TrillType tt)     { _trillType = tt; }
      TrillType trillType() const         { return _trillType; }
      QString trillTypeName() const;
      Accidental* accidental() const      { return _accidental; }
      void setAccidental(Accidental* a)   { _accidental = a; }

      Segment* segment() const          { return (Segment*)parent(); }
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual void setYoff(qreal) override;
      };


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Trill::TrillType)

#endif

