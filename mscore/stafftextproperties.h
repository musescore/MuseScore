//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#ifndef __STAFFTEXTPROPERTIES_H__
#define __STAFFTEXTPROPERTIES_H__

#include "ui_stafftext.h"

namespace Ms {

class StaffTextBase;

//---------------------------------------------------------
//   StaffTextProperties
//    Dialog
//---------------------------------------------------------

class StaffTextProperties : public QDialog, public Ui::StaffTextProperties {
      Q_OBJECT

      StaffTextBase* _staffText;
      QToolButton* vb[4][4];
      QComboBox* channelCombo[4];
      QPushButton* stops[4][16];
      int curTabIndex;
      void saveChannel(int channel);

      virtual void hideEvent(QHideEvent*);
   private slots:
      void saveValues();
      void channelItemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
      void voiceButtonClicked(int);
      void tabChanged(int tab);
      void setSwingControls(bool);

   public:
      StaffTextProperties(const StaffTextBase*, QWidget* parent = 0);
      ~StaffTextProperties();

      const StaffTextBase* staffTextBase() const { return _staffText; }
      };
}

#endif
