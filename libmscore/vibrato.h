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

class VibratoSegment final : public LineSegment {
      std::vector<SymId> _symbols;

      void symbolLine(SymId start, SymId fill);
      void symbolLine(SymId start, SymId fill, SymId end);
      virtual Sid getPropertyStyle(Pid) const override;

   protected:
   public:
      VibratoSegment(Spanner* sp, Score* s) : LineSegment(sp, s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)      {}

      ElementType type() const override      { return ElementType::VIBRATO_SEGMENT; }
      VibratoSegment* clone() const override { return new VibratoSegment(*this); }

      Vibrato* vibrato() const               { return toVibrato(spanner()); }

      void draw(QPainter*) const override;
      void layout() override;

      Element* propertyDelegate(Pid) override;

      Shape shape() const override;
      std::vector<SymId> symbols() const           { return _symbols; }
      void setSymbols(const std::vector<SymId>& s) { _symbols = s; }
      };

//---------------------------------------------------------
//   Vibrato
//---------------------------------------------------------

class Vibrato final : public SLine {

      Sid getPropertyStyle(Pid) const override;

   public:
      enum class Type : char {
            GUITAR_VIBRATO, GUITAR_VIBRATO_WIDE, VIBRATO_SAWTOOTH, VIBRATO_SAWTOOTH_WIDE
            };
   private:
      Type _vibratoType;
      bool _playArticulation;

   public:
      Vibrato(Score* s);
      ~Vibrato();

      Vibrato* clone() const override   { return new Vibrato(*this);   }
      ElementType type() const override { return ElementType::VIBRATO; }

      void layout() override;
      LineSegment* createLineSegment() override;

      void write(XmlWriter&) const override;
      void read(XmlReader&) override;

      void setVibratoType(const QString& s);
      void undoSetVibratoType(Type val);
      void setVibratoType(Type tt)        { _vibratoType = tt; }
      Type vibratoType() const              { return _vibratoType; }
      void setPlayArticulation(bool val)  { _playArticulation = val;}
      bool playArticulation() const       { return _playArticulation; }
      static QString type2name(Vibrato::Type t);
      QString vibratoTypeName() const;
      QString vibratoTypeUserName() const;

      Segment* segment() const          { return (Segment*)parent(); }

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;
      Pid propertyId(const QStringRef& xmlName) const override;
      QString accessibleInfo() const override;
      };

//---------------------------------------------------------
//   VibratoTableItem
//---------------------------------------------------------

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

