//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: tempotext.h 5500 2012-03-28 16:28:26Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEMPOTEXT_H__
#define __TEMPOTEXT_H__

#include "text.h"

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

class TempoText : public Text  {
      Q_OBJECT

      qreal _tempo;          // beats per second
      bool _followText;       // parse text to determine tempo

   public:
      TempoText(Score*);
      virtual TempoText* clone() const { return new TempoText(*this); }
      virtual ElementType type() const { return TEMPO_TEXT; }
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);
      Segment* segment() const   { return (Segment*)parent(); }
      Measure* measure() const   { return (Measure*)parent()->parent(); }
      qreal tempo() const        { return _tempo;      }
      void setTempo(qreal v)     { _tempo = v;         }
      bool followText() const    { return _followText; }
      void setFollowText(bool v) { _followText = v;    }
      virtual void textChanged();
      };

#endif
