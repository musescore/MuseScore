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

const QString MixerTrackPart::unselStyle = "#controlWidget {"
                                       "      background: #444;"
                                       "      border-left: 2px solid #888;"
                                       "      border-top: 2px solid #888;"
                                       "      border-right: 2px solid #000;"
                                       "      border-bottom: 2px solid #000;"
                                       "}";

const QString MixerTrackPart::selStyle = "#controlWidget {"
                                     "      background: #666;"
                                     "      border-left: 2px solid #888;"
                                     "      border-top: 2px solid #888;"
                                     "      border-right: 2px solid #000;"
                                     "      border-bottom: 2px solid #000;"
                                     "}";

const QString MixerTrackPart::sliderStyle = ".QSlider::groove:vertical {"
                            "    border: 1px solid #111;"
                            "    background-color: #333;"
                            "    width: 6px;"
                            "    margin: 24px 12px;"
                            "}"

                            "QSlider::handle:vertical {"
                            "    image: url(:/data/icons/mixer-slider-handle-vertical.svg);"
                            "    margin: -24px -12px;"
                            "    height: -1px;"
                            "}"

                            "QSlider::sub-page:vertical {"
                            "    background: #222;"
                            "    border-left: 1px solid #000;"
                            "    border-top: 1px solid #000;"
                            "    border-bottom: 1px solid #888;"
                            "    border-right: 1px solid #888;"
                            "    border-radius:4px;"
                            "    margin:12px 12px;"
                            "}"

                            "QSlider::add-page:vertical {"
                            "    background: %1;"
                            "    border-left: 1px solid #000;"
                            "    border-top: 1px solid #000;"
                            "    border-bottom: 1px solid #888;"
                            "    border-right: 1px solid #888;"
                            "    border-radius:4px;"
                            "    margin:12px 12px;"
                            "}";



//---------------------------------------------------------
//   MixerTrack
//---------------------------------------------------------

MixerTrackPart::MixerTrackPart(QWidget *parent, MixerTrackItemPtr mti, bool expanded) :
      QWidget(parent), _mti(mti)
      {
      setupUi(this);

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
      volumeSlider->setToolTip("Volume: " + QString::number(chan->volume()));
      volumeSlider->setMaxValue(100);
      volumeSlider->setMajorTickSpacing(10);
      volumeSlider->setMinorTickSpacing(5);

      QIcon iconSliderHead;
      iconSliderHead.addFile(QStringLiteral(":/data/icons/mixer-slider-handle-vertical.svg"), QSize(), QIcon::Normal, QIcon::Off);
      volumeSlider->setSliderHeadIcon(iconSliderHead);

      panSlider->setValue(chan->pan());
      panSlider->setToolTip("Pan: " + QString::number(chan->pan()));
      panSlider->setMaxValue(180);
      panSlider->setMinValue(-180);

//      panSlider->setEnableMouseWheel(false);
//      QIcon icon3;
//      icon3.addFile(QStringLiteral(":/data/icons/mixer-dial.svg"), QSize(), QIcon::Normal, QIcon::Off);
//      panSlider->setKnobIcon(icon3);

      connect(volumeSlider, SIGNAL(valueChanged(double)),         SLOT(volumeChanged(double)));
      connect(panSlider,    SIGNAL(valueChanged(double, int)), SLOT(panChanged(double)));

      connect(volumeSlider, SIGNAL(sliderPressed()), SLOT(controlSelected()));
      connect(panSlider, SIGNAL(sliderPressed(int)), SLOT(controlSelected()));
      }

//---------------------------------------------------------
//   ~MixerTrack
//---------------------------------------------------------

MixerTrackPart::~MixerTrackPart()
      {
      Channel* chan = _mti->focusedChan();
      chan->removeListener(this);

//      qDebug("_destruct MixerTrack %p", this);
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
//      QString partName = part->partName();
//      if (!chan->name().isEmpty()) {
//            if (chan->name() != Channel::DEFAULT_NAME) {
//                  partName += "-";
//                  partName += qApp->translate("InstrumentsXML", chan->name().toUtf8().data());
//                  }
//            }
      QString text = QString("%1")
                  .arg(part->partName());
      trackLabel->setText(text);

      MidiPatch* mp = synti->getPatchInfo(chan->synti(), chan->bank(), chan->program());


      QString tooltip = QString("Part Name: %1\n"
                                "Bank: %4\nProgram: %5\nPatch: %6")
                  .arg(part->partName(),
                       QString::number(chan->bank()),
                       QString::number(chan->program()),
                       mp ? mp->name : "~no patch~");
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
//      volumeSlider->setStyleSheet(sliderStyle.arg(brightCol.name()));
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

void MixerTrackPart::volumeChanged(double value)
      {
      _mti->setVolume(value);
      volumeSlider->setToolTip("Volume: " + QString::number(value));
      }

//---------------------------------------------------------
//   panChanged
//---------------------------------------------------------

void MixerTrackPart::panChanged(double value)
      {
      _mti->setPan((int)value);
      panSlider->setToolTip("Pan: " + QString::number(value));
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

void MixerTrackPart::mouseReleaseEvent(QMouseEvent * event)
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
      setStyleSheet(_selected ? selStyle : unselStyle);

      emit(selectedChanged(sel));

      if (_selected && _group)
            _group->notifyTrackSelected(this);
      }

}
