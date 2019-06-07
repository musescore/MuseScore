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
#include "pitchspelling.h"
#include "chordlist.h"
#include "harmony.h"

namespace Ms {

//---------------------------------------------------
//   RealizedHarmony
///   creates empty realized harmony
//---------------------------------------------------
RealizedHarmony::RealizedHarmony(Harmony* h) : _harmony(h), _notes(QMap<int, int>()),
      _voicing(Voicing::AUTO), _rhythm(Rhythm::AUTO), _dirty(1)
      {
      //TODO - PHV
      }

//---------------------------------------------------
//   setVoicing
///   sets the voicing and dirty flag if the passed
///   voicing is different than current
//---------------------------------------------------
void RealizedHarmony::setVoicing(Voicing v)
      {
      if (_voicing == v)
            return;
      _voicing = v;
      _dirty = 1;
      }

//---------------------------------------------------
//   setRhythm
///   sets the rhythm and dirty flag if the passed
///   rhythm is different than current
//---------------------------------------------------
void RealizedHarmony::setRhythm(Rhythm r)
      {
      if (_rhythm == r)
            return;
      _rhythm = r;
      _dirty = 1;
      }

//---------------------------------------------------
//   notes
///   returns the list of notes
//---------------------------------------------------
const QMap<int, int>& RealizedHarmony::notes() const
      {
      //TODO - PHV: do something when dirty?
      return _notes;
      }

//---------------------------------------------------
//   update
///   updates the current note map, this is where all
///   of the rhythm and voicing choices matter since
///   the voicing algorithms depend on this.
//---------------------------------------------------
void RealizedHarmony::update(int rootTpc, int bassTpc)
      {
      if (!_dirty)
            return;

      _notes.clear();
      switch (_voicing) {
            case Voicing::ROOT_ONLY:
            case Voicing::AUTO:
            default:
                  _notes.insert(tpc2pitch(rootTpc) + 48, rootTpc);
                  break;
            }
      }

}
