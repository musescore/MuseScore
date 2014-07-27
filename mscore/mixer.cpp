//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: mixer.cpp 5651 2012-05-19 15:57:26Z lasconic $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "mixer.h"
#include "seq.h"
#include "libmscore/undo.h"
#include "synthcontrol.h"
#include "synthesizer/msynthesizer.h"
#include "mscore/preferences.h"

#define _setValue(__x, __y) \
      __x->blockSignals(true); \
      __x->setValue(__y); \
      __x->blockSignals(false);

#define _setChecked(__x, __y) \
      __x->blockSignals(true); \
      __x->setChecked(__y); \
      __x->blockSignals(false);

namespace Ms {

extern bool useFactorySettings;

//---------------------------------------------------------
//   PartEdit
//---------------------------------------------------------

PartEdit::PartEdit(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setupUi(this);
      connect(patch,    SIGNAL(activated(int)),           SLOT(patchChanged(int)));
      connect(volume,   SIGNAL(valueChanged(double,int)), SLOT(volChanged(double)));
      connect(pan,      SIGNAL(valueChanged(double,int)), SLOT(panChanged(double)));
      connect(chorus,   SIGNAL(valueChanged(double,int)), SLOT(chorusChanged(double)));
      connect(reverb,   SIGNAL(valueChanged(double,int)), SLOT(reverbChanged(double)));
      connect(mute,     SIGNAL(toggled(bool)),            SLOT(muteChanged(bool)));
      connect(solo,     SIGNAL(toggled(bool)),            SLOT(soloToggled(bool)));
      connect(portN,    SIGNAL(valueChanged(int)),        SLOT(channelChanged(int)));
      connect(channelN, SIGNAL(valueChanged(int)),        SLOT(channelChanged(int)));
      portN->setMaximum(Ms::MAX_MIDI_PORT);
      }

//---------------------------------------------------------
//   setPart
//---------------------------------------------------------

void PartEdit::setPart(Part* p, Channel* a)
      {
      channel = a;
      part    = p;
      QString s = part->partName();
      if (!a->name.isEmpty() && a->name != "normal")
            s += "-" + a->name;
      partName->setText(s);
      _setValue(volume,a->volume);
      _setValue(reverb,a->reverb);
      _setValue(pan,a->pan);
      for (int i = 0; i < patch->count(); ++i) {
            MidiPatch* p = (MidiPatch*)patch->itemData(i, Qt::UserRole).value<void*>();
            if (a->synti == p->synti && a->program == p->prog && a->bank == p->bank) {
                  patch->setCurrentIndex(i);
                  break;
                  }
            }
      drumset->setVisible((p->instr()->useDrumset() != DrumsetKind::NONE));
      _setValue(portN,    part->score()->midiMapping(a->channel)->port + 1);
      _setValue(channelN, part->score()->midiMapping(a->channel)->channel + 1);
      }

//---------------------------------------------------------
//   Mixer
//---------------------------------------------------------

Mixer::Mixer(QWidget* parent)
   : QScrollArea(parent)
      {
      setWindowTitle(tr("MuseScore: Mixer"));
      setWidgetResizable(true);
      setWindowFlags(Qt::Dialog);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      QWidget* area = new QWidget(this);
      vb = new QVBoxLayout;
      vb->setMargin(0);
      vb->setSpacing(0);
      area->setLayout(vb);
      setWidget(area);

      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup("Mixer");
            resize(settings.value("size", QSize(484, 184)).toSize());
            move(settings.value("pos", QPoint(10, 10)).toPoint());
            settings.endGroup();
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
//   updateAll
//---------------------------------------------------------

void Mixer::updateAll(Score* score)
      {
      cs = score;
      int n = -vb->count();
      if (cs) {
            QList<MidiMapping>* mm = cs->midiMapping();
            n = mm->size() - vb->count();
            }
      while (n < 0) {
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(0));
            vb->removeItem(wi);
            delete wi->widget();
            delete wi;
            ++n;
            }
      while (n > 0) {
            PartEdit* pe = new PartEdit;
            connect(pe, SIGNAL(soloChanged(bool)), SLOT(updateSolo(bool)));
            vb->addWidget(pe);
            --n;
            }
      patchListChanged();
      }

PartEdit* Mixer::partEdit(int index)
      {
      if(index < vb->count()) {
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(index));
            return (PartEdit*) wi->widget();
            }
      return 0;
      }

//---------------------------------------------------------
//   patchListChanged
//---------------------------------------------------------

void Mixer::patchListChanged()
      {
      if (!cs)
            return;
      int idx = 0;
      QList<MidiMapping>* mm = cs->midiMapping();
      const QList<MidiPatch*> pl = synti->getPatchInfo();
      foreach (const MidiMapping& m, *mm) {
            QWidgetItem* wi  = (QWidgetItem*)(vb->itemAt(idx));
            PartEdit* pe     = (PartEdit*)(wi->widget());
            bool drum = (m.part->instr()->useDrumset() != DrumsetKind::NONE);
            pe->patch->clear();
            foreach(const MidiPatch* p, pl) {
                  if (p->drum == drum)
                        pe->patch->addItem(p->name, QVariant::fromValue<void*>((void*)p));
                  }
            pe->setPart(m.part, m.articulation);
            idx++;
            }
      // Update solo & mute only after creating all controls (we need to sync all controls)
      idx = 0;
      foreach (const MidiMapping& m, *mm) {
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(idx));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            pe->mute->setChecked(m.articulation->mute);
            pe->solo->setChecked(m.articulation->solo);
            idx++;
            }
      }

//---------------------------------------------------------
//   showMixer
//---------------------------------------------------------

void MuseScore::showMixer(bool val)
      {
      QAction* a = getAction("toggle-mixer");
      if (mixer == 0) {
            mixer = new Mixer(this);
            if (synthControl)
                  connect(synthControl, SIGNAL(soundFontChanged()), mixer, SLOT(patchListChanged()));
            connect(synti, SIGNAL(soundFontChanged()), mixer, SLOT(patchListChanged()));
            connect(mixer, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            }
      connect(cs, SIGNAL(updateMixer()), mixer, SLOT(updateAll()));
      mixer->updateAll(cs);
      mixer->setVisible(val);
      }

//---------------------------------------------------------
//   patchChanged
//---------------------------------------------------------

void PartEdit::patchChanged(int n, bool syncControls)
      {
      if (part == 0)
            return;
      const MidiPatch* p = (MidiPatch*)patch->itemData(n, Qt::UserRole).value<void*>();
      if (p == 0) {
            qDebug("PartEdit::patchChanged: no patch");
            return;
            }
      Score* score = part->score();
      if (score) {
            score->startCmd();
            score->undo(new ChangePatch(channel, p));
            score->endCmd();
            mscore->endCmd();
            }
      sync(syncControls);
      }

//---------------------------------------------------------
//   volChanged
//---------------------------------------------------------

void PartEdit::volChanged(double val, bool syncControls)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_VOLUME, iv);
      channel->volume = iv;
      sync(syncControls);
      }

//---------------------------------------------------------
//   panChanged
//---------------------------------------------------------

void PartEdit::panChanged(double val, bool syncControls)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_PANPOT, iv);
      channel->pan = iv;
      sync(syncControls);
      }

//---------------------------------------------------------
//   reverbChanged
//---------------------------------------------------------

void PartEdit::reverbChanged(double val, bool syncControls)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_REVERB_SEND, iv);
      channel->reverb = iv;
      sync(syncControls);
      }

//---------------------------------------------------------
//   chorusChanged
//---------------------------------------------------------

void PartEdit::chorusChanged(double val, bool syncControls)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_CHORUS_SEND, iv);
      channel->chorus = iv;
      sync(syncControls);
      }

//---------------------------------------------------------
//   muteChanged
//---------------------------------------------------------

void PartEdit::muteChanged(bool val, bool syncControls)
      {
      if (val)
            seq->stopNotes(channel->channel);
      channel->mute = val;
      sync(syncControls);
      }

//---------------------------------------------------------
//   soloToggled
//---------------------------------------------------------

void PartEdit::soloToggled(bool val, bool syncControls)
      {
      channel->solo = val;
      channel->soloMute = !val;
      if (val) {
            channel->mute = false;
            _setChecked(mute, false);
            foreach(Part* part, part->score()->parts()) {
                  InstrumentList* il = part->instrList();
                  for (auto i = il->begin(); i != il->end(); ++i) {
                        for (int k = 0; k < i->second.channel().size(); ++k) {
                              Channel* a = &(i->second.channel(k));
                              bool sameChannel = (part->score()->midiChannel(a->channel) == part->score()->midiChannel(channel->channel)
                                                  && part->score()->midiPort(a->channel) == part->score()->midiPort(channel->channel)) || (channel == a);
                              a->soloMute = (!sameChannel && !a->solo);
                              a->solo     = (sameChannel || a->solo);
                              if (a->soloMute)
                                    seq->stopNotes(a->channel);
                              }
                        }
                  }
            emit soloChanged(true);
            }
      else { //do nothing except if it's the last solo to be switched off
            seq->stopNotes(channel->channel);
            bool found = false;
            foreach(Part* part, part->score()->parts()) {
                  InstrumentList* il = part->instrList();
                  for (auto i = il->begin(); i != il->end(); ++i) {
                        for (int k = 0; k < i->second.channel().size(); ++k) {
                              Channel* a = &(i->second.channel(k));
                              if(!(part->score()->midiChannel(a->channel) == part->score()->midiChannel(channel->channel)
                                   && part->score()->midiPort(a->channel) == part->score()->midiPort(channel->channel))
                                   && a->solo){
                                    found = true;
                                    break;
                                    }
                              }
                        }
                  }
            if (!found) {
                  foreach(Part* part, part->score()->parts()) {
                        InstrumentList* il = part->instrList();
                        for (auto i = il->begin(); i != il->end(); ++i) {
                              for (int k = 0; k < i->second.channel().size(); ++k) {
                                    Channel* a = &(i->second.channel(k));
                                    a->soloMute = false;
                                    a->solo     = false;
                                    }
                              }
                        }
                  emit soloChanged(false);
                }
            }
      sync(syncControls);
      }

//---------------------------------------------------------
//   updateSolo
//---------------------------------------------------------

void Mixer::updateSolo(bool val)
      {
      for(int i = 0; i <vb->count(); i++ ){
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(i));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            pe->mute->setEnabled(!val);
            }
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void Mixer::writeSettings()
      {
      QSettings settings;
      settings.beginGroup("Mixer");
      settings.setValue("size", size());
      settings.setValue("pos", pos());
      settings.endGroup();
      }

//---------------------------------------------------------
//   channelChanged
//   handles port & channel change
//---------------------------------------------------------

void PartEdit::channelChanged(int)
      {
      seq->stopNotes(channel->channel);
      int p =    portN->value() - 1;
      int c = channelN->value() - 1;
      if (c == 16) {
            c  = 0;
            p += 1;
            }

      int newChannel = p*16+c;
      if (part->instr()->useDrumset())
            newChannel = p*16+9; // Port 9 is special for drums

      // Set new channel
      part->score()->midiMapping(channel->channel)->channel = newChannel % 16;
      part->score()->midiMapping(channel->channel)->port    = newChannel / 16;

      // Update the controls. Block signals to prevent looping
      _setValue(channelN, newChannel % 16 + 1);
      _setValue(portN,    newChannel / 16 + 1);

      // Sync to control with the same port and channel
      int count = this->parentWidget()->layout()->count();
      for(int i = 0; i<count; i++) {
            QWidgetItem* wi = (QWidgetItem*)(this->parentWidget()->layout()->itemAt(i));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            if (pe != 0 && pe != this
                && this->channelN->value() == pe->channelN->value()
                && this->portN->value() == pe->portN->value()) {
                  blockSignals(true);
                  volume->setValue(pe->volume->value());
                  channel->volume = lrint(pe->volume->value());
                  pan->setValue(pe->pan->value());
                  channel->pan = lrint(pe->pan->value());
                  reverb->setValue(pe->reverb->value());
                  channel->reverb = lrint(pe->reverb->value());
                  chorus->setValue(pe->chorus->value());
                  channel->chorus = lrint(pe->chorus->value());
                  mute->setChecked(pe->mute->isChecked());
                  channel->mute = pe->mute->isChecked();
                  channel->solo = pe->solo->isChecked();
                  patch->setCurrentIndex(pe->patch->currentIndex());
                  MidiPatch* p = (MidiPatch*)patch->itemData(patch->currentIndex(), Qt::UserRole).value<void*>();
                  channel->program = p->prog;
                  channel->bank = p->bank;
                  channel->synti = p->synti;
                  blockSignals(false);
                  solo->setChecked(pe->solo->isChecked());
                  channel->updateInitList();
                  break;
                  }
            }

      // Initializing an instrument with new channel
      foreach(const MidiCoreEvent& e, channel->init) {
            if (e.type() == ME_INVALID)
                  continue;
            NPlayEvent event(e.type(), channel->channel, e.dataA(), e.dataB());
            seq->sendEvent(event);
            }

      // Add new JACK/ALSA MIDI Out ports if not enough
      if (preferences.useJackMidi || preferences.useAlsaAudio)
            part->score()->updateMaxPort();
      }

//---------------------------------------------------------
//   sync
//   sync volume, pan, etc.
//---------------------------------------------------------

void PartEdit::sync(bool syncControls)
      {
      // Sync instruments with same midi port and channel
      if (!syncControls)
            return;
      int count = this->parentWidget()->layout()->count();
      for(int i = 0; i<count; i++) {
            QWidgetItem* wi = (QWidgetItem*)(this->parentWidget()->layout()->itemAt(i));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            if (pe != 0 && pe != this
                && this->channelN->value() == pe->channelN->value()
                && this->portN->value() == pe->portN->value()) {
                  pe->syncChannel(this->channel);
                  if (volume->value() != pe->volume->value()) {
                        _setValue(pe->volume, this->volume->value());
                        emit pe->volChanged(this->volume->value(), false);
                        }
                  if (pan->value() != pe->pan->value()) {
                        _setValue(pe->pan, this->pan->value());
                        emit pe->panChanged(this->pan->value(), false);
                        }
                  if (reverb->value() != pe->reverb->value()) {
                        _setValue(pe->reverb, this->reverb->value());
                        emit pe->reverbChanged(this->reverb->value(), false);
                        }
                  if (chorus->value() != pe->chorus->value()) {
                        _setValue(pe->chorus, this->chorus->value());
                        emit pe->chorusChanged(this->chorus->value(), false);
                        }
                  if (mute->isChecked() != pe->mute->isChecked()) {
                        _setChecked(pe->mute, channel->mute);
                        emit pe->muteChanged(channel->mute, false);
                        }
                  if (solo->isChecked() != pe->solo->isChecked()) {
                        _setChecked(pe->solo, channel->solo);
                        emit pe->soloToggled(channel->solo, false);
                        }
                  if (patch->currentIndex() != pe->patch->currentIndex()) {
                        pe->patch->blockSignals(true);
                        pe->patch->setCurrentIndex(this->patch->currentIndex());
                        pe->patch->blockSignals(false);
                        emit pe->patchChanged(this->patch->currentIndex(), false);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   syncChannel
//---------------------------------------------------------

void PartEdit::syncChannel(Channel* from)
      {
      QString name = channel->name;
      QString descr = channel->descr;
      int ch = channel->channel;
      *channel = *from;
      channel->name = name;
      channel->descr = descr;
      channel->channel = ch;
      }
}

