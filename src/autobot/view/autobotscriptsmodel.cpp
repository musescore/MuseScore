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
}

QVariant AutobotScriptsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const Script& script = m_scripts.at(index.row());
    switch (role) {
    case ScriptTitle: return script.title;
    case ScriptIndex: return index.row();
    }
    return QVariant();
}

int AutobotScriptsModel::rowCount(const QModelIndex&) const
{
    return m_scripts.size();
}

QHash<int, QByteArray> AutobotScriptsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { ScriptTitle, "scriptTitle" },
        { ScriptIndex, "scriptIndex" },
    };
    return roles;
}

void AutobotScriptsModel::load()
{
    beginResetModel();

    RetVal<Scripts> scripts = scriptsRepository()->scripts();
    if (!scripts.ret) {
        LOGE() << "failed get scripts, err: " << scripts.ret.toString();
    } else {
        m_scripts = scripts.val;
    }

    endResetModel();
}

void AutobotScriptsModel::runScript(int scriptIndex)
{
    const Script& script = m_scripts.at(scriptIndex);
    LOGD() << script.path;
}
