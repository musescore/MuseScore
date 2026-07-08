/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "undohistorymodel.h"
#include "engraving/dom/masterscore.h"
#include "notation/imasternotation.h"

#include "notation/inotation.h"
#include "notation/inotationinteraction.h" // IWYU pragma: keep
#include "notation/inotationundostack.h"

using namespace mu::notation;
using namespace muse;

UndoHistoryModel::UndoHistoryModel(QObject* parent)
    : QAbstractListModel(parent), Contextable(iocCtxForQmlObject(this))
{
}

void UndoHistoryModel::classBegin()
{
    init();
}

void UndoHistoryModel::init()
{
    onCurrentNotationChanged();

    context()->currentNotationChanged().onNotify(this, [this] {
        onCurrentNotationChanged();
    });

    emit snapshotsChanged();
}

void UndoHistoryModel::onCurrentNotationChanged()
{
    auto stack = undoStack();

    if (stack) {
        stack->stackChanged().onNotify(this, [this] {
            onUndoRedo();
        }, Asyncable::Mode::SetReplace /* FIXME */);
    }

    beginResetModel();
    m_rowCount = stack ? int(stack->undoRedoActionCount()) + 1 : 0;
    endResetModel();

    emit currentIndexChanged();
    addFileOpenedSnapshot();
}

void UndoHistoryModel::onUndoRedo()
{
    auto stack = undoStack();

    int newRowCount = stack ? int(stack->undoRedoActionCount()) + 1 : 0;

    if (m_rowCount < newRowCount) {
        beginInsertRows(QModelIndex(), m_rowCount, newRowCount - 1);
        m_rowCount = newRowCount;
        endInsertRows();
    } else if (m_rowCount > newRowCount) {
        beginRemoveRows(QModelIndex(), newRowCount, m_rowCount - 1);
        m_rowCount = newRowCount;
        endRemoveRows();
    }

    // When performing a new action after undoing one or more actions, the
    // redo stack is cleared, and the new action is pushed onto the stack;
    // that means that the item at the current index now represents the new
    // action, rather than the action on the redo stack.
    int newCurrentIndex = stack ? int(stack->currentStateIndex()) : 0;
    emit dataChanged(index(newCurrentIndex), index(newCurrentIndex));

    emit currentIndexChanged();
}

QVariant UndoHistoryModel::data(const QModelIndex& index, int role) const
{
    auto stack = undoStack();
    int row = index.row();
    if (!stack || row < 0 || row >= int(stack->undoRedoActionCount()) + 1) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        if (row == 0) {
            if (!m_lastRestoredSnapshotName.isEmpty()) {
                return qtrc("notation/undohistory", "Load version: %1").arg(std::move(m_lastRestoredSnapshotName));
            }
            return qtrc("notation/undohistory", "File opened");
        }
        return stack->lastActionNameAtIdx(static_cast<size_t>(row)).qTranslated();
    default:
        return {};
    }
}

int UndoHistoryModel::rowCount(const QModelIndex&) const
{
    return m_rowCount;
}

QHash<int, QByteArray> UndoHistoryModel::roleNames() const
{
    return { { Qt::DisplayRole, "text" } };
}

int UndoHistoryModel::currentIndex() const
{
    if (auto stack = undoStack()) {
        return int(stack->currentStateIndex());
    }

    return 0;
}

void UndoHistoryModel::undoRedoToIndex(int index)
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    return notation->interaction()->undoRedoToIndex(static_cast<size_t>(index));
}

INotationUndoStackPtr UndoHistoryModel::undoStack() const
{
    INotationPtr notation = context()->currentNotation();
    return notation ? notation->undoStack() : nullptr;
}

void UndoHistoryModel::addSnapshot(const QString& name)
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }
    IMasterNotationPtr masterNotation = notation->masterNotation();
    if (!masterNotation) {
        return;
    }
    MasterScore* masterScore = masterNotation->masterScore();
    if (!masterScore) {
        return;
    }

    mu::engraving::String snapName(name);
    masterScore->addSnapshot(snapName);
    emit snapshotsChanged();
}

void UndoHistoryModel::updateSnapshot(int index)
{
    MasterScore* masterScore = context()->currentNotation()->masterNotation()->masterScore();
    masterScore->updateSnapshot(index);
    emit snapshotsChanged();
}

void UndoHistoryModel::removeSnapshot(int index)
{
    MasterScore* masterScore = context()->currentNotation()->masterNotation()->masterScore();
    if (!masterScore) {
        return;
    }

    if (index < 0 || index >= int(masterScore->snapshots().size())) {
        LOGW() << "Invalid snapshot index: " << index;
        return;
    }

    masterScore->removeSnapshot(index);
    emit snapshotsChanged();
}

void UndoHistoryModel::restoreSnapshot(int index)
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    IMasterNotationPtr masterNotation = notation->masterNotation();
    if (!masterNotation) {
        return;
    }

    MasterScore* masterScore = masterNotation->masterScore();
    if (!masterScore) {
        LOGW() << "Could not get master score for restoring snapshot";
        return;
    }
    if (index < 0 || index >= int(masterScore->snapshots().size())) {
        LOGW() << "Invalid snapshot index: " << index;
        return;
    }

    bool hasUnsavedChanges = notation->undoStack() && !notation->undoStack()->isStackClean();

    if (hasUnsavedChanges) {
        std::string snapshotName = masterScore->snapshots()[index].name.toStdString();

        std::string title = muse::trc("notation/undohistory", "Do you want to save changes before restoring snapshot?");
        std::string body = muse::trc("notation/undohistory", "Your changes will be lost if you do not save them.");

        interactive()->warning(title, body,
        {
            interactive()->buttonData(IInteractive::Button::Save),
            interactive()->buttonData(IInteractive::Button::Discard),
            interactive()->buttonData(IInteractive::Button::Cancel)
        }, int(IInteractive::Button::Cancel))
        .onResolve(this, [this, index, masterScore, masterNotation](const IInteractive::Result& result) {
            if (result.isButton(IInteractive::Button::Cancel)) {
                return;
            }
            if (result.isButton(IInteractive::Button::Save)) {
                dispatcher()->dispatch("file-save");
            }
            doRestoreSnapshot(index, masterScore, masterNotation);
        });
    } else {
        doRestoreSnapshot(index, masterScore, masterNotation);
    }
}

void UndoHistoryModel::doRestoreSnapshot(int index, MasterScore* masterScore, IMasterNotationPtr masterNotation)
{
    context()->setCurrentNotation(masterNotation->notation());

    // muse::IDList partIds;
    // for (const mu::engraving::Part* part : masterScore->parts()) {
    //     partIds.push_back(part->id());
    // }
    // if (!partIds.empty()) {
    //     masterNotation->parts()->removeParts(partIds);
    // }

    masterScore->restoreSnapshot(index);
    m_lastRestoredSnapshotName = masterScore->snapshots()[index].name;
    onCurrentNotationChanged();
}

QVariantList UndoHistoryModel::snapshots() const
{
    QVariantList result;
    MasterScore* masterScore = context()->currentNotation()->masterNotation()->masterScore();
    if (!masterScore) {
        return result;
    }
    for (MasterScore::Snapshot& snap : masterScore->snapshots()) {
        QVariantMap item;
        item["name"] = QString::fromStdString(snap.name.toStdString());
        result.append(item);
    }
    return result;
}

void UndoHistoryModel::renameSnapshot(int index, const QString& newName)
{
    MasterScore* masterScore = context()->currentNotation()->masterNotation()->masterScore();
    masterScore->snapshots()[index].name = newName;
    emit snapshotsChanged();
}

void UndoHistoryModel::addFileOpenedSnapshot()
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    IMasterNotationPtr masterNotation = notation->masterNotation();
    if (!masterNotation) {
        return;
    }

    MasterScore* masterScore = masterNotation->masterScore();
    if (!masterScore) {
        return;
    }

    if (!m_fileOpenedSnapshotExists) {
        masterScore->addSnapshot(mu::engraving::String(u"File opened"), true /* fileOpened */);
        m_fileOpenedSnapshotExists = true;
        emit snapshotsChanged();
    }
}
