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
#include "shortcut.h"
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
    : QDialog(parent)
{
    setObjectName("ToolbarEditor");
    setupUi(this);

    for (auto i : toolbars) {
        toolbarList->addItem(qApp->translate("toolbar", i));
    }
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
    add->setIcon(*icons[int(Icons::goNext_ICON)]);
    remove->setIcon(*icons[int(Icons::goPrevious_ICON)]);

    MuseScore::restoreGeometry(this);
}

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void ToolbarEditor::init()
{
    Workspace* currentWorkspace = WorkspacesManager::currentWorkspace();
    QString name = currentWorkspace->name();
    QString mainTitle = tr("Customize Toolbars");
    setWindowTitle(mainTitle + " (" + name + ")");

    // defensive - don't expect to be here if workspace is not customizable
    bool canBeCustomized = currentWorkspace->canCustomizeToolbars();

    add->setEnabled(canBeCustomized);
    remove->setEnabled(canBeCustomized);
    up->setEnabled(canBeCustomized);
    down->setEnabled(canBeCustomized);

    // Syncs the editor with the current toolbars
    new_toolbars->at(0) = mscore->noteInputMenuEntries();
    new_toolbars->at(1) = mscore->fileOperationEntries();
    new_toolbars->at(2) = mscore->playbackControlEntries();
    toolbarChanged(toolbarList->currentRow());    // populate lists
}

//---------------------------------------------------------
//   accepted
//---------------------------------------------------------

void ToolbarEditor::accepted()
{
    if (!WorkspacesManager::currentWorkspace()->canCustomizeToolbars()) {
        return;
    }
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
//   listItem
//---------------------------------------------------------
QListWidgetItem* ToolbarEditor::listItem(const char* id)
{
    QListWidgetItem* item = _listItem(id);

    if (!item) {
        return nullptr;
    }

    // add a sensible height for cases where there is no icon
    // to avoid row looking squashed
    item->setSizeHint(QSize(item->sizeHint().width(), 26));

    item->setData(Qt::UserRole, QVariant::fromValue((void*)id));
    return item;
}

QListWidgetItem* ToolbarEditor::_listItem(const char* id)
{
    if (QString(id).isEmpty()) {
        return new QListWidgetItem(tr("Separator"));
    }

    if (!strcmp("view-mode", id)) {
        return new QListWidgetItem(tr("View mode"));
    }

    if (!strcmp("zoom-options", id)) {
        return new QListWidgetItem(tr("Zoom options"));
    }

    QAction* action = getAction(id);

    if (action) {
        QString itemName = Shortcut::getShortcut(id)->text();

        // in this case we can't use the menu name (it's too short
        // as it's used for the button label). So use the shortcut
        // description instead
        if (strncmp(id, "voice-", 6) == 0) {
            itemName = Shortcut::getShortcut(id)->descr();
        }

        return new QListWidgetItem(action->icon(), itemName);
    }

    qDebug() << "ToolbarEditor does not recognize id for toolbar item: " << QString(id);
    return nullptr;
}

//---------------------------------------------------------
//   populateLists
//---------------------------------------------------------

void ToolbarEditor::populateLists(const std::list<const char*>& all, std::list<const char*>* current)
{
    actionList->clear();
    for (auto currentId : *current) {
        QListWidgetItem* item = listItem(currentId);
        if (!item) {
            continue;
        }

        actionList->addItem(item);
    }

    refreshAvailableList(all, current);
}

void ToolbarEditor::refreshAvailableList(const std::list<const char*>& all, std::list<const char*>* current)
{
    availableList->clear();

    for (auto id : all) {
        bool found = false;

        for (auto allId : *current) {
            if (strcmp(allId, id) == 0) {
                found = true;
                break;
            }
        }

        if (found) {
            continue;
        }

        if (QString(id).isEmpty()) {
            continue;
        }

        QListWidgetItem* item = listItem(id);
        if (!item) {
            continue;
        }

        availableList->addItem(item);
    }

    // add the Separator item
    availableList->addItem(listItem(""));
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
    int currentRow = availableList->currentRow();

    if (currentRow == -1) {
        return;
    }

    QListWidgetItem* item = availableList->item(currentRow);

    if (isSpacer(item)) {
        item = listItem("");
    } else {
        item = availableList->takeItem(currentRow);
    }

    currentRow = actionList->currentRow();

    if (currentRow == -1) {
        actionList->addItem(item);
    } else {
        actionList->insertItem(currentRow, item);
    }

    updateNewToolbar(toolbarList->currentRow());
}

//---------------------------------------------------------
//   removeAction
//---------------------------------------------------------

void ToolbarEditor::removeAction()
{
    int actionListRow = actionList->currentRow();

    if (actionListRow == -1) {
        return;
    }

    QListWidgetItem* item = actionList->takeItem(actionListRow);
    updateNewToolbar(toolbarList->currentRow());

    if (isSpacer(item)) {
        return;
    }

    int toolbarListRow = toolbarList->currentRow();
    switch (toolbarListRow) {
    case 0:
        refreshAvailableList(MuseScore::allNoteInputMenuEntries(), new_toolbars->at(toolbarListRow));
        break;
    case 1:
        refreshAvailableList(MuseScore::allFileOperationEntries(), new_toolbars->at(toolbarListRow));
        break;
    case 2:
        refreshAvailableList(MuseScore::allPlaybackControlEntries(), new_toolbars->at(toolbarListRow));
    }
}

//---------------------------------------------------------
//   upAction
//---------------------------------------------------------

void ToolbarEditor::upAction()
{
    int cr = actionList->currentRow();
    if (cr <= 0) {
        return;
    }
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
    if (cr == -1 || cr == actionList->count() - 1) {
        return;
    }
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
    case 0:             // NoteInput
        populateLists(MuseScore::allNoteInputMenuEntries(), new_toolbars->at(tb));
        break;
    case 1:             //FileOperations
        populateLists(MuseScore::allFileOperationEntries(), new_toolbars->at(tb));
        break;
    case 2:             //PlaybackControls
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

void ToolbarEditor::updateNewToolbar(int toolbar_to_update)
{
    // Updates the current toolbar to the actionList
    std::list<const char*>* toolbar_action_list = new std::list<const char*>();
    for (int i = 0; i < actionList->count(); ++i) {
        QListWidgetItem* a = actionList->item(i);
        toolbar_action_list->push_back((const char*)(a->data(Qt::UserRole).value<void*>()));
    }
    new_toolbars->at(toolbar_to_update) = toolbar_action_list;
}
} // namespace Ms
