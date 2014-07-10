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

#ifndef __TEMPOTEXT_H__
#define __TEMPOTEXT_H__

#include "durationtype.h"
#include "text.h"

namespace Ms {

//-------------------------------------------------------------------
//   @@ TempoText
///    Tempo marker which determines the midi tempo.
//
//   @P tempo       qreal  tempo in beats per second (beat=1/4)
//   @P followText  bool   determine tempo from text
//-------------------------------------------------------------------

class TempoText : public Text  {
      Q_OBJECT
      Q_PROPERTY(qreal tempo         READ tempo      WRITE undoSetTempo)
      Q_PROPERTY(bool  followText    READ followText WRITE undoSetFollowText)

      qreal _tempo;          // beats per second
      bool _followText;       // parse text to determine tempo

   public:
      TempoText(Score*);
      virtual TempoText* clone() const override   { return new TempoText(*this); }
      virtual Element::Type type() const override { return Element::Type::TEMPO_TEXT; }
      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      Segment* segment() const   { return (Segment*)parent(); }
      Measure* measure() const   { return (Measure*)parent()->parent(); }

      qreal tempo() const        { return _tempo;      }
      void setTempo(qreal v)     { _tempo = v;         }
      void undoSetTempo(qreal v);

      bool followText() const    { return _followText; }
      void setFollowText(bool v) { _followText = v;    }
      void undoSetFollowText(bool v);

      virtual void textChanged() override;
      virtual void layout();
      
      static int findTempoDuration(const QString& s, int& len, TDuration& dur);
      static QString duration2tempoTextString(const TDuration dur);

      QVariant getProperty(P_ID propertyId) const override;
      bool setProperty(P_ID propertyId, const QVariant&) override;
      QVariant propertyDefault(P_ID id) const override;
      virtual QString accessibleInfo() override;
      };



}     // namespace Ms
#endif
