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
#include "scoreappearancesettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::notation;

ScoreAppearanceSettingsModel::ScoreAppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_SCORE_APPEARANCE);
    setTitle(qtrc("inspector", "Score appearance"));
}

bool ScoreAppearanceSettingsModel::hideEmptyStaves() const
{
    return styleValue(StyleId::hideEmptyStaves).toBool();
}

void ScoreAppearanceSettingsModel::setHideEmptyStaves(bool hide)
{
    if (setStyleValue(StyleId::hideEmptyStaves, hide)) {
        emit hideEmptyStavesChanged();
    }
}

bool ScoreAppearanceSettingsModel::dontHideEmptyStavesInFirstSystem() const
{
    return styleValue(StyleId::dontHideStavesInFirstSystem).toBool();
}

void ScoreAppearanceSettingsModel::setDontHideEmptyStavesInFirstSystem(bool dont)
{
    if (setStyleValue(StyleId::dontHideStavesInFirstSystem, dont)) {
        emit dontHideEmptyStavesInFirstSystemChanged();
    }
}

bool ScoreAppearanceSettingsModel::showBracketsWhenSpanningSingleStaff() const
{
    return styleValue(StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden).toBool();
}

void ScoreAppearanceSettingsModel::setShowBracketsWhenSpanningSingleStaff(bool show)
{
    if (setStyleValue(StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden, show)) {
        emit showBracketsWhenSpanningSingleStaffChanged();
    }
}

bool ScoreAppearanceSettingsModel::isEmpty() const
{
    return !isNotationExisting();
}

void ScoreAppearanceSettingsModel::onCurrentNotationChanged()
{
    AbstractInspectorModel::onCurrentNotationChanged();

    emit hideEmptyStavesChanged();
    emit dontHideEmptyStavesInFirstSystemChanged();
    emit showBracketsWhenSpanningSingleStaffChanged();
}

void ScoreAppearanceSettingsModel::onNotationChanged(const engraving::PropertyIdSet&, const engraving::StyleIdSet& changedStyleIdSet)
{
    if (mu::contains(changedStyleIdSet, StyleId::hideEmptyStaves)) {
        emit hideEmptyStavesChanged();
    }

    if (mu::contains(changedStyleIdSet, StyleId::dontHideStavesInFirstSystem)) {
        emit dontHideEmptyStavesInFirstSystemChanged();
    }

    if (mu::contains(changedStyleIdSet, StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden)) {
        emit showBracketsWhenSpanningSingleStaffChanged();
    }
}

void ScoreAppearanceSettingsModel::showPageSettings()
{
    dispatcher()->dispatch("page-settings");
}

void ScoreAppearanceSettingsModel::showStyleSettings()
{
    dispatcher()->dispatch("edit-style");
}
