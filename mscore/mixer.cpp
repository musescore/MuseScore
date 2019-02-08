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

#include "musescore.h"
#include "parteditbase.h"

#include "libmscore/excerpt.h"
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "mixer.h"
#include "seq.h"
#include "libmscore/undo.h"
#include "synthcontrol.h"
#include "synthesizer/msynthesizer.h"
#include "preferences.h"
#include <QtGlobal>
#include <qmessagebox.h>
#include <accessibletoolbutton.h>
#include "mixerdetails.h"
#include "mixertrack.h"
#include "mixertrackchannel.h"
#include "mixertrackpart.h"
#include "mixertrackitem.h"

namespace Ms {

#define _setValue(__x, __y) \
      __x->blockSignals(true); \
      __x->setValue(__y); \
      __x->blockSignals(false);

#define _setChecked(__x, __y) \
      __x->blockSignals(true); \
      __x->setChecked(__y); \
      __x->blockSignals(false);


double volumeToUserRange(char v) { return v * 100.0 / 128.0; }
double panToUserRange(char v) { return (v / 128.0) * 360.0; }
double chorusToUserRange(char v) { return v * 100.0 / 128.0; }
double reverbToUserRange(char v) { return v * 100.0 / 128.0; }

const float minDecibels = -3;

//0 to 100
char userRangeToVolume(double v) { return (char)qBound(0, (int)(v / 100.0 * 128.0), 127); }
//-180 to 180
char userRangeToPan(double v) { return (char)qBound(0, (int)((v / 360.0) * 128.0), 127); }
//0 to 100
char userRangeToChorus(double v) { return (char)qBound(0, (int)(v / 100.0 * 128.0), 127); }
//0 to 100
char userRangeToReverb(double v) { return (char)qBound(0, (int)(v / 100.0 * 128.0), 127); }

//---------------------------------------------------------
//   Mixer
//---------------------------------------------------------

Mixer::Mixer(QWidget* parent)
    : QDockWidget("Mixer", parent),
      showDetails(true),
      trackHolder(nullptr)
      {
      setupUi(this);

      setWindowFlags(Qt::Tool);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));

      trackAreaLayout = new QHBoxLayout;
      trackAreaLayout->setMargin(0);
      trackAreaLayout->setSpacing(0);
      trackArea->setLayout(trackAreaLayout);

      mixerDetails = new MixerDetails(this);
      detailsLayout = new QGridLayout(this);

      detailsLayout->addWidget(mixerDetails);
      detailsLayout->setContentsMargins(0, 0, 0, 0);
      detailsArea->setLayout(detailsLayout);

      //Range in decibels
      masterSlider->setMaxValue(0);
      masterSlider->setMinValue(minDecibels);
      masterSlider->setNumMinorTicks(4);
      masterSlider->setNumMajorTicks(3);
      masterSlider->setHilightColor(QColor(51, 153, 255));
      float decibels = qBound(minDecibels, log10(synti->gain()), 0.0f);
      masterSlider->setValue(decibels);
      masterSlider->setToolTip(tr("Decibels: %1").arg(decibels));

      masterSpin->setMaximum(0);
      masterSpin->setMinimum(minDecibels);
      masterSpin->setSingleStep(.1);
      masterSpin->setValue(decibels);


      QIcon iconSliderHead;
      iconSliderHead.addFile(QStringLiteral(":/data/icons/mixer-slider-handle-vertical.svg"), QSize(), QIcon::Normal, QIcon::Off);
      masterSlider->setSliderHeadIcon(iconSliderHead);

      connect(toggleDetailsButton, &QPushButton::toggled, this, &Mixer::showDetailsToggled);
      connect(masterSlider, SIGNAL(valueChanged(double)), SLOT(masterVolumeChanged(double)));
      connect(masterSpin, SIGNAL(valueChanged(double)), SLOT(masterVolumeChanged(double)));
      connect(synti, SIGNAL(gainChanged(float)), SLOT(synthGainChanged(float)));
      connect(tracks_scrollArea->horizontalScrollBar(), SIGNAL(rangeChanged(int, int)), SLOT(adjustScrollPosition(int, int)));
      connect(tracks_scrollArea->horizontalScrollBar(), SIGNAL(valueChanged(int)), SLOT(checkKeptScrollValue(int)));

      enablePlay = new EnablePlayForWidget(this);
      readSettings();
      retranslate(true);
      }

//---------------------------------------------------------
//   showDetailsToggled
//---------------------------------------------------------

void Mixer::showDetailsToggled(bool shown)
      {
      showDetails = shown;
      if (showDetails)
            detailsLayout->addWidget(mixerDetails);
      else
            detailsLayout->removeWidget(mixerDetails);
      }

//---------------------------------------------------------
//   synthGainChanged
//---------------------------------------------------------

void Mixer::synthGainChanged(float)
      {
      float decibels = qBound(minDecibels, log10f(synti->gain()), 0.0f);

      masterSlider->blockSignals(true);
      masterSlider->setValue(decibels);
      masterSlider->setToolTip(tr("Decibels: %1").arg(decibels));
      masterSlider->blockSignals(false);

      masterSpin->blockSignals(true);
      masterSpin->setValue(decibels);
      masterSpin->blockSignals(false);
      }

void Mixer::adjustScrollPosition(int, int)
      {
      if (_needToKeepScrollPosition)
            tracks_scrollArea->horizontalScrollBar()->setValue(_scrollPosition);
      }

void Mixer::checkKeptScrollValue(int scrollPos)
      {
      if (_needToKeepScrollPosition) {
            tracks_scrollArea->horizontalScrollBar()->setValue(_scrollPosition);
            if (_scrollPosition == scrollPos)
                  _needToKeepScrollPosition = false;
            }
      }

void Mixer::keepScrollPosition()
      {
      _scrollPosition = tracks_scrollArea->horizontalScrollBar()->sliderPosition();
      _needToKeepScrollPosition = true;
      }

//---------------------------------------------------------
//   masterVolumeChanged
//---------------------------------------------------------

void Mixer::masterVolumeChanged(double decibels)
      {
      float gain = qBound(0.0f, powf(10, (float)decibels), 1.0f);
      synti->setGain(gain);

      masterSlider->blockSignals(true);
      masterSlider->setValue(decibels);
      masterSlider->setToolTip(tr("Decibels: %1").arg(decibels));
      masterSlider->blockSignals(false);

      masterSpin->blockSignals(true);
      masterSpin->setValue(decibels);
      masterSpin->blockSignals(false);
      }

//---------------------------------------------------------
//   on_partOnlyCheckBox_toggled
//---------------------------------------------------------

void Mixer::on_partOnlyCheckBox_toggled(bool checked)
      {
      if (!_activeScore->excerpt())
            return;

      mscore->setPlayPartOnly(checked);
      setPlaybackScore(_activeScore->masterScore()->playbackScore());

      // Prevent muted channels from sounding
      for (const MidiMapping& mm : *_activeScore->masterScore()->midiMapping()) {
            const Channel* ch = mm.articulation;
            if (ch && (ch->mute() || ch->soloMute()))
                  seq->stopNotes(ch->channel());
            }
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void Mixer::retranslate(bool firstTime)
      {
      setWindowTitle(tr("Mixer"));
      if (!firstTime) {
            for (int i = 0; i < trackAreaLayout->count(); i++) {
                  PartEdit* p = getPartAtIndex(i);
                  if (p) p->retranslateUi(p);
                  }
            }
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Mixer::closeEvent(QCloseEvent* ev)
      {
      emit closed(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void Mixer::showEvent(QShowEvent* e)
      {
      enablePlay->showEvent(e);
      QWidget::showEvent(e);
      activateWindow();
      setFocus();
      }

//---------------------------------------------------------
//   eventFilter
//---------------------------------------------------------

bool Mixer::eventFilter(QObject* obj, QEvent* e)
      {
      if (enablePlay->eventFilter(obj, e))
            return true;
      return QWidget::eventFilter(obj, e);
      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void Mixer::keyPressEvent(QKeyEvent* ev) {
      if (ev->key() == Qt::Key_Escape && ev->modifiers() == Qt::NoModifier) {
            close();
            return;
            }
      QWidget::keyPressEvent(ev);
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void Mixer::changeEvent(QEvent *event)
      {
      QWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }

//---------------------------------------------------------
//   partEdit
//---------------------------------------------------------

PartEdit* Mixer::getPartAtIndex(int)
      {
      return 0;
      }

//---------------------------------------------------------
//   setPlaybackScore
//---------------------------------------------------------

void Mixer::setPlaybackScore(Score* score)
      {
      if (_score != score) {
            _score = score;
            mixerDetails->setTrack(0);
            }
      updateTracks();
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Mixer::setScore(Score* score)
      {
      // No equality check, this function seems to need to cause
      // mixer update every time it gets called.
      _activeScore = score;
      setPlaybackScore(_activeScore->masterScore()->playbackScore());

      partOnlyCheckBox->setChecked(mscore->playPartOnly());
      partOnlyCheckBox->setEnabled(_activeScore && !_activeScore->isMaster());
      }

//---------------------------------------------------------
//   updateTracks
//---------------------------------------------------------

void Mixer::updateTracks()
      {
      MixerTrackItem* oldSel = mixerDetails->track().get();

      Part* selPart = oldSel ? oldSel->part() : 0;
      Channel* selChan = oldSel ? oldSel->chan() : 0;

      if (_score && !selPart) {
            //If nothing selected, select first available track
            if (!_score->parts().isEmpty())
                  {
                  selPart = _score->parts()[0]->masterPart();
                  selChan = selPart->instrument(0)->playbackChannel(0, _score->masterScore());
                  }

            }


      if (trackHolder) {
            trackAreaLayout->removeWidget(trackHolder);
            trackHolder->deleteLater();
            trackHolder = 0;
            }

      trackList.clear();
      mixerDetails->setTrack(0);


      if (!_score)
            return;

      trackHolder = new QWidget();
      QHBoxLayout* holderLayout = new QHBoxLayout();
      holderLayout->setContentsMargins(0, 0, 0, 0);
      holderLayout->setSpacing(0);
      trackHolder->setLayout(holderLayout);

      trackAreaLayout->addWidget(trackHolder);

      for (Part* localPart : _score->parts()) {
            Part* part = localPart->masterPart();
            //Add per part tracks
            bool expanded = expandedParts.contains(part);
            const InstrumentList* il = part->instruments();
            Instrument* proxyInstr = nullptr;
            Channel* proxyChan = nullptr;
            if (!il->empty()) {
                  il->begin();
                  proxyInstr = il->begin()->second;
                  proxyChan = proxyInstr->playbackChannel(0, _score->masterScore());
                  }

            MixerTrackItemPtr mti = std::make_shared<MixerTrackItem>(
                              MixerTrackItem::TrackType::PART, part, proxyInstr, proxyChan);

            MixerTrackPart* track = new MixerTrackPart(this, mti, expanded);
            track->setGroup(this);
            trackList.append(track);
            holderLayout->addWidget(track);

            if (selPart == part &&
                (selChan == 0 || !expanded)) {
                  track->setSelected(true);
                  mixerDetails->setTrack(mti);
                  }

            if (expanded) {
                  //Add per channel tracks
                  const InstrumentList* il1 = part->instruments();
                  for (auto it = il1->begin(); it != il1->end(); ++it) {
                        Instrument* instr = it->second;
                        for (int i = 0; i < instr->channel().size(); ++i) {
                              Channel* chan = instr->playbackChannel(i, _score->masterScore());
                              MixerTrackItemPtr mti1 = std::make_shared<MixerTrackItem>(
                                                MixerTrackItem::TrackType::CHANNEL, part, instr, chan);
//                              MixerTrackItemPtr mti = new MixerTrackItem(
//                                                MixerTrackItem::TrackType::CHANNEL, part, instr, chan);
                              MixerTrackChannel* track1 = new MixerTrackChannel(this, mti1);
                              track1->setGroup(this);
                              trackList.append(track1);
                              holderLayout->addWidget(track1);

                              if (selPart == part &&
                                  selChan == chan) {
                                    track1->setSelected(true);
                                    mixerDetails->setTrack(mti1);
                                    }
                              }
                        }
                  }
            }

      holderLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
      keepScrollPosition();
      }

//---------------------------------------------------------
//   midiPrefsChanged
//---------------------------------------------------------

void Mixer::midiPrefsChanged(bool)
      {
      updateTracks();
      }

//---------------------------------------------------------
//   notifyTrackSelected
//---------------------------------------------------------

void Mixer::expandToggled(Part* part, bool expanded)
      {
      if (expanded)
            expandedParts.insert(part);
      else
            expandedParts.remove(part);

      updateTracks();
      }

//---------------------------------------------------------
//   notifyTrackSelected
//---------------------------------------------------------

void Mixer::notifyTrackSelected(MixerTrack* track)
      {
      for (MixerTrack *mt: trackList) {
            if (!(mt->mti()->part() == track->mti()->part() &&
                  mt->mti()->chan() == track->mti()->chan() &&
                  mt->mti()->trackType() == track->mti()->trackType())) {
                  mt->setSelected(false);
                  }
            }
      mixerDetails->setTrack(track->mti());
      }


//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void Mixer::writeSettings()
      {
      MuseScore::saveGeometry(this);
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void Mixer::readSettings()
      {
      resize(QSize(480, 600)); //ensure default size if no geometry in settings
      MuseScore::restoreGeometry(this);
      }


//---------------------------------------------------------
//   showMixer
//---------------------------------------------------------

void MuseScore::showMixer(bool val)
      {
      if (!cs)
            return;

      QAction* a = getAction("toggle-mixer");
      if (mixer == 0) {
            mixer = new Mixer(this);
            mscore->stackUnder(mixer);
            if (synthControl)
                  connect(synthControl, SIGNAL(soundFontChanged()), mixer, SLOT(updateTrack()));
            connect(synti, SIGNAL(soundFontChanged()), mixer, SLOT(updateTracks()));
            connect(mixer, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            }
      mixer->setScore(cs);
      mixer->setVisible(val);
      }

}
