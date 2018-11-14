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

#include "mixertrackchannel.h"

#include "musescore.h"

#include "libmscore/score.h"
#include "libmscore/part.h"
#include "mixer.h"
#include "mixertrackitem.h"
#include "seq.h"
#include "libmscore/undo.h"
#include "synthcontrol.h"
#include "synthesizer/msynthesizer.h"
#include "preferences.h"

namespace Ms {

const QString MixerTrackChannel::unselStyleLight = "#controlWidget {"
                                       "      background: #aaa;"
                                       "      border-left: 2px solid #ddd;"
                                       "      border-top: 2px solid #ddd;"
                                       "      border-right: 2px solid #777;"
                                       "      border-bottom: 2px solid #777;"
                                       "}";

const QString MixerTrackChannel::selStyleLight = "#controlWidget {"
                                     "      background: #ccc;"
                                     "      border-left: 2px solid #eee;"
                                     "      border-top: 2px solid #eee;"
                                     "      border-right: 2px solid #777;"
                                     "      border-bottom: 2px solid #777;"
                                     "}";


const QString MixerTrackChannel::unselStyleDark = "#controlWidget {"
                                       "      background: #444;"
                                       "      border-left: 2px solid #888;"
                                       "      border-top: 2px solid #888;"
                                       "      border-right: 2px solid #000;"
                                       "      border-bottom: 2px solid #000;"
                                       "}"
                                       ".expandBn {"
                                       "     background-color: #aaa;"
                                       "}";

const QString MixerTrackChannel::selStyleDark = "#controlWidget {"
                                     "      background: #666;"
                                     "      border-left: 2px solid #888;"
                                     "      border-top: 2px solid #888;"
                                     "      border-right: 2px solid #000;"
                                     "      border-bottom: 2px solid #000;"
                                     "}"
                                     ".expandBn {"
                                     "     background-color: #aaa;"
                                     "}";


//---------------------------------------------------------
//   MixerTrack
//---------------------------------------------------------

MixerTrackChannel::MixerTrackChannel(QWidget *parent, MixerTrackItemPtr mti) :
      QWidget(parent), _mti(mti), _selected(false), _group(0)
      {
      setupUi(this);

      connect(soloBn, SIGNAL(toggled(bool)), SLOT(updateSolo(bool)));
      connect(muteBn, SIGNAL(toggled(bool)), SLOT(updateMute(bool)));

      updateNameLabel();

      //set up rest
      Channel* chan = mti->chan();
      soloBn->setChecked(chan->solo());
      muteBn->setChecked(chan->mute());

      chan->addListener(this);
      volumeSlider->setValue(chan->volume());
      volumeSlider->setToolTip("Volume: " + QString::number(chan->volume()));
      volumeSlider->setMaxValue(127);
      volumeSlider->setNumMajorTicks(10);
      volumeSlider->setNumMinorTicks(5);

      QIcon iconSliderHead;
      iconSliderHead.addFile(QStringLiteral(":/data/icons/mixer-slider-handle-vertical.svg"), QSize(), QIcon::Normal, QIcon::Off);
      volumeSlider->setSliderHeadIcon(iconSliderHead);

      panSlider->setValue(chan->pan());
      panSlider->setToolTip("Pan: " + QString::number(chan->pan()));
      panSlider->setMaxValue(127);
      panSlider->setMinValue(0);

      connect(volumeSlider, SIGNAL(valueChanged(double)),      SLOT(volumeChanged(double)));
      connect(panSlider,    SIGNAL(valueChanged(double, int)), SLOT(panChanged(double)));

      connect(volumeSlider, SIGNAL(sliderPressed()), SLOT(controlSelected()));
      connect(panSlider, SIGNAL(sliderPressed(int)), SLOT(controlSelected()));

      applyStyle();
      }

//---------------------------------------------------------
//   ~MixerTrack
//---------------------------------------------------------

MixerTrackChannel::~MixerTrackChannel()
      {
      if (_mti) {
            Channel* chan = _mti->chan();
            chan->removeListener(this);
            }
      }

//---------------------------------------------------------
//   expandToggled
//---------------------------------------------------------

void MixerTrackChannel::applyStyle()
      {
      QString style;
      switch (preferences.globalStyle()){
            case MuseScoreStyleType::DARK_FUSION:
                  style = _selected ? selStyleDark : unselStyleDark;
                  break;
            case MuseScoreStyleType::LIGHT_FUSION:
                  style = _selected ? selStyleLight : unselStyleLight;
                  break;
            }

      setStyleSheet(style);
      }

//---------------------------------------------------------
//   updateNameLabel
//---------------------------------------------------------

void MixerTrackChannel::updateNameLabel()
      {
      Part* part = _mti->part();
      Instrument* instr = _mti->instrument();
      Channel* chan = _mti->chan();

      QString shortName;
      if (instr->shortNames().count())
            shortName = instr->shortNames().first().name() + "-";
      else
            shortName = "";
      QString text = QString("%1%2").arg(shortName, chan->name());
      trackLabel->setText(text);

      MidiPatch* mp = synti->getPatchInfo(chan->synti(), chan->bank(), chan->program());

      QString tooltip = QString("Part Name: %1\n"
                                "Instrument: %2\n"
                                "Channel: %3\n"
                                "Bank: %4\n"
                                "Program: %5\n"
                                "Patch: %6")
                  .arg(part->partName(),
                       instr->trackName(),
                       chan->name(),
                       QString::number(chan->bank()),
                       QString::number(chan->program()),
                       mp ? mp->name : "~no patch~");

      trackLabel->setToolTip(tooltip);

      QColor bgCol((QRgb)chan->color());
      QString trackColorName = bgCol.name();
      int val = bgCol.value();

      QString trackStyle = QString(".QLabel {"
                 "border: 2px solid black;"
                 "background: %1;"
                 "color: %2;"
                 "padding: 6px 0px;"
             "}").arg(trackColorName, val > 128 ? "black" : "white");

      trackLabel->setStyleSheet(trackStyle);

      QColor bgPartCol((QRgb)part->color());
      QString partColorName = bgPartCol.name();
      val = bgPartCol.value();

      //Part header
      partLabel->setText(part->partName());

      QString partStyle = QString(".QLabel {"
                 "border: 2px solid black;"
                 "background: %1;"
                 "color: %2;"
                 "padding: 6px 0px;"
             "}").arg(partColorName, val > 128 ? "black" : "white");

      partLabel->setStyleSheet(partStyle);
      partLabel->setToolTip(QString("This channel is a child of part %1").arg(part->partName()));



      //Update component colors
      qreal h, s, v;
      bgCol.getHsvF(&h, &s, &v);
      QColor brightCol = QColor::fromHsvF(h, s, 1);
      panSlider->setScaleValueColor(brightCol);
      volumeSlider->setHilightColor(brightCol);
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void MixerTrackChannel::paintEvent(QPaintEvent*)
      {
      applyStyle();
      }


//---------------------------------------------------------
//   disconnectChannelListener
//---------------------------------------------------------

void MixerTrackChannel::disconnectChannelListener()
      {
      //Channel has been destroyed.  Don't remove listener when invoking destructor.
      _mti = nullptr;
      }

//---------------------------------------------------------
//   propertyChanged
//---------------------------------------------------------

void MixerTrackChannel::propertyChanged(Channel::Prop property)
      {
      Channel* chan = _mti->chan();

      switch (property) {
            case Channel::Prop::VOLUME: {
                  volumeSlider->blockSignals(true);
                  volumeSlider->setValue(chan->volume());
                  volumeSlider->setToolTip("Volume: " + QString::number(chan->volume()));
                  volumeSlider->blockSignals(false);
                  break;
                  }
            case Channel::Prop::PAN: {
                  panSlider->blockSignals(true);
                  panSlider->setValue(chan->pan());
                  panSlider->setToolTip("Pan: " + QString::number(chan->pan()));
                  panSlider->blockSignals(false);
                  break;
                  }
            case Channel::Prop::MUTE: {
                  muteBn->blockSignals(true);
                  muteBn->setChecked(chan->mute());
                  muteBn->blockSignals(false);
                  break;
                  }
            case Channel::Prop::SOLO: {
                  soloBn->blockSignals(true);
                  soloBn->setChecked(chan->solo());
                  soloBn->blockSignals(false);
                  break;
                  }
            case Channel::Prop::COLOR: {
                  updateNameLabel();
                  break;
                  }
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   volumeChanged
//---------------------------------------------------------

void MixerTrackChannel::volumeChanged(double value)
      {
      _mti->setVolume(value);
      volumeSlider->setToolTip("Volume: " + QString::number(value));
      }

//---------------------------------------------------------
//   panChanged
//---------------------------------------------------------

void MixerTrackChannel::panChanged(double value)
      {
      _mti->setPan(value);
      panSlider->setToolTip("Pan: " + QString::number(value));
      }

//---------------------------------------------------------
//   updateSolo
//---------------------------------------------------------

void MixerTrackChannel::updateSolo(bool val)
      {
      _mti->setSolo(val);
      }

//---------------------------------------------------------
//   udpateMute
//---------------------------------------------------------

void MixerTrackChannel::updateMute(bool val)
      {
      _mti->setMute(val);
      }

//---------------------------------------------------------
//   controlSelected
//---------------------------------------------------------

void MixerTrackChannel::controlSelected()
      {
      setSelected(true);
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void MixerTrackChannel::mouseReleaseEvent(QMouseEvent*)
      {
      setSelected(true);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void MixerTrackChannel::setSelected(bool sel)
      {
      if (_selected == sel)
            return;

      _selected = sel;
      applyStyle();

      emit(selectedChanged(sel));

      if (_selected && _group)
            _group->notifyTrackSelected(this);
      }

}
