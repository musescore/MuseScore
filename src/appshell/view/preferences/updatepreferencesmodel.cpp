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

#include "updatepreferencesmodel.h"

#include "log.h"

using namespace mu::appshell;

UpdatePreferencesModel::UpdatePreferencesModel(QObject* parent)
    : QObject(parent)
{
}

bool UpdatePreferencesModel::isAppUpdatable() const
{
    return configuration()->isAppUpdatable();
}

bool UpdatePreferencesModel::needCheckForNewAppVersion() const
{
    return configuration()->needCheckForUpdate();
}

void UpdatePreferencesModel::setNeedCheckForNewAppVersion(bool value)
{
    if (value == needCheckForNewAppVersion()) {
        return;
    }

    configuration()->setNeedCheckForUpdate(value);
    emit needCheckForNewAppVersionChanged(value);
}
