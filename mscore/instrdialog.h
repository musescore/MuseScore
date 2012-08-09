//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: instrdialog.h 5377 2012-02-25 10:24:05Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __INSTRDIALOG_H__
#define __INSTRDIALOG_H__

#include "ui_instrdialog.h"
#include "globals.h"
#include "libmscore/mscore.h"

struct InstrumentTemplate;
class Instrument;
class Part;
class Staff;
class Score;
class EditInstrument;

enum { ITEM_KEEP, ITEM_DELETE, ITEM_ADD };
enum { PART_LIST_ITEM = QTreeWidgetItem::UserType, STAFF_LIST_ITEM };

//---------------------------------------------------------
//   PartListItem
//---------------------------------------------------------

class PartListItem : public QTreeWidgetItem {

   public:
      int op;
      Part* part;
      const InstrumentTemplate* it;

      PartListItem(Part* p, QTreeWidget* lv);
      PartListItem(const InstrumentTemplate* i, QTreeWidget* lv);
      bool visible() const;
      void setVisible(bool val);
      };

//---------------------------------------------------------
//   StaffListItem
//---------------------------------------------------------

class StaffListItem : public QTreeWidgetItem {
      ClefType _clef;
      int _partIdx;
      bool _linked;

   public:
      StaffListItem();
      StaffListItem(PartListItem* li);

      int op;
      Staff* staff;
      int partIdx() const      { return _partIdx; }
      void setPartIdx(int val);
      int staffIdx;

      void setClef(ClefType val);
      ClefType clef() const    { return _clef;    }
      void setLinked(bool val);
      bool linked() const      { return _linked;  }
      };

//---------------------------------------------------------
//   InstrumentsDialog
//---------------------------------------------------------

class InstrumentsDialog : public QDialog, public Ui::InstrumentDialogBase {
      Q_OBJECT
      Score* cs;
      EditInstrument* editInstrument;

   private slots:
      void on_instrumentList_itemSelectionChanged();
      void on_instrumentList_itemDoubleClicked(QTreeWidgetItem* item, int);
      void on_partiturList_itemSelectionChanged();
      void on_addButton_clicked();
      void on_removeButton_clicked();
      void on_upButton_clicked();
      void on_downButton_clicked();
      void on_belowButton_clicked();
      void on_linkedButton_clicked();
      void on_saveButton_clicked();
      void on_loadButton_clicked();
      void buildTemplateList();
      virtual void accept();

   public:
      InstrumentsDialog(QWidget* parent = 0);
      void setScore(Score* s) { cs = s; }
      void genPartList();
      };

//---------------------------------------------------------
//   InstrumentTemplateListItem
//---------------------------------------------------------

class InstrumentTemplateListItem : public QTreeWidgetItem {
      InstrumentTemplate* _instrumentTemplate;
      QString _group;

   public:
      InstrumentTemplateListItem(QString group, QTreeWidget* parent);
      InstrumentTemplateListItem(InstrumentTemplate* i, InstrumentTemplateListItem* parent);
      InstrumentTemplateListItem(InstrumentTemplate* i, QTreeWidget* parent);

      InstrumentTemplate* instrumentTemplate() const { return _instrumentTemplate; }
      virtual QString text(int col) const;
      };

extern void populateInstrumentList(QTreeWidget* instrumentList, bool extended);
#endif

