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
#include "profilerviewmodel.h"

#include "global/profiler.h"

#include "log.h"

using namespace muse::diagnostics;
using namespace muse::profiler;

ProfilerViewModel::ProfilerViewModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant ProfilerViewModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const Item& item = m_list.at(index.row());
    switch (role) {
    case rData: return QVariant::fromValue(item.data);
    case rGroup: return QVariant::fromValue(item.group);
    default: break;
    }

    return QVariant();
}

int ProfilerViewModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_list.count();
}

QHash<int, QByteArray> ProfilerViewModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { rData, "dataRole" },
        { rGroup, "groupRole" },
    };
    return roles;
}

void ProfilerViewModel::reload()
{
    m_allList.clear();

    QString group = "Main thread";
    QString str = QString::fromStdString(Profiler::instance()->threadsDataString(Profiler::Data::OnlyMain));
    QStringList list = str.split("\n");
    foreach (const QString& data, list) {
        Item item;
        item.group = group;
        item.data = data;

        m_allList.append(item);
    }

    group = "Other thread";
    str = QString::fromStdString(Profiler::instance()->threadsDataString(Profiler::Data::OnlyOther));
    list = str.split("\n");
    foreach (const QString& data, list) {
        Item item;
        item.group = group;
        item.data = data;

        m_allList.append(item);
    }

    find(m_searchText);
}

void ProfilerViewModel::find(const QString& str)
{
    beginResetModel();

    m_searchText = str;

    if (m_searchText.isEmpty()) {
        m_list = m_allList;
    } else {
        m_list.clear();
        for (const Item& item : m_allList) {
            if (item.data.contains(m_searchText, Qt::CaseInsensitive)) {
                m_list.append(item);
            }
        }
    }

    endResetModel();
}

void ProfilerViewModel::clear()
{
    PROFILER_CLEAR;
    reload();
}

void ProfilerViewModel::print()
{
    PROFILER_PRINT;
}
