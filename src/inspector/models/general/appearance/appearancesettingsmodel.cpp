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

    m_horizontalOffset = buildPropertyItem(Pid::OFFSET,
                                           [this](engraving::Pid, const QVariant& newValue) {
        setProperty(m_elementsForOffsetProperty, Pid::OFFSET, newValue,
                    [](const QVariant& value, const engraving::EngravingItem* element) {
            double newX = value.toDouble();
            newX *= element->offsetIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
            return PointF(newX, element->getProperty(Pid::OFFSET).value<PointF>().y());
        });
    }, [this](engraving::Sid sid, const QVariant& value) {
        // TODO: What if m_verticalOffset->value() is invalid?
        setStyleValue(sid, PointF(value.toDouble(), m_verticalOffset->value().toDouble()));
    });

    m_verticalOffset = buildPropertyItem(Pid::OFFSET,
                                         [this](engraving::Pid, const QVariant& newValue) {
        setProperty(m_elementsForOffsetProperty, Pid::OFFSET, newValue,
                    [](const QVariant& value, const engraving::EngravingItem* element) {
            double newY = value.toDouble();
            newY *= element->offsetIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
            return PointF(element->getProperty(Pid::OFFSET).value<PointF>().x(), newY);
        });
    }, [this](engraving::Sid sid, const QVariant& value) {
        // TODO: What if m_horizontalOffset->value() is invalid?)
        setStyleValue(sid, PointF(m_horizontalOffset->value().toDouble(), value.toDouble()));
    });
}

void AppearanceSettingsModel::requestElements()
{
    m_elementList = m_repository->takeAllElements();

    static const std::unordered_set<ElementType> applyOffsetToChordTypes {
        ElementType::NOTE,
        ElementType::STEM,
        ElementType::HOOK,
    };

    QSet<EngravingItem*> elementsForOffsetProperty;

    for (EngravingItem* element : m_elementList) {
        if (!mu::contains(applyOffsetToChordTypes, element->type())) {
            elementsForOffsetProperty.insert(element);
            continue;
        }

        EngravingItem* parent = element->parentItem();
        if (parent && parent->isChord()) {
            elementsForOffsetProperty.insert(parent);
        }
    }

    m_elementsForOffsetProperty = elementsForOffsetProperty.values();
}

void AppearanceSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::LEADING_SPACE,
        Pid::USER_STRETCH,
        Pid::MIN_DISTANCE,
        Pid::COLOR,
        Pid::Z,
        Pid::OFFSET,
    };

    loadProperties(propertyIdSet);

    updateIsVerticalOffsetAvailable();
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

void AppearanceSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void AppearanceSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (mu::contains(propertyIdSet, Pid::LEADING_SPACE)) {
        loadPropertyItem(m_leadingSpace, roundedDoubleElementInternalToUiConverter(Pid::LEADING_SPACE));
    }

    if (mu::contains(propertyIdSet, Pid::USER_STRETCH)) {
        loadPropertyItem(m_measureWidth, roundedDoubleElementInternalToUiConverter(engraving::Pid::USER_STRETCH));
    }

    if (mu::contains(propertyIdSet, Pid::MIN_DISTANCE)) {
        loadPropertyItem(m_minimumDistance, roundedDoubleElementInternalToUiConverter(Pid::MIN_DISTANCE));
    }

    if (mu::contains(propertyIdSet, Pid::COLOR)) {
        loadPropertyItem(m_color);
    }

    if (mu::contains(propertyIdSet, Pid::Z)) {
        loadPropertyItem(m_arrangeOrder);
    }

    if (mu::contains(propertyIdSet, Pid::OFFSET)) {
        loadPropertyItem(m_horizontalOffset, [](const engraving::PropertyValue& propertyValue, const engraving::EngravingItem* element) {
            double x = propertyValue.value<PointF>().x();
            x /= element->offsetIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
            return x;
        }, m_elementsForOffsetProperty);

        loadPropertyItem(m_verticalOffset, [](const engraving::PropertyValue& propertyValue, const engraving::EngravingItem* element) {
            double y = propertyValue.value<PointF>().y();
            y /= element->offsetIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
            return y;
        }, m_elementsForOffsetProperty);
    }

    emit isSnappedToGridChanged(isSnappedToGrid());
}

Page* AppearanceSettingsModel::page() const
{
    return toPage(m_elementList.first()->findAncestor(ElementType::PAGE));
}

std::vector<EngravingItem*> AppearanceSettingsModel::allElementsInPage() const
{
    return page()->elements();
}

std::vector<EngravingItem*> AppearanceSettingsModel::allOverlappingElements() const
{
    RectF bbox = m_elementList.first()->abbox();
    for (EngravingItem* element : m_elementList) {
        bbox |= element->abbox();
    }
    if (bbox.width() == 0 || bbox.height() == 0) {
        LOGD() << "Bounding box appears to have a size of 0, so we'll get all the elements in the page";
        return allElementsInPage();
    }
    return page()->items(bbox);
}

void AppearanceSettingsModel::pushBackwardsInOrder()
{
    std::vector<EngravingItem*> elements = allOverlappingElements();
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
    std::vector<EngravingItem*> elements = allOverlappingElements();
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
    std::vector<EngravingItem*> elements = allElementsInPage();
    EngravingItem* minElement = *std::min_element(elements.begin(), elements.end(), elementLessThan);

    if (m_elementList.contains(minElement)) {
        m_arrangeOrder->setValue(minElement->z());
    } else {
        m_arrangeOrder->setValue(minElement->z() - REARRANGE_ORDER_STEP);
    }
}

void AppearanceSettingsModel::pushToFrontInOrder()
{
    std::vector<EngravingItem*> elements = allElementsInPage();
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

bool AppearanceSettingsModel::isVerticalOffsetAvailable() const
{
    return m_isVerticalOffsetAvailable;
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

void AppearanceSettingsModel::setIsVerticalOffsetAvailable(bool isAvailable)
{
    if (isAvailable == m_isVerticalOffsetAvailable) {
        return;
    }

    m_isVerticalOffsetAvailable = isAvailable;
    emit isVerticalOffsetAvailableChanged(m_isVerticalOffsetAvailable);
}

void AppearanceSettingsModel::updateIsVerticalOffsetAvailable()
{
    bool isAvailable = true;
    for (EngravingItem* item : m_elementList) {
        if (item->isBeam()) {
            isAvailable = false;
            break;
        }
    }
    setIsVerticalOffsetAvailable(isAvailable);
}
