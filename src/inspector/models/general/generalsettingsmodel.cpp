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
using namespace mu::engraving;

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
    m_isVisible = buildPropertyItem(Pid::VISIBLE, [this](const mu::engraving::Pid, const QVariant& newValue) {
        onVisibleChanged(newValue.toBool());
    });

    m_isSmall = buildPropertyItem(Pid::SMALL, [this](const mu::engraving::Pid, const QVariant& newValue) {
        setPropertyValue(m_elementsForIsSmallProperty, Pid::SMALL, newValue.toBool());
    });

    m_isAutoPlaceAllowed = buildPropertyItem(Pid::AUTOPLACE);
    m_isPlayable = buildPropertyItem(Pid::PLAY);
}

void GeneralSettingsModel::requestElements()
{
    m_elementList = m_repository->takeAllElements();

    QSet<EngravingItem*> elementsForIsSmallProperty;

    for (EngravingItem* element : m_elementList) {
        // Trill cue note is small by definition, so isSmall property does not apply
        if (element->isNote() && toNote(element)->isTrillCueNote()) {
            continue;
        }

        EngravingItem* chord = element->findAncestor(ElementType::CHORD);

        if (chord) {
            elementsForIsSmallProperty.insert(chord);
        } else {
            elementsForIsSmallProperty.insert(element);
        }
    }

    m_elementsForIsSmallProperty = elementsForIsSmallProperty.values();
}

void GeneralSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::VISIBLE,
        Pid::AUTOPLACE,
        Pid::PLAY,
        Pid::SMALL,
    };

    loadProperties(propertyIdSet);
}

void GeneralSettingsModel::resetProperties()
{
    m_isVisible->resetToDefault();
    m_isAutoPlaceAllowed->resetToDefault();
    m_isPlayable->resetToDefault();
    m_isSmall->resetToDefault();
}

void GeneralSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void GeneralSettingsModel::loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet)
{
    if (mu::contains(propertyIdSet, Pid::VISIBLE)) {
        loadPropertyItem(m_isVisible);
    }

    if (mu::contains(propertyIdSet, Pid::AUTOPLACE)) {
        loadPropertyItem(m_isAutoPlaceAllowed);
    }

    if (mu::contains(propertyIdSet, Pid::PLAY)) {
        bool isMaster = isMasterNotation();
        m_isPlayable->setIsVisible(isMaster);

        if (isMaster) {
            loadPropertyItem(m_isPlayable);
        }
    }

    if (mu::contains(propertyIdSet, Pid::SMALL)) {
        loadPropertyItem(m_isSmall, m_elementsForIsSmallProperty);
    }
}

void GeneralSettingsModel::onVisibleChanged(bool visible)
{
    beginCommand();

    Score* score = currentNotation()->elements()->msScore();

    for (EngravingItem* item : m_elementList) {
        score->undoChangeVisible(item, visible);
    }

    updateNotation();
    endCommand();
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
