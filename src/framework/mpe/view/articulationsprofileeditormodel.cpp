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

#include "articulationsprofileeditormodel.h"

#include "global/translation.h"

using namespace mu;
using namespace mu::mpe;

static const QString PROFILE_EXTENSION = "(*.json)";

ArticulationsProfileEditorModel::ArticulationsProfileEditorModel(QObject* parent)
    : QAbstractListModel(parent)
{
    setProfile(profilesRepository()->createNew());
}

void ArticulationsProfileEditorModel::requestToOpenProfile()
{
    QString filter = qtrc("mpe", "MPE Articulations Profile") + " (*.json)";
    m_profilePath = interactive()->selectOpeningFile(qtrc("mpe", "Open MPE Articulations Profile"), "", filter);

    setProfile(profilesRepository()->loadProfile(m_profilePath));
}

void ArticulationsProfileEditorModel::requestToSaveProfile()
{
    QString filter = qtrc("mpe", "MPE Articulations Profile") + " " + PROFILE_EXTENSION;

    if (m_profilePath.empty()) {
        m_profilePath = interactive()->selectSavingFile(qtrc("mpe", "Save MPE Articulations Profile"), "", filter);
    }

    profilesRepository()->saveProfile(m_profilePath, nullptr);
}

int ArticulationsProfileEditorModel::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QVariant ArticulationsProfileEditorModel::data(const QModelIndex& index, int /*role*/) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    return QVariant::fromValue(m_items.at(index.row()));
}

QHash<int, QByteArray> ArticulationsProfileEditorModel::roleNames() const
{
    static QHash<int, QByteArray> roles = {
        { PatternsScopeItem, "scopeItem" }
    };

    return roles;
}

void ArticulationsProfileEditorModel::setProfile(ArticulationsProfilePtr ptr)
{
    m_profile = std::move(ptr);

    loadItems();
}

void ArticulationsProfileEditorModel::loadItems()
{
    if (!m_profile) {
        return;
    }

    beginResetModel();

    if (!m_profile->isValid()) {
        m_items << new ArticulationPatternsScopeItem(this, ArticulationType::None);
    } else {
        for (const auto& pair : m_profile->data()) {
            m_items << new ArticulationPatternsScopeItem(this, pair.first, pair.second);
        }
    }

    endResetModel();
}
