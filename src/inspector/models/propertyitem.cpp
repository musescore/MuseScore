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
#include "propertyitem.h"

#include "property.h"

using namespace mu::inspector;
using namespace mu::engraving;

PropertyItem::PropertyItem(const mu::engraving::Pid propertyId, QObject* parent)
    : QObject(parent), m_isVisible(true)
{
    m_propertyId = propertyId;
}

void PropertyItem::fillValues(const QVariant& currentValue, const QVariant& defaultValue)
{
    updateCurrentValue(currentValue);

    setDefaultValue(defaultValue);

    emit isModifiedChanged(isModified());
}

void PropertyItem::updateCurrentValue(const QVariant& currentValue)
{
    if (m_currentValue == currentValue) {
        return;
    }

    m_currentValue = currentValue;

    emit isUndefinedChanged(isUndefined());
    emit valueChanged();
}

void PropertyItem::resetToDefault()
{
    setValue(m_defaultValue);
}

void PropertyItem::applyToStyle()
{
    emit applyToStyleRequested(m_styleId, m_currentValue);
}

mu::engraving::Pid PropertyItem::propertyId() const
{
    return m_propertyId;
}

QVariant PropertyItem::value() const
{
    return m_currentValue;
}

QVariant PropertyItem::defaultValue() const
{
    return m_defaultValue;
}

bool PropertyItem::isUndefined() const
{
    return !m_currentValue.isValid();
}

bool PropertyItem::isEnabled() const
{
    return m_isEnabled;
}

bool PropertyItem::isVisible() const
{
    return m_isVisible;
}

bool PropertyItem::isStyled() const
{
    return m_styleId != mu::engraving::Sid::NOSTYLE;
}

bool PropertyItem::isModified() const
{
    return m_currentValue != m_defaultValue;
}

void PropertyItem::setStyleId(const mu::engraving::Sid styleId)
{
    if (m_styleId == styleId) {
        return;
    }

    m_styleId = styleId;
    emit isStyledChanged();
}

void PropertyItem::setValue(const QVariant& value)
{
    if (m_currentValue == value) {
        return;
    }

    updateCurrentValue(value);

    emit propertyModified(m_propertyId, m_currentValue);
    emit isModifiedChanged(isModified());
}

void PropertyItem::setDefaultValue(const QVariant& defaultValue)
{
    if (m_defaultValue == defaultValue) {
        return;
    }

    m_defaultValue = defaultValue;
    emit defaultValueChanged(m_defaultValue);
}

void PropertyItem::setIsEnabled(bool isEnabled)
{
    if (m_isEnabled == isEnabled) {
        return;
    }

    m_isEnabled = isEnabled;
    emit isEnabledChanged(m_isEnabled);
}

void PropertyItem::setIsVisible(bool isVisible)
{
    if (m_isVisible == isVisible) {
        return;
    }

    m_isVisible = isVisible;
    emit isVisibleChanged(isVisible);
}
