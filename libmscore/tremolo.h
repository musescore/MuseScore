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

#ifndef __TREMOLO_H__
#define __TREMOLO_H__

#include "durationtype.h"
#include "symbol.h"

namespace Ms {

class Chord;

// Tremolo subtypes:
enum class TremoloType : signed char {
      INVALID_TREMOLO = -1,
      R8=0, R16, R32, R64, BUZZ_ROLL,  // one note tremolo (repeat)
      C8, C16, C32, C64     // two note tremolo (change)
      };

enum class TremoloPlacement : signed char {
      DEFAULT = 0,
      STEM_CENTER
      };

//---------------------------------------------------------
//   @@ Tremolo
//---------------------------------------------------------

class Tremolo final : public Element {
      TremoloType _tremoloType;
      Chord* _chord1;
      Chord* _chord2;
      TDuration _durationType;
      QPainterPath path;

      int _lines;       // derived from _subtype
      TremoloPlacement _tremoloPlacement = TremoloPlacement::DEFAULT;

      QPainterPath basePath() const;
      void computeShape();
      void layoutOneNoteTremolo(qreal x, qreal y, qreal spatium);
      void layoutTwoNotesTremolo(qreal x, qreal y, qreal h, qreal spatium);

   public:
      Tremolo(Score*);
      Tremolo(const Tremolo&);
      Tremolo &operator=(const Tremolo&) = delete;
      virtual Tremolo* clone() const       { return new Tremolo(*this); }
      virtual ElementType type() const     { return ElementType::TREMOLO; }
      virtual int subtype() const override { return (int) _tremoloType; }
      virtual QString subtypeName() const override;

      QString tremoloTypeName() const;
      void setTremoloType(const QString& s);
      static TremoloType name2Type(const QString& s);
      static QString type2name(TremoloType t);

      Chord* chord() const { return toChord(parent()); }

      void setTremoloType(TremoloType t);
      TremoloType tremoloType() const      { return _tremoloType; }

      virtual qreal mag() const;
      virtual void draw(QPainter*) const;
      virtual void layout();
      void layout2();
      virtual void write(XmlWriter& xml) const;
      virtual void read(XmlReader&);

      Chord* chord1() const { return _chord1; }
      Chord* chord2() const { return _chord2; }

      TDuration durationType() const { return _durationType; }
      void setDurationType(TDuration d) { _durationType = d; }

      void setChords(Chord* c1, Chord* c2) {
            _chord1 = c1;
            _chord2 = c2;
            }
      Fraction tremoloLen() const;
      bool twoNotes() const { return tremoloType() >= TremoloType::C8; } // is it a two note tremolo?
      int lines() const { return _lines; }

      bool placeMidStem() const;

      virtual void spatiumChanged(qreal oldValue, qreal newValue) override;
      virtual void localSpatiumChanged(qreal oldValue, qreal newValue) override;
      virtual void styleChanged() override;

      virtual QString accessibleInfo() const override;

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual Pid propertyId(const QStringRef& xmlName) const override;
      virtual QString propertyUserValue(Pid) const override;
      };


}     // namespace Ms
#endif

