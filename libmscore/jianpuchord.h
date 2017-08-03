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

#ifndef __JIANPUCHORD_H__
#define __JIANPUCHORD_H__

#include "chord.h"
#include "chordrest.h"

class QPainter;

namespace Ms {

class Note;
class Hook;
class Chord;
class StaffFactory;

//---------------------------------------------------------
//   @@ JianpuChord
///    Graphical representation of a chord in Jianpu (numbered notation).
///    Single notes are handled as degenerated chords.
//
//---------------------------------------------------------

class JianpuChord : public Chord {
      Q_GADGET

   public:
      JianpuChord(Score* s = 0);
      JianpuChord(const Chord&, bool link = false, StaffFactory* fac = nullptr);
      JianpuChord(const JianpuChord&, bool link = false, StaffFactory* fac = nullptr);
      ~JianpuChord();
      JianpuChord &operator=(const JianpuChord&) = delete;

      virtual JianpuChord* clone() const { return new JianpuChord(*this, false); }
      virtual Element* linkedClone()     { return new JianpuChord(*this, true); }

      virtual void read(XmlReader& xml) override;
      virtual bool readProperties(XmlReader& xml) override;
      virtual void write(XmlWriter& xml) const override;

      virtual void processSiblings(std::function<void(Element*)> func) const override;
      virtual void layoutStem() override;
      virtual void layout() override;
      void layout2();
      };

}     // namespace Ms
#endif

