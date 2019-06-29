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

#ifndef __MIXERTRACKCHANNEL_H__
#define __MIXERTRACKCHANNEL_H__

#include "ui_mixertrackchannel.h"
#include "mixertrackgroup.h"
#include "mixertrack.h"
#include "mixertrackitem.h"
#include "libmscore/instrument.h"
#include "awl/fastlog.h"    // for the log volume slider


// obq-note
// This class has been re-purposed for the new mixer design.
// This is the Widget that is included in the TreeView
// It shows volume, mute, and solo controls.
//
// It is a ChannelListener for the channel represented by the
// mixerTrackItem. So when the underlying channel changes this
// object will receive a propertyChanged call.

namespace Ms {

class MixerTrackItem;

//---------------------------------------------------------
//   MixerTrack
//---------------------------------------------------------

class MixerTrackChannel : public QWidget, public Ui::MixerTrackChannel, public ChannelListener
      {
      Q_OBJECT

      MixerTreeWidgetItem * treeWidgetItem;   // to enable selecting item when user interacts with controls

      void setupAdditionalUi();
      void setupSlotsAndSignals();
      void update();
      MixerTrackItem* mixerTrackItem() {return treeWidgetItem->mixerTrackItem; };
      
public slots:
      void stripMuteToggled(bool);
      void stripSoloToggled(bool);
      void stripVolumeSliderMoved(int);
      void takeSelection();

      void updateUiControls(); // for showing/hiding color and panning

      signals:
      void userInteraction(QTreeWidgetItem*);

protected:
      void propertyChanged(Channel::Prop property) override;      // ChannelListener method
            
public:
      explicit MixerTrackChannel(MixerTreeWidgetItem*);
      };


// Class for the "master volume" control - borrows the UI from MixerTrackChannel
class MixerMasterChannel : public QWidget, public Ui::MixerTrackChannel
      {
      Q_OBJECT

      void setupAdditionalUi();
      void setupSlotsAndSignals();
      void update();

      public slots:
      void masterVolumeSliderMoved(int);
      void updateUiControls(); // for showing/hiding color and panning

      public:
      explicit MixerMasterChannel();
      void volumeChanged(float);
      };


class MixerVolumeSlider : public QSlider
      {
      Q_OBJECT
      double _positionValue;
      double _minValue;
      double _maxValue;

      void setMinLogValue(double min);
      void setMaxLogValue(double max);
      void setLogRange(double min, double max) { setMinLogValue(min); setMaxLogValue(max); };

   public:
      MixerVolumeSlider(QWidget* parent);

      double doubleValue() const;
      void setDoubleValue(double);

      void sliderChange(QAbstractSlider::SliderChange change) override;

      signals:
      void doubleValueChanged(double);
      };

}
#endif // __MIXERTRACKCHANNEL_H__
