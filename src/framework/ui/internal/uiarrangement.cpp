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

static const std::string DATA_NAME("uiarrangement");

static const QString UI_SETTINGS_KEY("uisettings");
static const QString UI_STATES_KEY("uistates");
static const QString UI_TOOLACTIONS_KEY("uitoolactions");

void UiArrangement::init()
{
    RetValCh<IWorkspacePtr> w = workspaceManager()->currentWorkspace();
    w.ch.onReceive(this, [this](const IWorkspacePtr) {
        updateData();
    });

    updateData();
}

IWorkspacePtr UiArrangement::currentWorkspace() const
{
    return workspaceManager()->currentWorkspace().val;
}

void UiArrangement::updateData()
{
    TRACEFUNC;

    IWorkspacePtr workspace = currentWorkspace();

    QJsonObject _wdata;
    QJsonObject _fdata;

    auto workspaceData = [&]() {
        if (_wdata.isEmpty()) {
            RetVal<Data> d = workspace->readData(DATA_NAME);
            _wdata = d.val.data.toObject();
        }
        return _wdata;
    };

    auto fileData = [&]() {
        if (_fdata.isEmpty()) {
            _fdata = readFromFile();
        }
        return _fdata;
    };

    if (workspace && workspace->isManaged(UI_SETTINGS_KEY.toStdString())) {
        m_settings = workspaceData().value(UI_SETTINGS_KEY).toObject();
    } else {
        m_settings = fileData().value(UI_SETTINGS_KEY).toObject();
    }

    if (workspace && workspace->isManaged(UI_STATES_KEY.toStdString())) {
        m_states = workspaceData().value(UI_STATES_KEY).toObject();
    } else {
        m_states = fileData().value(UI_STATES_KEY).toObject();
    }

    if (workspace && workspace->isManaged(UI_TOOLACTIONS_KEY.toStdString())) {
        m_toolactions = workspaceData().value(UI_TOOLACTIONS_KEY).toObject();
    } else {
        m_toolactions = fileData().value(UI_TOOLACTIONS_KEY).toObject();
    }
}

void UiArrangement::saveData()
{
    TRACEFUNC;

    QJsonObject toWorkspace;
    QJsonObject toFile;

    IWorkspacePtr workspace = currentWorkspace();

    if (workspace && workspace->isManaged(UI_SETTINGS_KEY.toStdString())) {
        toWorkspace[UI_SETTINGS_KEY] = m_settings;
    } else {
        toFile[UI_SETTINGS_KEY] = m_settings;
    }

    if (workspace && workspace->isManaged(UI_STATES_KEY.toStdString())) {
        toWorkspace[UI_STATES_KEY] = m_states;
    } else {
        toFile[UI_STATES_KEY] = m_states;
    }

    if (workspace && workspace->isManaged(UI_TOOLACTIONS_KEY.toStdString())) {
        toWorkspace[UI_TOOLACTIONS_KEY] = m_toolactions;
    } else {
        toFile[UI_TOOLACTIONS_KEY] = m_toolactions;
    }

    if (workspace && !toWorkspace.isEmpty()) {
        Ret ret = workspace->writeData(DATA_NAME, { toWorkspace });
        if (!ret) {
            LOGE() << ret.toString();
        }
    }

    if (!toFile.isEmpty()) {
        Ret ret = writeToFile(toFile);
        if (!ret) {
            LOGE() << ret.toString();
        }
    }
}

mu::Ret UiArrangement::writeToFile(const QJsonObject& data)
{
    NOT_IMPLEMENTED;
    return Ret();
}

QJsonObject UiArrangement::readFromFile() const
{
    NOT_IMPLEMENTED;
    return QJsonObject();
}

QString UiArrangement::value(const QString& key) const
{
    QJsonValue val = m_settings.value(key);
    return val.toString();
}

void UiArrangement::setValue(const QString& key, const QString& val)
{
    m_settings[key] = val;
    saveData();
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
    saveData();
}

QList<QString> UiArrangement::toolbarActions(const QString& toolbarName) const
{
    QJsonArray actsArr = m_toolactions.value(toolbarName).toArray();

    QList<QString> acts;
    acts.reserve(actsArr.size());
    for (const QJsonValue& v : actsArr) {
        acts.push_back(v.toString());
    }
    return acts;
}

void UiArrangement::setToolbarActions(const QString& toolbarName, const QList<QString>& actions)
{
    QJsonArray arr;
    for (const QString& a : actions) {
        arr.append(a);
    }
    m_toolactions[toolbarName] = arr;
    saveData();
}
