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
      connect(drumset,  SIGNAL(toggled(bool)),            SLOT(drumsetToggled(bool)));
      }

//---------------------------------------------------------
//   setPart
//---------------------------------------------------------

void PartEdit::setPart(Part* p, Channel* a)
      {
      Channel dummy;
      channel = a;
      part    = p;
      QString s = part->partName();
      if (!a->name.isEmpty()) {
            if (a->name != "normal") {
                  s += "-";
                  s += qApp->translate("InstrumentsXML", a->name.toUtf8().data());
                  }
            }
      partName->setText(s);
      mute->setChecked(a->mute);
      solo->setChecked(a->solo);
      volume->setValue(a->volume);
      volume->setDclickValue1(dummy.volume);
      volume->setDclickValue2(dummy.volume);
      reverb->setValue(a->reverb);
      reverb->setDclickValue1(dummy.reverb);
      reverb->setDclickValue2(dummy.reverb);
      chorus->setValue(a->chorus);
      chorus->setDclickValue1(dummy.chorus);
      chorus->setDclickValue2(dummy.chorus);
      pan->setValue(a->pan);
      pan->setDclickValue1(0);
      pan->setDclickValue2(0);
      for (int i = 0; i < patch->count(); ++i) {
            MidiPatch* p = (MidiPatch*)patch->itemData(i, Qt::UserRole).value<void*>();
            if (a->synti == p->synti && a->program == p->prog && a->bank == p->bank) {
                  patch->setCurrentIndex(i);
                  break;
                  }
            }
      drumset->blockSignals(true);
      drumset->setChecked(p->instrument()->useDrumset());
      drumset->blockSignals(false);
      }

//---------------------------------------------------------
//   Mixer
//---------------------------------------------------------

Mixer::Mixer(QWidget* parent)
   : QScrollArea(parent)
      {
      setObjectName("Mixer");
      setWindowTitle(tr("Mixer"));
      setWidgetResizable(true);
      setWindowFlags(Qt::Tool);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      QWidget* area = new QWidget(this);
      vb = new QVBoxLayout;
      vb->setMargin(0);
      vb->setSpacing(0);
      area->setLayout(vb);
      setWidget(area);

      enablePlay = new EnablePlayForWidget(this);
      readSettings();
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
      QScrollArea::showEvent(e);
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
      return QScrollArea::eventFilter(obj, e);
      }

void Mixer::keyPressEvent(QKeyEvent* ev) {
      if (ev->key() == Qt::Key_Escape && ev->modifiers() == Qt::NoModifier) {
            close();
            return;
            }
      QWidget::keyPressEvent(ev);
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

//---------------------------------------------------------
//   partEdit
//---------------------------------------------------------

PartEdit* Mixer::partEdit(int index)
      {
      if (index < vb->count()) {
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
            bool drum        = m.part->instrument()->useDrumset();
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
            mscore->stackUnder(mixer);
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
            score->undo(new ChangePatch(score, channel, p));
            score->setLayoutAll(true);
            score->endCmd();
            }
      channel->updateInitList();
      }

//---------------------------------------------------------
//   volChanged
//---------------------------------------------------------

void PartEdit::volChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_VOLUME, iv);
      channel->volume = iv;
      channel->updateInitList();
      }

//---------------------------------------------------------
//   panChanged
//---------------------------------------------------------

void PartEdit::panChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_PANPOT, iv);
      channel->pan = iv;
      channel->updateInitList();
      }

//---------------------------------------------------------
//   reverbChanged
//---------------------------------------------------------

void PartEdit::reverbChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_REVERB_SEND, iv);
      channel->reverb = iv;
      channel->updateInitList();
      }

//---------------------------------------------------------
//   chorusChanged
//---------------------------------------------------------

void PartEdit::chorusChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_CHORUS_SEND, iv);
      channel->chorus = iv;
      channel->updateInitList();
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
            for (Part* p : part->score()->parts()) {
                  const InstrumentList* il = p->instruments();
                  for (auto i = il->begin(); i != il->end(); ++i) {
                        const Instrument* instr = i->second;
                        for (Channel* a : instr->channel()) {
                              a->soloMute = (channel != a && !a->solo);
                              a->solo     = (channel == a || a->solo);
                              if (a->soloMute)
                                    seq->stopNotes(a->channel);
                              }
                        }
                  }
            emit soloChanged(true);
            }
      else { //do nothing except if it's the last solo to be switched off
            bool found = false;
            for (Part* p : part->score()->parts()) {
                  const InstrumentList* il = p->instruments();
                  for (auto i = il->begin(); i != il->end(); ++i) {
                        const Instrument* instr = i->second;
                        for (Channel* a : instr->channel()) {
                              if (a->solo){
                                    found = true;
                                    break;
                                    }
                              }
                        }
                  }
            if (!found){
                  foreach(Part* p, part->score()->parts()) {
                        const InstrumentList* il = p->instruments();
                        for (auto i = il->begin(); i != il->end(); ++i) {
                              const Instrument* instr = i->second;
                              for (Channel* a : instr->channel()) {
                                    a->soloMute = false;
                                    a->solo     = false;
                                    }
                              }
                        }
                  emit soloChanged(false);
                  }
            }
      }

//---------------------------------------------------------
//   drumsetToggled
//---------------------------------------------------------

void PartEdit::drumsetToggled(bool val)
      {
      if (part == 0)
            return;

      Score* score = part->score();
      score->startCmd();

      part->undoChangeProperty(P_ID::USE_DRUMSET, val);
      patch->clear();
      const QList<MidiPatch*> pl = synti->getPatchInfo();
      for (const MidiPatch* p : pl) {
            if (p->drum == val)
                  patch->addItem(p->name, QVariant::fromValue<void*>((void*)p));
            }

      // switch to first instrument
      const MidiPatch* p = (MidiPatch*)patch->itemData(0, Qt::UserRole).value<void*>();
      if (p == 0) {
            qDebug("PartEdit::patchChanged: no patch");
            return;
            }
      score->undo(new ChangePatch(score, channel, p));
      score->setLayoutAll(true);
      score->endCmd();
      }

//---------------------------------------------------------
//   updateSolo
//---------------------------------------------------------

void Mixer::updateSolo(bool val)
      {
      for (int i = 0; i < vb->count(); i++ ){
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
}

