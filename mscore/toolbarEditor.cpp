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
      "Note Input"
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
      setupUi(this);

      for (auto i : toolbars)
            toolbarList->addItem(QString(i));
      toolbarList->setCurrentRow(0);

      connect(toolbarList, SIGNAL(currentRowChanged(int)), SLOT(toolbarChanged(int)));
      connect(add, SIGNAL(clicked()), SLOT(addAction()));
      connect(remove, SIGNAL(clicked()), SLOT(removeAction()));
      connect(up, SIGNAL(clicked()), SLOT(upAction()));
      connect(down, SIGNAL(clicked()), SLOT(downAction()));
      connect(buttonBox, SIGNAL(accepted()), SLOT(accepted()));
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
      workspace->setText(name);
      toolbarChanged(0);  // populate lists
      }

//---------------------------------------------------------
//   accepted
//---------------------------------------------------------

void ToolbarEditor::accepted()
      {
      if (Workspace::currentWorkspace->readOnly())
            return;
      std::list<const char*> l;
      for (int i = 0; i < actionList->count(); ++i) {
            QListWidgetItem* a = actionList->item(i);
            l.push_back((const char*)(a->data(Qt::UserRole).value<void*>()));
            }
      mscore->setNoteInputMenuEntries(l);
      mscore->populateNoteInputMenu();
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
            if (a)
                  item = new QListWidgetItem(a->icon(), QString(i));
            else
                  item = new QListWidgetItem(QString(i));
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
                  QListWidgetItem* item;
                  if (a)
                        item = new QListWidgetItem(a->icon(), QString(i));
                  else
                        item = new QListWidgetItem(QString(i));
                  item->setData(Qt::UserRole, QVariant::fromValue((void*)i));
                  availableList->addItem(item);
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
      }

//---------------------------------------------------------
//   toolbarChanged
//---------------------------------------------------------

void ToolbarEditor::toolbarChanged(int tb)
      {
      switch (tb) {
            case 0:     // NoteInput
                  populateLists(MuseScore::allNoteInputMenuEntries(), mscore->noteInputMenuEntries());
                  break;
            }
      }


} // namespace Ms

