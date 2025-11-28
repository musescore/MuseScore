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
#include "scoredisplaysettingsmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::inspector;
using namespace mu::notation;

ScoreDisplaySettingsModel::ScoreDisplaySettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_SCORE_DISPLAY);
    setTitle(muse::qtrc("inspector", "Show"));
    createProperties();
}

void ScoreDisplaySettingsModel::onCurrentNotationChanged()
{
    updateAll();

    if (auto notation = currentNotation()) {
        notation->interaction()->scoreConfigChanged().onReceive(this, [this](ScoreConfigType configType) {
            updateFromConfig(configType);
        }, Asyncable::Mode::SetReplace);
    }
}

void ScoreDisplaySettingsModel::createProperties()
{
    updateAll();
}

void ScoreDisplaySettingsModel::requestElements()
{
    //!Note the model work only with the parent score, no need to request other child elements
}

bool ScoreDisplaySettingsModel::isEmpty() const
{
    return !isNotationExisting();
}

void ScoreDisplaySettingsModel::loadProperties()
{
    updateAll();
}

void ScoreDisplaySettingsModel::resetProperties()
{
    setShouldShowInvisible(false);
    setShouldShowFormatting(false);
    setShouldShowFrames(false);
    setShouldShowPageMargins(false);
}

ScoreConfig ScoreDisplaySettingsModel::scoreConfig() const
{
    if (!currentNotation()) {
        return ScoreConfig();
    }

    return currentNotation()->interaction()->scoreConfig();
}

bool ScoreDisplaySettingsModel::shouldShowInvisible() const
{
    return m_shouldShowInvisible;
}

bool ScoreDisplaySettingsModel::shouldShowFormatting() const
{
    return m_shouldShowFormatting;
}

bool ScoreDisplaySettingsModel::shouldShowFrames() const
{
    return m_shouldShowFrames;
}

bool ScoreDisplaySettingsModel::shouldShowPageMargins() const
{
    return m_shouldShowPageMargins;
}

bool ScoreDisplaySettingsModel::shouldShowSoundFlags() const
{
    return m_shouldShowSoundFlags;
}

void ScoreDisplaySettingsModel::setShouldShowInvisible(bool shouldShowInvisible)
{
    if (m_shouldShowInvisible == shouldShowInvisible) {
        return;
    }

    dispatcher()->dispatch("show-invisible");
    updateShouldShowInvisible(shouldShowInvisible);
}

void ScoreDisplaySettingsModel::setShouldShowFormatting(bool shouldShowFormatting)
{
    if (m_shouldShowFormatting == shouldShowFormatting) {
        return;
    }

    dispatcher()->dispatch("show-unprintable");
    updateShouldShowFormatting(shouldShowFormatting);
}

void ScoreDisplaySettingsModel::setShouldShowFrames(bool shouldShowFrames)
{
    if (m_shouldShowFrames == shouldShowFrames) {
        return;
    }

    dispatcher()->dispatch("show-frames");
    updateShouldShowFrames(shouldShowFrames);
}

void ScoreDisplaySettingsModel::setShouldShowPageMargins(bool shouldShowPageMargins)
{
    if (m_shouldShowPageMargins == shouldShowPageMargins) {
        return;
    }

    dispatcher()->dispatch("show-pageborders");
    updateShouldShowPageMargins(shouldShowPageMargins);
}

void ScoreDisplaySettingsModel::setShouldShowSoundFlags(bool shouldShowSoundFlags)
{
    if (m_shouldShowSoundFlags == shouldShowSoundFlags) {
        return;
    }

    dispatcher()->dispatch("show-soundflags");
    updateShouldShowSoundFlags(shouldShowSoundFlags);
}

void ScoreDisplaySettingsModel::updateShouldShowInvisible(bool isVisible)
{
    if (isVisible == m_shouldShowInvisible) {
        return;
    }

    m_shouldShowInvisible = isVisible;
    emit shouldShowInvisibleChanged(isVisible);
}

void ScoreDisplaySettingsModel::updateShouldShowFormatting(bool isVisible)
{
    if (isVisible == m_shouldShowFormatting) {
        return;
    }

    m_shouldShowFormatting = isVisible;
    emit shouldShowFormattingChanged(isVisible);
}

void ScoreDisplaySettingsModel::updateShouldShowFrames(bool isVisible)
{
    if (isVisible == m_shouldShowFrames) {
        return;
    }

    m_shouldShowFrames = isVisible;
    emit shouldShowFramesChanged(isVisible);
}

void ScoreDisplaySettingsModel::updateShouldShowPageMargins(bool isVisible)
{
    if (isVisible == m_shouldShowPageMargins) {
        return;
    }

    m_shouldShowPageMargins = isVisible;
    emit shouldShowPageMarginsChanged(isVisible);
}

void ScoreDisplaySettingsModel::updateShouldShowSoundFlags(bool isVisible)
{
    if (isVisible == m_shouldShowSoundFlags) {
        return;
    }

    m_shouldShowSoundFlags = isVisible;
    emit shouldShowSoundFlagsChanged(isVisible);
}

void ScoreDisplaySettingsModel::updateFromConfig(ScoreConfigType configType)
{
    switch (configType) {
    case notation::ScoreConfigType::ShowInvisibleElements:
        updateShouldShowInvisible(scoreConfig().isShowInvisibleElements);
        break;
    case notation::ScoreConfigType::ShowUnprintableElements:
        updateShouldShowFormatting(scoreConfig().isShowUnprintableElements);
        break;
    case notation::ScoreConfigType::ShowFrames:
        updateShouldShowFrames(scoreConfig().isShowFrames);
        break;
    case notation::ScoreConfigType::ShowPageMargins:
        updateShouldShowPageMargins(scoreConfig().isShowPageMargins);
        break;
    case notation::ScoreConfigType::ShowSoundFlags:
        updateShouldShowSoundFlags(scoreConfig().isShowSoundFlags);
        break;
    default:
        break;
    }
}

void ScoreDisplaySettingsModel::updateAll()
{
    auto config = scoreConfig();

    updateShouldShowInvisible(config.isShowInvisibleElements);
    updateShouldShowFormatting(config.isShowUnprintableElements);
    updateShouldShowFrames(config.isShowFrames);
    updateShouldShowSoundFlags(config.isShowSoundFlags);
    updateShouldShowPageMargins(config.isShowPageMargins);
}
