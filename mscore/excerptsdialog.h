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
class PartItem;

//---------------------------------------------------------
//   StaffListItem
//---------------------------------------------------------

class StaffItem : public QTreeWidgetItem {
      int               _firstTrack;
      QList<StaffItem*> _linked;

   public:
      StaffItem();
      StaffItem(int, int, int, const QMultiMap<int, int>, bool, PartItem* li);

      void setLink(StaffItem*);

      void setData(int column, int role, const QVariant& value) override;

      QMultiMap<int, int> tracks(int&) const;

      void dump();
      };

//---------------------------------------------------------
//   PartItem
//---------------------------------------------------------

class PartItem : public QTreeWidgetItem {
      Part*             _part;
      QList<StaffItem*> _staffItems;

   public:
      PartItem(Part*, const QMultiMap<int, int>, int&, bool, QTreeWidget* parent = 0);
      ~PartItem();

      Part* part() const                   { return _part;       }
      QList<StaffItem*> staffItems() const { return _staffItems; }

      StaffItem* findStaffItem(QTreeWidgetItem*) const;
      QMultiMap<int, int> tracks(int&) const;

      void dump();
      };

//---------------------------------------------------------
//   ExcerptItem
//---------------------------------------------------------

class ExcerptItem : public QListWidgetItem {
      Excerpt*            _excerpt;
      QList<PartItem*>    _partItems;
      QMultiMap<int, int> _tracks;

   public:
      ExcerptItem(Excerpt*, QListWidget* parent = 0);
      ~ExcerptItem();

      Excerpt* excerpt() { return _excerpt; }

      void addPartItem(PartItem*);
      void removePartItem(PartItem*);
      QList<PartItem*> partItems() const              { return _partItems; }
      PartItem* findPartItem(QTreeWidgetItem*) const;
      bool isEmpty() const                            { return _partItems.isEmpty(); }
      bool isPartScore() const                        { return _excerpt->partScore(); }

      QString title() const                           { return text(); }
      void setTitle(const QString);

      QMultiMap<int, int> tracks() const;

      void dump(const char*);
      };

//---------------------------------------------------------
//   InstrumentItem
//---------------------------------------------------------

class InstrumentItem : public QListWidgetItem {
      Part* _part;

   public:
      InstrumentItem(Part*, QListWidget* parent = 0);

      PartItem* newPartItem(int) const;
      };

//---------------------------------------------------------
//   ExcerptsDialog
//---------------------------------------------------------

class ExcerptsDialog : public QDialog, private Ui::ExcerptsDialog {
      Q_OBJECT
      MasterScore* score;

      QString createName(const QString&);
      PartItem* getCurrentPartItem() const;
      ExcerptItem* getCurrentExcerptItem() const;
      ExcerptItem* getExcerptItemAt(int) const;
      InstrumentItem* getCurrentInstrumentItem() const;
      void clearPartList();
      void setWidgetState();

      virtual void accept();

   private slots:
      void deleteClicked();
      void singlePartClicked();
      void allPartsClicked();
      void moveUpClicked();
      void moveDownClicked();
      void excerptChanged(QListWidgetItem* cur, QListWidgetItem* prev);
      void instrumentChanged(QListWidgetItem* cur, QListWidgetItem* prev);
      void partChanged(QTreeWidgetItem* cur, QTreeWidgetItem* prev);
      void partDoubleClicked(QTreeWidgetItem*, int);
      void createNewExcerpt(ExcerptItem*);
      void titleChanged(const QString&);
      ExcerptItem* getExcerptItem(Excerpt* e);

      void doubleClickedInstrument(QTreeWidgetItem*);
      void addButtonClicked();
      void removeButtonClicked();

   public:
      ExcerptsDialog(MasterScore*, QWidget* parent = 0);
      ~ExcerptsDialog();
      };


} // namespace Ms
#endif

