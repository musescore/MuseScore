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

#ifndef __MIXERDETAILS_H__
#define __MIXERDETAILS_H__

#include "ui_mixerdetails.h"
#include "libmscore/instrument.h"
#include "mixertrackitem.h"

#include <QPointer>

namespace Ms {


class MixerTrackItem;


//---------------------------------------------------------
//   MixerDetails
//---------------------------------------------------------

class MixerDetails : public QWidget, public Ui::MixerDetails
      {
      Q_OBJECT

      QPointer<MixerTrackItem> _mti;

      void updateFromTrack();

public slots:
      void partNameChanged();
      void trackColorChanged(QColor);
      void patchChanged(int);
      void volumeChanged(double);
      void panChanged(double);
      void chorusChanged(double);
      void reverbChanged(double);
      void drumkitToggled(bool);
      void midiChannelChanged(int);

      void notifyVolumeChanged(char);
      void notifyPanChanged(char);
      void notifyChorusChanged(char);
      void notifyReverbChanged(char);
      void notifyColorChanged(int);
      void notifyPartNameChanged(QString);
public:
      explicit MixerDetails(QWidget *parent);
      ~MixerDetails() override;

      MixerTrackItem* track() { return _mti; }
      void setTrack(MixerTrackItem* track);
//      void propertyChanged(Channel::Prop property) override;
//      void disconnectChannelListener() override;

      };

}
#endif // __MIXERDETAILS_H__
