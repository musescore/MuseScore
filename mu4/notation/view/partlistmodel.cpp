//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "partlistmodel.h"

#include "iexcerptnotation.h"

#include "log.h"
#include "translation.h"

#include <QItemSelectionModel>

using namespace mu::notation;

PartListModel::PartListModel(QObject* parent)
    : QAbstractListModel(parent), m_selectionModel(new QItemSelectionModel(this))
{
    m_roles.insert(RoleTitle, "title");
    m_roles.insert(RoleIsSelected, "isSelected");
    m_roles.insert(RoleIsMain, "isMain");
}

void PartListModel::load()
{
    beginResetModel();

    m_notations << masterNotation();
    for (IExcerptNotationPtr excerpt : masterNotation()->excerpts().val) {
        m_notations << excerpt;
    }

    endResetModel();
}

QVariant PartListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    INotationPtr notation = m_notations[index.row()];

    switch (role) {
    case RoleTitle:
        return notation->metaInfo().title;
    case RoleIsSelected:
        return m_selectionModel->isSelected(index);
    case RoleIsMain:
        return notation == masterNotation();
    }

    return QVariant();
}

int PartListModel::rowCount(const QModelIndex&) const
{
    return m_notations.size();
}

QHash<int, QByteArray> PartListModel::roleNames() const
{
    return m_roles;
}

void PartListModel::createNewPart()
{
    Meta meta;
    meta.title = qtrc("notation", "Part");

    INotationPtr notation = notationCreator()->newExcerptNotation();

    notation->setMetaInfo(meta);
    notation->setOpened(true);

    insertNotation(m_notations.size(), notation);
    m_currentNotation = notation;
}

void PartListModel::selectPart(int index)
{
    if (!isIndexValid(index)) {
        return;
    }

    QModelIndex modelIndex = this->index(index);
    m_selectionModel->select(modelIndex, QItemSelectionModel::SelectionFlag::Toggle);
    emit dataChanged(modelIndex, modelIndex);
}

void PartListModel::removePart(int index)
{
    if (!isIndexValid(index)) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_notations[index]->setOpened(false);
    m_notations.removeAt(index);
    endRemoveRows();
}

void PartListModel::setPartTitle(int index, const QString& title)
{
    if (!isIndexValid(index)) {
        return;
    }

    INotationPtr notation = m_notations[index];
    Meta meta = notation->metaInfo();

    if (meta.title == title) {
        return;
    }

    meta.title = title;
    notation->setMetaInfo(meta);

    QModelIndex modelIndex = this->index(index);
    emit dataChanged(modelIndex, modelIndex);
}

void PartListModel::copyPart(int index)
{
    if (!isIndexValid(index)) {
        return;
    }

    INotationPtr copy = m_notations[index]->clone();
    Meta meta = copy->metaInfo();
    meta.title += qtrc("notation", " (copy)");

    copy->setMetaInfo(meta);

    insertNotation(index + 1, copy);
}

void PartListModel::insertNotation(int destinationIndex, INotationPtr notation)
{
    beginInsertRows(QModelIndex(), destinationIndex, destinationIndex);
    m_notations.insert(destinationIndex, notation);
    endInsertRows();
}

void PartListModel::removeSelectedParts()
{
    QList<int> rows = selectedRows();
    if (rows.empty()) {
        return;
    }

    QList<INotationPtr> notationsToRemove;

    for (int row : rows) {
        notationsToRemove.push_back(m_notations[row]);
    }

    for (INotationPtr notation : notationsToRemove) {
        int row = m_notations.indexOf(notation);
        removePart(row);
    }

    m_selectionModel->clear();
}

void PartListModel::openSelectedParts()
{
    QList<int> rows = selectedRows();
    if (rows.empty()) {
        return;
    }

    for (int row : rows) {
        m_notations[row]->setOpened(true);
    }

    m_currentNotation = m_notations[rows.last()];
}

QList<int> PartListModel::selectedRows() const
{
    QList<int> result;

    for (const QModelIndex& index: m_selectionModel->selectedIndexes()) {
        result << index.row();
    }

    return result;
}

void PartListModel::apply()
{
    ExcerptNotationList newExcerpts;

    for (INotationPtr notation : m_notations) {
        IExcerptNotationPtr excerpt = std::dynamic_pointer_cast<IExcerptNotation>(notation);

        if (excerpt) {
            newExcerpts.push_back(excerpt);
        }
    }

    masterNotation()->setExcerpts(newExcerpts);

    if (m_currentNotation) {
        m_currentNotation->setOpened(true);
        context()->setCurrentNotation(m_currentNotation);
    }
}

bool PartListModel::isIndexValid(int index) const
{
    return index >= 0 && index < m_notations.size();
}

IMasterNotationPtr PartListModel::masterNotation() const
{
    return context()->currentMasterNotation();
}
