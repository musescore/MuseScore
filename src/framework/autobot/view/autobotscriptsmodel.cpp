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
#include "autobotscriptsmodel.h"

#include "io/path.h"

#include "log.h"

using namespace muse::autobot;

static QString typeToString(ScriptType t)
{
    switch (t) {
    case ScriptType::Undefined: return "Undefined";
    case ScriptType::TestCase: return "TestCase";
    case ScriptType::Custom: return "Custom";
    }
    return "";
}

ScriptType typeFromString(const QString& str)
{
    if (str == "TestCase") {
        return ScriptType::TestCase;
    }
    if (str == "Custom") {
        return ScriptType::Custom;
    }
    return ScriptType::Undefined;
}

AutobotScriptsModel::AutobotScriptsModel(QObject* parent)
    : QAbstractListModel(parent), Injectable(muse::iocCtxForQmlObject(this))
{
}

AutobotScriptsModel::~AutobotScriptsModel()
{
    stopRunAllTC();
}

QVariant AutobotScriptsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const Script& script = m_scripts.at(index.row());
    switch (role) {
    case rTitle: return script.title;
    case rDescription: return script.description;
    case rPath: return script.path.toQString();
    case rIndex: return index.row();
    case rType: return typeToString(script.type);
    case rStatus: return IAutobot::statusToString(m_statuses.value(script.path, IAutobot::Status::Undefined));
    case rSelected: return m_selected.value(index.row(), true);
    }
    return QVariant();
}

int AutobotScriptsModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_scripts.size());
}

QHash<int, QByteArray> AutobotScriptsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { rTitle, "titleRole" },
        { rDescription, "descriptionRole" },
        { rType, "typeRole" },
        { rPath, "pathRole" },
        { rIndex, "indexRole" },
        { rStatus, "statusRole" },
        { rSelected, "selectedRole" },
    };
    return roles;
}

void AutobotScriptsModel::load()
{
    autobot()->statusChanged().onReceive(this, [this](const io::path_t& path, const IAutobot::Status& status) {
        setStatus(path, status);

        if (status == IAutobot::Status::Error) {
            stopRunAllTC();
        }
    });

    autobot()->speedModeChanged().onReceive(this, [this](const SpeedMode&) {
        emit speedModeChanged();
    });

    beginResetModel();

    m_scripts = scriptsRepository()->scripts();
    std::sort(m_scripts.begin(), m_scripts.end(), [](const Script& f, const Script& s) {
        if (f.type != s.type) {
            return f.type < s.type;
        }
        return f.title < s.title;
    });

    //! NOTE Select all
    for (size_t i = 0; i < m_scripts.size(); ++i) {
        m_selected[int(i)] = true;
    }

    endResetModel();
}

void AutobotScriptsModel::setStatus(const io::path_t& path, IAutobot::Status st)
{
    m_statuses[path] = st;
    for (size_t i = 0; i < m_scripts.size(); ++i) {
        if (m_scripts.at(i).path == path) {
            emit dataChanged(index(int(i)), index(int(i)), { rStatus });
            break;
        }
    }
}

void AutobotScriptsModel::runScript(int scriptIndex)
{
    const Script& script = m_scripts.at(scriptIndex);
    autobot()->execScript(script.path);
}

void AutobotScriptsModel::runAllTC()
{
    //! NOTE Reset all statuses
    for (const Script& s : m_scripts) {
        setStatus(s.path, IAutobot::Status::Undefined);
    }

    m_currentTCIndex = -1;

    setIsRunAllTCMode(true);
    tryRunNextTC();
}

bool AutobotScriptsModel::tryRunNextTC()
{
    if (!isRunAllTCMode()) {
        LOGD() << "not is run all TC mode";
        return false;
    }

    int prevIndex = m_currentTCIndex;
    ++m_currentTCIndex;
    size_t currentIndex = static_cast<size_t>(m_currentTCIndex);
    if (!(currentIndex < m_scripts.size())) {
        LOGD() << "no more scripts";
        return false;
    }

    //! NOTE Find next TC
    for (size_t i = currentIndex; i < m_scripts.size(); ++i) {
        currentIndex = i;
        if (m_scripts.at(currentIndex).type == ScriptType::TestCase && m_selected.value(int(currentIndex), true)) {
            break;
        }
    }

    if (prevIndex == static_cast<int>(currentIndex)) {
        LOGD() << "no more TC scripts";
        return false;
    }

    m_currentTCIndex = static_cast<int>(currentIndex);
    const Script& tc = m_scripts.at(currentIndex);
    LOGD() << "requireStartTC: " << tc.path;
    emit requireStartTC(tc.path.toQString());

    return true;
}

void AutobotScriptsModel::stopRunAllTC()
{
    setIsRunAllTCMode(false);
    autobot()->abort();
}

bool AutobotScriptsModel::isRunAllTCMode() const
{
    return m_isRunAllTCMode;
}

void AutobotScriptsModel::setIsRunAllTCMode(bool arg)
{
    if (m_isRunAllTCMode == arg) {
        return;
    }
    m_isRunAllTCMode = arg;
    emit isRunAllTCModeChanged();
}

void AutobotScriptsModel::toggleSelect(int idx)
{
    bool isOldAllSel = isAllSelected(m_scripts.at(size_t(idx)).type);
    m_selected[idx] = !m_selected.value(idx, true);
    emit dataChanged(index(idx), index(idx), { rSelected });

    bool isAllSel = isAllSelected(m_scripts.at(size_t(idx)).type);

    if (isOldAllSel != isAllSel) {
        emit isAllSelectedChanged(typeToString(m_scripts.at(size_t(idx)).type), isAllSel);
    }
}

void AutobotScriptsModel::toggleAllSelect(const QString& typeStr)
{
    ScriptType type = typeFromString(typeStr);
    bool isAllSel = isAllSelected(type);
    for (size_t i = 0; i < m_scripts.size(); ++i) {
        if (m_scripts.at(i).type != type) {
            continue;
        }

        m_selected[int(i)] = !isAllSel;
        emit dataChanged(index(int(i)), index(int(i)), { rSelected });
    }

    emit isAllSelectedChanged(typeStr, !isAllSel);
}

bool AutobotScriptsModel::isAllSelected(const QString& typeStr) const
{
    ScriptType type = typeFromString(typeStr);
    return isAllSelected(type);
}

bool AutobotScriptsModel::isAllSelected(const ScriptType& type) const
{
    for (size_t i = 0; i < m_scripts.size(); ++i) {
        if (m_scripts.at(i).type != type) {
            continue;
        }

        if (!m_selected.value(int(i), true)) {
            return false;
        }
    }

    return true;
}

QString AutobotScriptsModel::speedMode() const
{
    return speedModeToString(autobot()->speedMode());
}

void AutobotScriptsModel::setSpeedMode(const QString& newSpeedMode)
{
    autobot()->setSpeedMode(speedModeFromString(newSpeedMode));
}
