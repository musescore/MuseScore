//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: timesig.h 5149 2011-12-29 08:38:43Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TIMESIG_H__
#define __TIMESIG_H__

#include "element.h"
#include "sig.h"
#include "mscore.h"

class MuseScoreView;
class Segment;
class QPainter;

enum TimeSigType {
      TSIG_NORMAL,            // use sz/sn text
      TSIG_FOUR_FOUR,         // common time (4/4)
      TSIG_ALLA_BREVE         // cut time (2/2)
      };

//---------------------------------------------------------------------------------------
//   @@ TimeSig
///    This class represents a time signature.
//
//    @P numerator          int
//    @P denominator        int
//    @P numeratorStretch   int
//    @P denominatorStretch int
//    @P numeratorString   QString  text of numerator
//    @P denominatorString QString  text of denominator
//    @P showCourtesySig bool show courtesy time signature for this sig if appropriate
//---------------------------------------------------------------------------------------

class TimeSig : public Element {
      Q_OBJECT
      Q_PROPERTY(QString numeratorString   READ numeratorString   WRITE undoSetNumeratorString)
      Q_PROPERTY(QString denominatorString READ denominatorString WRITE undoSetDenominatorString)
      Q_PROPERTY(bool showCourtesySig      READ showCourtesySig   WRITE undoSetShowCourtesySig)
      Q_PROPERTY(int numerator             READ numerator)
      Q_PROPERTY(int denominator           READ denominator)
      Q_PROPERTY(int numeratorStretch      READ numeratorStretch)
      Q_PROPERTY(int denominatorStretch    READ denominatorStretch)

      TimeSigType _subtype;
      bool	_showCourtesySig;
      QString _numeratorString;     // calculated from actualSig() if !customText
      QString _denominatorString;
      QPointF pz, pn;
      Fraction _sig;
      Fraction _stretch;      // localSig / globalSig
      bool customText;        // if false, sz and sn are calculated from actualSig()

   public:
      TimeSig(Score* = 0);

      QString ssig() const;
      void setSSig(const QString&);

      TimeSig* clone() const             { return new TimeSig(*this); }
      ElementType type() const           { return TIMESIG; }

      TimeSigType subtype() const        { return _subtype; }

      void draw(QPainter*) const;
      void write(Xml& xml) const;
      void read(const QDomElement&);
      void layout1();
      Space space() const;

      Fraction sig() const               { return _sig; }
      void setSig(const Fraction& f, TimeSigType st = TSIG_NORMAL);
      int numerator() const              { return _sig.numerator(); }
      int denominator() const            { return _sig.denominator(); }

      Fraction stretch() const           { return _stretch;   }
      void setStretch(const Fraction& s) { _stretch = s;      }
      int numeratorStretch() const       { return _stretch.numerator(); }
      int denominatorStretch() const     { return _stretch.denominator(); }

      bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      Element* drop(const DropData&);

      Segment* segment() const           { return (Segment*)parent(); }
      Measure* measure() const           { return (Measure*)parent()->parent(); }

      bool showCourtesySig() const       { return _showCourtesySig; };
      void setShowCourtesySig(bool v)    { _showCourtesySig = v;    };
      void undoSetShowCourtesySig(bool v);

      QString numeratorString() const    { return _numeratorString;   }
      void setNumeratorString(const QString&);
      void undoSetNumeratorString(const QString&);

      QString denominatorString() const  { return _denominatorString; }
      void setDenominatorString(const QString&);
      void undoSetDenominatorString(const QString&);

      void setFrom(const TimeSig*);

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);
      };

#endif

