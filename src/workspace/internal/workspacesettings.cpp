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
#include "workspacesettings.h"

#include "log.h"

using namespace mu::workspace;
using namespace mu::async;

static const std::string DATA_NAME("settings");
static std::map<WorkspaceSettings::Tag, QString> s_tags = {
    { WorkspaceSettings::Tag::Settings, QString("settings") },
    { WorkspaceSettings::Tag::UiArrangement, QString("uiarrangement") }
};

static const QString& tagToString(WorkspaceSettings::Tag tag)
{
    for (const auto& p : s_tags) {
        if (p.first == tag) {
            return p.second;
        }
    }
    static QString null;
    return null;
}

void WorkspaceSettings::init()
{
    RetValCh<IWorkspacePtr> w = manager()->currentWorkspace();
    if (w.val) {
        RetVal<Data> d = w.val->readData(DATA_NAME);
        m_data = d.val.data.toObject();
    }

    w.ch.onReceive(this, [this](const IWorkspacePtr w) {
        RetVal<Data> d = w->readData(DATA_NAME);
        m_data = d.val.data.toObject();

        m_valuesChanged.notify();

        for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
            it->second.send(value(it->first));
        }
    });
}

bool WorkspaceSettings::isManage(Tag tag) const
{
    const QString& tagStr = tagToString(tag);
    IF_ASSERT_FAILED(!tagStr.isEmpty()) {
        return false;
    }

    return m_data.contains(tagStr);
}

mu::Val WorkspaceSettings::value(const Key& key) const
{
    const QString& tagStr = tagToString(key.tag);
    IF_ASSERT_FAILED(!tagStr.isEmpty()) {
        return Val();
    }

    QJsonObject obj = m_data.value(tagStr).toObject();
    QJsonValue val = obj.value(QString::fromStdString(key.key));
    if (val.isUndefined()) {
        return Val();
    }

    return Val(val.toString().toStdString());
}

void WorkspaceSettings::setValue(const Key& key, const mu::Val& value)
{
    if (!currentWorkspace()) {
        return;
    }

    const QString& tagStr = tagToString(key.tag);
    IF_ASSERT_FAILED(!tagStr.isEmpty()) {
        return;
    }

    QJsonObject obj = m_data.value(tagStr).toObject();
    obj[QString::fromStdString(key.key)] = value.toQString();

    m_data[tagStr] = obj;

    Data wdata;
    wdata.data = m_data;
    currentWorkspace()->writeData(DATA_NAME, wdata);

    m_valuesChanged.notify();
    auto it = m_channels.find(key);
    if (it != m_channels.end()) {
        Channel<Val> channel = it->second;
        channel.send(value);
    }
}

Channel<mu::Val> WorkspaceSettings::valueChanged(const Key& key) const
{
    return m_channels[key];
}

Notification WorkspaceSettings::valuesChanged() const
{
    return m_valuesChanged;
}

IWorkspacePtr WorkspaceSettings::currentWorkspace() const
{
    return manager()->currentWorkspace().val;
}
