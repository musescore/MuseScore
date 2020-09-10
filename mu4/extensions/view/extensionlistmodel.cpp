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
#include "extensionlistmodel.h"

#include "log.h"

using namespace mu::extensions;

ExtensionListModel::ExtensionListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles.insert(rCode, "code");
    m_roles.insert(rName, "name");
    m_roles.insert(rDescription, "description");
    m_roles.insert(rVersion, "version");
    m_roles.insert(rFileSize, "fileSize");
    m_roles.insert(rStatus, "status");
}

QVariant ExtensionListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Extension item = m_list[index.row()];

    switch (role) {
    case rCode:
        return QVariant::fromValue(item.code);
    case rName:
        return QVariant::fromValue(item.name);
    case rDescription:
        return QVariant::fromValue(item.description);
    case rVersion:
        return QVariant::fromValue(item.version);
    case rFileSize:
        return QVariant::fromValue(item.fileSize);
    case rStatus:
        return QVariant::fromValue(static_cast<int>(item.status));
    }

    return QVariant();
}

int ExtensionListModel::rowCount(const QModelIndex&) const
{
    return m_list.count();
}

QHash<int, QByteArray> mu::extensions::ExtensionListModel::roleNames() const
{
    return m_roles;
}

void ExtensionListModel::load()
{
    ValCh<ExtensionsHash> extensions = extensionsController()->extensions();

    beginResetModel();
    m_list = extensions.val.values();
    endResetModel();

    RetCh<Extension> extensionChanged = extensionsController()->extensionChanged();
    extensionChanged.ch.onReceive(this, [this](const Extension& newExtension) {
        for (int i = 0; i < m_list.count(); i++) {
            if (m_list[i].code == newExtension.code) {
                m_list[i] = newExtension;
                QModelIndex index = createIndex(i, 0);
                emit dataChanged(index, index);
                return;
            }
        }
    });
}

void ExtensionListModel::install(QString code)
{
    int index = itemIndexByCode(code);

    if (index < 0 || index > m_list.count()) {
        return;
    }

    RetCh<ExtensionProgress> installRet = extensionsController()->install(m_list.at(index).code);
    if (!installRet.ret) {
        LOGE() << installRet.ret.toString();
        return;
    }

    installRet.ch.onReceive(this, [this](const ExtensionProgress& progress) {
        emit this->progress(progress.status, progress.indeterminate, progress.current, progress.total);
    });

    installRet.ch.onClose(this, [this]() {
        emit finish();
    });
}

void ExtensionListModel::uninstall(QString code)
{
    int index = itemIndexByCode(code);

    if (index < 0 || index > m_list.count()) {
        return;
    }

    Ret uninstallRet = extensionsController()->uninstall(m_list.at(index).code);
    if (!uninstallRet) {
        LOGE() << uninstallRet.toString();
        return;
    }

    emit finish();
}

void ExtensionListModel::update(QString code)
{
    int index = itemIndexByCode(code);

    if (index < 0 || index > m_list.count()) {
        return;
    }

    RetCh<ExtensionProgress> updateRet = extensionsController()->update(m_list.at(index).code);
    if (!updateRet.ret) {
        LOGE() << updateRet.ret.toString();
        return;
    }

    updateRet.ch.onReceive(this, [this](const ExtensionProgress& progress) {
        emit this->progress(progress.status, progress.indeterminate, progress.current, progress.total);
    });

    updateRet.ch.onClose(this, [this]() {
        emit finish();
    });
}

int ExtensionListModel::itemIndexByCode(const QString& code) const
{
    for (int i = 0; i < m_list.count(); i++) {
        if (m_list[i].code == code) {
            return i;
        }
    }

    return -1;
}
