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

#ifndef __MIXERMASTERCHANNEL_H__
#define __MIXERMASTERCHANNEL_H__

#include "ui_mixermasterchannel.h"
#include "mixertrackitem.h"
#include "libmscore/instrument.h"


namespace Ms {

class MixerMasterChannel : public QWidget, public Ui::MixerMasterChannel
      {
      Q_OBJECT

      void setupAdditionalUi();
      void setupSlotsAndSignals();
      void update();

      public slots:
      void masterVolumeSliderMoved(int);
      void updateUiControls(); // for showing/hiding color

      public:
      explicit MixerMasterChannel();
      void volumeChanged(float);
      };

} // namespace Ms
#endif // __MIXERMASTERCHANNEL_H__
