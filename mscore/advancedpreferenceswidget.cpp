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

#include "advancedpreferenceswidget.h"
#include "musescore.h"

namespace Ms {

AdvancedPreferencesWidget::AdvancedPreferencesWidget(QWidget* parent) :
      QWidget(parent),
      ui(new Ui::AdvancedPreferencesWidget)
      {
      setObjectName("AdvancedPreferencesWidget");
      ui->setupUi(this);

      connect(ui->resetToDefaultButton, &QPushButton::clicked, ui->treePreferencesWidget, &PreferencesListWidget::resetSelectedPreferencesToDefault);
      connect(ui->treePreferencesWidget, &QTreeWidget::itemSelectionChanged, this, &AdvancedPreferencesWidget::enableResetPreferenceToDefault);
      connect(ui->searchLineEdit,  &QLineEdit::textChanged, ui->treePreferencesWidget, &PreferencesListWidget::filter);
      }

AdvancedPreferencesWidget::~AdvancedPreferencesWidget()
      {
      delete ui;
      }

void AdvancedPreferencesWidget::updatePreferences() const
      {
      ui->treePreferencesWidget->updatePreferences();
      }

void AdvancedPreferencesWidget::enableResetPreferenceToDefault()
      {
      if (!ui->treePreferencesWidget->selectedItems().count()) {
            setEnabled(false);
            return;
            }

      // if at least one of the selected items is a PreferenceItem, enable resetToDefaultButton.
      for (QTreeWidgetItem* item: ui->treePreferencesWidget->selectedItems()) {
            // it would be faster, but less safe to use (item->childCount()
            // to determine if the item is a PreferenceItem.
            PreferenceItem* pref = dynamic_cast<PreferenceItem*>(item);
            if (pref) {
                  ui->resetToDefaultButton->setEnabled(true);
                  return;
                  }
            }
      // if it gets here, it means none of the selected items were PreferenceItems, so disable the button.
      ui->resetToDefaultButton->setEnabled(false);
      }

} // namespace Ms

