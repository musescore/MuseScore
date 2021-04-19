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
    ValCh<ExtensionsHash> extensions = extensionsService()->extensions();

    beginResetModel();
    m_list = extensions.val.values();
    endResetModel();

    RetCh<Extension> extensionChanged = extensionsService()->extensionChanged();
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

    RetCh<ExtensionProgress> installRet = extensionsService()->install(m_list.at(index).code);
    if (!installRet.ret) {
        LOGE() << installRet.ret.toString();
        return;
    }

    installRet.ch.onReceive(this, [this, code](const ExtensionProgress& progress) {
        emit this->progress(code, progress.status, progress.indeterminate, progress.current, progress.total);
    }, Asyncable::AsyncMode::AsyncSetRepeat);

    installRet.ch.onClose(this, [this, code]() {
        emit finish(extension(code));
    }, Asyncable::AsyncMode::AsyncSetRepeat);
}

void ExtensionListModel::uninstall(QString code)
{
    int index = itemIndexByCode(code);

    if (index < 0 || index > m_list.count()) {
        return;
    }

    Ret uninstallRet = extensionsService()->uninstall(m_list.at(index).code);
    if (!uninstallRet) {
        LOGE() << uninstallRet.toString();
        return;
    }

    emit finish(extension(code));
}

void ExtensionListModel::update(QString code)
{
    int index = itemIndexByCode(code);

    if (index < 0 || index > m_list.count()) {
        return;
    }

    RetCh<ExtensionProgress> updateRet = extensionsService()->update(m_list.at(index).code);
    if (!updateRet.ret) {
        LOGE() << updateRet.ret.toString();
        return;
    }

    updateRet.ch.onReceive(this, [this, code](const ExtensionProgress& progress) {
        emit this->progress(code, progress.status, progress.indeterminate, progress.current, progress.total);
    }, Asyncable::AsyncMode::AsyncSetRepeat);

    updateRet.ch.onClose(this, [this, code]() {
        emit finish(extension(code));
    }, Asyncable::AsyncMode::AsyncSetRepeat);
}

void ExtensionListModel::openFullDescription(QString code)
{
    int index = itemIndexByCode(code);
    if (index < 0 || index > m_list.count()) {
        return;
    }

    // TODO: implement after getting the link of extension
    NOT_IMPLEMENTED;
}

QVariantMap ExtensionListModel::extension(QString code)
{
    int index = itemIndexByCode(code);
    if (index < 0 || index > m_list.count()) {
        return QVariantMap();
    }

    QVariantMap result;

    QHash<int, QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> i(names);
    while (i.hasNext()) {
        i.next();
        QModelIndex idx = this->index(index, 0);
        QVariant data = idx.data(i.key());
        result[i.value()] = data;
    }

    return result;
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
