//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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

bool UpdatePreferencesModel::needCheckForNewExtensionsVersion() const
{
    return extensionsConfiguration()->needCheckForUpdate();
}

void UpdatePreferencesModel::setNeedCheckForNewAppVersion(bool value)
{
    if (value == needCheckForNewAppVersion()) {
        return;
    }

    configuration()->setNeedCheckForUpdate(value);
    emit needCheckForNewAppVersionChanged(value);
}

void UpdatePreferencesModel::setNeedCheckForNewExtensionsVersion(bool value)
{
    if (value == needCheckForNewExtensionsVersion()) {
        return;
    }

    extensionsConfiguration()->setNeedCheckForUpdate(value);
    emit needCheckForNewExtensionsVersionChanged(value);
}
