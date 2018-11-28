//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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

#ifndef __MIXERTRACK_H__
#define __MIXERTRACK_H__

#include "mixertrackitem.h"
#include <QWidget>

class MixerTrackGroup;

namespace Ms {

class MixerTrack {
public:
      virtual ~MixerTrack() {}
      virtual QWidget* getWidget() = 0;
      virtual MixerTrackGroup* group() = 0;
      virtual MixerTrackItemPtr mti() = 0;
      virtual bool selected() = 0;
      virtual void setSelected(bool) = 0;
      };

}

#endif // MIXERTRACK_H
