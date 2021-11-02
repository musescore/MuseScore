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
    };
    return roles;
}

void AutobotScriptsModel::load()
{
    beginResetModel();

    m_scripts = scriptsRepository()->scripts();

    endResetModel();
}

void AutobotScriptsModel::runScript(int scriptIndex)
{
    const Script& script = m_scripts.at(scriptIndex);
    autobot()->execScript(script.path);
}
