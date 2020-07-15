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
#include "languagelistmodel.h"

#include "log.h"

using namespace mu::languages;

LanguageListModel::LanguageListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles.insert(rName, "name");
    m_roles.insert(rFileSize, "fileSize");
    m_roles.insert(rStatus, "status");
    m_roles.insert(rIsCurrent, "isCurrent");
}

QVariant LanguageListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Language item = m_list[index.row()];

    switch (role) {
    case rName:
        return QVariant::fromValue(item.name);
    case rFileSize:
        return QVariant::fromValue(item.fileSize);
    case rStatus:
        return QVariant::fromValue(static_cast<int>(item.status));
    case rIsCurrent:
        return QVariant::fromValue(item.isCurrent);
    }

    return QVariant();
}

int LanguageListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_list.count();
}

QHash<int, QByteArray> LanguageListModel::roleNames() const
{
    return m_roles;
}

void LanguageListModel::load()
{
    m_list.clear();

    ValCh<LanguagesHash> languages = languagesController()->languages();

    beginResetModel();
    QList<Language> languageList = languages.val.values();
    std::sort(languageList.begin(), languageList.end(), [](const Language& l, const Language& r){
        return l.code < r.code;
    });

    m_list = languageList;
    endResetModel();

    RetCh<Language> languageChanged = languagesController()->languageChanged();
    languageChanged.ch.onReceive(this, [this](const Language& newLanguage) {
        for (int i = 0; i < m_list.count(); i++) {
            if (m_list[i].code == newLanguage.code) {
                m_list[i] = newLanguage;
                QModelIndex index = createIndex(i, 0);
                emit dataChanged(index, index);
                return;
            }
        }
    });
}

void LanguageListModel::updateList()
{
    languagesController()->refreshLanguages();
    load();
}

void LanguageListModel::install(int index)
{
    if (index < 0 || index > m_list.count()) {
        return;
    }

    Ret ret = languagesController()->install(m_list.at(index).code);
    if (!ret) {
        LOGE() << "Error" << ret.code() << ret.text();
        return;
    }
}

void LanguageListModel::uninstall(int index)
{
    if (index < 0 || index > m_list.count()) {
        return;
    }

    Ret ret = languagesController()->uninstall(m_list.at(index).code);
    if (!ret) {
        LOGE() << "Error" << ret.code() << ret.text();
        return;
    }
}

void LanguageListModel::setLanguage(int index)
{
    if (index < 0 || index > m_list.count()) {
        return;
    }

    Ret ret = languagesController()->setLanguage(m_list.at(index).code);
    if (!ret) {
        LOGE() << "Error" << ret.code() << ret.text();
        return;
    }
}
