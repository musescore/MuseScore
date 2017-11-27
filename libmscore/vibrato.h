//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __VIBRATO_H__
#define __VIBRATO_H__

#include "line.h"

namespace Ms {

class Vibrato;
class Accidental;

//---------------------------------------------------------
//   @@ VibratoSegment
//---------------------------------------------------------

class VibratoSegment : public LineSegment {
      std::vector<SymId> _symbols;

      void symbolLine(SymId start, SymId fill);
      void symbolLine(SymId start, SymId fill, SymId end);

   protected:
   public:
      VibratoSegment(Score* s) : LineSegment(s)      {}
      Vibrato* vibrato() const                       { return (Vibrato*)spanner(); }
      virtual ElementType type() const override      { return ElementType::VIBRATO_SEGMENT; }
      virtual VibratoSegment* clone() const override { return new VibratoSegment(*this); }
      virtual void draw(QPainter*) const override;
      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;
      virtual void layout() override;
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all) override;
      Shape shape() const override;

      std::vector<SymId> symbols() const           { return _symbols; }
      void setSymbols(const std::vector<SymId>& s) { _symbols = s; }
      };

//---------------------------------------------------------
//   Vibrato
//---------------------------------------------------------

class Vibrato : public SLine {
   public:
      enum class Type : char {
            TRILL_LINE, UPPRALL_LINE, DOWNPRALL_LINE, PRALLPRALL_LINE,
            GUITAR_VIBRATO, GUITAR_VIBRATO_WIDE, VIBRATO_SAWTOOTH_NARROW, VIBRATO_SAWTOOTH, VIBRATO_SAWTOOTH_WIDE
            };
   private:
      Type _vibratoType;
      Accidental* _accidental;
      MScore::OrnamentStyle _ornamentStyle; // for use in ornaments such as trill
      bool _playArticulation;

   public:
      Vibrato(Score* s);
      virtual ~Vibrato();
      virtual Vibrato* clone() const override   { return new Vibrato(*this);   }
      virtual ElementType type() const override { return ElementType::VIBRATO; }

      virtual void layout() override;
      virtual LineSegment* createLineSegment() override;
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;

      void setVibratoType(const QString& s);
      void undoSetVibratoType(Type val);
      void setVibratoType(Type tt)        { _vibratoType = tt; }
      Type vibratoType() const              { return _vibratoType; }
      void setOrnamentStyle(MScore::OrnamentStyle val) { _ornamentStyle = val;}
      MScore::OrnamentStyle ornamentStyle() const { return _ornamentStyle;}
      void setPlayArticulation(bool val)  { _playArticulation = val;}
      bool playArticulation() const       { return _playArticulation; }
      QString vibratoTypeName() const;
      QString vibratoTypeUserName() const;
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

struct VibratoTableItem {
      Vibrato::Type type;
      const char* name;
      QString userName;
      };

extern const VibratoTableItem vibratoTable[];
extern int vibratoTableSize();

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Vibrato::Type);

#endif

