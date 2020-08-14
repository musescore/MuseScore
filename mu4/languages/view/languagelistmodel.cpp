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
#include "translation.h"

using namespace mu::languages;

LanguageListModel::LanguageListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles.insert(rCode, "code");
    m_roles.insert(rName, "name");
    m_roles.insert(rFileSize, "fileSize");
    m_roles.insert(rStatus, "status");
    m_roles.insert(rStatusTitle, "statusTitle");
    m_roles.insert(rIsCurrent, "isCurrent");
}

QVariant LanguageListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Language item = m_list[index.row()];

    switch (role) {
    case rCode:
        return QVariant::fromValue(item.code);
    case rName:
        return QVariant::fromValue(item.name);
    case rFileSize:
        return QVariant::fromValue(item.fileSize);
    case rStatus:
        return QVariant::fromValue(static_cast<int>(item.status));
    case rStatusTitle:
        return QVariant::fromValue(languageStatusTitle(item));
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

bool defaultSort(const Language& l, const Language& r)
{
    if (l.isCurrent && r.isCurrent) {
        return l.code < r.code;
    } else if (l.isCurrent) {
        return true;
    } else if (r.isCurrent) {
        return false;
    }

    auto installed = [](const Language& language) -> bool {
                         return language.status == LanguageStatus::Status::Installed
                                || language.status == LanguageStatus::Status::NeedUpdate;
                     };

    if (installed(l) && installed(r)) {
        return l.code < r.code;
    } else if (installed(l)) {
        return true;
    } else if (installed(r)) {
        return false;
    }

    return l.code < r.code;
}

void LanguageListModel::load()
{
    m_list.clear();

    ValCh<LanguagesHash> languages = languagesController()->languages();

    beginResetModel();
    QList<Language> languageList = languages.val.values();
    std::sort(languageList.begin(), languageList.end(), defaultSort);
    m_list = languageList;
    endResetModel();

    RetCh<Language> languageChanged = languagesController()->languageChanged();
    languageChanged.ch.onReceive(this, [this](const Language& newLanguage) {
        int index = itemIndexByCode(newLanguage.code);

        if (index < 0 || index > m_list.count()) {
            return;
        }

        m_list[index] = newLanguage;
        QModelIndex _index = createIndex(index, 0);
        emit dataChanged(_index, _index);
    });
}

void LanguageListModel::install(QString code)
{
    int index = itemIndexByCode(code);

    if (index < 0 || index > m_list.count()) {
        return;
    }

    RetCh<LanguageProgress> installRet = languagesController()->install(m_list.at(index).code);
    if (!installRet.ret) {
        LOGE() << "Error" << installRet.ret.code() << installRet.ret.text();
        return;
    }

    installRet.ch.onReceive(this, [this](const LanguageProgress& progress) {
        emit this->progress(progress.status, progress.indeterminate, progress.current, progress.total);
    });

    installRet.ch.onClose(this, [this]() {
        emit finish();
    });
}

void LanguageListModel::uninstall(QString code)
{
    int index = itemIndexByCode(code);

    if (index < 0 || index > m_list.count()) {
        return;
    }

    Ret ret = languagesController()->uninstall(m_list.at(index).code);
    if (!ret) {
        LOGE() << "Error" << ret.code() << ret.text();
        return;
    }
}

void LanguageListModel::openPreferences()
{
    interactive()->open("musescore://settings");
}

int LanguageListModel::itemIndexByCode(const QString& code) const
{
    for (int i = 0; i < m_list.count(); i++) {
        if (m_list[i].code == code) {
            return i;
        }
    }

    return -1;
}

QString LanguageListModel::languageStatusTitle(const Language& language) const
{
    QStringList status;

    if (language.status == LanguageStatus::Status::Installed) {
        if (language.isCurrent) {
            status << qtrc("languages", "selected");
        } else {
            status << qtrc("languages", "installed");
        }
    }

    if (language.status == LanguageStatus::Status::NeedUpdate) {
        status << qtrc("languages", "update available");
    }

    if (language.status == LanguageStatus::Status::NoInstalled) {
        status << qtrc("languages", "not installed");
    }

    QString result = status.join(", ");
    if (!result.isEmpty()) {
        result[0] = result[0].toUpper();
    }

    return result;
}
