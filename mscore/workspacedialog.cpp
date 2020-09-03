//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "workspacedialog.h"
#include "workspace.h"
#include "preferences.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   createNewWorkspace
//---------------------------------------------------------

void MuseScore::createNewWorkspace()
      {
      if (!_workspaceDialog)
            _workspaceDialog = new WorkspaceDialog();

      _workspaceDialog->editMode = false;
      _workspaceDialog->display();
      updateIcons();
      }

//---------------------------------------------------------
//   editWorkspace
//---------------------------------------------------------

void MuseScore::editWorkspace()
      {
      if (!WorkspacesManager::currentWorkspace())
            return;

      if (WorkspacesManager::currentWorkspace()->readOnly())
            return;

      if (!_workspaceDialog)
            _workspaceDialog = new WorkspaceDialog();

      _workspaceDialog->editMode = true;
      _workspaceDialog->display();
      updateIcons();
      }

//---------------------------------------------------------
//   WorkspaceDialog
//---------------------------------------------------------

WorkspaceDialog::WorkspaceDialog(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("WorkspaceDialog");
      setupUi(this);
      retranslateUi(this);
      setModal(true);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      MuseScore::restoreGeometry(this);

      connect(buttonBox, SIGNAL(accepted()), SLOT(accepted()));
      connect(buttonBox, SIGNAL(rejected()), SLOT(close()));
      }

//---------------------------------------------------------
//   display
//---------------------------------------------------------

void WorkspaceDialog::display()
      {
      // TODO: clear search box?
      if (editMode) {
            componentsCheck->setChecked(WorkspacesManager::currentWorkspace()->getSaveComponents());
            toolbarsCheck->setChecked(WorkspacesManager::currentWorkspace()->getSaveToolbars());
            menubarCheck->setChecked(WorkspacesManager::currentWorkspace()->getSaveMenuBar());
            prefsCheck->setChecked(preferences.getUseLocalPreferences());
            nameLineEdit->setText(WorkspacesManager::currentWorkspace()->name());
            setWindowTitle(tr("Edit Workspace"));
            }
      else {
            componentsCheck->setChecked(false);
            toolbarsCheck->setChecked(false);
            menubarCheck->setChecked(false);
            prefsCheck->setChecked(false);
            nameLineEdit->setText("");
            setWindowTitle(tr("Create New Workspace"));
            }
      show();
      }

//---------------------------------------------------------
//   accepted
//---------------------------------------------------------

void WorkspaceDialog::accepted()
      {
      QString s = nameLineEdit->text();
      if (s.isEmpty())
            return;
      s = s.replace( QRegExp( "[" + QRegExp::escape( "\\/:*?\"<>|" ) + "]" ), "_" ); //FAT/NTFS special chars

      for (;;) {
            if (editMode && s == WorkspacesManager::currentWorkspace()->name())
                  break;
            bool notFound = true;
            for (Workspace* p : WorkspacesManager::workspaces()) {
                  if ((qApp->translate("Ms::Workspace", p->name().toUtf8()).toLower() == s.toLower())) {
                        notFound = false;
                        break;
                        }
                  }
            if (!notFound) {
                  s = QInputDialog::getText(this,
                     tr("Read Workspace Name"),
                     tr("'%1' does already exist,\nplease choose a different name:").arg(s)
                     );
                  if (s.isEmpty())
                        return;
                  s = s.replace( QRegExp( "[" + QRegExp::escape( "\\/:*?\"<>|" ) + "]" ), "_" ); //FAT/NTFS special chars
                  }
            else
                  break;
            }

      Workspace* newWorkspace = editMode ? WorkspacesManager::currentWorkspace() : WorkspacesManager::createNewWorkspace(s);
      if (!editMode) {
            //save current workspace
            if (WorkspacesManager::currentWorkspace()->dirty())
                  WorkspacesManager::currentWorkspace()->save();
            }

      //update workspace properties with the dialog values
      newWorkspace->setSaveComponents(componentsCheck->isChecked());
      newWorkspace->setSaveToolbars(toolbarsCheck->isChecked());
      newWorkspace->setSaveMenuBar(menubarCheck->isChecked());
      preferences.setUseLocalPreferences(prefsCheck->isChecked());
      
      //save newly created/edited workspace
      newWorkspace->save();

      //rename if we edit name of the existing workspace
      if (editMode && newWorkspace->name() != s)
            newWorkspace->rename(s);

      if (!editMode) {
            mscore->changeWorkspace(newWorkspace);
            preferences.updateLocalPreferences();
            }
            
      preferences.setPreference(PREF_APP_WORKSPACE, WorkspacesManager::currentWorkspace()->translatableName());
      emit mscore->workspacesChanged();
      close();
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void WorkspaceDialog::changeEvent(QEvent *event)
      {
      QWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }
}
