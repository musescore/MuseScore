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

#ifndef __STAFFFACTORY_H__
#define __STAFFFACTORY_H__

/**
 \file
 Factory interface to make elements of various staff types.
*/

#include "score.h"
#include "element.h"
#include "chord.h"
#include "note.h"
#include "rest.h"
#include "hook.h"
#include "beam.h"
#include "notedot.h"


namespace Ms {

//---------------------------------------------------------------------------------------
//   @@ StaffFactory
///    Factory interface class to make elements of a staff.
//---------------------------------------------------------------------------------------
class StaffFactory {
   public:
      virtual ~StaffFactory() { }

      virtual Chord* makeChord(Score* s = nullptr) = 0;
      virtual Note* makeNote(Score* s = nullptr) = 0;
      virtual Rest* makeRest(Score* s = nullptr) = 0;
      virtual Hook* makeHook(Score* s = nullptr) = 0;
      virtual Beam* makeBeam(Score* s = nullptr) = 0;
      virtual NoteDot* makeNoteDot(Score* s = nullptr) = 0;

      virtual Element* cloneElement(Element* other, bool link = false) = 0;
      virtual Chord* cloneChord(const Chord* other, bool link = false) = 0;
      virtual Note* cloneNote(const Note* other, bool link = false) = 0;
      virtual Rest* cloneRest(const Rest* other, bool link = false) = 0;
      virtual Hook* cloneHook(const Hook* other, bool link = false) = 0;
      virtual Beam* cloneBeam(const Beam* other, bool link = false) = 0;
      virtual NoteDot* cloneNoteDot(const NoteDot* other, bool link = false) = 0;
      };

}     // namespace Ms

#endif

