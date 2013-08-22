//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TIE_H__
#define __TIE_H__

#include "mscore.h"
#include "slur.h"

class QPainter;

namespace Ms {

class Note;
class Score;
class MuseScoreView;
struct SlurPos;

//---------------------------------------------------------
//   @@ Tie
///   ties have Note's as startElement/endElement
//---------------------------------------------------------

class Tie : public SlurTie {
      Q_OBJECT

      static Note* editStartNote;
      static Note* editEndNote;

   public:
      Tie(Score* = 0);
      virtual Tie* clone() const override       { return new Tie(*this);  }
      virtual ElementType type() const override { return TIE;             }
      void setStartNote(Note* note);
      void setEndNote(Note* note)         { setEndElement((Element*)note);      }
      Note* startNote() const             { return (Note*) startElement();      }
      Note* endNote() const               { return (Note*) endElement();        }
      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void layout() override;
      virtual void slurPos(SlurPos*) override;
      virtual void computeBezier(SlurSegment*, QPointF so = QPointF());
      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void endEdit() override;
      };

}     // namespace Ms
#endif

