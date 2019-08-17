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
      Vibrato* vibrato() const                       { return toVibrato(spanner()); }
      virtual ElementType type() const override      { return ElementType::VIBRATO_SEGMENT; }
      virtual VibratoSegment* clone() const override { return new VibratoSegment(*this); }
      virtual void draw(QPainter*) const override;
      virtual void layout() override;

      virtual Element* propertyDelegate(Pid) override;

      Shape shape() const override;
      std::vector<SymId> symbols() const           { return _symbols; }
      void setSymbols(const std::vector<SymId>& s) { _symbols = s; }
      };

//---------------------------------------------------------
//   Vibrato
//---------------------------------------------------------

class Vibrato final : public SLine {

      virtual Sid getPropertyStyle(Pid) const override;

   public:
      enum class Type : char {
            GUITAR_VIBRATO, GUITAR_VIBRATO_WIDE, VIBRATO_SAWTOOTH, VIBRATO_SAWTOOTH_WIDE
            };
   private:
      Type _vibratoType;
      bool _playArticulation;

   public:
      Vibrato(Score* s);
      virtual ~Vibrato();
      virtual Vibrato* clone() const override   { return new Vibrato(*this);   }
      virtual ElementType type() const override { return ElementType::VIBRATO; }

      virtual void layout() override;
      virtual LineSegment* createLineSegment() override;
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;

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

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;
      virtual Pid propertyId(const QStringRef& xmlName) const override;
      virtual QString accessibleInfo() const override;
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

