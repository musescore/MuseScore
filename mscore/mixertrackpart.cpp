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

#include "mixertrackpart.h"

#include "musescore.h"

#include "libmscore/score.h"
#include "libmscore/part.h"
#include "mixer.h"
#include "mixertrackitem.h"


namespace Ms {

/* Not used in the re-designed mixer */

MixerTrackPart::MixerTrackPart(QWidget *parent, MixerTrackItem* mixerTrackItem, bool expanded) :
      QWidget(parent), mixerTrackItem(mixerTrackItem)
      {
      setupUi(this);
      }

void MixerTrackPart::propertyChanged(Channel::Prop property)
      {
      }

}
