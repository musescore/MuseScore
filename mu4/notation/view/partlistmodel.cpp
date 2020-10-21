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

using namespace mu::notation;

PartListModel::PartListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles.insert(RoleTitle, "title");
}

void PartListModel::load()
{
    beginResetModel();

    m_excerpts = masterNotation()->excerpts();

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

void PartListModel::removeParts(const QModelIndexList& indexes)
{
    for (const QModelIndex& index: indexes) {
        int row = index.row();
        beginRemoveRows(QModelIndex(), row, row);
        m_excerptsToRemove.push_back(m_excerpts[row]);
        m_excerpts.erase(m_excerpts.begin() + row);
        endRemoveRows();
    }
}

void PartListModel::openParts(const QModelIndexList& indexes)
{
    if (indexes.isEmpty()) {
        return;
    }

    for (const QModelIndex& index: indexes) {
        m_excerpts[index.row()]->setOpened(true);
    }

    context()->setCurrentNotation(m_excerpts[indexes.last().row()]);
}

void PartListModel::apply()
{
    for (IExcerptNotationPtr excerpt: m_excerptsToRemove) {
        masterNotation()->removeExcerpt(excerpt);
    }
}

IMasterNotationPtr PartListModel::masterNotation() const
{
    return context()->currentMasterNotation();
}
