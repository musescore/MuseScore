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
#include "appearancesettingsmodel.h"

#include "types/commontypes.h"
#include "translation.h"

#include "log.h"

using namespace mu::inspector;
using namespace muse::actions;
using namespace mu::engraving;

static constexpr int REARRANGE_ORDER_STEP = 50;

AppearanceSettingsModel::AppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    createProperties();

    setTitle(muse::qtrc("inspector", "Appearance"));
}

void AppearanceSettingsModel::createProperties()
{
    m_leadingSpace = buildPropertyItem(Pid::LEADING_SPACE);
    m_measureWidth = buildPropertyItem(Pid::USER_STRETCH);
    m_minimumDistance = buildPropertyItem(Pid::MIN_DISTANCE);
    m_color = buildPropertyItem(Pid::COLOR);
    m_arrangeOrder = buildPropertyItem(Pid::Z);
    m_offset = buildPointFPropertyItem(Pid::OFFSET, [this](const mu::engraving::Pid, const QVariant& newValue) {
        setPropertyValue(m_elementsForOffsetProperty, Pid::OFFSET, newValue);
        loadProperties();
    }, [this](const mu::engraving::Pid) {
        resetPropertyValue(m_elementsForOffsetProperty, Pid::OFFSET);
        loadProperties();
    });
}

void AppearanceSettingsModel::requestElements()
{
    m_elementList.clear();

    static const std::unordered_set<ElementType> noAvailableChangeAppearanceTypes {
        ElementType::SOUND_FLAG
    };

    for (EngravingItem* element : m_repository->takeAllElements()) {
        if (muse::contains(noAvailableChangeAppearanceTypes, element->type())) {
            continue;
        }

        m_elementList << element;
    }

    static const std::unordered_set<ElementType> applyOffsetToChordTypes {
        ElementType::NOTE,
        ElementType::STEM,
        ElementType::HOOK,
    };

    QSet<EngravingItem*> elementsForArrangeProperty;
    for (EngravingItem* element : m_elementList) {
        if (element->findAncestor(ElementType::PAGE)) {
            elementsForArrangeProperty.insert(element);
        }
    }
    m_elementsForArrangeProperty = elementsForArrangeProperty.values();

    QSet<EngravingItem*> elementsForOffsetProperty;
    for (EngravingItem* element : m_elementList) {
        if (!muse::contains(applyOffsetToChordTypes, element->type())) {
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
    m_offset->resetToDefault();
}

void AppearanceSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void AppearanceSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::LEADING_SPACE)) {
        loadPropertyItem(m_leadingSpace, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::USER_STRETCH)) {
        loadPropertyItem(m_measureWidth, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::MIN_DISTANCE)) {
        loadPropertyItem(m_minimumDistance, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::COLOR)) {
        loadPropertyItem(m_color);
    }

    if (muse::contains(propertyIdSet, Pid::Z)) {
        loadPropertyItem(m_arrangeOrder, m_elementsForArrangeProperty);
    }

    if (muse::contains(propertyIdSet, Pid::OFFSET)) {
        loadPropertyItem(m_offset, m_elementsForOffsetProperty);
    }

    emit isSnappedToGridChanged(isSnappedToGrid());
}

Page* AppearanceSettingsModel::page() const
{
    return toPage(m_elementsForArrangeProperty.first()->findAncestor(ElementType::PAGE));
}

std::vector<EngravingItem*> AppearanceSettingsModel::allElementsInPage() const
{
    return page()->elements();
}

std::vector<EngravingItem*> AppearanceSettingsModel::allOverlappingElements() const
{
    RectF bbox = m_elementsForArrangeProperty.first()->pageBoundingRect();
    for (EngravingItem* element : m_elementsForArrangeProperty) {
        bbox |= element->pageBoundingRect();
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

    int minZ = (*std::min_element(m_elementsForArrangeProperty.begin(), m_elementsForArrangeProperty.end(), elementLessThan))->z();
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

    int maxZ = (*std::max_element(m_elementsForArrangeProperty.begin(), m_elementsForArrangeProperty.end(), elementLessThan))->z();
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

    if (m_elementsForArrangeProperty.contains(minElement)) {
        m_arrangeOrder->setValue(minElement->z());
    } else {
        m_arrangeOrder->setValue(minElement->z() - REARRANGE_ORDER_STEP);
    }
}

void AppearanceSettingsModel::pushToFrontInOrder()
{
    std::vector<EngravingItem*> elements = allElementsInPage();
    EngravingItem* maxElement = *std::max_element(elements.begin(), elements.end(), elementLessThan);

    if (m_elementsForArrangeProperty.contains(maxElement)) {
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

PropertyItem* AppearanceSettingsModel::offset() const
{
    return m_offset;
}

bool AppearanceSettingsModel::isVerticalOffsetAvailable() const
{
    return m_isVerticalOffsetAvailable;
}

bool AppearanceSettingsModel::isSnappedToGrid() const
{
    bool isSnapped = notationConfiguration()->isSnappedToGrid(muse::Orientation::Horizontal);
    isSnapped &= notationConfiguration()->isSnappedToGrid(muse::Orientation::Vertical);

    return isSnapped;
}

void AppearanceSettingsModel::setIsSnappedToGrid(bool isSnapped)
{
    if (isSnappedToGrid() == isSnapped) {
        return;
    }

    notationConfiguration()->setIsSnappedToGrid(muse::Orientation::Horizontal, isSnapped);
    notationConfiguration()->setIsSnappedToGrid(muse::Orientation::Vertical, isSnapped);

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
