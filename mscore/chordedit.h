//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: chordedit.h 4388 2011-06-18 13:17:58Z wschweer $
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __CHORDEDIT_H__
#define __CHORDEDIT_H__

#include "libmscore/harmony.h"
#include "ui_chordedit.h"

class DegreeTabDelegate;

//---------------------------------------------------------
//   class ChordEdit
//---------------------------------------------------------

class ChordEdit : public QDialog, Ui::ChordEdit {
      Q_OBJECT

      Score* score;
      Harmony* _harmony;

      QButtonGroup* rootGroup;
      QButtonGroup* accidentalsGroup;
      QButtonGroup* extensionGroup;
      QStandardItemModel* model;
      DegreeTabDelegate* delegate;
      void updateDegrees();
      bool isValidDegree(int r);

      int numberOfDegrees();
      HDegree degree(int i);
      void addDegree(HDegree d);

      void setRoot(int val);
      void setExtension(int val);
      void setBase(int val);

      int base();
      int root();
      const ChordDescription* extension();

   private slots:
      void otherToggled(bool);
      void chordChanged();
      void addButtonClicked();
      void deleteButtonClicked();
      void modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

   public:
      ChordEdit(Score* s, QWidget* parent = 0);
      ~ChordEdit();

      void setHarmony(const Harmony* h);
      const Harmony* harmony() const { return _harmony; }
      };

//---------------------------------------------------------
//   class DegreeTabDelegate
//---------------------------------------------------------

class DegreeTabDelegate : public QItemDelegate
      {
      Q_OBJECT

   public:
      DegreeTabDelegate(QObject *parent = 0);

      QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
         const QModelIndex& index) const;

      void setEditorData(QWidget* editor, const QModelIndex& index) const;
      void setModelData(QWidget* editor, QAbstractItemModel* model,
         const QModelIndex& index) const;

      void updateEditorGeometry(QWidget* editor,
         const QStyleOptionViewItem& option, const QModelIndex& index) const;
      };

#endif

