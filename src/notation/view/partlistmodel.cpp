/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "partlistmodel.h"

#include "log.h"
#include "translation.h"

#include "uicomponents/view/itemmultiselectionmodel.h"

using namespace mu::notation;
using namespace mu::uicomponents;
using namespace mu::framework;

static QString defaultPartTitle()
{
    return mu::qtrc("notation", "Part");
}

PartListModel::PartListModel(QObject* parent)
    : QAbstractListModel(parent), m_selectionModel(new ItemMultiSelectionModel(this))
{
    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged, this, &PartListModel::selectionChanged);
}

void PartListModel::load()
{
    beginResetModel();

    IMasterNotationPtr masterNotation = this->masterNotation();
    if (!masterNotation) {
        endResetModel();
        return;
    }

    for (IExcerptNotationPtr excerpt : masterNotation->excerpts().val) {
        m_excerpts << excerpt;
    }

    for (IExcerptNotationPtr excerpt : masterNotation->availableExcerpts()) {
        m_excerpts << excerpt;
    }

    endResetModel();
}

QVariant PartListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    IExcerptNotationPtr excerpt = m_excerpts[index.row()];

    switch (role) {
    case RoleTitle:
        return excerpt->metaInfo().title;
    case RoleIsSelected:
        return m_selectionModel->isSelected(index);
    }

    return QVariant();
}

int PartListModel::rowCount(const QModelIndex&) const
{
    return m_excerpts.size();
}

QHash<int, QByteArray> PartListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "title" },
        { RoleIsSelected, "isSelected" }
    };

    return roles;
}

bool PartListModel::hasSelection() const
{
    return m_selectionModel->hasSelection();
}

bool PartListModel::isRemovingAvailable() const
{
    if (!m_selectionModel->hasSelection()) {
        return false;
    }

    for (int index : selectedRows()) {
        if (!m_excerpts[index]->isInited()) {
            return false;
        }
    }

    return true;
}

void PartListModel::createNewPart()
{
    Meta meta;
    meta.title = defaultPartTitle();

    IExcerptNotationPtr excerpt = masterNotation()->newExcerptNotation();
    excerpt->setMetaInfo(meta);

    int index = m_excerpts.size();
    insertExcerpt(index, excerpt);
}

void PartListModel::selectPart(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    QModelIndexList previousSelectedIndexes = m_selectionModel->selectedIndexes();
    QModelIndex modelIndex = index(partIndex);
    m_selectionModel->select(modelIndex);
    QModelIndexList newSelectedIndexes = m_selectionModel->selectedIndexes();

    QSet<QModelIndex> indexesToUpdate(previousSelectedIndexes.begin(), previousSelectedIndexes.end());
    indexesToUpdate = indexesToUpdate.unite(QSet<QModelIndex>(newSelectedIndexes.begin(), newSelectedIndexes.end()));
    indexesToUpdate << modelIndex;

    for (const QModelIndex& indexToUpdate : indexesToUpdate) {
        emit dataChanged(indexToUpdate, indexToUpdate);
    }
}

void PartListModel::removePart(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    constexpr int partCount = 1;

    if (userAgreesToRemoveParts(partCount)) {
        doRemovePart(partIndex);
    }
}

void PartListModel::doRemovePart(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    bool isCurrentContation = context()->currentNotation() == m_excerpts[partIndex]->notation();

    beginRemoveRows(QModelIndex(), partIndex, partIndex);

    masterNotation()->notation()->undoStack()->prepareChanges();
    masterNotation()->removeExcerpts({ m_excerpts[partIndex] });
    masterNotation()->notation()->undoStack()->commitChanges();

    m_excerpts.removeAt(partIndex);
    endRemoveRows();

    if (isCurrentContation) {
        context()->setCurrentNotation(context()->currentMasterNotation()->notation());
    }
}

void PartListModel::setPartTitle(int partIndex, const QString& title)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    IExcerptNotationPtr excerpt = m_excerpts[partIndex];
    Meta meta = excerpt->metaInfo();

    if (meta.title == title) {
        return;
    }

    meta.title = title;
    excerpt->setMetaInfo(meta);

    notifyAboutNotationChanged(partIndex);
}

void PartListModel::validatePartTitle(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    IExcerptNotationPtr excerpt = m_excerpts[partIndex];
    Meta meta = excerpt->metaInfo();

    if (!meta.title.isEmpty()) {
        return;
    }

    meta.title = defaultPartTitle();
    excerpt->setMetaInfo(meta);

    notifyAboutNotationChanged(partIndex);
}

void PartListModel::notifyAboutNotationChanged(int index)
{
    QModelIndex modelIndex = this->index(index);
    emit dataChanged(modelIndex, modelIndex);
}

void PartListModel::copyPart(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    IExcerptNotationPtr copy = m_excerpts[partIndex]->clone();
    Meta meta = copy->metaInfo();
    meta.title += qtrc("notation", " (copy)");

    copy->setMetaInfo(meta);

    insertExcerpt(partIndex + 1, copy);
}

void PartListModel::insertExcerpt(int destinationIndex, IExcerptNotationPtr excerpt)
{
    beginInsertRows(QModelIndex(), destinationIndex, destinationIndex);
    m_excerpts.insert(destinationIndex, excerpt);
    endInsertRows();

    emit partAdded(destinationIndex);
    selectPart(destinationIndex);
}

void PartListModel::removeSelectedParts()
{
    QList<int> rows = selectedRows();
    if (rows.empty()) {
        return;
    }

    if (!userAgreesToRemoveParts(rows.count())) {
        return;
    }

    QList<IExcerptNotationPtr> excerptsToRemove;

    for (int row : rows) {
        excerptsToRemove.push_back(m_excerpts[row]);
    }

    for (IExcerptNotationPtr excerpt : excerptsToRemove) {
        int row = m_excerpts.indexOf(excerpt);
        doRemovePart(row);
    }

    m_selectionModel->clear();
}

bool PartListModel::userAgreesToRemoveParts(int partCount) const
{
    QString question = mu::qtrc("notation", "Are you sure you want to delete %1?")
                       .arg(partCount > 1 ? "these parts" : "this part");

    IInteractive::Result result = interactive()->question("", question.toStdString(), {
        IInteractive::Button::Yes, IInteractive::Button::No
    });

    return result.standartButton() == IInteractive::Button::Yes;
}

QList<int> PartListModel::selectedRows() const
{
    QList<int> result;

    for (const QModelIndex& index: m_selectionModel->selectedIndexes()) {
        result << index.row();
    }

    return result;
}

void PartListModel::openSelectedParts()
{
    QList<int> rows = selectedRows();
    if (rows.empty()) {
        return;
    }

    ExcerptNotationList newExcerpts;
    for (int index : rows) {
        newExcerpts.push_back(m_excerpts[index]);
    }

    masterNotation()->notation()->undoStack()->prepareChanges();
    masterNotation()->initExcerpts(newExcerpts);
    masterNotation()->notation()->undoStack()->commitChanges();

    context()->setCurrentNotation(m_excerpts[rows.last()]->notation());
}

bool PartListModel::isExcerptIndexValid(int index) const
{
    return index >= 0 && index < m_excerpts.size();
}

IMasterNotationPtr PartListModel::masterNotation() const
{
    return context()->currentMasterNotation();
}
