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

#include "notationswitchlistmodel.h"

using namespace mu::notation;

NotationSwitchListModel::NotationSwitchListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void NotationSwitchListModel::load()
{
    loadNotations();

    context()->currentMasterNotationChanged().onNotify(this, [this]() {
        loadNotations();

        if (!masterNotation()) {
            return;
        }

        masterNotation()->excerpts().ch.onReceive(this, [this](ExcerptNotationList) {
            loadNotations();
        });

        listenNotationSavingStatus(masterNotation());
    });

    context()->currentNotationChanged().onNotify(this, [this]() {
        INotationPtr notation = context()->currentNotation();
        if (!notation) {
            return;
        }

        int currentNotationIndex = m_notations.indexOf(notation);
        emit currentNotationIndexChanged(currentNotationIndex);
    });
}

void NotationSwitchListModel::loadNotations()
{
    beginResetModel();
    m_notations.clear();

    if (!masterNotation()) {
        endResetModel();
        return;
    }

    m_notations << masterNotation();
    listenNotationOpeningStatus(masterNotation());

    for (IExcerptNotationPtr excerpt: masterNotation()->excerpts().val) {
        if (excerpt->opened().val) {
            m_notations << excerpt;
        }

        listenNotationOpeningStatus(excerpt);
    }

    endResetModel();
}

void NotationSwitchListModel::listenNotationOpeningStatus(INotationPtr notation)
{
    notation->opened().ch.onReceive(this, [this, notation](bool opened) {
        if (opened) {
            if (m_notations.contains(notation)) {
                return;
            }

            beginInsertRows(QModelIndex(), m_notations.size(), m_notations.size());
            m_notations << notation;
            endInsertRows();
        } else {
            int notationIndex = m_notations.indexOf(notation);
            beginRemoveRows(QModelIndex(), notationIndex, notationIndex);
            m_notations.removeAt(notationIndex);
            endRemoveRows();
        }
    });
}

void NotationSwitchListModel::listenNotationSavingStatus(IMasterNotationPtr masterNotation)
{
    masterNotation->needSave().notification.onNotify(this, [this]() {
        for (int i = 0; i < m_notations.size(); i++) {
            QModelIndex index = this->index(i);
            emit dataChanged(index, index, { RoleNeedSave });
        }
    });
}

IMasterNotationPtr NotationSwitchListModel::masterNotation() const
{
    return context()->currentMasterNotation();
}

QVariant NotationSwitchListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    INotationPtr notation = m_notations[index.row()];
    Meta meta = notation->metaInfo();

    switch (role) {
    case RoleTitle: return QVariant::fromValue(meta.title);
    case RoleNeedSave: return QVariant::fromValue(masterNotation()->needSave().val);
    }

    return QVariant();
}

int NotationSwitchListModel::rowCount(const QModelIndex&) const
{
    return m_notations.size();
}

QHash<int, QByteArray> NotationSwitchListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "title" },
        { RoleNeedSave, "needSave" }
    };

    return roles;
}

void NotationSwitchListModel::setCurrentNotation(int index)
{
    if (!isIndexValid(index)) {
        return;
    }

    context()->setCurrentNotation(m_notations[index]);
}

void NotationSwitchListModel::closeNotation(int index)
{
    if (!isIndexValid(index)) {
        return;
    }

    INotationPtr notation = m_notations[index];

    if (context()->currentNotation() == notation) {
        context()->setCurrentNotation(nullptr);
    }

    notation->setOpened(false);
}

bool NotationSwitchListModel::isIndexValid(int index) const
{
    return index >= 0 && index < m_notations.size();
}
