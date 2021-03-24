//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "abfilesmodel.h"

using namespace mu::autobot;

AbFilesModel::AbFilesModel(QObject* parent)
    : QAbstractListModel(parent)
{
    update();
}

QVariant AbFilesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const File& file = m_files.val.at(index.row());
    switch (role) {
    case FileTitle: return io::basename(file.path).toQString();
    case FileIndex: return index.row();
    case IsCurrentFile: return m_fileIndex.val == index.row();
    }
    return QVariant();
}

int AbFilesModel::rowCount(const QModelIndex&) const
{
    return m_files.val.size();
}

QHash<int,QByteArray> AbFilesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { FileTitle, "fileTitle" },
        { FileIndex, "fileIndex" },
        { IsCurrentFile, "isCurrentFile" },
    };
    return roles;
}

void AbFilesModel::update()
{
    beginResetModel();
    m_files = autobot()->files();
    m_files.notification.onNotify(this, [this]() {
        update();
    });

    m_fileIndex = autobot()->currentFileIndex();
    m_fileIndex.ch.onReceive(this, [this](int idx) {
        int old = m_fileIndex.val;
        m_fileIndex.val = idx;
        emit dataChanged(index(old), index(old));
        emit dataChanged(index(idx), index(idx));
    });

    endResetModel();
}
