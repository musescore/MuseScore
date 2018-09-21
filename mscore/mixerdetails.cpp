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

#include "mixerdetails.h"

#include "musescore.h"

#include "libmscore/score.h"
#include "libmscore/part.h"
#include "mixer.h"
#include "mixertrack.h"
#include "mixertrackitem.h"
#include "seq.h"
#include "libmscore/undo.h"
#include "synthcontrol.h"
#include "synthesizer/msynthesizer.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   MixerDetails
//---------------------------------------------------------

MixerDetails::MixerDetails(QWidget *parent) :
      QWidget(parent), _mti(0)
      {
      setupUi(this);

      connect(partNameLineEdit,    SIGNAL(editingFinished()),              SLOT(partNameChanged()));
      connect(trackColorLabel,     SIGNAL(colorChanged(QColor)),           SLOT(trackColorChanged(QColor)));
      connect(patchCombo,          SIGNAL(activated(int)),                 SLOT(patchChanged(int)));
      connect(volumeSlider,          &QSlider::valueChanged,     this,     &MixerDetails::volumeChanged);
      connect(volumeSpinBox,       SIGNAL(valueChanged(double)),           SLOT(volumeChanged(double)));
      connect(panSlider,           &QSlider::valueChanged,       this,     &MixerDetails::panChanged);
      connect(panSpinBox,          SIGNAL(valueChanged(double)),           SLOT(panChanged(double)));
      connect(chorusSlider,        &QSlider::valueChanged,       this,     &MixerDetails::chorusChanged);
      connect(chorusSpinBox,       SIGNAL(valueChanged(double)),           SLOT(chorusChanged(double)));
      connect(reverbSlider,        &QSlider::valueChanged,       this,     &MixerDetails::reverbChanged);
      connect(reverbSpinBox,       SIGNAL(valueChanged(double)),           SLOT(reverbChanged(double)));
      connect(portSpinBox,         SIGNAL(valueChanged(int)),              SLOT(midiChannelChanged(int)));
      connect(channelSpinBox,      SIGNAL(valueChanged(int)),              SLOT(midiChannelChanged(int)));
      connect(drumkitCheck,        SIGNAL(toggled(bool)),                  SLOT(drumkitToggled(bool)));

      updateFromTrack();
      }

//---------------------------------------------------------
//   ~MixerDetails
//---------------------------------------------------------

MixerDetails::~MixerDetails()
      {
      if (_mti) {
            //Remove old attachment
            _mti->midiMap()->articulation->removeListener(this);
            }
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void MixerDetails::setTrack(MixerTrackItemPtr track)
      {
      if (_mti) {
            //Remove old attachment
            Channel* chan = _mti->focusedChan();
            chan->removeListener(this);
            }

      _mti = track;

      if (_mti) {
            //Listen to new track
            Channel* chan = _mti->focusedChan();
            chan->addListener(this);
            }
      updateFromTrack();
      }


//---------------------------------------------------------
//   updateFromTrack
//---------------------------------------------------------

void MixerDetails::updateFromTrack()
      {

      if (!_mti) {
            drumkitCheck->setChecked(false);
            patchCombo->clear();
            partNameLineEdit->setText("");
            channelLabel->setText("");
            volumeSlider->setValue(0);
            volumeSpinBox->setValue(0);
            panSlider->setValue(0);
            panSpinBox->setValue(0);
            reverbSlider->setValue(0);
            reverbSpinBox->setValue(0);
            chorusSlider->setValue(0);
            chorusSpinBox->setValue(0);
            portSpinBox->setValue(0);
            channelSpinBox->setValue(0);
            trackColorLabel->setColor(QColor());

            drumkitCheck->setEnabled(false);
            patchCombo->setEnabled(false);
            partNameLineEdit->setEnabled(false);
            volumeSlider->setEnabled(false);
            volumeSpinBox->setEnabled(false);
            panSlider->setEnabled(false);
            panSpinBox->setEnabled(false);
            reverbSlider->setEnabled(false);
            reverbSpinBox->setEnabled(false);
            chorusSlider->setEnabled(false);
            chorusSpinBox->setEnabled(false);
            portSpinBox->setEnabled(false);
            channelSpinBox->setEnabled(false);
            trackColorLabel->setEnabled(false);
            return;
            }

      drumkitCheck->setEnabled(true);
      patchCombo->setEnabled(true);
      partNameLineEdit->setEnabled(true);
      volumeSlider->setEnabled(true);
      volumeSpinBox->setEnabled(true);
      panSlider->setEnabled(true);
      panSpinBox->setEnabled(true);
      reverbSlider->setEnabled(true);
      reverbSpinBox->setEnabled(true);
      chorusSlider->setEnabled(true);
      chorusSpinBox->setEnabled(true);
      portSpinBox->setEnabled(true);
      channelSpinBox->setEnabled(true);
      trackColorLabel->setEnabled(true);


      MidiMapping* midiMap = _mti->midiMap();
      Part* part = _mti->part();
      Channel* chan = _mti->focusedChan();

      //Check if drumkit
      bool drum = midiMap->part->instrument()->useDrumset();
      drumkitCheck->blockSignals(true);
      drumkitCheck->setChecked(drum);
      drumkitCheck->blockSignals(false);


      //Populate patch combo
      patchCombo->blockSignals(true);
      patchCombo->clear();
      const QList<MidiPatch*> pl = synti->getPatchInfo();
      int patchIndex = 0;

      for (const MidiPatch* p : pl) {
            if (p->drum == drum || p->synti != "Fluid") {
                      patchCombo->addItem(p->name, QVariant::fromValue<void*>((void*)p));
                      if (p->bank == chan->bank() && p->prog == chan->program())
                              patchIndex = patchCombo->count() - 1;
                  }
            }
      patchCombo->setCurrentIndex(patchIndex);

      patchCombo->blockSignals(false);

      QString partName = part->partName();
      if (!chan->name().isEmpty())
            channelLabel->setText(qApp->translate("InstrumentsXML", chan->name().toUtf8().data()));
      else
            channelLabel->setText("");
      partNameLineEdit->setText(partName);
      partNameLineEdit->setToolTip(partName);



      trackColorLabel->blockSignals(true);
      volumeSlider->blockSignals(true);
      volumeSpinBox->blockSignals(true);
      panSlider->blockSignals(true);
      panSpinBox->blockSignals(true);
      reverbSlider->blockSignals(true);
      reverbSpinBox->blockSignals(true);
      chorusSlider->blockSignals(true);
      chorusSpinBox->blockSignals(true);

      //      trackColorLabel->setColor(QColor(chan->color() | 0xff000000));
      trackColorLabel->setColor(QColor(_mti->color() | 0xff000000));

      volumeSlider->setValue(chan->volume());
      volumeSpinBox->setValue(chan->volume());
      panSlider->setValue(chan->pan());
      panSpinBox->setValue(chan->pan());
      reverbSlider->setValue(chan->reverb());
      reverbSpinBox->setValue(chan->reverb());
      chorusSlider->setValue(chan->chorus());
      chorusSpinBox->setValue(chan->chorus());

      portSpinBox->setValue(part->masterScore()->midiMapping(chan->channel())->port + 1);
      channelSpinBox->setValue(part->masterScore()->midiMapping(chan->channel())->channel + 1);

      trackColorLabel->blockSignals(false);
      volumeSlider->blockSignals(false);
      volumeSpinBox->blockSignals(false);
      panSlider->blockSignals(false);
      panSpinBox->blockSignals(false);
      reverbSlider->blockSignals(false);
      reverbSpinBox->blockSignals(false);
      chorusSlider->blockSignals(false);
      chorusSpinBox->blockSignals(false);

      }

//---------------------------------------------------------
//   partNameChanged
//---------------------------------------------------------

void MixerDetails::partNameChanged()
      {
      if (!_mti)
            return;

      QString text = partNameLineEdit->text();
      Part* part = _mti->midiMap()->part;
      if (part->partName() == text) {
            return;
            }

      Score* score = part->score();
      if (score) {
            score->startCmd();
            score->undo(new ChangePart(part, _mti->instrument(), text));
            score->endCmd();
            }
      }

//---------------------------------------------------------
//   trackColorChanged
//---------------------------------------------------------

void MixerDetails::trackColorChanged(QColor col)
      {
      Part* part = _mti->midiMap()->part;
      Channel* channel = _mti->midiMap()->articulation;

      if (trackColorLabel->color() != col) {
            trackColorLabel->blockSignals(true);
            trackColorLabel->setColor(col);
            trackColorLabel->blockSignals(false);
            }

      _mti->setColor(col.rgb());
      }

//---------------------------------------------------------
//   propertyChanged
//---------------------------------------------------------

void MixerDetails::propertyChanged(Channel::Prop property)
      {
      if (!_mti)
            return;

      MidiMapping* _midiMap = _mti->midiMap();
      Channel* chan = _midiMap->articulation;

      switch (property) {
            case Channel::Prop::VOLUME: {
                  volumeChanged(chan->volume());
                  break;
                  }
            case Channel::Prop::PAN: {
                  panChanged(chan->pan());
                  break;
                  }
            case Channel::Prop::COLOR: {
                  trackColorChanged(chan->color());
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   volumeChanged
//---------------------------------------------------------

void MixerDetails::volumeChanged(double value)
      {
      if (!_mti)
            return;

      volumeSlider->blockSignals(true);
      volumeSpinBox->blockSignals(true);

      volumeSlider->setValue((int)value);
      volumeSpinBox->setValue(value);

      volumeSlider->blockSignals(false);
      volumeSpinBox->blockSignals(false);

      _mti->setVolume(value);
      }


//---------------------------------------------------------
//   panChanged
//---------------------------------------------------------

void MixerDetails::panChanged(double value)
      {
      if (!_mti)
            return;

      int v = (int)value;

      panSlider->blockSignals(true);
      panSpinBox->blockSignals(true);

      panSlider->setValue((int)v);
      panSpinBox->setValue(v);

      panSlider->blockSignals(false);
      panSpinBox->blockSignals(false);

      _mti->setPan(v);
      }


//---------------------------------------------------------
//   reverbChanged
//---------------------------------------------------------

void MixerDetails::reverbChanged(double v)
      {
      if (!_mti)
            return;

      reverbSlider->blockSignals(true);
      reverbSpinBox->blockSignals(true);

      reverbSlider->setValue((int)v);
      reverbSpinBox->setValue(v);

      reverbSlider->blockSignals(false);
      reverbSpinBox->blockSignals(false);

      _mti->setReverb(v);
      }

//---------------------------------------------------------
//   chorusChanged
//---------------------------------------------------------

void MixerDetails::chorusChanged(double v)
      {
      if (!_mti)
            return;

      chorusSlider->blockSignals(true);
      chorusSpinBox->blockSignals(true);

      chorusSlider->setValue((int)v);
      chorusSpinBox->setValue(v);

      chorusSlider->blockSignals(false);
      chorusSpinBox->blockSignals(false);

      _mti->setChorus(v);
      }

//---------------------------------------------------------
//   patchChanged
//---------------------------------------------------------

void MixerDetails::patchChanged(int n)
      {
      if (!_mti)
            return;

      const MidiPatch* p = (MidiPatch*)patchCombo->itemData(n, Qt::UserRole).value<void*>();
      if (p == 0) {
            qDebug("PartEdit::patchChanged: no patch");
            return;
            }

      Part* part = _mti->midiMap()->part;
      Channel* channel = _mti->midiMap()->articulation;
      Score* score = part->score();
      if (score) {
            score->startCmd();
            score->undo(new ChangePatch(score, channel, p));
            score->setLayoutAll();
            score->endCmd();
            }
      channel->updateInitList();
      }

//---------------------------------------------------------
//   drumkitToggled
//---------------------------------------------------------

void MixerDetails::drumkitToggled(bool val)
      {
      if (_mti == 0)
            return;

      Instrument *instr = _mti->instrument();
      if (instr->useDrumset() == val)
            return;

      const MidiPatch* newPatch = 0;
      const QList<MidiPatch*> pl = synti->getPatchInfo();
      for (const MidiPatch* p : pl) {
            if (p->drum == val) {
                  newPatch = p;
                  break;
                  }
            }

      Part* part = _mti->part();
      Score* score = part->score();
      Channel* channel = _mti->chan();
      if (newPatch) {
            score->startCmd();
            part->undoChangeProperty(Pid::USE_DRUMSET, val);
            score->undo(new ChangePatch(score, channel, newPatch));
            score->setLayoutAll();
            score->endCmd();
            }
      channel->updateInitList();
      }

//---------------------------------------------------------
//   midiChannelChanged
//   handles MIDI port & channel change
//---------------------------------------------------------

void MixerDetails::midiChannelChanged(int)
      {
      if (_mti == 0)
            return;

      Part* part = _mti->part();
      Channel* channel = _mti->focusedChan();

      seq->stopNotes(channel->channel());
      int p =    portSpinBox->value() - 1;
      int c = channelSpinBox->value() - 1;

      MidiMapping* midiMap = _mti->midiMap();
      midiMap->port = p;
      midiMap->channel = c;

//      channel->updateInitList();
      part->score()->setInstrumentsChanged(true);
      part->score()->setLayoutAll();
      seq->initInstruments();

      // Update MIDI Out ports
      int maxPort = max(p, part->score()->masterScore()->midiPortCount());
      part->score()->masterScore()->setMidiPortCount(maxPort);
      if (seq->driver() && (preferences.getBool(PREF_IO_JACK_USEJACKMIDI) || preferences.getBool(PREF_IO_ALSA_USEALSAAUDIO)))
            seq->driver()->updateOutPortCount(maxPort + 1);
      }


}
