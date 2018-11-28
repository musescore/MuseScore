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

namespace Ms {

struct MidiMapping;
class MixerTrackItem;

//---------------------------------------------------------
//   MixerTrack
//---------------------------------------------------------

class MixerTrackChannel : public QWidget, public Ui::MixerTrackChannel, public ChannelListener, public MixerTrack
      {
      Q_OBJECT

      MixerTrackItemPtr _mti;

      bool _selected;
      static const QString unselStyleLight;
      static const QString selStyleLight;
      static const QString unselStyleDark;
      static const QString selStyleDark;
      static const QString sliderStyle;

      MixerTrackGroup* _group;

      void updateNameLabel();

signals:
      void selectedChanged(bool);

public slots:
      void updateSolo(bool);
      void updateMute(bool);
      void setSelected(bool) override;
      void volumeChanged(double);
      void panChanged(double);
      void controlSelected();
      void applyStyle();

protected:
      void mouseReleaseEvent(QMouseEvent * event) override;
      void propertyChanged(Channel::Prop property) override;

public:
      explicit MixerTrackChannel(QWidget *parent, MixerTrackItemPtr trackItem);

      bool selected() override { return _selected; }
      QWidget* getWidget() override { return this; }
      MixerTrackGroup* group() override { return _group; }
      MixerTrackItemPtr mti() override { return _mti; }
      void setGroup(MixerTrackGroup* group) { _group = group; }
      void paintEvent(QPaintEvent* evt) override;
      };
}

#endif // __MIXERTRACKCHANNEL_H__
