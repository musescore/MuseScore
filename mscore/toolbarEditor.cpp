//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "toolbarEditor.h"
#include "musescore.h"
#include "workspace.h"

namespace Ms {

static const char* toolbars[] = {
      "Note Input",
      "File Operations",
      "Playback Controls"
      };

//---------------------------------------------------------
//   showToolbarEditor
//---------------------------------------------------------

void MuseScore::showToolbarEditor()
      {
      if (!editToolbars) {
            editToolbars = new ToolbarEditor(this);
            }
      editToolbars->init();
      editToolbars->show();
      }

//---------------------------------------------------------
//   ToolbarEditor
//---------------------------------------------------------

ToolbarEditor::ToolbarEditor(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("ToolbarEditor");
      setupUi(this);

      for (auto i : toolbars)
            toolbarList->addItem(QString(i));
      toolbarList->setCurrentRow(0);

      new_toolbars = new std::vector<std::list<const char*>*>();
      new_toolbars->push_back(mscore->noteInputMenuEntries());
      new_toolbars->push_back(mscore->fileOperationEntries());
      new_toolbars->push_back(mscore->playbackControlEntries());

      connect(toolbarList, SIGNAL(currentRowChanged(int)), SLOT(toolbarChanged(int)));
      connect(add, SIGNAL(clicked()), SLOT(addAction()));
      connect(remove, SIGNAL(clicked()), SLOT(removeAction()));
      connect(up, SIGNAL(clicked()), SLOT(upAction()));
      connect(down, SIGNAL(clicked()), SLOT(downAction()));
      connect(buttonBox, SIGNAL(accepted()), SLOT(accepted()));

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void ToolbarEditor::init()
      {
      QString name = Workspace::currentWorkspace->name();
      bool writable = !Workspace::currentWorkspace->readOnly();
      if (!writable) {
            name += tr(" (not changeable)");
            }
      add->setEnabled(writable);
      remove->setEnabled(writable);
      up->setEnabled(writable);
      down->setEnabled(writable);
      workspaceName->setText(name);

      // Syncs the editor with the current toolbars
      new_toolbars->at(0) = mscore->noteInputMenuEntries();
      new_toolbars->at(1) = mscore->fileOperationEntries();
      new_toolbars->at(2) = mscore->playbackControlEntries();
      toolbarChanged(toolbarList->currentRow());  // populate lists
      }

//---------------------------------------------------------
//   accepted
//---------------------------------------------------------

void ToolbarEditor::accepted()
      {
      if (Workspace::currentWorkspace->readOnly())
            return;
      // Updates the toolbars
      mscore->setNoteInputMenuEntries(*(new_toolbars->at(0)));
      mscore->setFileOperationEntries(*(new_toolbars->at(1)));
      mscore->setPlaybackControlEntries(*(new_toolbars->at(2)));
      mscore->populateNoteInputMenu();
      mscore->populateFileOperations();
      mscore->populatePlaybackControls();
      Workspace::currentWorkspace->setDirty(true);
      }

//---------------------------------------------------------
//   populateLists
//---------------------------------------------------------

void ToolbarEditor::populateLists(const std::list<const char*>& all, std::list<const char*>* current)
      {
      actionList->clear();
      availableList->clear();
      for (auto i : *current) {
            QAction* a = getAction(i);
            QListWidgetItem* item;
            QString actionName = QString(i);
            if (a)
                  item = new QListWidgetItem(a->icon(), actionName);
            else if (actionName.isEmpty())
                  item = new QListWidgetItem(tr("spacer"));
            else
                  item = new QListWidgetItem(actionName);
            item->setData(Qt::UserRole, QVariant::fromValue((void*)i));
            actionList->addItem(item);
            }
      for (auto i : all) {
            bool found = false;
            for (auto k : *current) {
                  if (strcmp(k, i) == 0) {
                        found = true;
                        break;
                        }
                  }
            if (!found) {
                  QAction* a = getAction(i);
                  QListWidgetItem* item = 0;
                  QString actionName = QString(i);
                  if (a)
                        item = new QListWidgetItem(a->icon(), actionName);
                  else if (!actionName.isEmpty())
                        item = new QListWidgetItem(QString(i));
				  if (item) {
                        item->setData(Qt::UserRole, QVariant::fromValue((void*)i));
                        availableList->addItem(item);
                        }
                  }
            }
      QListWidgetItem* item = new QListWidgetItem(tr("spacer"));
      item->setData(Qt::UserRole, QVariant::fromValue((void*)""));
      availableList->addItem(item);
      }

//---------------------------------------------------------
//   isSpacer
//---------------------------------------------------------

bool ToolbarEditor::isSpacer(QListWidgetItem* item) const
      {
      return !*(const char*)(item->data(Qt::UserRole).value<void*>());
      }

//---------------------------------------------------------
//   addAction
//---------------------------------------------------------

void ToolbarEditor::addAction()
      {
      int cr = availableList->currentRow();
      if (cr == -1)
            return;
      QListWidgetItem* item = availableList->item(cr);

      if (isSpacer(item)) {
            QListWidgetItem* nitem = new QListWidgetItem(item->text());
            nitem->setData(Qt::UserRole, QVariant::fromValue((void*)""));
            item = nitem;
            }
      else
            item = availableList->takeItem(cr);
      cr = actionList->currentRow();
      if (cr == -1)
            actionList->addItem(item);
      else
            actionList->insertItem(cr, item);
      updateNewToolbar(toolbarList->currentRow());
      }

//---------------------------------------------------------
//   removeAction
//---------------------------------------------------------

void ToolbarEditor::removeAction()
      {
      int cr = actionList->currentRow();
      if (cr == -1)
            return;
      QListWidgetItem* item = actionList->takeItem(cr);
      if (!isSpacer(item))
            availableList->addItem(item);
      updateNewToolbar(toolbarList->currentRow());
      }

//---------------------------------------------------------
//   upAction
//---------------------------------------------------------

void ToolbarEditor::upAction()
      {
      int cr = actionList->currentRow();
      if (cr <= 0)
            return;
      QListWidgetItem* item = actionList->takeItem(cr);
      actionList->insertItem(cr - 1, item);
      actionList->setCurrentRow(cr - 1);
      updateNewToolbar(toolbarList->currentRow());
      }

//---------------------------------------------------------
//   downAction
//---------------------------------------------------------

void ToolbarEditor::downAction()
      {
      int cr = actionList->currentRow();
      if (cr == -1 || cr == actionList->count()-1)
            return;
      QListWidgetItem* item = actionList->takeItem(cr);
      actionList->insertItem(cr + 1, item);
      actionList->setCurrentRow(cr + 1);
      updateNewToolbar(toolbarList->currentRow());
      }

//---------------------------------------------------------
//   toolbarChanged
//---------------------------------------------------------

void ToolbarEditor::toolbarChanged(int tb)
      {
      switch (tb) {
            case 0:     // NoteInput
                  populateLists(MuseScore::allNoteInputMenuEntries(), new_toolbars->at(tb));
                  break;
            case 1:     //FileOperations
                  populateLists(MuseScore::allFileOperationEntries(), new_toolbars->at(tb));
                  break;
            case 2:     //PlaybackControls
                  populateLists(MuseScore::allPlaybackControlEntries(), new_toolbars->at(tb));
            }
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void ToolbarEditor::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

//---------------------------------------------------------
//   updateNewToolbar
//---------------------------------------------------------

void ToolbarEditor::updateNewToolbar(int toolbar_to_update) {
      // Updates the current toolbar to the actionList
      std::list<const char*> *toolbar_action_list = new std::list<const char*>();
      for (int i = 0; i < actionList->count(); ++i) {
            QListWidgetItem* a = actionList->item(i);
            toolbar_action_list->push_back((const char*)(a->data(Qt::UserRole).value<void*>()));
            }
      new_toolbars->at(toolbar_to_update) = toolbar_action_list;
      }

} // namespace Ms

