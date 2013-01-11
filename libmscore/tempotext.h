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

//-------------------------------------------------------------------
//   @@ TempoText
///   Tempo marker which determines the midi tempo.
//
//    @P tempo qreal      tempo in beats per second (beat=1/4)
//    @P followText bool  determine tempo from text
//-------------------------------------------------------------------

class TempoText : public Text  {
      Q_OBJECT
      Q_PROPERTY(qreal tempo         READ tempo      WRITE undoSetTempo)
      Q_PROPERTY(bool  followText    READ followText WRITE undoSetFollowText)

      qreal _tempo;          // beats per second
      bool _followText;       // parse text to determine tempo

   public:
      TempoText(Score*);
      virtual TempoText* clone() const { return new TempoText(*this); }
      virtual ElementType type() const { return TEMPO_TEXT; }
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      Segment* segment() const   { return (Segment*)parent(); }
      Measure* measure() const   { return (Measure*)parent()->parent(); }

      qreal tempo() const        { return _tempo;      }
      void setTempo(qreal v)     { _tempo = v;         }
      void undoSetTempo(qreal v);

      bool followText() const    { return _followText; }
      void setFollowText(bool v) { _followText = v;    }
      void undoSetFollowText(bool v);

      virtual void textChanged();
      virtual void layout();

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;
      };

#endif
