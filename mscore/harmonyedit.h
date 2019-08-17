//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2009-2011 Werner Schweer and others
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

#ifndef __HARMONYEDIT_H__
#define __HARMONYEDIT_H__

#include "ui_harmonyedit.h"

namespace Ms {

class Palette;
class ChordList;

//---------------------------------------------------------
//    ChordStyleEditor
//---------------------------------------------------------

class ChordStyleEditor : public QDialog, Ui::ChordStyleEditor {
      Q_OBJECT

      bool _dirty;
      Palette* sp1;
      Score* score;
      ChordList* chordList;

      void loadChordDescriptionFile(const QString&);
      void setChordList(ChordList* cl);

   private slots:
      void fileButtonClicked();
      void saveButtonClicked();
      void harmonyChanged(QTreeWidgetItem*, QTreeWidgetItem*);

   public slots:
      virtual void accept();

   public:
      ChordStyleEditor(QWidget* parent = 0);
      void setScore(Score*);
      bool dirty() const { return _dirty; }
      void save();
      void restore();
      };


} // namespace Ms
#endif

