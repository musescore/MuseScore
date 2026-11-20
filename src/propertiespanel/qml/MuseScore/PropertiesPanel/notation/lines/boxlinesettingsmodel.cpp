/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "boxlinesettingsmodel.h"

#include "translation.h"

#include "types/commontypes.h"
#include "types/linetypes.h"
#include "ui/view/iconcodes.h"

#include "engraving/dom/boxline.h"

using namespace mu::propertiespanel;
using namespace mu::engraving;

BoxLineSettingsModel::BoxLineSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                           IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository, ElementType::BOXLINE)
{
    setModelType(PropertiesPanelModelType::TYPE_BOXLINE);
    setTitle(muse::qtrc("propertiespanel", "Box"));
    setIcon(muse::ui::IconCode::Code::SQUARE);
    createProperties();
}

PropertyItem* BoxLineSettingsModel::position() const
{
    return m_position;
}

PropertyItem* BoxLineSettingsModel::height() const
{
    return m_height;
}

PropertyItem* BoxLineSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* BoxLineSettingsModel::lineStyle() const
{
    return m_lineStyle;
}

PropertyItem* BoxLineSettingsModel::dashLineLength() const
{
    return m_dashLineLength;
}

PropertyItem* BoxLineSettingsModel::dashGapLength() const
{
    return m_dashGapLength;
}

double BoxLineSettingsModel::minHeight() const
{
    return BoxLine::MIN_HEIGHT.val();
}

double BoxLineSettingsModel::maxHeight() const
{
    return BoxLine::MAX_HEIGHT.val();
}

void BoxLineSettingsModel::createProperties()
{
    m_position = buildPropertyItem(
        Pid::PLACEMENT,
        [this](const Pid, const QVariant& newValue) { setPosition(static_cast<Position>(newValue.toInt())); },
        nullptr,
        [this](const Pid) { resetPosition(); });
    m_height = buildPropertyItem(Pid::BOXLINE_HEIGHT);
    m_thickness = buildPropertyItem(Pid::LINE_WIDTH);
    m_lineStyle = buildPropertyItem(Pid::LINE_STYLE);
    m_dashLineLength = buildPropertyItem(Pid::DASH_LINE_LEN);
    m_dashGapLength = buildPropertyItem(Pid::DASH_GAP_LEN);
}

void BoxLineSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::PLACEMENT,
        Pid::BOXLINE_CENTER_ON_STAFF,
        Pid::BOXLINE_HEIGHT,
        Pid::LINE_WIDTH,
        Pid::LINE_STYLE,
        Pid::DASH_LINE_LEN,
        Pid::DASH_GAP_LEN,
    };

    loadProperties(propertyIdSet);
}

void BoxLineSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void BoxLineSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::PLACEMENT) || muse::contains(propertyIdSet, Pid::BOXLINE_CENTER_ON_STAFF)) {
        loadPosition();
    }

    if (muse::contains(propertyIdSet, Pid::BOXLINE_HEIGHT)) {
        loadPropertyItem(m_height, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::LINE_WIDTH)) {
        loadPropertyItem(m_thickness, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::LINE_STYLE)) {
        loadPropertyItem(m_lineStyle);
    }

    if (muse::contains(propertyIdSet, Pid::DASH_LINE_LEN)) {
        loadPropertyItem(m_dashLineLength, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::DASH_GAP_LEN)) {
        loadPropertyItem(m_dashGapLength, formatDoubleFunc);
    }

    updateDashPropertiesAvailability();
}

void BoxLineSettingsModel::updateDashPropertiesAvailability()
{
    const auto style = static_cast<LineTypes::LineStyle>(m_lineStyle->value().toInt());
    const bool isDashed = style == LineTypes::LineStyle::LINE_STYLE_DASHED;

    m_dashLineLength->setIsEnabled(isDashed);
    m_dashGapLength->setIsEnabled(isDashed);
}

void BoxLineSettingsModel::loadPosition()
{
    QVariant value;
    bool isUndefined = false;
    bool isModified = false;

    for (const EngravingItem* item : m_elementList) {
        IF_ASSERT_FAILED(item) {
            continue;
        }

        Position position = Position::Center;
        if (!item->getProperty(Pid::BOXLINE_CENTER_ON_STAFF).toBool()) {
            const PlacementV placement = item->getProperty(Pid::PLACEMENT).value<PlacementV>();
            position = placement == PlacementV::ABOVE ? Position::Above : Position::Below;
        }

        if (!value.isValid()) {
            value = static_cast<int>(position);
        } else if (value.toInt() != static_cast<int>(position)) {
            isUndefined = true;
        }

        for (Pid pid : { Pid::PLACEMENT, Pid::BOXLINE_CENTER_ON_STAFF }) {
            if (item->getProperty(pid) != item->propertyDefault(pid)) {
                isModified = true;
            }
        }

        if (isUndefined && isModified) {
            break;
        }
    }

    m_position->setIsEnabled(value.isValid());
    m_position->updateCurrentValue(isUndefined ? QVariant() : value);
    m_position->setIsModified(isModified);
}

void BoxLineSettingsModel::setPosition(Position position)
{
    if (m_elementList.empty()) {
        return;
    }

    beginCommand(muse::TranslatableString("undoableAction", "Edit %1").arg(propertyUserName(Pid::PLACEMENT)));

    const bool center = position == Position::Center;
    for (EngravingItem* item : m_elementList) {
        IF_ASSERT_FAILED(item) {
            continue;
        }

        item->undoChangeProperty(Pid::BOXLINE_CENTER_ON_STAFF, center);
        if (!center) {
            const PlacementV placement = position == Position::Above ? PlacementV::ABOVE : PlacementV::BELOW;
            item->undoChangeProperty(Pid::PLACEMENT, placement);
        }
    }

    updateNotation();
    endCommand();
    loadPosition();
}

void BoxLineSettingsModel::resetPosition()
{
    if (m_elementList.empty()) {
        return;
    }

    beginCommand(muse::TranslatableString("undoableAction", "Reset %1").arg(propertyUserName(Pid::PLACEMENT)));

    for (EngravingItem* item : m_elementList) {
        IF_ASSERT_FAILED(item) {
            continue;
        }

        item->undoResetProperty(Pid::BOXLINE_CENTER_ON_STAFF);
        item->undoResetProperty(Pid::PLACEMENT);
    }

    updateNotation();
    endCommand();
    loadPosition();
}
