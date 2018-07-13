//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#ifndef ADVANCEDPREFERENCESWIDGET_H
#define ADVANCEDPREFERENCESWIDGET_H

#include "ui_advancedpreferenceswidget.h"

namespace Ms {

class AdvancedPreferencesWidget : public QWidget
{
      Q_OBJECT

   public:
      explicit AdvancedPreferencesWidget(QWidget* parent = nullptr);
      ~AdvancedPreferencesWidget();

      inline void save() { ui->treePreferencesWidget->save(); }
      inline void updatePreferences() const;

   private:
      Ui::AdvancedPreferencesWidget* ui;

   private slots:
      void enableResetPreferenceToDefault();
};

} // namespace Ms

#endif // ADVANCEDPREFERENCESWIDGET_H
