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
#include "icons.h"

namespace Ms {

static const char* toolbars[] = {
      QT_TRANSLATE_NOOP("toolbar", "Note Input"),
      QT_TRANSLATE_NOOP("toolbar", "File Operations"),
      QT_TRANSLATE_NOOP("toolbar", "Playback Controls")
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
   : AbstractDialog(parent)
      {
      setObjectName("ToolbarEditor");
      setupUi(this);

      for (auto i : toolbars)
            toolbarList->addItem(qApp->translate("toolbar", i));
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
      
      up->setIcon(*icons[int(Icons::arrowUp_ICON)]);
      down->setIcon(*icons[int(Icons::arrowDown_ICON)]);
      add->setIcon(*icons[int(Icons::goPrevious_ICON)]);
      remove->setIcon(*icons[int(Icons::goNext_ICON)]);

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void ToolbarEditor::init()
      {
      for (int i = 0; i < toolbarList->count(); i++)
            toolbarList->item(i)->setText(qApp->translate("toolbar", toolbars[i]));

      QString name = WorkspacesManager::currentWorkspace()->name();
      bool writable = !WorkspacesManager::currentWorkspace()->readOnly();
      if (!writable) {
            name += " " + tr("(not changeable)");
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
//   retranslate
//---------------------------------------------------------

void ToolbarEditor::retranslate()
      {
      retranslateUi(this);
      init();
      }

//---------------------------------------------------------
//   accepted
//---------------------------------------------------------

void ToolbarEditor::accepted()
      {
      if (WorkspacesManager::currentWorkspace()->readOnly())
            return;
      // Updates the toolbars
      mscore->setNoteInputMenuEntries(*(new_toolbars->at(0)));
      mscore->setFileOperationEntries(*(new_toolbars->at(1)));
      mscore->setPlaybackControlEntries(*(new_toolbars->at(2)));
      mscore->populateNoteInputMenu();
      mscore->populateFileOperations();
      mscore->populatePlaybackControls();
      WorkspacesManager::currentWorkspace()->setDirty(true);
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
            QListWidgetItem* item = nullptr;
            QString id = QString(i);
            if (a)
                  // Remove '&', because text contains '&' signs for mnemonics,
                  // but in a QListWidgetItem, you get the '&' instead of the mnemonic.
                  item = new QListWidgetItem(a->icon(), a->text().remove('&'));
            else if (id.isEmpty())
                  item = new QListWidgetItem(tr("Separator"));
            else
                  item = new QListWidgetItem(id);
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
                  QListWidgetItem* item = nullptr;
                  QString id = QString(i);
                  if (a)
                        item = new QListWidgetItem(a->icon(), a->text().remove('&'));
                  else if (!id.isEmpty())
                        item = new QListWidgetItem(id);
                  if (item) {
                        item->setData(Qt::UserRole, QVariant::fromValue((void*)i));
                        availableList->addItem(item);
                        }
                  }
            }
      QListWidgetItem* item = new QListWidgetItem(tr("Separator"));
      item->setData(Qt::UserRole, QVariant::fromValue((void*)""));
      availableList->addItem(item);
      }

//---------------------------------------------------------
//   isSeparator
//---------------------------------------------------------

bool ToolbarEditor::isSeparator(QListWidgetItem* item) const
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

      if (isSeparator(item)) {
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
      if (!isSeparator(item))
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

