//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __INSTRWIDGET_H__
#define __INSTRWIDGET_H__

#include "ui_instrwidget.h"
#include "libmscore/clef.h"

namespace Ms {

//class EditInstrument;
class InstrumentTemplate;
class Instrument;
class Part;
class Staff;
class StaffType;
class Score;
class InstrumentGenre;

enum class ListItemOp : char { KEEP, I_DELETE, ADD, UPDATE };
enum { PART_LIST_ITEM = QTreeWidgetItem::UserType, STAFF_LIST_ITEM };

//---------------------------------------------------------
//   PartListItem
//---------------------------------------------------------

class PartListItem : public QTreeWidgetItem {

   public:
      ListItemOp op;
      Part* part;
      const InstrumentTemplate* it;

      PartListItem(Part* p, QTreeWidget* lv);
      PartListItem(const InstrumentTemplate* i, QTreeWidget* lv);
      bool visible() const;
      void setVisible(bool val);
      void updateClefs();
      };

//---------------------------------------------------------
//   StaffListItem
//---------------------------------------------------------

class StaffListItem : public QObject, public QTreeWidgetItem {
      Q_OBJECT
      int _partIdx;
      bool _linked;
      ClefTypeList _defaultClefType;    // clef for "normal" stafftype
      ClefTypeList _clefType;           // actual clef
      QComboBox* _staffTypeCombo { nullptr };
      Staff* _staff              { 0       };
      ListItemOp _op             { ListItemOp::KEEP };
      static int customStandardIdx;
      static int customPercussionIdx;
      static int customTablatureIdx;
      static constexpr int CUSTOM_STAFF_TYPE_IDX = -1000;

      int staffTypeIdx(int idx) const;

   private slots:
      void staffTypeChanged(int);

   public:
      StaffListItem();
      StaffListItem(PartListItem* li);

      int partIdx() const                       { return _partIdx; }
      void setPartIdx(int val);

      void setClefType(const ClefTypeList& val);
      const ClefTypeList& clefType() const              { return _clefType;    }

      void setDefaultClefType(const ClefTypeList& val)  { _defaultClefType = val; }
      const ClefTypeList& defaultClefType() const       { return _defaultClefType; }

      void setLinked(bool val);
      bool linked() const                       { return _linked;  }
      void setStaffType(const StaffType*);
      void setStaffType(int);
      const StaffType* staffType() const;
      int staffTypeIdx() const;
      void initStaffTypeCombo(bool forceRecreate = false);

      QComboBox* staffTypeCombo() { return _staffTypeCombo; }

      Staff* staff() const        { return _staff; }
      void setStaff(Staff* s)     { _staff = s; }
      ListItemOp op() const       { return _op; }
      void setOp(ListItemOp v)    { _op = v; }
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


//---------------------------------------------------------
//   InstrumentsWidget
//---------------------------------------------------------

class InstrumentsWidget : public QWidget, public Ui::InstrumentsWidget {
      Q_OBJECT

   private slots:
      void on_instrumentList_itemSelectionChanged();
      void on_instrumentList_itemDoubleClicked(QTreeWidgetItem* item, int);
      void on_partiturList_itemSelectionChanged();
      void on_addButton_clicked();
      void on_removeButton_clicked();
      void on_upButton_clicked();
      void on_downButton_clicked();
      StaffListItem* on_addStaffButton_clicked();
      void on_addLinkedStaffButton_clicked();

      void on_instrumentSearch_textChanged(const QString &);

      void on_instrumentGenreFilter_currentIndexChanged(int);
      void filterInstrumentsByGenre(QTreeWidget *, QString);

   public slots:
      void buildTemplateList();

   signals:
      void completeChanged(bool);

   public:
      InstrumentsWidget(QWidget* parent = 0);
      void genPartList(Score*);
      void writeSettings();
      void init();
      void createInstruments(Score*);
      QTreeWidget* getPartiturList();
      };

extern void populateInstrumentList(QTreeWidget* instrumentList);
extern void populateGenreCombo(QComboBox* combo);

} // namespace Ms

extern QList<Ms::InstrumentGenre *> instrumentGenres;

#endif

