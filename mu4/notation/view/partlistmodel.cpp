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

    for (IExcerptNotationPtr excerpt : masterNotation()->excerpts()) {
        m_excerpts << excerpt;
    }

    endResetModel();
}

QVariant PartListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case RoleTitle:
        return m_excerpts[index.row()]->metaInfo().title;
    case RoleIsSelected:
        return m_selectionModel->isSelected(index);
    }

    return QVariant();
}

int PartListModel::rowCount(const QModelIndex&) const
{
    return int(m_excerpts.size());
}

QHash<int, QByteArray> PartListModel::roleNames() const
{
    return m_roles;
}

void PartListModel::createNewPart()
{
    Meta meta;
    meta.title = qtrc("notation", "Part");

    IExcerptNotationPtr excerpt = masterNotation()->appendExcerpt(meta);
    excerpt->setOpened(true);
    context()->setCurrentNotation(excerpt);
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
    m_excerpts.removeAt(index);
    endRemoveRows();
}

void PartListModel::setPartTitle(int index, const QString& name)
{
    if (!isIndexValid(index)) {
        return;
    }

    IExcerptNotationPtr notation = m_excerpts[index];
    Meta meta = notation->metaInfo();

    if (meta.title == name) {
        return;
    }

    meta.title = name;
    notation->setMetaInfo(meta);

    QModelIndex modelIndex = this->index(index);
    emit dataChanged(modelIndex, modelIndex);
}

void PartListModel::copyPart(int index)
{
    Q_UNUSED(index)
    NOT_IMPLEMENTED;
}

void PartListModel::removeSelectedParts()
{
    QList<int> rows = selectedRows();
    if (rows.empty()) {
        return;
    }

    for (int row : selectedRows()) {
        m_excerptsToRemove.push_back(m_excerpts[row]);
    }

    for (IExcerptNotationPtr excerpt : m_excerptsToRemove) {
        int row = m_excerpts.indexOf(excerpt);
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
        m_excerpts[row]->setOpened(true);
    }

    context()->setCurrentNotation(m_excerpts[rows.last()]);
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
    for (IExcerptNotationPtr excerpt: m_excerptsToRemove) {
        masterNotation()->removeExcerpt(excerpt);
    }
}

bool PartListModel::isIndexValid(int index) const
{
    return index > 0 && index < m_excerpts.size();
}

IMasterNotationPtr PartListModel::masterNotation() const
{
    return context()->currentMasterNotation();
}
