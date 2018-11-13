//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: mixer.h 4388 2011-06-18 13:17:58Z wschweer $
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

#ifndef __MIXERTRACKGROUP_H__
#define __MIXERTRACKGROUP_H__

namespace Ms {

class Part;
class MixerTrack;

//---------------------------------------------------------
//   MixerTrackGroup
//---------------------------------------------------------

class MixerTrackGroup
      {
public:
      virtual ~MixerTrackGroup() {}
      virtual void expandToggled(Part* part, bool expanded) = 0;
      virtual void notifyTrackSelected(MixerTrack* track) = 0;
      };

}

#endif // MIXERTRACKGROUP_H
