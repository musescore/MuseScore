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

using namespace mu::autobot;

AutobotScriptsModel::AutobotScriptsModel(QObject* parent)
    : QAbstractListModel(parent)
{
    autobot()->statusChanged().onReceive(this, [this](const io::path& path, const IAutobot::Status& status) {
        setStatus(path, status);

        if (status == IAutobot::Status::Error) {
            stopRunAllTC();
        }
    });
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

    auto typeToString = [](ScriptType t) {
        switch (t) {
        case ScriptType::Undefined: return "Undefined";
        case ScriptType::TestCase: return "TestCase";
        case ScriptType::Custom: return "Custom";
        }
        return "";
    };

    const Script& script = m_scripts.at(index.row());
    switch (role) {
    case rTitle: return script.title;
    case rDescription: return script.description;
    case rPath: return script.path.toQString();
    case rIndex: return index.row();
    case rType: return typeToString(script.type);
    case rStatus: return IAutobot::statusToString(m_statuses.value(script.path, IAutobot::Status::Undefined));
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
    };
    return roles;
}

void AutobotScriptsModel::load()
{
    beginResetModel();

    m_scripts = scriptsRepository()->scripts();
    std::sort(m_scripts.begin(), m_scripts.end(), [](const Script& f, const Script& s) {
        if (f.type != s.type) {
            return f.type < s.type;
        }
        return f.title < s.title;
    });

    endResetModel();
}

void AutobotScriptsModel::setStatus(const io::path& path, IAutobot::Status st)
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
        if (m_scripts.at(currentIndex).type == ScriptType::TestCase) {
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
