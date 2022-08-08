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
    updateStyleValue(StyleId::hideEmptyStaves, hide);
}

bool ScoreAppearanceSettingsModel::dontHideEmptyStavesInFirstSystem() const
{
    return styleValue(StyleId::dontHideStavesInFirstSystem).toBool();
}

void ScoreAppearanceSettingsModel::setDontHideEmptyStavesInFirstSystem(bool dont)
{
    updateStyleValue(StyleId::dontHideStavesInFirstSystem, dont);
}

bool ScoreAppearanceSettingsModel::showBracketsWhenSpanningSingleStaff() const
{
    return styleValue(StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden).toBool();
}

void ScoreAppearanceSettingsModel::setShowBracketsWhenSpanningSingleStaff(bool show)
{
    updateStyleValue(StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden, show);
}

bool ScoreAppearanceSettingsModel::isEmpty() const
{
    return !isNotationExisting();
}

void ScoreAppearanceSettingsModel::showPageSettings()
{
    dispatcher()->dispatch("page-settings");
}

void ScoreAppearanceSettingsModel::showStyleSettings()
{
    dispatcher()->dispatch("edit-style");
}

void ScoreAppearanceSettingsModel::onStyleChanged()
{
    emit styleChanged();
}
