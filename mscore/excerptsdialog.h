//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

#ifndef __EXCERPTSDIALOG_H__
#define __EXCERPTSDIALOG_H__

#include "ui_excerptsdialog.h"
#include "libmscore/excerpt.h"

namespace Ms {

class MasterScore;
class Excerpt;
class Part;
class Staff;

//---------------------------------------------------------
//   ExcerptItem
//---------------------------------------------------------

class ExcerptItem : public QListWidgetItem {
      Excerpt* _excerpt;

   public:
      ExcerptItem(Excerpt*, QListWidget* parent = 0);
      Excerpt* excerpt() { return _excerpt; }
      };

//---------------------------------------------------------
//   PartItem
//---------------------------------------------------------

class PartItem : public QTreeWidgetItem {
      Part* _part;

   public:
      PartItem(Part*, QTreeWidget* parent = 0);
      Part* part() const                    { return _part;   }
      };

//---------------------------------------------------------
//   ScorePartsItem
//---------------------------------------------------------

class InstrumentItem : public QListWidgetItem {
      PartItem* _partItem;

   public:
      InstrumentItem(PartItem*, QListWidget* parent = 0);
      PartItem* partItem() const { return _partItem; }
      };

//---------------------------------------------------------
//   StaffListItem
//---------------------------------------------------------

class StaffItem : public QTreeWidgetItem {
      Staff* _staff { 0 };

   public:
      StaffItem();
      StaffItem(PartItem* li);

      Staff* staff() const        { return _staff;    }
      void setStaff(Staff* s)     { _staff = s;       }
      void setData(int column, int role, const QVariant& value) override;
      };

//---------------------------------------------------------
//   ExcerptsDialog
//---------------------------------------------------------

class ExcerptsDialog : public QDialog, private Ui::ExcerptsDialog {
      Q_OBJECT
      MasterScore* score;

      QString createName(const QString&);

      virtual void accept();

   private slots:
      void deleteClicked();
      void newClicked();
      void newAllClicked();
      void moveUpClicked();
      void moveDownClicked();
      void excerptChanged(QListWidgetItem* cur, QListWidgetItem* prev);
      void partDoubleClicked(QTreeWidgetItem*, int);
      void partClicked(QTreeWidgetItem*, int);
      void createExcerptClicked(QListWidgetItem*);
      void titleChanged(const QString&);
      ExcerptItem* isInPartsList(Excerpt* e);

      QMultiMap<int, int> mapTracks();
      void assignTracks(QMultiMap<int, int> );

      void doubleClickedInstrument(QTreeWidgetItem*);
      void addButtonClicked();
      void removeButtonClicked();

   public:
      ExcerptsDialog(MasterScore*, QWidget* parent = 0);
      };


} // namespace Ms
#endif

