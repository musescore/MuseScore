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

#include "log.h"
#include "translation.h"

using namespace mu::inspector;
using namespace mu::notation;

ScoreSettingsModel::ScoreSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_SCORE_DISPLAY);
    setTitle(qtrc("inspector", "Show"));
    createProperties();

    m_shouldShowInvisible = isNotationExisting() ? scoreConfig().isShowInvisibleElements : true;
    m_shouldShowFormatting = isNotationExisting() ? scoreConfig().isShowUnprintableElements : true;
    m_shouldShowFrames = isNotationExisting() ? scoreConfig().isShowFrames : true;
    m_shouldShowPageMargins = isNotationExisting() ? scoreConfig().isShowPageMargins : false;

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
    emit shouldShowFormattingChanged(shouldShowFormatting());
    emit shouldShowFramesChanged(shouldShowFrames());
    emit shouldShowPageMarginsChanged(shouldShowPageMargins());
}

void ScoreSettingsModel::resetProperties()
{
    setShouldShowInvisible(false);
    setShouldShowFormatting(false);
    setShouldShowFrames(false);
    setShouldShowPageMargins(false);
}

ScoreConfig ScoreSettingsModel::scoreConfig() const
{
    IF_ASSERT_FAILED(context()) {
        return ScoreConfig();
    }

    if (!context()->currentNotation()) {
        return ScoreConfig();
    }

    return context()->currentNotation()->interaction()->scoreConfig();
}

mu::async::Channel<ScoreConfigType> ScoreSettingsModel::scoreConfigChanged() const
{
    IF_ASSERT_FAILED(context()) {
        return mu::async::Channel<ScoreConfigType>();
    }

    if (!context()->currentNotation()) {
        return mu::async::Channel<ScoreConfigType>();
    }

    return context()->currentNotation()->interaction()->scoreConfigChanged();
}

bool ScoreSettingsModel::shouldShowInvisible() const
{
    return m_shouldShowInvisible;
}

bool ScoreSettingsModel::shouldShowFormatting() const
{
    return m_shouldShowFormatting;
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

    dispatcher()->dispatch("show-invisible");
    updateShouldShowInvisible(shouldShowInvisible);
}

void ScoreSettingsModel::setShouldShowFormatting(bool shouldShowFormatting)
{
    if (m_shouldShowFormatting == shouldShowFormatting) {
        return;
    }

    dispatcher()->dispatch("show-unprintable");
    updateShouldShowFormatting(shouldShowFormatting);
}

void ScoreSettingsModel::setShouldShowFrames(bool shouldShowFrames)
{
    if (m_shouldShowFrames == shouldShowFrames) {
        return;
    }

    dispatcher()->dispatch("show-frames");
    updateShouldShowFrames(shouldShowFrames);
}

void ScoreSettingsModel::setShouldShowPageMargins(bool shouldShowPageMargins)
{
    if (m_shouldShowPageMargins == shouldShowPageMargins) {
        return;
    }

    dispatcher()->dispatch("show-pageborders");
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

void ScoreSettingsModel::updateShouldShowFormatting(bool isVisible)
{
    if (isVisible == m_shouldShowFormatting) {
        return;
    }

    m_shouldShowFormatting = isVisible;
    emit shouldShowFormattingChanged(isVisible);
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

void ScoreSettingsModel::updateFromConfig(ScoreConfigType configType)
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
    default:
        break;
    }
}

void ScoreSettingsModel::updateAll()
{
    updateShouldShowInvisible(scoreConfig().isShowInvisibleElements);
    updateShouldShowFormatting(scoreConfig().isShowUnprintableElements);
    updateShouldShowFrames(scoreConfig().isShowFrames);
    updateShouldShowPageMargins(scoreConfig().isShowPageMargins);
}

void ScoreSettingsModel::setupConnections()
{
    if (isNotationExisting()) {
        updateAll();

        scoreConfigChanged().onReceive(this, [this](ScoreConfigType configType) {
            updateFromConfig(configType);
        });
    }

    currentNotationChanged().onNotify(this, [this]() {
        updateAll();

        scoreConfigChanged().onReceive(this, [this](ScoreConfigType configType) {
            updateFromConfig(configType);
        });
    });
}
