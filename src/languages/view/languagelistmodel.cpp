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

void LanguageListModel::init()
{
    load();

    setupConnections();
}

void LanguageListModel::load()
{
    m_list.clear();

    ValCh<LanguagesHash> languages = languagesService()->languages();

    beginResetModel();
    QList<Language> languageList = languages.val.values();
    std::sort(languageList.begin(), languageList.end(), defaultSort);
    m_list = languageList;
    endResetModel();
}

void LanguageListModel::setupConnections()
{
    RetCh<Language> languageChanged = languagesService()->languageChanged();
    languageChanged.ch.onReceive(this, [this](const Language& newLanguage) {
        int index = itemIndexByCode(newLanguage.code);

        if (index < 0 || index > m_list.count()) {
            return;
        }

        m_list[index] = newLanguage;
        QModelIndex _index = createIndex(index, 0);
        emit dataChanged(_index, _index);
    });

    ValCh<LanguagesHash> languages = languagesService()->languages();
    languages.ch.onReceive(this, [this](const LanguagesHash&) {
        load();
    });

    languagesService()->currentLanguage().ch.onReceive(this, [this](const Language&) {
        load();
    });
}

void LanguageListModel::install(QString code)
{
    int index = itemIndexByCode(code);

    if (index < 0 || index > m_list.count()) {
        return;
    }

    RetCh<LanguageProgress> installRet = languagesService()->install(m_list.at(index).code);
    if (!installRet.ret) {
        LOGE() << "Error" << installRet.ret.code() << installRet.ret.text();
        return;
    }

    installRet.ch.onReceive(this, [this, code](const LanguageProgress& progress) {
        emit this->progress(code, progress.status, progress.indeterminate, progress.current, progress.total);
    }, Asyncable::AsyncMode::AsyncSetRepeat);

    installRet.ch.onClose(this, [this, code]() {
        emit finish(language(code));
    }, Asyncable::AsyncMode::AsyncSetRepeat);
}

void LanguageListModel::update(QString code)
{
    int index = itemIndexByCode(code);

    if (index < 0 || index > m_list.count()) {
        return;
    }

    RetCh<LanguageProgress> updateRet = languagesService()->update(m_list.at(index).code);
    if (!updateRet.ret) {
        LOGE() << "Error" << updateRet.ret.code() << updateRet.ret.text();
        return;
    }

    updateRet.ch.onReceive(this, [this, code](const LanguageProgress& progress) {
        emit this->progress(code, progress.status, progress.indeterminate, progress.current, progress.total);
    }, Asyncable::AsyncMode::AsyncSetRepeat);

    updateRet.ch.onClose(this, [this, code]() {
        emit finish(language(code));
    }, Asyncable::AsyncMode::AsyncSetRepeat);
}

void LanguageListModel::uninstall(QString code)
{
    int index = itemIndexByCode(code);

    if (index < 0 || index > m_list.count()) {
        return;
    }

    Ret ret = languagesService()->uninstall(m_list.at(index).code);
    if (!ret) {
        LOGE() << "Error" << ret.code() << ret.text();
        return;
    }

    emit finish(language(code));
}

void LanguageListModel::openPreferences()
{
    interactive()->open("musescore://preferences?currentPageId=general");
}

QVariantMap LanguageListModel::language(QString code)
{
    int index = itemIndexByCode(code);

    if (index < 0 || index > m_list.count()) {
        return QVariantMap();
    }

    QVariantMap result;

    QHash<int,QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> i(names);
    while (i.hasNext()) {
        i.next();
        QModelIndex idx = this->index(index, 0);
        QVariant data = idx.data(i.key());
        result[i.value()] = data;
    }

    return result;
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

    if (language.status == LanguageStatus::Status::Installed
        || language.status == LanguageStatus::Status::NeedUpdate) {
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
