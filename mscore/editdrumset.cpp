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

namespace Ms {

enum Column : char { PITCH, NOTE, SHORTCUT, NAME };

//---------------------------------------------------------
//   noteHeadNames (see also libmscore/note.{h,cpp})
//   same order as NoteHead::Group, "Sol" and "Alt. Brevis" omitted,
//   as not being useful for drums
//---------------------------------------------------------

const char* noteHeadNames[int(NoteHead::Group::HEAD_GROUPS)] = {
      QT_TRANSLATE_NOOP("EditDrumset", "Normal"),
      QT_TRANSLATE_NOOP("EditDrumset", "Cross"),
      QT_TRANSLATE_NOOP("EditDrumset", "Diamond"),
      QT_TRANSLATE_NOOP("EditDrumset", "Triangle"),
      QT_TRANSLATE_NOOP("EditDrumset", "Mi"),
      QT_TRANSLATE_NOOP("EditDrumset", "Slash"),
      QT_TRANSLATE_NOOP("EditDrumset", "XCircle"),
      QT_TRANSLATE_NOOP("EditDrumset", "Do"),
      QT_TRANSLATE_NOOP("EditDrumset", "Re"),
      QT_TRANSLATE_NOOP("EditDrumset", "Fa"),
      QT_TRANSLATE_NOOP("EditDrumset", "La"),
      QT_TRANSLATE_NOOP("EditDrumset", "Ti"),
      };

//---------------------------------------------------------
//   EditDrumset
//---------------------------------------------------------

EditDrumset::EditDrumset(const Drumset* ds, QWidget* parent)
   : QDialog(parent)
      {
      nDrumset = *ds;
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      drumNote->setGrid(70, 80);
      drumNote->setDrawGrid(false);
      drumNote->setReadOnly(true);

      updateList();

      noteHead->addItem(tr("invalid"));
      for (int i = 0; i < int(NoteHead::Group::HEAD_GROUPS) - 2 ; ++i)
            noteHead->addItem(tr(noteHeadNames[i]));

      connect(pitchList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
         SLOT(itemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(bboxClicked(QAbstractButton*)));
      connect(name, SIGNAL(textChanged(const QString&)), SLOT(nameChanged(const QString&)));
      connect(noteHead, SIGNAL(currentIndexChanged(int)), SLOT(valueChanged()));
      connect(staffLine, SIGNAL(valueChanged(int)), SLOT(valueChanged()));
      connect(voice, SIGNAL(currentIndexChanged(int)), SLOT(valueChanged()));
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
            item->setText(Column::PITCH, QString("%1").arg(i));
            item->setText(Column::NOTE, pitch2string(i));
            if (nDrumset.shortcut(i) == 0)
                  item->setText(Column::SHORTCUT, "");
            else {
                  QString s(QChar(nDrumset.shortcut(i)));
                  item->setText(Column::SHORTCUT, s);
                  }
            item->setText(Column::NAME, qApp->translate("drumset", nDrumset.name(i).toUtf8().constData()));
            item->setData(0, Qt::UserRole, i);
            }
      }

void EditDrumset::updateList2()
      {
      for (int i = 0; i < pitchList->topLevelItemCount(); ++i) {
            QTreeWidgetItem* item = pitchList->topLevelItem(i);
            int pitch = item->data(0, Qt::UserRole).toInt();
            if (nDrumset.shortcut(pitch) == 0)
                  item->setText(Column::SHORTCUT, "");
            else {
                  QString s(QChar(nDrumset.shortcut(pitch)));
                  item->setText(Column::SHORTCUT, s);
                  }
            item->setText(Column::NAME, qApp->translate("drumset", nDrumset.name(pitch).toUtf8().constData()));
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
            item->setText(Column::NAME, name);
      }

//---------------------------------------------------------
//   shortcutChanged
//---------------------------------------------------------

void EditDrumset::shortcutChanged()
      {
      QTreeWidgetItem* item = pitchList->currentItem();
      if (!item)
            return;

      int pitch = item->data(Column::PITCH, Qt::UserRole).toInt();
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
                  item->setText(Column::SHORTCUT, "");
            else
                  item->setText(Column::SHORTCUT, shortcut->currentText());
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
                  qDebug("EditDrumSet: unknown button");
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void EditDrumset::apply()
      {
      valueChanged();  //save last changes in name
      }

//---------------------------------------------------------
//   itemChanged
//---------------------------------------------------------

void EditDrumset::itemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
      {
      if (previous) {
            int pitch = previous->data(0, Qt::UserRole).toInt();
            nDrumset.drum(pitch).name          = name->text();
            nDrumset.drum(pitch).notehead      = NoteHead::Group(noteHead->currentIndex() - 1);
            nDrumset.drum(pitch).line          = staffLine->value();
            nDrumset.drum(pitch).voice         = voice->currentIndex();
            if (shortcut->currentIndex() == 7)
                  nDrumset.drum(pitch).shortcut = 0;
            else
                  nDrumset.drum(pitch).shortcut = "ABCDEFG"[shortcut->currentIndex()];
            nDrumset.drum(pitch).stemDirection = Direction(stemDirection->currentIndex());
            previous->setText(Column::NAME, qApp->translate("drumset", nDrumset.name(pitch).toUtf8().constData()));
            }
      if (current == 0)
            return;
      name->blockSignals(true);
      staffLine->blockSignals(true);
      voice->blockSignals(true);
      stemDirection->blockSignals(true);
      noteHead->blockSignals(true);

      int pitch = current->data(0, Qt::UserRole).toInt();
      name->setText(qApp->translate("drumset", nDrumset.name(pitch).toUtf8().constData()));
      staffLine->setValue(nDrumset.line(pitch));
      qDebug("BEFORE %d", nDrumset.voice(pitch));
      voice->setCurrentIndex(nDrumset.voice(pitch));
      qDebug("AFTER %d", nDrumset.voice(pitch));
      stemDirection->setCurrentIndex(int(nDrumset.stemDirection(pitch)));
      NoteHead::Group nh = nDrumset.noteHead(pitch);
      noteHead->setCurrentIndex(int(nh) + 1);
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
      if(!pitchList->currentItem())
            return;
      int pitch = pitchList->currentItem()->data(Column::PITCH, Qt::UserRole).toInt();
      nDrumset.drum(pitch).name          = name->text();
      nDrumset.drum(pitch).notehead      = NoteHead::Group(noteHead->currentIndex() - 1);
      nDrumset.drum(pitch).line          = staffLine->value();
      nDrumset.drum(pitch).voice         = voice->currentIndex();
      nDrumset.drum(pitch).stemDirection = Direction(stemDirection->currentIndex());
      if (QString(QChar(nDrumset.drum(pitch).shortcut)) != shortcut->currentText()) {
            if (shortcut->currentText().isEmpty())
                  nDrumset.drum(pitch).shortcut = 0;
            else
                  nDrumset.drum(pitch).shortcut = shortcut->currentText().at(0).toLatin1();
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
      NoteHead::Group noteHead = nDrumset.noteHead(pitch);
      int voice     = nDrumset.voice(pitch);
      Direction dir = nDrumset.stemDirection(pitch);
      bool up;
      if (dir == Direction::UP)
            up = true;
      else if (dir == Direction::DOWN)
            up = false;
      else
            up = line > 4;
      Chord* chord = new Chord(gscore);
      chord->setDurationType(TDuration::DurationType::V_QUARTER);
      chord->setStemDirection(dir);
      chord->setTrack(voice);
      chord->setUp(up);
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
      chord->add(stem);
      stem->setPos(chord->stemPos());
      drumNote->add(0,  chord, qApp->translate("drumset", nDrumset.name(pitch).toUtf8().constData()));
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

      XmlReader e(&fp);
      nDrumset.clear();
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "Drum")
                              nDrumset.load(e);
                        else
                              e.unknown();
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
            QString s = tr("Open File\n%1\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, tr("MuseScore: Open File"), s.arg(f.fileName()));
            return;
            }
      valueChanged();  //save last changes in name
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
}

