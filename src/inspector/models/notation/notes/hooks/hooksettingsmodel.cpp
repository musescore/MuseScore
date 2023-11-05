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
#include "hooksettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

HookSettingsModel::HookSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_HOOK);
    setTitle(qtrc("inspector", "Flag")); // internally called "Hook", but "Flag" in SMuFL, so here externally too

    createProperties();
}

void HookSettingsModel::createProperties()
{
    m_horizontalOffset = buildPropertyItem(mu::engraving::Pid::OFFSET, [](const QVariant& value, const engraving::EngravingItem* element) {
        double newX = value.toDouble();
        newX *= element->sizeIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
        return PointF(newX, element->getProperty(mu::engraving::Pid::OFFSET).value<PointF>().y());
    }, [this](const QVariant& value) {
        // TODO: What if m_verticalOffset->value() is invalid?
        return PointF(value.toDouble(), m_verticalOffset->value().toDouble());
    });

    m_verticalOffset = buildPropertyItem(mu::engraving::Pid::OFFSET, [](const QVariant& value, const engraving::EngravingItem* element) {
        double newY = value.toDouble();
        newY *= element->sizeIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
        return PointF(element->getProperty(mu::engraving::Pid::OFFSET).value<PointF>().x(), newY);
    }, [this](const QVariant& value) {
        // TODO: What if m_horizontalOffset->value() is invalid?
        return PointF(m_horizontalOffset->value().toDouble(), value.toDouble());
    });
}

void HookSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::HOOK);
}

void HookSettingsModel::loadProperties()
{
    loadPropertyItem(m_horizontalOffset, [](const engraving::PropertyValue& propertyValue, const engraving::EngravingItem* element) {
        double x = propertyValue.value<PointF>().x();
        x /= element->sizeIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
        return x;
    });

    loadPropertyItem(m_verticalOffset, [](const engraving::PropertyValue& propertyValue, const engraving::EngravingItem* element) {
        double y = propertyValue.value<PointF>().y();
        y /= element->sizeIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
        return y;
    });
}

void HookSettingsModel::resetProperties()
{
    m_horizontalOffset->resetToDefault();
    m_verticalOffset->resetToDefault();
}

PropertyItem* HookSettingsModel::horizontalOffset() const
{
    return m_horizontalOffset;
}

PropertyItem* HookSettingsModel::verticalOffset() const
{
    return m_verticalOffset;
}
