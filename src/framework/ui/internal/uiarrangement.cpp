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

#include "uiarrangement.h"

#include <QJsonValue>
#include <QJsonArray>

#include "log.h"

using namespace mu::ui;
using namespace mu::workspace;

void UiArrangement::init()
{
    workspacesDataProvider()->dataChanged(DataKey::UiSettings).onNotify(this, [this]() {
        updateData(DataKey::UiSettings, m_settings);
    });

    workspacesDataProvider()->dataChanged(DataKey::UiSettings).onNotify(this, [this]() {
        updateData(DataKey::UiStates, m_states);
    });

    workspacesDataProvider()->dataChanged(DataKey::UiSettings).onNotify(this, [this]() {
        updateData(DataKey::UiToolActions, m_toolactions);
    });

    updateData(DataKey::UiSettings, m_settings);
    updateData(DataKey::UiStates, m_states);
    updateData(DataKey::UiToolActions, m_toolactions);
}

void UiArrangement::updateData(DataKey key, QJsonObject& obj)
{
    RetVal<Data> data = workspacesDataProvider()->data(key);
    if (!data.ret) {
        LOGE() << "failed get data: " << int(key) << ", ret: " << data.ret.toString();
        return;
    }

    obj = data.val.data.toObject();
}

void UiArrangement::saveData(workspace::DataKey key, const QJsonObject& obj)
{
    workspacesDataProvider()->setData(key, { obj });
}

QString UiArrangement::value(const QString& key) const
{
    QJsonValue val = m_settings.value(key);
    return val.toString();
}

void UiArrangement::setValue(const QString& key, const QString& val)
{
    m_settings[key] = val;
    saveData(DataKey::UiSettings, m_settings);
}

mu::async::Notification UiArrangement::valueChanged(const QString& key) const
{
    NOT_IMPLEMENTED;
    return mu::async::Notification();
}

QByteArray UiArrangement::state(const QString& key) const
{
    QJsonValue val = m_states.value(key);
    QString valStr = val.toString();
    return valStr.toLocal8Bit();
}

void UiArrangement::setState(const QString& key, const QByteArray& data)
{
    m_states[key] = QString::fromLocal8Bit(data);
    saveData(DataKey::UiStates, m_states);
}

mu::actions::ActionCodeList UiArrangement::toolbarActions(const QString& toolbarName) const
{
    QJsonArray actsArr = m_toolactions.value(toolbarName).toArray();

    actions::ActionCodeList acts;
    acts.reserve(actsArr.size());
    for (const QJsonValue& v : actsArr) {
        acts.push_back(v.toString().toStdString());
    }
    return acts;
}

void UiArrangement::setToolbarActions(const QString& toolbarName, const actions::ActionCodeList& actions)
{
    QJsonArray arr;
    for (const actions::ActionCode& a : actions) {
        arr.append(QString::fromStdString(a));
    }
    m_toolactions[toolbarName] = arr;
    saveData(DataKey::UiToolActions, m_toolactions);
}

mu::async::Notification UiArrangement::toolbarActionsChanged(const QString& toolbarName) const
{
    NOT_IMPLEMENTED;
    return mu::async::Notification();
}
