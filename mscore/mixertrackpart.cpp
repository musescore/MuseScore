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
#include "seq.h"
#include "libmscore/undo.h"
#include "synthcontrol.h"
#include "synthesizer/msynthesizer.h"
#include "preferences.h"

namespace Ms {

const QString MixerTrackPart::unselStyleLight = "#controlWidget {"
                                       "      background: #aaa;"
                                       "      border-left: 2px solid #ddd;"
                                       "      border-top: 2px solid #ddd;"
                                       "      border-right: 2px solid #777;"
                                       "      border-bottom: 2px solid #777;"
                                       "}";

const QString MixerTrackPart::selStyleLight = "#controlWidget {"
                                     "      background: #ccc;"
                                     "      border-left: 2px solid #eee;"
                                     "      border-top: 2px solid #eee;"
                                     "      border-right: 2px solid #777;"
                                     "      border-bottom: 2px solid #777;"
                                     "}";


const QString MixerTrackPart::unselStyleDark = "#controlWidget {"
                                       "      background: #444;"
                                       "      border-left: 2px solid #888;"
                                       "      border-top: 2px solid #888;"
                                       "      border-right: 2px solid #000;"
                                       "      border-bottom: 2px solid #000;"
                                       "}"
                                       ".expandBn {"
                                       "     background-color: #aaa;"
                                       "}";

const QString MixerTrackPart::selStyleDark = "#controlWidget {"
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

MixerTrackPart::MixerTrackPart(QWidget *parent, MixerTrackItemPtr mti, bool expanded) :
      QWidget(parent), _mti(mti), _selected(false), _group(0)
      {
      setupUi(this);

      int numChannels = 0;
      Part* part = _mti->part();
      const InstrumentList* il = part->instruments();
      for (auto it = il->begin(); it != il->end(); ++it) {
            Instrument* instr = it->second;
            numChannels += instr->channel().size();
            }

      expandBn->setEnabled(numChannels > 1);
      expandBn->setChecked(expanded);

      connect(expandBn, SIGNAL(toggled(bool)), SLOT(expandToggled(bool)));

      connect(soloBn, SIGNAL(toggled(bool)), SLOT(updateSolo(bool)));
      connect(muteBn, SIGNAL(toggled(bool)), SLOT(updateMute(bool)));

      updateNameLabel();

      //set up rest

      Channel* chan = _mti->focusedChan();

      soloBn->setChecked(chan->solo());
      muteBn->setChecked(chan->mute());

      chan->addListener(this);
      volumeSlider->setValue(chan->volume());
      volumeSlider->setToolTip(tr("Volume: %1").arg(QString::number(chan->volume())));
      volumeSlider->setMaxValue(127);
      volumeSlider->setMinValue(0);
      volumeSlider->setNumMajorTicks(10);
      volumeSlider->setNumMinorTicks(4);

      QIcon iconSliderHead;
      iconSliderHead.addFile(QStringLiteral(":/data/icons/mixer-slider-handle-vertical.svg"), QSize(), QIcon::Normal, QIcon::Off);
      volumeSlider->setSliderHeadIcon(iconSliderHead);

      panSlider->setValue(chan->pan());
      panSlider->setToolTip(tr("Pan: %1").arg(QString::number(chan->pan())));
      panSlider->setMaxValue(127);
      panSlider->setMinValue(0);
      panSlider->setDclickValue1(64);

      connect(volumeSlider, SIGNAL(valueChanged(double)),      SLOT(volumeChanged(double)));
      connect(panSlider,    SIGNAL(valueChanged(double, int)), SLOT(panChanged(double)));

      connect(volumeSlider, SIGNAL(sliderPressed()),    SLOT(controlSelected()));
      connect(panSlider,    SIGNAL(sliderPressed(int)), SLOT(controlSelected()));

      applyStyle();
      }

//---------------------------------------------------------
//   expandToggled
//---------------------------------------------------------

void MixerTrackPart::applyStyle()
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
//   expandToggled
//---------------------------------------------------------

void MixerTrackPart::expandToggled(bool expanded)
      {
      _group->expandToggled(_mti->part(), expanded);
      }

//---------------------------------------------------------
//   updateNameLabel
//---------------------------------------------------------

void MixerTrackPart::updateNameLabel()
      {
      Part* part = _mti->part();
      Channel* chan = _mti->focusedChan();
      trackLabel->setText(part->partName());

      MidiPatch* mp = synti->getPatchInfo(chan->synti(), chan->bank(), chan->program());


      QString tooltip = tr("Part Name: %1\n"
                                "Primary Instrument: %2\n"
                                "Bank: %3\n"
                                "Program: %4\n"
                                "Patch: %5")
                  .arg(part->partName(),
                       part->longName(),
                       QString::number(chan->bank()),
                       QString::number(chan->program()),
                       mp ? mp->name : tr("~no patch~"));

      trackLabel->setToolTip(tooltip);

      QColor bgCol((QRgb)part->color());
      QString colName = bgCol.name();
      int val = bgCol.value();

      QString ss = QString(".QLabel {"
                 "border: 2px solid black;"
                 "background: %1;"
                "color: %2;"
                 "padding: 6px 0px;"
             "}").arg(colName, val > 128 ? "black" : "white");

      trackLabel->setStyleSheet(ss);

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

void MixerTrackPart::paintEvent(QPaintEvent*)
      {
      applyStyle();
      }

//---------------------------------------------------------
//   propertyChanged
//---------------------------------------------------------

void MixerTrackPart::propertyChanged(Channel::Prop property)
      {
      Channel* chan = _mti->focusedChan();

      switch (property) {
            case Channel::Prop::VOLUME: {
                  volumeSlider->blockSignals(true);
                  volumeSlider->setValue(chan->volume());
                  volumeSlider->setToolTip(tr("Volume: %1").arg(QString::number(chan->volume())));
                  volumeSlider->blockSignals(false);
                  break;
                  }
            case Channel::Prop::PAN: {
                  panSlider->blockSignals(true);
                  panSlider->setValue(chan->pan());
                  panSlider->setToolTip(tr("Pan: %1").arg(QString::number(chan->pan())));
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

void MixerTrackPart::volumeChanged(double value)
      {
      _mti->setVolume(value);
      volumeSlider->setToolTip(tr("Volume: %1").arg(QString::number(value)));
      }

//---------------------------------------------------------
//   panChanged
//---------------------------------------------------------

void MixerTrackPart::panChanged(double value)
      {
      _mti->setPan(value);
      panSlider->setToolTip(tr("Pan: %1").arg(QString::number(value)));
      }

//---------------------------------------------------------
//   updateSolo
//---------------------------------------------------------

void MixerTrackPart::updateSolo(bool val)
      {
      _mti->setSolo(val);
      }

//---------------------------------------------------------
//   udpateMute
//---------------------------------------------------------

void MixerTrackPart::updateMute(bool val)
      {
      _mti->setMute(val);
      }

//---------------------------------------------------------
//   controlSelected
//---------------------------------------------------------

void MixerTrackPart::controlSelected()
      {
      setSelected(true);
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void MixerTrackPart::mouseReleaseEvent(QMouseEvent*)
      {
      setSelected(true);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void MixerTrackPart::setSelected(bool sel)
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
