/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "scorepreferencesmodel.h"

using namespace muse;
using namespace mu::appshell;

ScorePreferencesModel::ScorePreferencesModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void ScorePreferencesModel::load()
{
    notationConfiguration()->defaultStyleFilePathChanged().onReceive(this, [this](const io::path_t&) {
        emit defaultStylePathChanged();
    });

    notationConfiguration()->partStyleFilePathChanged().onReceive(this, [this](const io::path_t&) {
        emit defaultPartStylePathChanged();
    });
}

QString ScorePreferencesModel::defaultStylePath() const
{
    return notationConfiguration()->defaultStyleFilePath().toQString();
}

void ScorePreferencesModel::setDefaultStylePath(const QString& path)
{
    if (defaultStylePath() == path) {
        return;
    }

    notationConfiguration()->setDefaultStyleFilePath(path.toStdString());
}

QString ScorePreferencesModel::defaultPartStylePath() const
{
    return notationConfiguration()->partStyleFilePath().toQString();
}

void ScorePreferencesModel::setDefaultPartStylePath(const QString& path)
{
    if (defaultPartStylePath() == path) {
        return;
    }

    notationConfiguration()->setPartStyleFilePath(path.toStdString());
}
