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
#include "generalsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

GeneralSettingsModel::GeneralSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    createProperties();

    setTitle(qtrc("inspector", "General"));
    setSectionType(InspectorSectionType::SECTION_GENERAL);
    setPlaybackProxyModel(new PlaybackProxyModel(this, repository));
    setAppearanceSettingsModel(new AppearanceSettingsModel(this, repository));
}

void GeneralSettingsModel::createProperties()
{
    m_isVisible = buildPropertyItem(mu::engraving::Pid::VISIBLE);
    m_isAutoPlaceAllowed = buildPropertyItem(mu::engraving::Pid::AUTOPLACE);
    m_isPlayable = buildPropertyItem(mu::engraving::Pid::PLAY);
    m_isSmall = buildPropertyItem(mu::engraving::Pid::SMALL);
}

void GeneralSettingsModel::requestElements()
{
    m_elementList = m_repository->takeAllElements();
}

void GeneralSettingsModel::loadProperties()
{
    loadPropertyItem(m_isVisible);
    loadPropertyItem(m_isAutoPlaceAllowed);
    loadPropertyItem(m_isPlayable);
    loadPropertyItem(m_isSmall);
}

void GeneralSettingsModel::resetProperties()
{
    m_isVisible->resetToDefault();
    m_isAutoPlaceAllowed->resetToDefault();
    m_isPlayable->resetToDefault();
    m_isSmall->resetToDefault();
}

PropertyItem* GeneralSettingsModel::isVisible() const
{
    return m_isVisible;
}

PropertyItem* GeneralSettingsModel::isAutoPlaceAllowed() const
{
    return m_isAutoPlaceAllowed;
}

PropertyItem* GeneralSettingsModel::isPlayable() const
{
    return m_isPlayable;
}

PropertyItem* GeneralSettingsModel::isSmall() const
{
    return m_isSmall;
}

QObject* GeneralSettingsModel::playbackProxyModel() const
{
    return m_playbackProxyModel;
}

QObject* GeneralSettingsModel::appearanceSettingsModel() const
{
    return m_appearanceSettingsModel;
}

void GeneralSettingsModel::setPlaybackProxyModel(PlaybackProxyModel* playbackProxyModel)
{
    m_playbackProxyModel = playbackProxyModel;
    emit playbackProxyModelChanged(m_playbackProxyModel);
}

void GeneralSettingsModel::setAppearanceSettingsModel(AppearanceSettingsModel* appearanceSettingsModel)
{
    m_appearanceSettingsModel = appearanceSettingsModel;
    emit appearanceSettingsModelChanged(m_appearanceSettingsModel);
}
