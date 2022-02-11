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
#include "appearancesettingsmodel.h"

#include "types/commontypes.h"
#include "translation.h"

#include "log.h"

using namespace mu::inspector;
using namespace mu::actions;
using namespace mu::framework;

static constexpr int REARRANGE_ORDER_STEP = 100;

AppearanceSettingsModel::AppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    createProperties();

    setTitle(qtrc("inspector", "Appearance"));
}

void AppearanceSettingsModel::createProperties()
{
    m_leadingSpace = buildPropertyItem(Ms::Pid::LEADING_SPACE);
    m_barWidth = buildPropertyItem(Ms::Pid::USER_STRETCH);
    m_minimumDistance = buildPropertyItem(Ms::Pid::MIN_DISTANCE);
    m_color = buildPropertyItem(Ms::Pid::COLOR);
    m_arrangeOrder = buildPropertyItem(Ms::Pid::Z);

    m_horizontalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QPointF(newValue.toDouble(), m_verticalOffset->value().toDouble()));
    });

    m_verticalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QPointF(m_horizontalOffset->value().toDouble(), newValue.toDouble()));
    });
}

void AppearanceSettingsModel::requestElements()
{
    m_elementList = m_repository->takeAllElements();
}

void AppearanceSettingsModel::loadProperties()
{
    loadPropertyItem(m_leadingSpace, formatDoubleFunc);
    loadPropertyItem(m_minimumDistance, formatDoubleFunc);

    loadPropertyItem(m_barWidth);
    loadPropertyItem(m_color);
    loadPropertyItem(m_arrangeOrder);

    loadOffsets();

    emit isSnappedToGridChanged(isSnappedToGrid());
}

void AppearanceSettingsModel::resetProperties()
{
    m_leadingSpace->resetToDefault();
    m_minimumDistance->resetToDefault();
    m_barWidth->resetToDefault();
    m_color->resetToDefault();
    m_arrangeOrder->resetToDefault();
    m_horizontalOffset->resetToDefault();
    m_verticalOffset->resetToDefault();
}

void AppearanceSettingsModel::updatePropertiesOnNotationChanged()
{
    loadOffsets();
}

void AppearanceSettingsModel::loadOffsets()
{
    loadPropertyItem(m_horizontalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().x());
    });

    loadPropertyItem(m_verticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().y());
    });
}

void AppearanceSettingsModel::pushBackInOrder()
{
    m_arrangeOrder->setValue(m_arrangeOrder->value().toInt() - REARRANGE_ORDER_STEP);
}

void AppearanceSettingsModel::pushFrontInOrder()
{
    m_arrangeOrder->setValue(m_arrangeOrder->value().toInt() + REARRANGE_ORDER_STEP);
}

void AppearanceSettingsModel::configureGrid()
{
    dispatcher()->dispatch("config-raster");
}

PropertyItem* AppearanceSettingsModel::leadingSpace() const
{
    return m_leadingSpace;
}

PropertyItem* AppearanceSettingsModel::barWidth() const
{
    return m_barWidth;
}

PropertyItem* AppearanceSettingsModel::minimumDistance() const
{
    return m_minimumDistance;
}

PropertyItem* AppearanceSettingsModel::color() const
{
    return m_color;
}

PropertyItem* AppearanceSettingsModel::arrangeOrder() const
{
    return m_arrangeOrder;
}

PropertyItem* AppearanceSettingsModel::horizontalOffset() const
{
    return m_horizontalOffset;
}

PropertyItem* AppearanceSettingsModel::verticalOffset() const
{
    return m_verticalOffset;
}

bool AppearanceSettingsModel::isSnappedToGrid() const
{
    bool isSnapped = notationConfiguration()->isSnappedToGrid(framework::Orientation::Horizontal);
    isSnapped &= notationConfiguration()->isSnappedToGrid(framework::Orientation::Vertical);

    return isSnapped;
}

void AppearanceSettingsModel::setIsSnappedToGrid(bool isSnapped)
{
    if (isSnappedToGrid() == isSnapped) {
        return;
    }

    notationConfiguration()->setIsSnappedToGrid(framework::Orientation::Horizontal, isSnapped);
    notationConfiguration()->setIsSnappedToGrid(framework::Orientation::Vertical, isSnapped);

    emit isSnappedToGridChanged(isSnappedToGrid());
}
