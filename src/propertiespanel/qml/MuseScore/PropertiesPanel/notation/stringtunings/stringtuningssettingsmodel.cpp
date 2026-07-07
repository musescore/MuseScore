/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "stringtuningssettingsmodel.h"

#include "notation/inotationselection.h"

#include "translation.h"

using namespace mu::propertiespanel;
using namespace mu::notation;

StringTuningsSettingsModel::StringTuningsSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                                       IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setModelType(PropertiesPanelModelType::TYPE_STRING_TUNINGS);
    setTitle(muse::qtrc("propertiespanel", "Fretted instruments"));
}

bool StringTuningsSettingsModel::isEmpty() const
{
    INotationSelectionPtr selection = this->selection();
    return !selection || !selection->isRange();
}

void StringTuningsSettingsModel::editStrings()
{
    dispatcher()->dispatch("edit-strings");
}
