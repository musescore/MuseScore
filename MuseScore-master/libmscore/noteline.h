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

#ifndef __NOTELINE_H__
#define __NOTELINE_H__

#include "textline.h"

namespace Ms {

class Note;

//---------------------------------------------------------
//   @@ NoteLine
//---------------------------------------------------------

class NoteLine : public TextLine {
      Q_OBJECT

      Note* _startNote;
      Note* _endNote;

   public:
      NoteLine(Score* s);
      NoteLine(const NoteLine&);
      ~NoteLine() {}

      virtual NoteLine* clone() const           { return new NoteLine(*this); }
      virtual Element::Type type() const        { return Element::Type::NOTELINE; }

      void setStartNote(Note* n)  { _startNote = n; }
      Note* startNote() const     { return _startNote; }
      void setEndNote(Note* n)    { _endNote = n; }
      Note* endNote() const       { return _endNote; }
      };


}     // namespace Ms
#endif

