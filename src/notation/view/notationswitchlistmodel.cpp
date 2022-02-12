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

#include "notationswitchlistmodel.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::project;

NotationSwitchListModel::NotationSwitchListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void NotationSwitchListModel::load()
{
    TRACEFUNC;

    onCurrentProjectChanged();
    context()->currentProjectChanged().onNotify(this, [this]() {
        onCurrentProjectChanged();
    });

    onCurrentNotationChanged();
    context()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });
}

void NotationSwitchListModel::onCurrentProjectChanged()
{
    loadNotations();

    INotationProjectPtr project = context()->currentProject();
    if (!project) {
        return;
    }

    project->masterNotation()->excerpts().ch.onReceive(this, [this](ExcerptNotationList) {
        loadNotations();
    });

    listenProjectSavingStatusChanged(project);
}

void NotationSwitchListModel::onCurrentNotationChanged()
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    int currentNotationIndex = m_notations.indexOf(notation);
    emit currentNotationIndexChanged(currentNotationIndex);
}

void NotationSwitchListModel::loadNotations()
{
    beginResetModel();
    m_notations.clear();

    IMasterNotationPtr masterNotation = this->masterNotation();
    if (!masterNotation) {
        endResetModel();
        return;
    }

    m_notations << masterNotation->notation();
    listenNotationOpeningStatus(masterNotation->notation());
    listenNotationTitleChanged(masterNotation->notation());

    for (IExcerptNotationPtr excerpt: masterNotation->excerpts().val) {
        if (excerpt->notation()->opened().val) {
            m_notations << excerpt->notation();
        }

        listenNotationOpeningStatus(excerpt->notation());
        listenNotationTitleChanged(excerpt->notation());
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

void NotationSwitchListModel::listenNotationTitleChanged(INotationPtr notation)
{
    notation->notationChanged().onNotify(this, [this, notation]() {
        int index = m_notations.indexOf(notation);
        QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex, { RoleTitle });
    });
}

void NotationSwitchListModel::listenProjectSavingStatusChanged(project::INotationProjectPtr project)
{
    project->needSave().notification.onNotify(this, [this, project]() {
        int index = m_notations.indexOf(project->masterNotation()->notation());
        QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex, { RoleNeedSave });
    });

    project->saveLocationChanged().onNotify(this, [this, project]() {
        int index = m_notations.indexOf(project->masterNotation()->notation());
        QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex, { RoleTitle });
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

    switch (role) {
    case RoleTitle: return QVariant::fromValue(notation->name());
    case RoleNeedSave: {
        bool needSave = context()->currentProject()->needSave().val && isMasterNotation(notation);
        return QVariant::fromValue(needSave);
    }
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

    if (isMasterNotation(notation)) {
        dispatcher()->dispatch("file-close");
    } else {
        notation->setOpened(false);
    }
}

bool NotationSwitchListModel::isIndexValid(int index) const
{
    return index >= 0 && index < m_notations.size();
}

bool NotationSwitchListModel::isMasterNotation(const INotationPtr notation) const
{
    return context()->currentMasterNotation()->notation() == notation;
}
