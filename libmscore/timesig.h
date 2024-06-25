//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TIMESIG_H__
#define __TIMESIG_H__

#include "element.h"
#include "groups.h"
#include "sig.h"

namespace Ms {

class MuseScoreView;
class Segment;

//---------------------------------------------------------
//   TimeSigType
//---------------------------------------------------------

enum class TimeSigType : char {
      NORMAL,            // use sz/sn text
      FOUR_FOUR,         // common time (4/4)
      ALLA_BREVE,        // cut time (2/2)
      CUT_BACH,          // cut time (Bach)
      CUT_TRIPLE,        // cut triple time (9/8)
      };

//---------------------------------------------------------------------------------------
//   @@ TimeSig
///    This class represents a time signature.
//---------------------------------------------------------------------------------------

class TimeSig final : public Element {
      QString _numeratorString;     // calculated from actualSig() if !customText
      QString _denominatorString;

      std::vector<SymId> ns;
      std::vector<SymId> ds;

      QPointF pz;
      QPointF pn;
      QPointF pointLargeLeftParen;
      QPointF pointLargeRightParen;
      Fraction _sig;
      Fraction _stretch;      // localSig / globalSig
      Groups _groups;

      QSizeF _scale;
      TimeSigType _timeSigType;
      bool _showCourtesySig;
      bool _largeParentheses;

   public:
      TimeSig(Score* = 0);

      QString ssig() const;
      void setSSig(const QString&);

      TimeSig* clone() const override          { return new TimeSig(*this);   }
      ElementType type() const override        { return ElementType::TIMESIG; }

      TimeSigType timeSigType() const    { return _timeSigType; }

      bool operator==(const TimeSig&) const;
      bool operator!=(const TimeSig& ts) const { return !(*this == ts); }

      qreal mag() const override;
      void draw(QPainter*) const override;
      void write(XmlWriter& xml) const override;
      void read(XmlReader&) override;
      void layout() override;

      Fraction sig() const               { return _sig; }
      void setSig(const Fraction& f, TimeSigType st = TimeSigType::NORMAL);
      int numerator() const              { return _sig.numerator(); }
      int denominator() const            { return _sig.denominator(); }

      Fraction stretch() const           { return _stretch;   }
      void setStretch(const Fraction& s) { _stretch = s;      }
      int numeratorStretch() const       { return _stretch.numerator(); }
      int denominatorStretch() const     { return _stretch.denominator(); }

      bool acceptDrop(EditData&) const override;
      Element* drop(EditData&) override;

      Segment* segment() const           { return (Segment*)parent(); }
      Measure* measure() const           { return (Measure*)parent()->parent(); }

      bool showCourtesySig() const       { return _showCourtesySig; }
      void setShowCourtesySig(bool v)    { _showCourtesySig = v;    }

      QString numeratorString() const    { return _numeratorString;   }
      void setNumeratorString(const QString&);

      QString denominatorString() const  { return _denominatorString; }
      void setDenominatorString(const QString&);

      void setLargeParentheses(bool v)    { _largeParentheses = v;    }

      void setScale(const QSizeF& s)      { _scale = s; }


      void setFrom(const TimeSig*);

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid id) const override;
      Pid propertyId(const QStringRef& xmlName) const override;

      const Groups& groups() const    { return _groups; }
      void setGroups(const Groups& e) { _groups = e; }

      bool isLocal() const                 { return _stretch != Fraction(1,1); }

      Element* nextSegmentElement() override;
      Element* prevSegmentElement() override;
      QString accessibleInfo() const override;
      };

}     // namespace Ms
#endif

