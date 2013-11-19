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

#ifndef __TIMESIG_H__
#define __TIMESIG_H__

#include "element.h"
#include "sig.h"
#include "mscore.h"
#include "groups.h"

class QPainter;

namespace Ms {

class MuseScoreView;
class Segment;

enum TimeSigType {
      TSIG_NORMAL,            // use sz/sn text
      TSIG_FOUR_FOUR,         // common time (4/4)
      TSIG_ALLA_BREVE         // cut time (2/2)
      };

//---------------------------------------------------------------------------------------
//   @@ TimeSig
///    This class represents a time signature.
//
//    @P numerator          int (read only)
//    @P denominator        int (read only)
//    @P numeratorStretch   int (read only)
//    @P denominatorStretch int (read only)
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
      Q_PROPERTY(Groups groups             READ groups            WRITE undoSetGroups)

      TimeSigType _timeSigType;
      QString _numeratorString;     // calculated from actualSig() if !customText
      QString _denominatorString;
      QPointF pz, pn;
      Fraction _sig;
      Fraction _stretch;      // localSig / globalSig
      bool	_showCourtesySig;
      bool customText;        // if false, sz and sn are calculated from actualSig()
      bool _needLayout;
      Groups _groups;

      void layout1();

   public:
      TimeSig(Score* = 0);

      QString ssig() const;
      void setSSig(const QString&);

      TimeSig* clone() const             { return new TimeSig(*this); }
      ElementType type() const           { return TIMESIG; }

      TimeSigType timeSigType() const    { return _timeSigType; }

      virtual qreal mag() const;
      void draw(QPainter*) const;
      void write(Xml& xml) const;
      void read(XmlReader&);
      void layout();
      Space space() const;

      Fraction sig() const               { return _sig; }
      void setSig(const Fraction& f, TimeSigType st = TSIG_NORMAL);
      Q_INVOKABLE void setSig(int z, int n, TimeSigType st = TSIG_NORMAL) { setSig(Fraction(z, n), st); }
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

      bool showCourtesySig() const       { return _showCourtesySig; }
      void setShowCourtesySig(bool v)    { _showCourtesySig = v;    }
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

      void setNeedLayout(bool nl) { _needLayout = nl; }

      const Groups& groups() const    { return _groups; }
      void setGroups(const Groups& e) { _groups = e; }
      void undoSetGroups(const Groups& e);

      Fraction globalSig() const           { return (_sig * _stretch).reduced();  }
      void setGlobalSig(const Fraction& f) { _stretch = (_sig / f).reduced(); }
      };


}     // namespace Ms
#endif

