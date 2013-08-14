//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __GROUPS__
#define __GROUPS__

#include "fraction.h"
#include "mscore.h"
#include "durationtype.h"

namespace Ms {

class ChordRest;
class Xml;
class XmlReader;

//---------------------------------------------------------
//   GroupNode
//---------------------------------------------------------

struct GroupNode {
      int pos;          // tick position, division 32nd
      int action;       // bits: cccc bbbb aaaa
                        // cc - 1/64  bb - 1/32  aa - 1/16
                        // bit pattern xxxx:
                        // 1 - start new beam
                        // 2 - start new 1/32 subbeam
                        // 3 - start new 1/64 subbeam

      bool operator==(const GroupNode& g) const { return g.pos == pos && g.action == action; }
      };

//---------------------------------------------------------
//   Groups
//    GroupNodes must be sorted by tick
//---------------------------------------------------------

class Groups : public std::vector<GroupNode> {

   public:
      Groups() {}
      Groups(const std::vector<GroupNode>& l) : std::vector<GroupNode>(l) {}

      void write(Xml&) const;
      void read(XmlReader&);

      BeamMode beamMode(int tick, TDuration::DurationType d) const;
      void addStop(int pos, TDuration::DurationType d, BeamMode bm);
      bool operator==(const Groups& g) const {
            if (g.size() != size())
                  return false;
            for (unsigned i = 0; i < size(); ++i) {
                  if (!(g[i] == (*this)[i]))
                        return false;
                  }
            return true;
            }
      void dump(const char*) const;

      static const Groups& endings(const Fraction& f);
      static BeamMode endBeam(ChordRest* cr);
      };


//---------------------------------------------------------
//   NoteGroup
//---------------------------------------------------------

struct NoteGroup {
      Fraction timeSig;
      Groups endings;
      };


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Groups)

#endif

