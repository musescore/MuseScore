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

    auto onCurrentNotationChanged = [this]() {
        emit styleChanged();

        if (style()) {
            style()->styleChanged().onNotify(this, [this]() {
                emit styleChanged();
            });
        }
    };

    onCurrentNotationChanged();
    context()->currentNotationChanged().onNotify(this, onCurrentNotationChanged);
}

bool ScoreAppearanceSettingsModel::hideEmptyStaves() const
{
    return style() ? style()->styleValue(StyleId::hideEmptyStaves).toBool() : false;
}

void ScoreAppearanceSettingsModel::setHideEmptyStaves(bool hide)
{
    if (hide == hideEmptyStaves() || !style()) {
        return;
    }

    style()->setStyleValue(StyleId::hideEmptyStaves, hide);
}

bool ScoreAppearanceSettingsModel::dontHideEmptyStavesInFirstSystem() const
{
    return style() ? style()->styleValue(StyleId::dontHideStavesInFirstSystem).toBool() : false;
}

void ScoreAppearanceSettingsModel::setDontHideEmptyStavesInFirstSystem(bool dont)
{
    if (dont == dontHideEmptyStavesInFirstSystem() || !style()) {
        return;
    }

    style()->setStyleValue(StyleId::dontHideStavesInFirstSystem, dont);
}

bool ScoreAppearanceSettingsModel::showBracketsWhenSpanningSingleStaff() const
{
    return style() ? style()->styleValue(StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden).toBool() : false;
}

void ScoreAppearanceSettingsModel::setShowBracketsWhenSpanningSingleStaff(bool show)
{
    if (show == showBracketsWhenSpanningSingleStaff() || !style()) {
        return;
    }

    style()->setStyleValue(StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden, show);
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
