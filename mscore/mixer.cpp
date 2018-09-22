//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: mixer.cpp 5651 2012-05-19 15:57:26Z lasconic $
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

#include "libmscore/score.h"
#include "libmscore/part.h"
#include "mixer.h"
#include "seq.h"
#include "libmscore/undo.h"
#include "synthcontrol.h"
#include "synthesizer/msynthesizer.h"
#include "preferences.h"
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



//---------------------------------------------------------
//   Mixer
//---------------------------------------------------------

Mixer::Mixer(QWidget* parent)
   : QWidget(parent, Qt::Dialog),
      showExpanded(false),
      trackHolder(0)
      {
      setupUi(this);

      setObjectName("Mixer");
      setWindowFlags(Qt::Tool);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      trackAreaLayout = new QHBoxLayout;
      trackAreaLayout->setMargin(0);
      trackAreaLayout->setSpacing(0);
      trackArea->setLayout(trackAreaLayout);

      mixerDetails = new MixerDetails(this);
      details_scrollArea->setWidget(mixerDetails);

      splitter->setSizes(QList<int>({100, 300}));

      enablePlay = new EnablePlayForWidget(this);
      readSettings();
      retranslate(true);
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

PartEdit* Mixer::getPartAtIndex(int index)
      {
      return 0;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Mixer::setScore(MasterScore* score)
      {
      _score = score;
      updateTracks();
      }

//---------------------------------------------------------
//   updateTracks
//---------------------------------------------------------

void Mixer::updateTracks()
      {
      MixerTrackItem* oldSel = mixerDetails->track().get();

      Part* selPart = oldSel ? oldSel->part() : 0;
      Channel* selChan = oldSel ? oldSel->chan() : 0;


      if (trackHolder) {
//            tracks_scrollArea->takeWidget();
            trackAreaLayout->removeWidget(trackHolder);
//            delete trackHolder;
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
//      trackHolder->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
      trackHolder->setLayout(holderLayout);

      trackAreaLayout->addWidget(trackHolder);

//      tracks_scrollArea->setWidget(trackHolder);

      for (Part* part : _score->parts()) {
            //Add per part tracks
            bool expanded = expandedParts.contains(part);
            MixerTrackItemPtr mti = std::make_shared<MixerTrackItem>(
                              MixerTrackItem::TrackType::PART, part, nullptr, nullptr);
//            MixerTrackItemPtr mti =
//                              new MixerTrackItem(MixerTrackItem::TrackType::PART, part, nullptr, nullptr);
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
                  const InstrumentList* il = part->instruments();
                  for (auto it = il->begin(); it != il->end(); ++it) {
                        Instrument* instr = it->second;
                        for (int i = 0; i < instr->channel().size(); ++i) {
                              Channel *chan = instr->channel()[i];
                              MixerTrackItemPtr mti = std::make_shared<MixerTrackItem>(
                                                MixerTrackItem::TrackType::CHANNEL, part, instr, chan);
//                              MixerTrackItemPtr mti = new MixerTrackItem(
//                                                MixerTrackItem::TrackType::CHANNEL, part, instr, chan);
                              MixerTrackChannel* track = new MixerTrackChannel(this, mti);
                              track->setGroup(this);
                              trackList.append(track);
//                              trackAreaLayout->addWidget(track);
                              holderLayout->addWidget(track);

                              if (selPart == part &&
                                  selChan == chan) {
                                    track->setSelected(true);
                                    mixerDetails->setTrack(mti);
                                    }
                              }
                        }
                  }
            }

      holderLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));


      update();
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
                mt->mti()->chan() == track->mti()->chan())) {
                  mt->setSelected(false);
                  }
            }
      mixerDetails->setTrack(track->mti());
//      selPart = track->mti()->part();
//      selChan = track->mti()->chan();
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
                  connect(synthControl, SIGNAL(soundFontChanged()), mixer, SLOT(patchListChanged()));
            connect(synti, SIGNAL(soundFontChanged()), mixer, SLOT(patchListChanged()));
            connect(mixer, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            }
      mixer->setScore(cs->masterScore());
      mixer->setVisible(val);
      }

}
