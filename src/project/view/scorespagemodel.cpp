/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "scorespagemodel.h"

#include <QString>
#include <QSet>

#include "actions/actiontypes.h"
#include "notation/notationtypes.h"
#include "types/projecttypes.h"
#include "translation.h"

using namespace mu::project;
using namespace muse::actions;

ScoresPageModel::ScoresPageModel(QObject* parent)
    : QObject(parent)
{
}

void ScoresPageModel::createNewScore()
{
    dispatcher()->dispatch("file-new");
}

void ScoresPageModel::openOther()
{
    dispatcher()->dispatch("file-open");
}

void ScoresPageModel::openScore(const QString& scorePath, const QString& displayNameOverride)
{
    dispatcher()->dispatch("file-open", ActionData::make_arg2<QUrl, QString>(QUrl::fromLocalFile(scorePath), displayNameOverride));
}

void ScoresPageModel::openScoreManager()
{
    interactive()->openUrl(museScoreComService()->scoreManagerUrl());
}

int ScoresPageModel::tabIndex() const
{
    return configuration()->homeScoresPageTabIndex();
}

void ScoresPageModel::setTabIndex(int index)
{
    if (index == tabIndex()) {
        return;
    }

    configuration()->setHomeScoresPageTabIndex(index);
    emit tabIndexChanged();
}

ScoresPageModel::ViewType ScoresPageModel::viewType() const
{
    return static_cast<ViewType>(configuration()->homeScoresPageViewType());
}

void ScoresPageModel::setViewType(ViewType type)
{
    if (viewType() == type) {
        return;
    }

    configuration()->setHomeScoresPageViewType(IProjectConfiguration::HomeScoresPageViewType(type));
    emit viewTypeChanged();
}

QVariantList ScoresPageModel::familyFilterOptions() const
{
    QVariantList options;

    // "All" option first
    options << QVariantMap{{ "id", "" }, { "name", qtrc("project", "All families") }};

    // Collect unique families from all instrument templates
    QSet<QString> seenFamilies;
    QList<QPair<QString, QString>> familyList;  // id, name pairs for sorting

    for (const notation::InstrumentTemplate* templ : instrumentsRepository()->instrumentTemplates()) {
        if (templ->family && !seenFamilies.contains(templ->family->id.toQString())) {
            seenFamilies.insert(templ->family->id.toQString());
            familyList.append({ templ->family->id.toQString(), templ->family->name.toQString() });
        }
    }

    // Sort alphabetically by name
    std::sort(familyList.begin(), familyList.end(), [](const auto& a, const auto& b) {
        return a.second.compare(b.second, Qt::CaseInsensitive) < 0;
    });

    for (const auto& family : familyList) {
        options << QVariantMap{{ "id", family.first }, { "name", family.second }};
    }

    // Add "Other" at end
    options << QVariantMap{{ "id", OTHER_FAMILY_ID }, { "name", qtrc("project", "Other") }};

    return options;
}

QVariantList ScoresPageModel::instrumentFilterOptions() const
{
    QVariantList options;
    options << QVariantMap{{ "id", "" }, { "name", qtrc("project", "All instruments") }};

    muse::String selectedFamily = muse::String::fromQString(m_selectedFamilyId);
    QSet<QString> seenInstruments;
    QList<QPair<QString, QString>> instrumentList;

    for (const notation::InstrumentTemplate* templ : instrumentsRepository()->instrumentTemplates()) {
        // If family selected, only show instruments in that family
        bool matchesFamily = m_selectedFamilyId.isEmpty()
                             || (templ->family && templ->family->id == selectedFamily);

        if (matchesFamily && !seenInstruments.contains(templ->id.toQString())) {
            seenInstruments.insert(templ->id.toQString());
            instrumentList.append({ templ->id.toQString(), templ->trackName.toQString() });
        }
    }

    // Sort alphabetically by name
    std::sort(instrumentList.begin(), instrumentList.end(), [](const auto& a, const auto& b) {
        return a.second.compare(b.second, Qt::CaseInsensitive) < 0;
    });

    for (const auto& instrument : instrumentList) {
        options << QVariantMap{{ "id", instrument.first }, { "name", instrument.second }};
    }

    return options;
}

QString ScoresPageModel::selectedFamilyId() const
{
    return m_selectedFamilyId;
}

void ScoresPageModel::setSelectedFamilyId(const QString& familyId)
{
    if (m_selectedFamilyId == familyId) {
        return;
    }

    m_selectedFamilyId = familyId;
    emit selectedFamilyIdChanged();
    emit instrumentFilterOptionsChanged();

    // Reset instrument filter when family changes
    setSelectedInstrumentId("");
}

QString ScoresPageModel::selectedInstrumentId() const
{
    return m_selectedInstrumentId;
}

void ScoresPageModel::setSelectedInstrumentId(const QString& instrumentId)
{
    if (m_selectedInstrumentId == instrumentId) {
        return;
    }

    m_selectedInstrumentId = instrumentId;
    emit selectedInstrumentIdChanged();
}
