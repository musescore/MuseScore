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
using namespace mu::engraving;

static constexpr int REARRANGE_ORDER_STEP = 50;

AppearanceSettingsModel::AppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    createProperties();

    setTitle(qtrc("inspector", "Appearance"));
}

void AppearanceSettingsModel::createProperties()
{
    m_leadingSpace = buildPropertyItem(Pid::LEADING_SPACE);
    m_measureWidth = buildPropertyItem(Pid::USER_STRETCH);
    m_minimumDistance = buildPropertyItem(Pid::MIN_DISTANCE);
    m_color = buildPropertyItem(Pid::COLOR);
    m_arrangeOrder = buildPropertyItem(Pid::Z);

    m_horizontalOffset = buildPropertyItem(Pid::OFFSET, [this](const Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QPointF(newValue.toDouble(), m_verticalOffset->value().toDouble()));
    });

    m_verticalOffset = buildPropertyItem(Pid::OFFSET, [this](const Pid pid, const QVariant& newValue) {
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

    loadPropertyItem(m_measureWidth);
    loadPropertyItem(m_color);
    loadPropertyItem(m_arrangeOrder);

    loadOffsets();

    emit isSnappedToGridChanged(isSnappedToGrid());
}

void AppearanceSettingsModel::resetProperties()
{
    m_leadingSpace->resetToDefault();
    m_minimumDistance->resetToDefault();
    m_measureWidth->resetToDefault();
    m_color->resetToDefault();
    m_arrangeOrder->resetToDefault();
    m_horizontalOffset->resetToDefault();
    m_verticalOffset->resetToDefault();
}

void AppearanceSettingsModel::updatePropertiesOnNotationChanged()
{
    loadPropertyItem(m_leadingSpace, formatDoubleFunc);
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

Page* AppearanceSettingsModel::getPage()
{
    return toPage(m_elementList.first()->findAncestor(ElementType::PAGE));
}

std::vector<EngravingItem*> AppearanceSettingsModel::getAllElementsInPage()
{
    return getPage()->elements();
}

std::vector<EngravingItem*> AppearanceSettingsModel::getAllOverlappingElements()
{
    RectF bbox = m_elementList.first()->abbox();
    for (EngravingItem* element : m_elementList) {
        bbox |= element->abbox();
    }
    if (bbox.width() == 0 || bbox.height() == 0) {
        LOGD() << "Bounding box appears to have a size of 0, so we'll get all the elements in the page";
        return getAllElementsInPage();
    }
    return getPage()->items(bbox);
}

void AppearanceSettingsModel::pushBackwardsInOrder()
{
    std::vector<EngravingItem*> elements = getAllOverlappingElements();
    std::sort(elements.begin(), elements.end(), elementLessThan);

    int minZ = (*std::min_element(m_elementList.begin(), m_elementList.end(), elementLessThan))->z();
    int i;
    for (i = 0; i < static_cast<int>(elements.size()); i++) {
        if (elements[i]->z() == minZ) {
            break;
        }
    }

    EngravingItem* elementBehind = elements[i - 1 >= 0 ? i - 1 : 0];
    m_arrangeOrder->setValue(elementBehind->z() - REARRANGE_ORDER_STEP);
}

void AppearanceSettingsModel::pushForwardsInOrder()
{
    std::vector<EngravingItem*> elements = getAllOverlappingElements();
    std::sort(elements.begin(), elements.end(), elementLessThan);

    int maxZ = (*std::max_element(m_elementList.begin(), m_elementList.end(), elementLessThan))->z();
    int elementsCount = static_cast<int>(elements.size());
    int i;
    for (i = elementsCount - 1; i > 0; i--) {
        if (elements[i]->z() == maxZ) {
            break;
        }
    }

    EngravingItem* elementInFront = elements[i + 1 < elementsCount ? i + 1 : elementsCount - 1];
    m_arrangeOrder->setValue(elementInFront->z() + REARRANGE_ORDER_STEP);
}

void AppearanceSettingsModel::pushToBackInOrder()
{
    std::vector<EngravingItem*> elements = getAllElementsInPage();
    EngravingItem* minElement = *std::min_element(elements.begin(), elements.end(), elementLessThan);

    if (m_elementList.contains(minElement)) {
        m_arrangeOrder->setValue(minElement->z());
    } else {
        m_arrangeOrder->setValue(minElement->z() - REARRANGE_ORDER_STEP);
    }
}

void AppearanceSettingsModel::pushToFrontInOrder()
{
    std::vector<EngravingItem*> elements = getAllElementsInPage();
    EngravingItem* maxElement = *std::max_element(elements.begin(), elements.end(), elementLessThan);

    if (m_elementList.contains(maxElement)) {
        m_arrangeOrder->setValue(maxElement->z());
    } else {
        m_arrangeOrder->setValue(maxElement->z() + REARRANGE_ORDER_STEP);
    }
}

void AppearanceSettingsModel::configureGrid()
{
    dispatcher()->dispatch("config-raster");
}

PropertyItem* AppearanceSettingsModel::leadingSpace() const
{
    return m_leadingSpace;
}

PropertyItem* AppearanceSettingsModel::measureWidth() const
{
    return m_measureWidth;
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
