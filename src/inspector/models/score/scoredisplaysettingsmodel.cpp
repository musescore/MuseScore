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
#include "scoredisplaysettingsmodel.h"

using namespace mu::inspector;

ScoreSettingsModel::ScoreSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_SCORE_DISPLAY);
    setTitle(qtrc("inspector", "Show"));
    createProperties();

    m_shouldShowInvisible = adapter()->isNotationExisting() ? adapter()->scoreConfig().isShowInvisibleElements : true;
    m_shouldShowUnprintable = adapter()->isNotationExisting() ? adapter()->scoreConfig().isShowUnprintableElements : true;
    m_shouldShowFrames = adapter()->isNotationExisting() ? adapter()->scoreConfig().isShowFrames : true;
    m_shouldShowPageMargins = adapter()->isNotationExisting() ? adapter()->scoreConfig().isShowPageMargins : false;

    setupConnections();
}

void ScoreSettingsModel::createProperties()
{
}

void ScoreSettingsModel::requestElements()
{
    //!Note the model work only with the parent score, no need to request other child elements
}

bool ScoreSettingsModel::hasAcceptableElements() const
{
    if (isNotationExisting()) {
        return true;
    }

    return false;
}

void ScoreSettingsModel::loadProperties()
{
    emit shouldShowInvisibleChanged(shouldShowInvisible());
    emit shouldShowUnprintableChanged(shouldShowUnprintable());
    emit shouldShowFramesChanged(shouldShowFrames());
    emit shouldShowPageMarginsChanged(shouldShowPageMargins());
}

void ScoreSettingsModel::resetProperties()
{
    setShouldShowInvisible(false);
    setShouldShowUnprintable(false);
    setShouldShowFrames(false);
    setShouldShowPageMargins(false);
}

bool ScoreSettingsModel::shouldShowInvisible() const
{
    return m_shouldShowInvisible;
}

bool ScoreSettingsModel::shouldShowUnprintable() const
{
    return m_shouldShowUnprintable;
}

bool ScoreSettingsModel::shouldShowFrames() const
{
    return m_shouldShowFrames;
}

bool ScoreSettingsModel::shouldShowPageMargins() const
{
    return m_shouldShowPageMargins;
}

void ScoreSettingsModel::setShouldShowInvisible(bool shouldShowInvisible)
{
    if (m_shouldShowInvisible == shouldShowInvisible) {
        return;
    }

    adapter()->toggleInvisibleElementsDisplaying();

    updateShouldShowInvisible(shouldShowInvisible);
}

void ScoreSettingsModel::setShouldShowUnprintable(bool shouldShowUnprintable)
{
    if (m_shouldShowUnprintable == shouldShowUnprintable) {
        return;
    }

    adapter()->toggleUnprintableElementsVisibility();

    updateShouldShowUnprintable(shouldShowUnprintable);
}

void ScoreSettingsModel::setShouldShowFrames(bool shouldShowFrames)
{
    if (m_shouldShowFrames == shouldShowFrames) {
        return;
    }

    adapter()->toggleFramesVisibility();

    updateShouldShowFrames(shouldShowFrames);
}

void ScoreSettingsModel::setShouldShowPageMargins(bool shouldShowPageMargins)
{
    if (m_shouldShowPageMargins == shouldShowPageMargins) {
        return;
    }

    adapter()->togglePageMarginsVisibility();

    updateShouldShowPageMargins(shouldShowPageMargins);
}

void ScoreSettingsModel::updateShouldShowInvisible(bool isVisible)
{
    if (isVisible == m_shouldShowInvisible) {
        return;
    }

    m_shouldShowInvisible = isVisible;
    emit shouldShowInvisibleChanged(isVisible);
}

void ScoreSettingsModel::updateShouldShowUnprintable(bool isVisible)
{
    if (isVisible == m_shouldShowUnprintable) {
        return;
    }

    m_shouldShowUnprintable = isVisible;
    emit shouldShowUnprintableChanged(isVisible);
}

void ScoreSettingsModel::updateShouldShowFrames(bool isVisible)
{
    if (isVisible == m_shouldShowFrames) {
        return;
    }

    m_shouldShowFrames = isVisible;
    emit shouldShowFramesChanged(isVisible);
}

void ScoreSettingsModel::updateShouldShowPageMargins(bool isVisible)
{
    if (isVisible == m_shouldShowPageMargins) {
        return;
    }

    m_shouldShowPageMargins = isVisible;
    emit shouldShowPageMarginsChanged(isVisible);
}

void ScoreSettingsModel::updateFromConfig(notation::ScoreConfigType configType)
{
    switch (configType) {
    case notation::ScoreConfigType::ShowInvisibleElements:
        updateShouldShowInvisible(adapter()->scoreConfig().isShowInvisibleElements);
        break;
    case notation::ScoreConfigType::ShowUnprintableElements:
        updateShouldShowUnprintable(adapter()->scoreConfig().isShowUnprintableElements);
        break;
    case notation::ScoreConfigType::ShowFrames:
        updateShouldShowFrames(adapter()->scoreConfig().isShowFrames);
        break;
    case notation::ScoreConfigType::ShowPageMargins:
        updateShouldShowPageMargins(adapter()->scoreConfig().isShowPageMargins);
        break;
    default:
        break;
    }
}

void ScoreSettingsModel::updateAll()
{
    updateShouldShowInvisible(adapter()->scoreConfig().isShowInvisibleElements);
    updateShouldShowUnprintable(adapter()->scoreConfig().isShowUnprintableElements);
    updateShouldShowFrames(adapter()->scoreConfig().isShowFrames);
    updateShouldShowPageMargins(adapter()->scoreConfig().isShowPageMargins);
}

void ScoreSettingsModel::setupConnections()
{
    if (adapter()->isNotationExisting()) {
        adapter()->currentNotationChanged().onNotify(this, [this]() {
            updateAll();
        });

        adapter()->scoreConfigChanged().onReceive(this, [this](notation::ScoreConfigType configType) {
            updateFromConfig(configType);
        });

        return;
    }

    adapter()->currentNotationChanged().onNotify(this, [this]() {
        updateAll();

        adapter()->scoreConfigChanged().onReceive(this, [this](notation::ScoreConfigType configType) {
            updateFromConfig(configType);
        });
    });
}
