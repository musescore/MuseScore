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

namespace Ms {

class Trill;
class Accidental;

//---------------------------------------------------------
//   @@ TrillSegment
//---------------------------------------------------------

class TrillSegment final : public LineSegment {
      std::vector<SymId> _symbols;

      void symbolLine(SymId start, SymId fill);
      void symbolLine(SymId start, SymId fill, SymId end);
      Sid getPropertyStyle(Pid) const override;

   protected:
   public:
      TrillSegment(Spanner* sp, Score* s) : LineSegment(sp, s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)      {}
      TrillSegment(Score* s) : LineSegment(s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)      {}

      Trill* trill() const                      { return (Trill*)spanner(); }
      ElementType type() const override         { return ElementType::TRILL_SEGMENT; }
      TrillSegment* clone() const override      { return new TrillSegment(*this); }
      void draw(QPainter*) const override;
      bool acceptDrop(EditData&) const override;
      Element* drop(EditData&) override;
      void layout() override;

      Element* propertyDelegate(Pid) override;

      void add(Element*) override;
      void remove(Element*) override;
      void scanElements(void* data, void (*func)(void*, Element*), bool all) override;
      Shape shape() const override;

      std::vector<SymId> symbols() const           { return _symbols; }
      void setSymbols(const std::vector<SymId>& s) { _symbols = s; }
      };

//---------------------------------------------------------
//   @@ Trill
//   @P trillType  enum (Trill.DOWNPRALL_LINE, .PRALLPRALL_LINE, .PURE_LINE, .TRILL_LINE, .UPPRALL_LINE)
//---------------------------------------------------------

class Trill final : public SLine {
      Sid getPropertyStyle(Pid) const override;

   public:
      enum class Type : char {
            TRILL_LINE, UPPRALL_LINE, DOWNPRALL_LINE, PRALLPRALL_LINE,
            };

   private:
      Type _trillType;
      Accidental* _accidental;
      MScore::OrnamentStyle _ornamentStyle; // for use in ornaments such as trill
      bool _playArticulation;

   public:
      Trill(Score* s);
      ~Trill();

      Trill* clone() const override     { return new Trill(*this); }
      ElementType type() const override { return ElementType::TRILL; }

      void layout() override;
      LineSegment* createLineSegment() override;
      void add(Element*) override;
      void remove(Element*) override;
      void write(XmlWriter&) const override;
      void read(XmlReader&) override;

      void setTrillType(const QString& s);
      void setTrillType(Type tt)          { _trillType = tt; }
      Type trillType() const              { return _trillType; }
      void setOrnamentStyle(MScore::OrnamentStyle val) { _ornamentStyle = val;}
      MScore::OrnamentStyle ornamentStyle() const { return _ornamentStyle;}
      void setPlayArticulation(bool val)  { _playArticulation = val;}
      bool playArticulation() const       { return _playArticulation; }
      static QString type2name(Trill::Type t);
      QString trillTypeName() const;
      QString trillTypeUserName() const;
      Accidental* accidental() const      { return _accidental; }
      void setAccidental(Accidental* a)   { _accidental = a; }

      Segment* segment() const          { return (Segment*)parent(); }
      void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;
      Pid propertyId(const QStringRef& xmlName) const override;

      QString accessibleInfo() const override;
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

