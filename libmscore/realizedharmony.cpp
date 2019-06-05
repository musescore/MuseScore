//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "realizedharmony.h"

namespace Ms {


//---------------------------------------------------
//   setDescription
///   sets the description and dirty flag if the passed
///   chord description is different than current
//---------------------------------------------------
void RealizedHarmony::setDescription(ChordDescription* cd)
      {
            if (description == cd)
                  return;
            description = cd;
            dirty = 1;
      }

//---------------------------------------------------
//   setVoicing
///   sets the voicing and dirty flag if the passed
///   voicing is different than current
//---------------------------------------------------
void RealizedHarmony::setVoicing(Voicing v)
      {
            if (voicing == v)
                  return;
            voicing = v;
            dirty = 1;
      }

//---------------------------------------------------
//   setRhythm
///   sets the rhythm and dirty flag if the passed
///   rhythm is different than current
//---------------------------------------------------
void RealizedHarmony::setRhythm(Rhythm r)
      {
            if (rhythm == r)
                  return;
            rhythm = r;
            dirty = 1;
      }

//---------------------------------------------------
//   update
///   updates the current note map, this is where all
///   of the rhythm and voicing choices matter since
///   the voicing algorithms depend on this.
//---------------------------------------------------
void RealizedHarmony::update(int rootTpc, int bassTpc)
      {
      switch (voicing) {
            case Voicing::AUTO:
            default:
                  //TODO - PHV: fill out
                  break;
            }
      }

}
