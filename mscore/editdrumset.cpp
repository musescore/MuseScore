//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: editdrumset.cpp 5384 2012-02-27 12:21:49Z wschweer $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "editdrumset.h"
#include "musescore.h"
#include "libmscore/xml.h"
#include "libmscore/utils.h"
#include "libmscore/chord.h"
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/stem.h"

enum { COL_PITCH, COL_NOTE, COL_SHORTCUT, COL_NAME };

//---------------------------------------------------------
//   noteHeadNames
//---------------------------------------------------------

const char* noteHeadNames[HEAD_GROUPS] = {
      QT_TRANSLATE_NOOP("noteheadnames", "normal"),
      QT_TRANSLATE_NOOP("noteheadnames", "cross"),
      QT_TRANSLATE_NOOP("noteheadnames", "diamond"),
      QT_TRANSLATE_NOOP("noteheadnames", "triangle"),
      QT_TRANSLATE_NOOP("noteheadnames", "mi"),
      QT_TRANSLATE_NOOP("noteheadnames", "slash"),
      QT_TRANSLATE_NOOP("noteheadnames", "xcircle"),
      QT_TRANSLATE_NOOP("noteheadnames", "do"),
      QT_TRANSLATE_NOOP("noteheadnames", "re"),
      QT_TRANSLATE_NOOP("noteheadnames", "fa"),
      QT_TRANSLATE_NOOP("noteheadnames", "la"),
      QT_TRANSLATE_NOOP("noteheadnames", "ti")
      };

//---------------------------------------------------------
//   EditDrumset
//---------------------------------------------------------

EditDrumset::EditDrumset(Drumset* ds, QWidget* parent)
   : QDialog(parent)
      {
      oDrumset = ds;
      nDrumset = *ds;
      setupUi(this);

      drumNote->setGrid(70, 80);
      drumNote->setDrawGrid(true);
      drumNote->setReadOnly(true);

      updateList();

      noteHead->addItem(tr("invalid"));
      for (int i = 0; i < HEAD_GROUPS; ++i)
            noteHead->addItem(noteHeadNames[i]);

      connect(pitchList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
         SLOT(itemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(bboxClicked(QAbstractButton*)));
      connect(name, SIGNAL(textChanged(const QString&)), SLOT(nameChanged(const QString&)));
      connect(noteHead, SIGNAL(currentIndexChanged(int)), SLOT(valueChanged()));
      connect(staffLine, SIGNAL(valueChanged(int)), SLOT(valueChanged()));
      connect(voice, SIGNAL(valueChanged(int)), SLOT(valueChanged()));
      connect(stemDirection, SIGNAL(currentIndexChanged(int)), SLOT(valueChanged()));
      connect(shortcut, SIGNAL(currentIndexChanged(int)), SLOT(shortcutChanged()));
      connect(loadButton, SIGNAL(clicked()), SLOT(load()));
      connect(saveButton, SIGNAL(clicked()), SLOT(save()));
      }

//---------------------------------------------------------
//   updateList
//---------------------------------------------------------

void EditDrumset::updateList()
      {
      pitchList->clear();
      for (int i = 0; i < 128; ++i) {
            QTreeWidgetItem* item = new QTreeWidgetItem(pitchList);
            item->setText(COL_PITCH, QString("%1").arg(i));
            item->setText(COL_NOTE, pitch2string(i));
            if (nDrumset.shortcut(i) == 0)
                  item->setText(COL_SHORTCUT, "");
            else {
                  QString s(QChar(nDrumset.shortcut(i)));
                  item->setText(COL_SHORTCUT, s);
                  }
            item->setText(COL_NAME, qApp->translate("drumset", qPrintable(nDrumset.name(i))));
            item->setData(0, Qt::UserRole, i);
            }
      }

void EditDrumset::updateList2()
      {
      for (int i = 0; i < pitchList->topLevelItemCount(); ++i) {
            QTreeWidgetItem* item = pitchList->topLevelItem(i);
            int pitch = item->data(0, Qt::UserRole).toInt();
            if (nDrumset.shortcut(pitch) == 0)
                  item->setText(COL_SHORTCUT, "");
            else {
                  QString s(QChar(nDrumset.shortcut(pitch)));
                  item->setText(COL_SHORTCUT, s);
                  }
            item->setText(COL_NAME, qApp->translate("drumset", qPrintable(nDrumset.name(pitch))));
            item->setData(0, Qt::UserRole, pitch);
            }
      }

//---------------------------------------------------------
//   nameChanged
//---------------------------------------------------------

void EditDrumset::nameChanged(const QString& name)
      {
      QTreeWidgetItem* item = pitchList->currentItem();
      if (item)
            item->setText(COL_NAME, name);
      }

//---------------------------------------------------------
//   shortcutChanged
//---------------------------------------------------------

void EditDrumset::shortcutChanged()
      {
      QTreeWidgetItem* item = pitchList->currentItem();
      if (!item)
            return;

      int pitch = item->data(COL_PITCH, Qt::UserRole).toInt();
      int sc;
      if (shortcut->currentIndex() == 7)
            sc = 0;
      else
            sc = "ABCDEFG"[shortcut->currentIndex()];

      if (QString(QChar(nDrumset.drum(pitch).shortcut)) != shortcut->currentText()) {
            //
            // remove conflicting shortcuts
            //
            for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
                  if (i == pitch)
                        continue;
                  if (nDrumset.drum(i).shortcut == sc)
                        nDrumset.drum(i).shortcut = 0;
                  }
            nDrumset.drum(pitch).shortcut = sc;
            if (shortcut->currentIndex() == 7)
                  item->setText(COL_SHORTCUT, "");
            else
                  item->setText(COL_SHORTCUT, shortcut->currentText());
            }
      updateList2();
      }

//---------------------------------------------------------
//   bboxClicked
//---------------------------------------------------------

void EditDrumset::bboxClicked(QAbstractButton* button)
      {
      QDialogButtonBox::ButtonRole br = buttonBox->buttonRole(button);
      switch(br) {
            case QDialogButtonBox::ApplyRole:
                  apply();
                  break;

            case QDialogButtonBox::AcceptRole:
                  apply();
                  // fall through

            case QDialogButtonBox::RejectRole:
                  close();
                  break;

            default:
                  qDebug("EditDrumSet: unknown button\n");
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void EditDrumset::apply()
      {
      *oDrumset = nDrumset;
      }

//---------------------------------------------------------
//   itemChanged
//---------------------------------------------------------

void EditDrumset::itemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
      {
      if (previous) {
            int pitch = previous->data(0, Qt::UserRole).toInt();
            nDrumset.drum(pitch).name          = name->text();
            nDrumset.drum(pitch).notehead      = NoteHeadGroup(noteHead->currentIndex() - 1);
            nDrumset.drum(pitch).line          = staffLine->value();
            nDrumset.drum(pitch).voice         = voice->value() - 1;
            if (shortcut->currentIndex() == 7)
                  nDrumset.drum(pitch).shortcut = 0;
            else
                  nDrumset.drum(pitch).shortcut = "ABCDEFG"[shortcut->currentIndex()];
            nDrumset.drum(pitch).stemDirection = Direction(stemDirection->currentIndex());
            previous->setText(COL_NAME, qApp->translate("drumset", qPrintable(nDrumset.name(pitch))));
            }
      if (current == 0)
            return;
      name->blockSignals(true);
      staffLine->blockSignals(true);
      voice->blockSignals(true);
      stemDirection->blockSignals(true);
      noteHead->blockSignals(true);

      int pitch = current->data(0, Qt::UserRole).toInt();
      name->setText(qApp->translate("drumset", qPrintable(nDrumset.name(pitch))));
      staffLine->setValue(nDrumset.line(pitch));
      voice->setValue(nDrumset.voice(pitch) + 1);
      stemDirection->setCurrentIndex(int(nDrumset.stemDirection(pitch)));
      int nh = nDrumset.noteHead(pitch);
      noteHead->setCurrentIndex(nh + 1);
      if (nDrumset.shortcut(pitch) == 0)
            shortcut->setCurrentIndex(7);
      else
            shortcut->setCurrentIndex(nDrumset.shortcut(pitch) - 'A');

      name->blockSignals(false);
      staffLine->blockSignals(false);
      voice->blockSignals(false);
      stemDirection->blockSignals(false);
      noteHead->blockSignals(false);

      updateExample();
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void EditDrumset::valueChanged()
      {
      int pitch = pitchList->currentItem()->data(COL_PITCH, Qt::UserRole).toInt();
      nDrumset.drum(pitch).name          = name->text();
      nDrumset.drum(pitch).notehead      = NoteHeadGroup(noteHead->currentIndex() - 1);
      nDrumset.drum(pitch).line          = staffLine->value();
      nDrumset.drum(pitch).voice         = voice->value() - 1;
      nDrumset.drum(pitch).stemDirection = Direction(stemDirection->currentIndex());
      if (QString(QChar(nDrumset.drum(pitch).shortcut)) != shortcut->currentText()) {
            if (shortcut->currentText().isEmpty())
                  nDrumset.drum(pitch).shortcut = 0;
            else
                  nDrumset.drum(pitch).shortcut = shortcut->currentText().at(0).toAscii();
            }
      updateExample();
      }

//---------------------------------------------------------
//   updateExample
//---------------------------------------------------------

void EditDrumset::updateExample()
      {
      int pitch = pitchList->currentItem()->data(0, Qt::UserRole).toInt();
      if (!nDrumset.isValid(pitch)) {
            drumNote->add(0,  0, "");
            return;
            }
      int line      = nDrumset.line(pitch);
      NoteHeadGroup noteHead = nDrumset.noteHead(pitch);
      int voice     = nDrumset.voice(pitch);
      Direction dir = nDrumset.stemDirection(pitch);
      bool up;
      if (dir == UP)
            up = true;
      else if (dir == DOWN)
            up = false;
      else
            up = line > 4;
      Chord* chord = new Chord(gscore);
      chord->setDurationType(TDuration::V_QUARTER);
      chord->setStemDirection(dir);
      chord->setTrack(voice);
      Note* note = new Note(gscore);
      note->setParent(chord);
      note->setTrack(voice);
      note->setPitch(pitch);
      note->setTpcFromPitch();
      note->setLine(line);
      note->setPos(0.0, gscore->spatium() * .5 * line);
      note->setHeadGroup(noteHead);
      chord->add(note);
      Stem* stem = new Stem(gscore);
      stem->setLen((up ? -3.0 : 3.0) * gscore->spatium());
      chord->setStem(stem);
      stem->setPos(note->stemPos(up));
      drumNote->add(0,  chord, qApp->translate("drumset", qPrintable(nDrumset.name(pitch))));
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void EditDrumset::load()
      {
      QString name = mscore->getDrumsetFilename(true);
      if (name.isEmpty())
            return;

      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly))
            return;

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&fp, false, &err, &line, &column)) {
            qDebug("error reading file %s at line %d column %d: %s\n",
               qPrintable(name), line, column, qPrintable(err));
            return;
            }

      docName = name;
      nDrumset.clear();
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString val(ee.text());
                        if (tag == "Drum")
                              nDrumset.load(ee);
                        else
                              domError(ee);
                        }
                  }
            }
      fp.close();
      updateList();
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void EditDrumset::save()
      {
      QString name = mscore->getDrumsetFilename(false);
      if (name.isEmpty())
            return;

      QFile f(name);
      if (!f.open(QIODevice::WriteOnly)) {
            QString s = tr("Open File\n") + f.fileName() + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, tr("MuseScore: Open File"), s);
            return;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      nDrumset.save(xml);
      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = tr("Write File failed: ") + f.errorString();
            QMessageBox::critical(this, tr("MuseScore: Write Drumset"), s);
            }
      }

