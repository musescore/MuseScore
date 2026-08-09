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
#include "organpedalmarkspreferencesmodel.h"
#include "translation.h"
#include "engraving/types/symnames.h"

using namespace muse;
using namespace mu::preferences;
using namespace mu::engraving;

static const SymIdList DEFAULT_ORGAN_PEDAL_MARKS {
    SymId::keyboardPedalToe2,
    SymId::keyboardPedalHeel1,
    SymId::keyboardPedalHeel3,
    SymId::keyboardPedalToe1,
    SymId::keyboardPedalHeel2,
};

OrganPedalMarksPreferencesModel::OrganPedalMarksPreferencesModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void OrganPedalMarksPreferencesModel::load()
{
    configuration()->organPedalMarksPlacementChanged().onNotify(this, [this]() {
        emit placementChanged();
    });

    configuration()->organPedalMarksPopupSetChanged().onNotify(this, [this]() {
        emit popupSetChanged();
    });

    configuration()->organPedalMarksNavigationPlacementChanged().onNotify(this, [this]() {
        emit navigationPlacementChanged();
    });

    configuration()->organPedalMarksDefaultPedalMarkChanged().onNotify(this, [this]() {
        emit defaultPedalMarkChanged();
    });
}

QVariantList OrganPedalMarksPreferencesModel::placements() const
{
    static const QVariantList _placements {
        QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/organpedalmark", "Above") },
            { QStringLiteral("value"), int(PlacementV::ABOVE) }
        },
        QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/organpedalmark", "Below") },
            { QStringLiteral("value"), int(PlacementV::BELOW) }
        },
    };

    return _placements;
}

int OrganPedalMarksPreferencesModel::placement() const
{
    return static_cast<int>(configuration()->organPedalMarksPlacement());
}

void OrganPedalMarksPreferencesModel::setPlacement(int newPlacement)
{
    if (newPlacement != placement()) {
        configuration()->setOrganPedalMarksPlacement(static_cast<PlacementV>(newPlacement));
    }
}

QVariantList OrganPedalMarksPreferencesModel::popupSets() const
{
    static const QVariantList _popupSets {
        QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/organpedalmark", "Default") },
            { QStringLiteral("value"), int(OrganPedalMarksPopupSet::DEFAULT) }
        },
        QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/organpedalmark", "Concise (without inverted pedal marks)") },
            { QStringLiteral("value"), int(OrganPedalMarksPopupSet::CONCISE) }
        },
    };

    return _popupSets;
}

int OrganPedalMarksPreferencesModel::popupSet() const
{
    return static_cast<int>(configuration()->organPedalMarksPopupSet());
}

void OrganPedalMarksPreferencesModel::setPopupSet(int newPopupSet)
{
    if (newPopupSet != popupSet()) {
        configuration()->setOrganPedalMarksPopupSet(static_cast<OrganPedalMarksPopupSet>(newPopupSet));
    }
}

QVariantList OrganPedalMarksPreferencesModel::navigationPlacements() const
{
    static const QVariantList _navigationPlacements {
        QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/organpedalmark", "Above") },
            { QStringLiteral("value"), int(OrganPedalMarksNavigationPlacement::ABOVE) }
        },
        QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/organpedalmark", "Below") },
            { QStringLiteral("value"), int(OrganPedalMarksNavigationPlacement::BELOW) }
        },
        QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/organpedalmark", "Alternating") },
            { QStringLiteral("value"), int(OrganPedalMarksNavigationPlacement::ALTERNATING) }
        },
        QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/organpedalmark", "As previous") },
            { QStringLiteral("value"), int(OrganPedalMarksNavigationPlacement::AS_PREVIOUS) }
        },
    };

    return _navigationPlacements;
}

int OrganPedalMarksPreferencesModel::navigationPlacement() const
{
    return static_cast<int>(configuration()->organPedalMarksNavigationPlacement());
}

void OrganPedalMarksPreferencesModel::setNavigationPlacement(int newNavigationPlacement)
{
    if (newNavigationPlacement != navigationPlacement()) {
        configuration()->setOrganPedalMarksNavigationPlacement(static_cast<OrganPedalMarksNavigationPlacement>(newNavigationPlacement));
    }
}

QVariantList OrganPedalMarksPreferencesModel::pedalMarks() const
{
    QVariantList _pedalMarks;

    for (SymId pedalMarkSymId : DEFAULT_ORGAN_PEDAL_MARKS) {
        _pedalMarks.append(QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/organpedalmark", SymNames::translatedUserNameForSymId(pedalMarkSymId)) },
            { QStringLiteral("value"), int(pedalMarkSymId) }
        });
    }

    return _pedalMarks;
}

int OrganPedalMarksPreferencesModel::defaultPedalMark() const
{
    return static_cast<int>(configuration()->organPedalMarksDefaultPedalMark());
}

void OrganPedalMarksPreferencesModel::setDefaultPedalMark(int pedalMark)
{
    if (pedalMark != defaultPedalMark()) {
        configuration()->setOrganPedalMarksDefaultPedalMark(static_cast<SymId>(pedalMark));
    }
}