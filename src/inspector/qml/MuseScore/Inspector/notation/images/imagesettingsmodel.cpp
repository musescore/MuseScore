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
#include "imagesettingsmodel.h"

#include "types/commontypes.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

ImageSettingsModel::ImageSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_IMAGE);
    setTitle(muse::qtrc("inspector", "Image"));
    setIcon(muse::ui::IconCode::Code::IMAGE_MOUNTAINS);
    createProperties();
}

void ImageSettingsModel::createProperties()
{
    m_isAspectRatioLocked = buildPropertyItem(Pid::LOCK_ASPECT_RATIO);

    m_shouldScaleToFrameSize
        = buildPropertyItem(Pid::AUTOSCALE, [this](const Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        emit requestReloadPropertyItems();
    });

    m_height = buildPropertyItem(Pid::IMAGE_HEIGHT, [this](const Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        if (m_isAspectRatioLocked->value().toBool()) {
            emit requestReloadPropertyItems();
        }
    });

    m_width = buildPropertyItem(Pid::IMAGE_WIDTH, [this](const Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        if (m_isAspectRatioLocked->value().toBool()) {
            emit requestReloadPropertyItems();
        }
    });

    m_isSizeInSpatiums
        = buildPropertyItem(Pid::SIZE_IS_SPATIUM, [this](const Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        emit requestReloadPropertyItems();
    });

    m_isImageFramed = buildPropertyItem(Pid::IMAGE_FRAMED);
}

void ImageSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::IMAGE);
}

void ImageSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::LOCK_ASPECT_RATIO,
        Pid::AUTOSCALE,
        Pid::IMAGE_HEIGHT,
        Pid::IMAGE_WIDTH,
        Pid::SIZE_IS_SPATIUM,
        Pid::IMAGE_FRAMED,
    };

    loadProperties(propertyIdSet);
}

void ImageSettingsModel::resetProperties()
{
    m_shouldScaleToFrameSize->resetToDefault();
    m_height->resetToDefault();
    m_width->resetToDefault();
    m_isAspectRatioLocked->resetToDefault();
    m_isSizeInSpatiums->resetToDefault();
    m_isImageFramed->resetToDefault();
}

void ImageSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void ImageSettingsModel::loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::AUTOSCALE)) {
        loadPropertyItem(m_shouldScaleToFrameSize);
    }

    if (muse::contains(propertyIdSet, Pid::IMAGE_HEIGHT)) {
        loadPropertyItem(m_height, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::IMAGE_WIDTH)) {
        loadPropertyItem(m_width, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::SIZE)) {
        loadPropertyItem(m_height, formatDoubleFunc);
        loadPropertyItem(m_width, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::LOCK_ASPECT_RATIO)) {
        loadPropertyItem(m_isAspectRatioLocked);
    }

    if (muse::contains(propertyIdSet, mu::engraving::Pid::SIZE_IS_SPATIUM)) {
        loadPropertyItem(m_isSizeInSpatiums);
    }

    if (muse::contains(propertyIdSet, Pid::IMAGE_FRAMED)) {
        loadPropertyItem(m_isImageFramed);
    }

    updateFrameScalingAvailability();
}

PropertyItem* ImageSettingsModel::shouldScaleToFrameSize() const
{
    return m_shouldScaleToFrameSize;
}

PropertyItem* ImageSettingsModel::height() const
{
    return m_height;
}

PropertyItem* ImageSettingsModel::width() const
{
    return m_width;
}

PropertyItem* ImageSettingsModel::isAspectRatioLocked() const
{
    return m_isAspectRatioLocked;
}

PropertyItem* ImageSettingsModel::isSizeInSpatiums() const
{
    return m_isSizeInSpatiums;
}

PropertyItem* ImageSettingsModel::isImageFramed() const
{
    return m_isImageFramed;
}

void ImageSettingsModel::updateFrameScalingAvailability()
{
    bool isAvailable = m_isImageFramed->value().toBool();

    m_shouldScaleToFrameSize->setIsEnabled(isAvailable);
}
