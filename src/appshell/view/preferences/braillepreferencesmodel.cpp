/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "braillepreferencesmodel.h"

#include "log.h"

using namespace mu::appshell;

BraillePreferencesModel::BraillePreferencesModel(QObject* parent)
    : QObject(parent)
{
}

bool BraillePreferencesModel::braillePanelEnabled() const
{
    return brailleConfiguration()->braillePanelEnabled();
}

void BraillePreferencesModel::setBraillePanelEnabled(bool value)
{
    if (value == braillePanelEnabled()) {
        return;
    }

    brailleConfiguration()->setBraillePanelEnabled(value);
    emit braillePanelEnabledChanged(value);
}

QString BraillePreferencesModel::brailleTable() const
{
    return brailleConfiguration()->brailleTable();
}

void BraillePreferencesModel::setBrailleTable(QString table)
{
    if (table == brailleTable()) {
        return;
    }

    brailleConfiguration()->setBrailleTable(table);
    emit brailleTableChanged(table);
}

QStringList BraillePreferencesModel::tables() const
{
    return brailleConfiguration()->brailleTableList();
}
