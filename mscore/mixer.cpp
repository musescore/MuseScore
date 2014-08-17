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
      mute->setChecked(a->mute);
      solo->setChecked(a->solo);
      volume->setValue(a->volume);
      reverb->setValue(a->reverb);
      chorus->setValue(a->chorus);
      pan->setValue(a->pan);
      for (int i = 0; i < patch->count(); ++i) {
            MidiPatch* p = (MidiPatch*)patch->itemData(i, Qt::UserRole).value<void*>();
            if (a->synti == p->synti && a->program == p->prog && a->bank == p->bank) {
                  patch->setCurrentIndex(i);
                  break;
                  }
            }
      drumset->setVisible((p->instr()->useDrumset() != DrumsetKind::NONE));
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
      QString s;
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
      mixer->updateAll(cs);
      mixer->setVisible(val);
      }

//---------------------------------------------------------
//   patchChanged
//---------------------------------------------------------

void PartEdit::patchChanged(int n)
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
      }

//---------------------------------------------------------
//   volChanged
//---------------------------------------------------------

void PartEdit::volChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_VOLUME, iv);
      channel->volume = iv;
      }

//---------------------------------------------------------
//   panChanged
//---------------------------------------------------------

void PartEdit::panChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_PANPOT, iv);
      channel->pan = iv;
      }

//---------------------------------------------------------
//   reverbChanged
//---------------------------------------------------------

void PartEdit::reverbChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_REVERB_SEND, iv);
      channel->reverb = iv;
      }

//---------------------------------------------------------
//   chorusChanged
//---------------------------------------------------------

void PartEdit::chorusChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_CHORUS_SEND, iv);
      channel->chorus = iv;
      }

//---------------------------------------------------------
//   muteChanged
//---------------------------------------------------------

void PartEdit::muteChanged(bool val)
      {
      if (val)
            seq->stopNotes(channel->channel);
      channel->mute = val;
      }

//---------------------------------------------------------
//   soloToggled
//---------------------------------------------------------

void PartEdit::soloToggled(bool val)
      {
      channel->solo = val;
      channel->soloMute = !val;
      if (val) {
            mute->setChecked(false);
            foreach(Part* part, part->score()->parts()) {
                  for (int i = 0; i < part->instr()->channel().size(); ++i) {
                        Channel* a = &part->instr()->channel(i);
                        a->soloMute = (channel != a && !a->solo);
                        a->solo     = (channel == a || a->solo);
                        if (a->soloMute)
                              seq->stopNotes(a->channel);
                        }
                  }
            emit soloChanged(true);
            }
      else { //do nothing except if it's the last solo to be switched off
            bool found = false;
            foreach(Part* part, part->score()->parts()) {
                  for (int i = 0; i < part->instr()->channel().size(); ++i) {
                        Channel* a = &part->instr()->channel(i);
                        if(a->solo){
                            found = true;
                            break;
                            }
                        }
                  }
            if (!found){
                foreach(Part* part, part->score()->parts()) {
                  for (int i = 0; i < part->instr()->channel().size(); ++i) {
                        Channel* a = &part->instr()->channel(i);
                        a->soloMute = false;
                        a->solo     = false;
                        }
                  }
                  emit soloChanged(false);
                }
            }
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
}

