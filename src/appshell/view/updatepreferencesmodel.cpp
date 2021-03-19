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

void UpdatePreferencesModel::load()
{
    emit needCheckForNewMuseScoreVersionChanged(needCheckForNewMuseScoreVersion());
    emit needCheckForNewExtensionsVersionChanged(needCheckForNewExtensionsVersion());
}

bool UpdatePreferencesModel::needCheckForNewMuseScoreVersion() const
{
    return false;
}

bool UpdatePreferencesModel::needCheckForNewExtensionsVersion() const
{
    return false;
}

void UpdatePreferencesModel::setNeedCheckForNewMuseScoreVersion(bool value)
{
    NOT_IMPLEMENTED;

    if (value == needCheckForNewMuseScoreVersion()) {
        return;
    }

    emit needCheckForNewMuseScoreVersionChanged(value);
}

void UpdatePreferencesModel::setNeedCheckForNewExtensionsVersion(bool value)
{
    NOT_IMPLEMENTED;

    if (value == needCheckForNewExtensionsVersion()) {
        return;
    }

    emit needCheckForNewExtensionsVersionChanged(value);
}
