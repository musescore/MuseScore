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

#include "standardfactory.h"
#include "staff.h"
#include "stafftype.h"
#include "chord.h"
#include "note.h"
#include "rest.h"
#include "hook.h"
#include "beam.h"
#include "notedot.h"

namespace Ms {

Chord* StandardFactory::makeChord(Score* s)
      {
      return new Chord(s);
      }

Note* StandardFactory::makeNote(Score* s)
      {
      return new Note(s);
      }

Rest* StandardFactory::makeRest(Score* s)
      {
      return new Rest(s);
      }

Hook* StandardFactory::makeHook(Score* s)
      {
      return new Hook(s);
      }

Beam* StandardFactory::makeBeam(Score* s)
      {
      return new Beam(s);
      }

NoteDot* StandardFactory::makeNoteDot(Score* s)
      {
      return new NoteDot(s);
      }

Element* StandardFactory::cloneElement(Element* other, bool link)
      {
      Q_ASSERT(other != nullptr);
      if (link)
            return other->linkedClone();
      else
            return other->clone();
      }

Chord* StandardFactory::cloneChord(const Chord* other, bool link)
      {
      Q_ASSERT(other != nullptr);
      return new Chord(*other, link);
      }

Note* StandardFactory::cloneNote(const Note* other, bool link)
      {
      Q_ASSERT(other != nullptr);
      return new Note(*other, link);
      }

Rest* StandardFactory::cloneRest(const Rest* other, bool link)
      {
      Q_ASSERT(other != nullptr);
      return new Rest(*other, link);
      }

Hook* StandardFactory::cloneHook(const Hook* other, bool /* link */)
      {
      Q_ASSERT(other != nullptr);
      return new Hook(*other);
      }

Beam* StandardFactory::cloneBeam(const Beam* other, bool /* link */)
      {
      Q_ASSERT(other != nullptr);
      return new Beam(*other);
      }

NoteDot* StandardFactory::cloneNoteDot(const NoteDot* other, bool /* link */)
      {
      Q_ASSERT(other != nullptr);
      return new NoteDot(*other);
      }

} // namespace Ms
