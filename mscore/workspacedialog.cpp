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
#include "palettebox.h"

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
      if (!Workspace::currentWorkspace && !Workspace::currentWorkspace->readOnly())
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
      mscore->getPaletteBox()->searchBox()->clear();
      if (editMode) {
            componentsCheck->setChecked(Workspace::currentWorkspace->getSaveComponents());
            toolbarsCheck->setChecked(Workspace::currentWorkspace->getSaveToolbars());
            menubarCheck->setChecked(Workspace::currentWorkspace->getSaveMenuBar());
            prefsCheck->setChecked(preferences.getUseLocalPreferences());
            nameLineEdit->setText(Workspace::currentWorkspace->name());
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
            if (editMode && s == Workspace::currentWorkspace->name())
                  break;
            bool notFound = true;
            for (Workspace* p : Workspace::workspaces()) {
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

      if (!editMode) {
            if (Workspace::currentWorkspace->dirty())
                  Workspace::currentWorkspace->save();
            Workspace::currentWorkspace = Workspace::createNewWorkspace(s);
            preferences.updateLocalPreferences();
            }

      Workspace::currentWorkspace->setSaveComponents(componentsCheck->isChecked());
      Workspace::currentWorkspace->setSaveToolbars(toolbarsCheck->isChecked());
      Workspace::currentWorkspace->setSaveMenuBar(menubarCheck->isChecked());
      preferences.setUseLocalPreferences(prefsCheck->isChecked());
      Workspace::currentWorkspace->save();

      if (editMode && Workspace::currentWorkspace->name() != s)
            Workspace::currentWorkspace->rename(s);

      preferences.setPreference(PREF_APP_WORKSPACE, Workspace::currentWorkspace->name());
      PaletteBox* paletteBox = mscore->getPaletteBox();
      paletteBox->updateWorkspaces();
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
