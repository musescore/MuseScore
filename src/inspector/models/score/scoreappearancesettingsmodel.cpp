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
#include "scoreappearancesettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::notation;

ScoreAppearanceSettingsModel::ScoreAppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_SCORE_APPEARANCE);
    setTitle(muse::qtrc("inspector", "Score appearance"));
}

bool ScoreAppearanceSettingsModel::hideEmptyStaves() const
{
    return styleValue(StyleId::hideEmptyStaves).toBool();
}

void ScoreAppearanceSettingsModel::setHideEmptyStaves(bool hide)
{
    if (updateStyleValue(StyleId::hideEmptyStaves, hide)) {
        emit hideEmptyStavesChanged();
    }
}

bool ScoreAppearanceSettingsModel::dontHideEmptyStavesInFirstSystem() const
{
    return styleValue(StyleId::dontHideStavesInFirstSystem).toBool();
}

void ScoreAppearanceSettingsModel::setDontHideEmptyStavesInFirstSystem(bool dont)
{
    if (updateStyleValue(StyleId::dontHideStavesInFirstSystem, dont)) {
        emit dontHideEmptyStavesInFirstSystemChanged();
    }
}

bool ScoreAppearanceSettingsModel::showBracketsWhenSpanningSingleStaff() const
{
    return styleValue(StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden).toBool();
}

void ScoreAppearanceSettingsModel::setShowBracketsWhenSpanningSingleStaff(bool show)
{
    if (updateStyleValue(StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden, show)) {
        emit showBracketsWhenSpanningSingleStaffChanged();
    }
}

bool ScoreAppearanceSettingsModel::isEmpty() const
{
    return !isNotationExisting();
}

void ScoreAppearanceSettingsModel::onCurrentNotationChanged()
{
    emit hideEmptyStavesChanged();
    emit dontHideEmptyStavesInFirstSystemChanged();
    emit showBracketsWhenSpanningSingleStaffChanged();
}

void ScoreAppearanceSettingsModel::onNotationChanged(const engraving::PropertyIdSet&, const engraving::StyleIdSet& changedStyleIdSet)
{
    if (muse::contains(changedStyleIdSet, StyleId::hideEmptyStaves)) {
        emit hideEmptyStavesChanged();
    }

    if (muse::contains(changedStyleIdSet, StyleId::dontHideStavesInFirstSystem)) {
        emit dontHideEmptyStavesInFirstSystemChanged();
    }

    if (muse::contains(changedStyleIdSet, StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden)) {
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
