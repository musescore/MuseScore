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

#ifndef __MIXERTRACKPART_H__
#define __MIXERTRACKPART_H__

#include "ui_mixertrackpart.h"
#include "libmscore/instrument.h"

namespace Ms {

class MidiMapping;
class MixerTrackItem;

/* Not used in the re-designed mixer */

class MixerTrackPart : public QWidget, public Ui::MixerTrackPart, public ChannelListener
      {
      Q_OBJECT

      MixerTrackItem* mixerTrackItem;

protected:
      void propertyChanged(Channel::Prop property) override;

public:
      explicit MixerTrackPart(QWidget *parent, MixerTrackItem* trackItem, bool expanded);
      };

}
#endif // __MIXERTRACKPART_H__
