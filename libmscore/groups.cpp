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

#include "groups.h"
#include "durationtype.h"
#include "chordrest.h"
#include "staff.h"
#include "tuplet.h"

namespace Ms {

//---------------------------------------------------------
//   noteGroups
//---------------------------------------------------------

static std::vector<NoteGroup> noteGroups {
      { Fraction(2,2),
            Groups( { { 4, 512}, { 8, 272}, {12, 512}, {16, 273}, {20, 512}, {24, 272}, {28, 512} })
            },
      { Fraction(4,4),
            Groups( { { 4, 512}, { 8, 272}, {12, 512}, {16, 273}, {20, 512}, {24, 272}, {28, 512} })
            },
      { Fraction(3,4),
            Groups( { { 4, 512}, { 8, 273}, { 12, 512}, {16, 273}, { 20, 512} })
            },
      { Fraction(2,4),
            Groups( { { 4, 512}, { 8, 273}, { 12, 512}, {0, 0} })
            },
      { Fraction(6,8),
            Groups( { { 4, 512}, { 8, 512}, { 12, 273}, { 16, 512}, { 20, 512}, {0, 0} })
            },
      { Fraction(9,8),
            Groups( { { 4, 512}, { 8, 512}, { 12, 273}, { 16, 512}, { 20, 512}, {24, 273}, { 18, 512}, { 32, 512} })
            },
      { Fraction(12,8),
            Groups( { { 4, 512}, { 8, 512}, { 12, 273}, { 16, 512}, { 20, 512}, {24, 273}, { 18, 512}, { 32, 512}, {36, 273}, { 40, 512}, { 44, 512} })
            },
      { Fraction(6,4),
            Groups( { { 4, 512}, { 8, 512}, { 12, 512}, { 16, 512}, { 20, 512}, { 24, 273}, { 28, 512}, { 32, 512}, { 36, 512}, { 40, 512}, { 44, 512} })
            },
      { Fraction(3,2),
            Groups( { { 4, 512}, { 8, 272}, { 12, 512}, { 16, 273}, { 20, 512}, { 24, 272}, { 28, 512}, { 32, 273}, { 36, 512}, { 40, 272}, { 44, 512} })
            },
      { Fraction(5,4),
            Groups( { { 4, 512}, { 8, 512}, { 12, 512}, { 16, 512}, { 20, 512}, { 24, 273}, { 28, 512}, { 32, 512}, { 36, 512} })
            },
      { Fraction(7,8),
            Groups( { { 4, 512}, { 8, 512}, { 12, 273}, { 16, 512}, { 20, 273}, { 24, 512} })
            },
      { Fraction(5,8),
            Groups( { { 4, 512}, { 8, 512}, { 12, 273}, { 16, 512} })
            },
      };

//---------------------------------------------------------
//   endBeam
//---------------------------------------------------------

BeamMode Groups::endBeam(ChordRest* cr)
      {
      if (cr->beamMode() != BeamMode::AUTO)
            return cr->beamMode();
      Q_ASSERT(cr->staff());

      if (cr->tuplet() && !cr->tuplet()->elements().isEmpty()) {
            if (cr->tuplet()->elements().front() == cr)     // end beam at new tuplet
                  return BeamMode::BEGIN;
            if (cr->tuplet()->elements().back() == cr)      // end beam at tuplet end
                  return BeamMode::END;
            return BeamMode::AUTO;
            }

      TDuration d = cr->durationType();
      const Groups& g = cr->staff()->group(cr->tick());
      return g.beamMode(cr->rtick(), d.type());
      }

//---------------------------------------------------------
//   beamMode
//---------------------------------------------------------

BeamMode Groups::beamMode(int tick, TDuration::DurationType d) const
      {
      int shift;
      switch (d) {
            case TDuration::DurationType::V_EIGHT: shift = 0; break;
            case TDuration::DurationType::V_16TH:  shift = 4; break;
            case TDuration::DurationType::V_32ND:  shift = 8; break;
            default:
                  return BeamMode::AUTO;
            }
      for (const GroupNode& e : *this) {
            if (e.pos * 60 < tick)
                  continue;
            if (e.pos * 60 > tick)
                  break;

            int action = (e.action >> shift) & 0xf;
            switch (action) {
                  case 0: return BeamMode::AUTO;
                  case 1: return BeamMode::BEGIN;
                  case 2: return BeamMode::BEGIN32;
                  case 3: return BeamMode::BEGIN64;
                  default:
                        qDebug("   Groups::beamMode: bad action %d", action);
                        return BeamMode::AUTO;
                  }
            }
      return BeamMode::AUTO;
      }

//---------------------------------------------------------
//   endings
//---------------------------------------------------------

const Groups& Groups::endings(const Fraction& f)
      {
      for (const NoteGroup& g : noteGroups) {
            if (g.timeSig.identical(f)) {
                  return g.endings;
                  }
            }
      NoteGroup g;
      g.timeSig = f;
      noteGroups.push_back(g);

      int pos = 0;
      switch(f.denominator()) {
            case 2:     pos = 16; break;
            case 4:     pos = 8; break;
            case 8:     pos = 4; break;
            case 16:    pos = 2; break;
            case 32:    pos = 1; break;
                  break;
            }
      for (int i = 1; i < f.numerator(); ++i) {
            GroupNode n;
            n.pos    = pos * i;
            n.action = 0x111;
            g.endings.push_back(n);
            }
      return noteGroups.back().endings;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Groups::write(Xml& xml) const
      {
      xml.stag("Groups");
      for (const GroupNode& n : *this)
            xml.tagE(QString("Node pos=\"%1\" action=\"%3\"")
               .arg(n.pos).arg(n.action));
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Groups::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Node") {
                  GroupNode n;
                  n.pos    = e.intAttribute("pos");
                  n.action = e.intAttribute("action");
                  push_back(n);
                  e.skipCurrentElement();
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   addStop
//---------------------------------------------------------

void Groups::addStop(int pos, TDuration::DurationType d, BeamMode bm)
      {
      int shift;
      switch (d) {
            case TDuration::DurationType::V_EIGHT: shift = 0; break;
            case TDuration::DurationType::V_16TH:  shift = 4; break;
            case TDuration::DurationType::V_32ND:  shift = 8; break;
            default:
                  return;
            }
      int action;
      if (bm == BeamMode::BEGIN)
            action = 1;
      else if (bm == BeamMode::BEGIN32)
            action = 2;
      else if (bm == BeamMode::BEGIN64)
            action = 3;
      else
            return;

      pos    /= 60;
      action <<= shift;

      auto i = begin();
      for (; i != end(); ++i) {
            if (i->pos == pos) {
                  i->action = (i->action & ~(0xf << shift)) | action;
                  return;
                  }
            if (i->pos > pos)
                  break;
            }
      insert(i, GroupNode({ pos, action }));
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Groups::dump(const char* m) const
      {
      printf("%s\n", m);
      for (const GroupNode& n : *this) {
            printf("  group tick %d action 0x%02x\n", n.pos * 60, n.action);
            }
      }

}

