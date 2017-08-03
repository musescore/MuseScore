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

#ifndef __STANDARDFACTORY_H__
#define __STANDARDFACTORY_H__

/**
 \file
 Factory class for Standard notation staff.
*/

#include "stafffactory.h"

namespace Ms {

//---------------------------------------------------------------------------------------
//   @@ StandardFactory
///    Concrete singleton factory class to make elements of Standard notation staff.
//---------------------------------------------------------------------------------------
class StandardFactory : public StaffFactory {
   public:
      // Singleton factory instance.
      static StandardFactory* instance()
            {
            static StandardFactory inst;
            return &inst;
            }

   private: // Prevent unwanted construction and destruction.
      StandardFactory() { }
      StandardFactory(const StandardFactory&) { }
      StandardFactory& operator=(const StandardFactory&) = delete;
      virtual ~StandardFactory() { }

   public: // From StaffFactory
      virtual Chord* makeChord(Score* s = nullptr) override;
      virtual Note* makeNote(Score* s = nullptr) override;
      virtual Rest* makeRest(Score* s = nullptr) override;
      virtual Hook* makeHook(Score* s = nullptr) override;
      virtual Beam* makeBeam(Score* s = nullptr) override;
      virtual NoteDot* makeNoteDot(Score* s = nullptr) override;

      virtual Element* cloneElement(Element* other, bool link = false) override;
      virtual Chord* cloneChord(const Chord* other, bool link = false) override;
      virtual Note* cloneNote(const Note* other, bool link = false) override;
      virtual Rest* cloneRest(const Rest* other, bool link = false) override;
      virtual Hook* cloneHook(const Hook* other, bool link = false) override;
      virtual Beam* cloneBeam(const Beam* other, bool link = false) override;
      virtual NoteDot* cloneNoteDot(const NoteDot* other, bool link = false) override;

      };

}     // namespace Ms

#endif

