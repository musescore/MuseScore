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
//   PartEdit
//---------------------------------------------------------

PartEdit::PartEdit(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setupUi(this);
      connect(patch,          SIGNAL(activated(int)),           SLOT(patchChanged(int)));
      connect(volume,         SIGNAL(valueChanged(double,int)), SLOT(volChanged(double)));
      connect(pan,            SIGNAL(valueChanged(double,int)), SLOT(panChanged(double)));
      connect(chorus,         SIGNAL(valueChanged(double,int)), SLOT(chorusChanged(double)));
      connect(reverb,         SIGNAL(valueChanged(double,int)), SLOT(reverbChanged(double)));
      connect(mute,           SIGNAL(toggled(bool)),            SLOT(muteChanged(bool)));
      connect(solo,           SIGNAL(toggled(bool)),            SLOT(soloToggled(bool)));
      connect(drumset,        SIGNAL(toggled(bool)),            SLOT(drumsetToggled(bool)));
      connect(portSpinBox,    SIGNAL(valueChanged(int)),        SLOT(midiChannelChanged(int)));
      connect(channelSpinBox, SIGNAL(valueChanged(int)),        SLOT(midiChannelChanged(int)));
      connect(expand,         SIGNAL(toggled(bool)),            SLOT(expandToggled(bool)));

      expandToggled(false);
      }

//---------------------------------------------------------
//   expandToggled
//---------------------------------------------------------

void PartEdit::expandToggled(bool val)
      {
      details->setVisible(val);
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
      _setValue(volume, a->volume);
      volume->setDclickValue1(dummy.volume);
      volume->setDclickValue2(dummy.volume);
      _setValue(reverb, a->reverb);
      reverb->setDclickValue1(dummy.reverb);
      reverb->setDclickValue2(dummy.reverb);
      _setValue(chorus, a->chorus);

      chorus->setDclickValue1(dummy.chorus);
      chorus->setDclickValue2(dummy.chorus);
      _setValue(pan, a->pan);

      pan->setDclickValue1(0);
      pan->setDclickValue2(0);
      for (int i = 0; i < patch->count(); ++i) {
            MidiPatch* p = (MidiPatch*)patch->itemData(i, Qt::UserRole).value<void*>();
            if (a->synti == p->synti && a->program == p->prog && a->bank == p->bank) {
                  patch->setCurrentIndex(i);
                  break;
                  }
            }
      _setChecked(drumset, p->instrument()->useDrumset());
      _setValue(portSpinBox,    part->masterScore()->midiMapping(a->channel)->port + 1);
      _setValue(channelSpinBox, part->masterScore()->midiMapping(a->channel)->channel + 1);

      QHBoxLayout* hb = voiceButtonBox;
      int idx = 0;
      for (Staff* staff : *part->staves()) {
            for (int voice = 0; voice < VOICES; ++voice) {
                  if (!voiceButtons.value(idx)) {
                        QToolButton* tb = new QToolButton;
                        tb->setText(QString("%1").arg(voice+1));
                        tb->setCheckable(true);
                        tb->setChecked(staff->playbackVoice(voice));
                        tb->setFocusPolicy(Qt::ClickFocus);
                        tb->setToolButtonStyle(Qt::ToolButtonTextOnly);
                        QPalette p(tb->palette());
                        p.setColor(QPalette::Base, MScore::selectColor[voice]);
                        tb->setPalette(p);
                        hb->addWidget(tb);
                        voiceButtons.append(tb);
                        connect(tb, SIGNAL(clicked()), SLOT(playbackVoiceChanged()));
                        }
                  ++idx;
                  }
            hb->addStretch(5);
            }
      while (voiceButtons.value(idx)) {
            QToolButton* tb = voiceButtons.value(idx);
            if (!tb)
                  break;
            voiceButtons.takeAt(idx);
            delete tb;
            }
      }

//---------------------------------------------------------
//   playbackVoiceChanged
//---------------------------------------------------------

void PartEdit::playbackVoiceChanged()
      {
      int idx = 0;
      Score* score = part->score();
      score->startCmd();
      for (Staff* staff : *part->staves()) {
            for (int voice = 0; voice < VOICES; ++voice) {
                  QToolButton* tb = voiceButtons[idx];
                  bool val = tb->isChecked();
                  if (val != staff->playbackVoice(voice)) {
                        switch (voice) {
                              case 0:
                              printf("undo\n");
                                    staff->undoChangeProperty(Pid::PLAYBACK_VOICE1, val);
                                    break;
                              case 1:
                                    staff->undoChangeProperty(Pid::PLAYBACK_VOICE2, val);
                                    break;
                              case 2:
                                    staff->undoChangeProperty(Pid::PLAYBACK_VOICE3, val);
                                    break;
                              case 3:
                                    staff->undoChangeProperty(Pid::PLAYBACK_VOICE4, val);
                                    break;
                              }
                        }
                  ++idx;
                  }
            }
      score->endCmd();
      }

//---------------------------------------------------------
//   Mixer
//---------------------------------------------------------

Mixer::Mixer(QWidget* parent)
   : QScrollArea(parent)
      {
      setObjectName("Mixer");
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
      retranslate(true);
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void Mixer::retranslate(bool firstTime)
      {
      setWindowTitle(tr("Mixer"));
      if (!firstTime) {
            for (int i = 0; i < vb->count(); i++) {
                  PartEdit* p = partEdit(i);
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
//   changeEvent
//---------------------------------------------------------

void Mixer::changeEvent(QEvent *event)
      {
      QScrollArea::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }

//---------------------------------------------------------
//   updateAll
//---------------------------------------------------------

void Mixer::updateAll(MasterScore* score)
      {
      cs = score;
      int n = -vb->count();
      if (n < 0) {
            QLayoutItem* wi = vb->itemAt(vb->count()-1);
            vb->removeItem(wi);
            delete wi;
            n += 1;
            }
      if (cs) {
            QList<MidiMapping>* mm = cs->midiMapping();
            n += mm->size();
            }
      while (n < 0) {
            QLayoutItem* wi = vb->itemAt(0);
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
      vb->addStretch(5);
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

      int idx = 0;
      QList<MidiMapping>* mm = cs->midiMapping();
      const QList<MidiPatch*> pl = synti->getPatchInfo();
      for (const MidiMapping& m : *mm) {
            QLayoutItem* wi  = (QWidgetItem*)(vb->itemAt(idx));
            if (!wi->widget())
                  continue;
            PartEdit* pe     = (PartEdit*)(wi->widget());
            bool drum        = m.part->instrument()->useDrumset();
            pe->patch->clear();
            for (const MidiPatch* p : pl) {
                  if (p->drum == drum || p->synti != "Fluid")
                        pe->patch->addItem(p->name, QVariant::fromValue<void*>((void*)p));
                  }
            pe->setPart(m.part, m.articulation);
            idx++;
            }
      // Update solo & mute only after creating all controls (we need to sync all controls)
      idx = 0;
      for (const MidiMapping& m : *mm) {
            QLayoutItem* wi = (QWidgetItem*)(vb->itemAt(idx));
            if (!wi->widget())
                  continue;
            PartEdit* pe    = (PartEdit*)(wi->widget());
            pe->mute->setChecked(m.articulation->mute);
            pe->solo->setChecked(m.articulation->solo);
            idx++;
            }
      }

//---------------------------------------------------------
//   midiPrefsChanged
//---------------------------------------------------------

void Mixer::midiPrefsChanged(bool showMidiControls)
      {
      if (!cs)
            return;
      for (int i = 0; i < vb->count(); i++ ){
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(i));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            pe->channelLabel  ->setVisible(showMidiControls);
            pe->portLabel     ->setVisible(showMidiControls);
            pe->channelSpinBox->setVisible(showMidiControls);
            pe->portSpinBox   ->setVisible(showMidiControls);

            if (!showMidiControls)
                  pe->hboxLayout->setSpacing(20);
            else
                  pe->hboxLayout->setSpacing(5);
            }
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
      mixer->updateAll(cs->masterScore());
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
            score->undo(new ChangePatch(score, channel, p));
            score->setLayoutAll();
            score->endCmd();
            }
      channel->updateInitList();
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
      channel->updateInitList();
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
      channel->updateInitList();
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
      channel->updateInitList();
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
      channel->updateInitList();
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
      channel->solo     = val;
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
      sync(syncControls);
      }

//---------------------------------------------------------
//   drumsetToggled
//---------------------------------------------------------

void PartEdit::drumsetToggled(bool val, bool syncControls)
      {
      if (part == 0)
            return;

      Score* score = part->score();
      score->startCmd();

      part->undoChangeProperty(Pid::USE_DRUMSET, val);
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
      score->setLayoutAll();
      score->endCmd();
      sync(syncControls);
      }

//---------------------------------------------------------
//   updateSolo
//---------------------------------------------------------

void Mixer::updateSolo(bool val)
      {
      for (int i = 0; i < vb->count(); i++ ){
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(i));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            if (pe)
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

//---------------------------------------------------------
//   sync
//   synchronizes controls with same MIDI port and channel
//---------------------------------------------------------

void PartEdit::sync(bool syncControls)
      {
      if (!syncControls)
            return;
      int count = this->parentWidget()->layout()->count();
      for(int i = 0; i < count; i++) {
            QWidgetItem* wi = (QWidgetItem*)(this->parentWidget()->layout()->itemAt(i));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            if (pe != 0 && pe != this
                && this->channelSpinBox->value() == pe->channelSpinBox->value()
                && this->portSpinBox->value() == pe->portSpinBox->value()) {

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

                  if (drumset->isChecked() != pe->drumset->isChecked()) {
                        _setChecked(pe->drumset, drumset->isChecked());
                        emit pe->drumsetToggled(drumset->isChecked(), false);
                        }
                  if (patch->currentIndex() != pe->patch->currentIndex()) {
                        pe->patch->blockSignals(true);
                        pe->patch->setCurrentIndex(this->patch->currentIndex());
                        pe->patch->blockSignals(false);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   midiChannelChanged
//   handles MIDI port & channel change
//---------------------------------------------------------

void PartEdit::midiChannelChanged(int)
      {
      if (part == 0)
            return;
      seq->stopNotes(channel->channel);
      int p =    portSpinBox->value() - 1;
      int c = channelSpinBox->value() - 1;

      // 1 is for going up, -1 for going down
      int direction = copysign(1, c - part->masterScore()->midiMapping(channel->channel)->channel);

      // Channel 9 is special for drums
      if (part->instrument()->useDrumset() && c != 9) {
            _setValue(channelSpinBox, 10);
            return;
            }
      else if (!part->instrument()->useDrumset() && c == 9) {
            c = 9 + direction;
            }

      if (c == 16) {
            c = 0;
            p++;
            }

      int newChannel = p*16+c;

      // If there is an instrument with the same MIDI port and channel, sync this instrument to a found one
      bool needSync = true;
      int elementsInMixer = parentWidget()->layout()->count();
      for (int i = 0; i < elementsInMixer; i++) {
            QWidgetItem* wi = (QWidgetItem*)(this->parentWidget()->layout()->itemAt(i));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            if (pe != 0 && pe != this
                        && pe->channelSpinBox->value() == this->channelSpinBox->value()
                        && pe->portSpinBox->value() == this->portSpinBox->value()) {
                  // Show datails if parameters are different
                  QString detailedText;
                  if (patch->currentIndex() != pe->patch->currentIndex())
                        detailedText += QString(tr("Sound: '%1'' vs '%2'\n")).arg(patch->itemText(patch->currentIndex()), pe->patch->itemText(pe->patch->currentIndex()));
                  if (this->volume->value() != pe->volume->value())
                        detailedText += QString(tr("Volume: %1 vs %2\n")).arg(QString::number(volume->value()),  QString::number(pe->volume->value()));
                  if (this->pan->value() != pe->pan->value())
                        detailedText += QString(tr("Pan: %1 vs %2\n")).arg(QString::number(pan->value()),  QString::number(pe->pan->value()));
                  if (this->reverb->value() != pe->reverb->value())
                        detailedText += QString(tr("Reverb: %1 vs %2\n")).arg(QString::number(reverb->value()),  QString::number(pe->reverb->value()));
                  if (this->chorus->value() != pe->chorus->value())
                        detailedText += QString(tr("Chorus: %1 vs %2\n")).arg(QString::number(chorus->value()),  QString::number(pe->chorus->value()));
                  if (this->solo->isChecked() != pe->solo->isChecked())
                        detailedText += QString(tr("Solo: %1 vs %2\n")).arg(solo->isChecked()?"Yes":"No", pe->solo->isChecked()?"Yes":"No");
                  if (this->mute->isChecked() != pe->mute->isChecked())
                        detailedText += QString(tr("Mute: %1 vs %2\n")).arg(mute->isChecked()?"Yes":"No", pe->mute->isChecked()?"Yes":"No");

                  if (!detailedText.isEmpty())
                        detailedText = QString(tr("Instrument '%1'    Instrument '%2'\n")).arg(this->partName->text(), pe->partName->text())+ detailedText;
                  QMessageBox msgBox;
                  msgBox.setIcon(QMessageBox::Warning);
                  QString text = QString(tr("There is already an instrument '%1' with MIDI port = %2 and channel = %3.")).arg(pe->partName->text(), QString::number(pe->portSpinBox->value()), QString::number(pe->channelSpinBox->value()));
                  msgBox.setText(text);
                  msgBox.setInformativeText(tr("Do you want to synchronize current instrument with an existing one?"));
                  msgBox.setDetailedText(detailedText);
                  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                  QPushButton *assignFreeChannel = msgBox.addButton(tr("Assign next free MIDI channel"), QMessageBox::HelpRole);
                  msgBox.setDefaultButton(QMessageBox::Ok);
                  if (msgBox.exec() == QMessageBox::Cancel) {
                        _setValue(channelSpinBox, part->masterScore()->midiMapping(channel->channel)->channel + 1);
                        _setValue(portSpinBox,    part->masterScore()->midiMapping(channel->channel)->port + 1);
                        needSync = false;
                        break;
                        }

                  if (msgBox.clickedButton() == assignFreeChannel) {
                        newChannel = part->masterScore()->getNextFreeMidiMapping();
                        break;
                        }
                  // Sync
                  _setValue(channelSpinBox, newChannel % 16 + 1);
                  _setValue(portSpinBox,    newChannel / 16 + 1);
                  part->masterScore()->midiMapping(channel->channel)->channel = newChannel % 16;
                  part->masterScore()->midiMapping(channel->channel)->port    = newChannel / 16;
                  channel->volume = lrint(pe->volume->value());
                  channel->pan    = lrint(pe->pan->value());
                  channel->reverb = lrint(pe->reverb->value());
                  channel->chorus = lrint(pe->chorus->value());
                  channel->mute = pe->mute->isChecked();
                  channel->solo = pe->solo->isChecked();

                  MidiPatch* newPatch = (MidiPatch*)pe->patch->itemData(pe->patch->currentIndex(), Qt::UserRole).value<void*>();
                  if (newPatch == 0)
                        return;
                  patch->setCurrentIndex(pe->patch->currentIndex());
                  channel->program = newPatch->prog;
                  channel->bank    = newPatch->bank;
                  channel->synti   = newPatch->synti;

                  part->masterScore()->setSoloMute();
                  part->score()->setInstrumentsChanged(true);
                  part->score()->setLayoutAll();
                  break;
                  }
            }
      channel->updateInitList();
      if (needSync) {
            _setValue(channelSpinBox, newChannel % 16 + 1);
            _setValue(portSpinBox,    newChannel / 16 + 1);
            part->masterScore()->midiMapping(channel->channel)->channel = newChannel % 16;
            part->masterScore()->midiMapping(channel->channel)->port    = newChannel / 16;
            part->score()->setInstrumentsChanged(true);
            part->score()->setLayoutAll();
            seq->initInstruments();
            }
      else {
            // Initializing an instrument with new channel
            foreach(const MidiCoreEvent& e, channel->init) {
                  if (e.type() == ME_INVALID)
                        continue;
                  NPlayEvent event(e.type(), channel->channel, e.dataA(), e.dataB());
                  seq->sendEvent(event);
                  }
            }

      // Update MIDI Out ports
      int maxPort = max(p, part->score()->masterScore()->midiPortCount());
      part->score()->masterScore()->setMidiPortCount(maxPort);
      if (seq->driver() && (preferences.getBool(PREF_IO_JACK_USEJACKMIDI) || preferences.getBool(PREF_IO_ALSA_USEALSAAUDIO)))
            seq->driver()->updateOutPortCount(maxPort + 1);
      }
}

