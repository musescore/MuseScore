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
      TSIG_FOUR_FOUR,         // common time
      TSIG_ALLA_BREVE         // cut time
      };

//---------------------------------------------------------
//   TimeSig
//    Time Signature
//---------------------------------------------------------

class TimeSig : public Element {
      TimeSigType _subtype;
      bool	_showCourtesySig;
      QString sz, sn;         // calculated from actualSig() if !customText
      QPointF pz, pn;
      Fraction _nominal;
      Fraction _stretch;      // _nominal / aktualSig()
      bool customText;        // if false, sz and sn are calculated from actualSig()

   public:
      TimeSig(Score*);
      TimeSig(Score* s, TimeSigType st);
      TimeSig(Score* s, int z, int n);
      TimeSig(Score* s, const Fraction& f);

      TimeSig* clone() const             { return new TimeSig(*this); }
      ElementType type() const           { return TIMESIG; }

      void setSubtype(TimeSigType val);
      TimeSigType subtype() const        { return _subtype; }

      void draw(QPainter*) const;
      void write(Xml& xml) const;
      void read(const QDomElement&);
      void layout();
      Space space() const;

      Fraction sig() const               { return _nominal; }
      void setSig(const Fraction& f)     { _nominal = f;    }
      void setSig(int a, int b)          { _nominal.set(a,b); }

      Fraction stretch() const           { return _stretch;  }
      void setStretch(const Fraction& f) { _stretch = f;    }
      Fraction actualSig() const         { return _nominal / _stretch; }
      void setActualSig(const Fraction& f);

      bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      Element* drop(const DropData&);

      Segment* segment() const           { return (Segment*)parent(); }
      Measure* measure() const           { return (Measure*)parent()->parent(); }

      bool showCourtesySig() const       { return _showCourtesySig; };
      void setShowCourtesySig(bool v)    { _showCourtesySig = v;    };

      QString zText() const              { return sz; }
      QString nText() const              { return sn; }
      void setText(const QString&, const QString&);
      void setFrom(const TimeSig*);
      };

#endif

