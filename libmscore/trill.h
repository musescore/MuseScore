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

      std::vector<SymId> _symbols;

      void symbolLine(SymId start, SymId fill);
      void symbolLine(SymId start, SymId fill, SymId end);

   protected:
   public:
      TrillSegment(Score* s) : LineSegment(s)      {}
      Trill* trill() const                         { return (Trill*)spanner(); }
      virtual Element::Type type() const override  { return Element::Type::TRILL_SEGMENT; }
      virtual TrillSegment* clone() const override { return new TrillSegment(*this); }
      virtual void draw(QPainter*) const override;
      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;
      virtual void layout() override;
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all) override;

      std::vector<SymId> symbols() const           { return _symbols; }
      void setSymbols(const std::vector<SymId>& s) { _symbols = s; }
      };

//---------------------------------------------------------
//   @@ Trill
//   @P trillType  enum (Trill.DOWNPRALL_LINE, .PRALLPRALL_LINE, .PURE_LINE, .TRILL_LINE, .UPPRALL_LINE)
//---------------------------------------------------------

class Trill : public SLine {
      Q_OBJECT
      Q_ENUMS(Type)

   public:
      enum class Type : char {
            TRILL_LINE, UPPRALL_LINE, DOWNPRALL_LINE, PRALLPRALL_LINE
            };

   private:
      Q_PROPERTY(Ms::Trill::Type trillType READ trillType WRITE undoSetTrillType)
      Type _trillType;
      Accidental* _accidental;
      MScore::OrnamentStyle _ornamentStyle; // for use in ornaments such as trill
      bool _playArticulation;

   public:
      Trill(Score* s);
      virtual ~Trill();
      virtual Trill* clone() const override       { return new Trill(*this); }
      virtual Element::Type type() const override { return Element::Type::TRILL; }

      virtual void layout() override;
      virtual LineSegment* createLineSegment() override;
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void write(Xml&) const override;
      virtual void read(XmlReader&) override;

      void setTrillType(const QString& s);
      void undoSetTrillType(Type val);
      void setTrillType(Type tt)          { _trillType = tt; }
      Type trillType() const              { return _trillType; }
      void setOrnamentStyle(MScore::OrnamentStyle val) { _ornamentStyle = val;}
      MScore::OrnamentStyle ornamentStyle() const { return _ornamentStyle;}
      void setPlayArticulation(bool val)  { _playArticulation = val;}
      bool playArticulation() const       { return _playArticulation; }
      QString trillTypeName() const;
      QString trillTypeUserName() const;
      Accidental* accidental() const      { return _accidental; }
      void setAccidental(Accidental* a)   { _accidental = a; }

      Segment* segment() const          { return (Segment*)parent(); }
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual void setYoff(qreal) override;

      virtual QString accessibleInfo() const override;
      };

struct TrillTableItem {
      Trill::Type type;
      const char* name;
      QString userName;
      };

extern const TrillTableItem trillTable[];
extern int trillTableSize();

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Trill::Type);

#endif

