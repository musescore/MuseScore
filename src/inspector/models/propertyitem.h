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
#ifndef MU_INSPECTOR_INSPECTORPROPERTY_H
#define MU_INSPECTOR_INSPECTORPROPERTY_H

#include <QObject>
#include <QVariant>

#include "engraving/dom/property.h"
#include "engraving/style/styledef.h"

namespace mu::inspector {
class PropertyItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QVariant defaultValue READ defaultValue NOTIFY defaultValueChanged)
    Q_PROPERTY(bool isUndefined READ isUndefined NOTIFY isUndefinedChanged)
    Q_PROPERTY(bool isVisible READ isVisible NOTIFY isVisibleChanged)
    Q_PROPERTY(bool isEnabled READ isEnabled NOTIFY isEnabledChanged)
    Q_PROPERTY(bool isStyled READ isStyled NOTIFY isStyledChanged)
    Q_PROPERTY(bool isModified READ isModified NOTIFY isModifiedChanged)

public:
    explicit PropertyItem(const mu::engraving::Pid propertyId, QObject* parent = nullptr);

    void fillValues(const QVariant& currentValue, const QVariant& defaultValue);
    void updateCurrentValue(const QVariant& currentValue);

    Q_INVOKABLE void resetToDefault();
    Q_INVOKABLE void applyToStyle();

    mu::engraving::Pid propertyId() const;
    QVariant value() const;
    QVariant defaultValue() const;
    bool isUndefined() const;
    bool isEnabled() const;
    bool isVisible() const;
    bool isStyled() const;
    bool isModified() const;

    void setStyleId(const mu::engraving::Sid styleId);

public slots:
    void setValue(const QVariant& value);
    void setDefaultValue(const QVariant& defaultValue);
    void setIsEnabled(bool isEnabled);
    void setIsVisible(bool isVisible);

signals:
    void valueChanged();
    void defaultValueChanged(QVariant defaultValue);
    void isUndefinedChanged(bool isUndefined);
    void isEnabledChanged(bool isEnabled);
    void isStyledChanged();
    void isVisibleChanged(bool isVisible);
    void isModifiedChanged(bool isModified);
    void resetToDefaultRequested();

    void propertyModified(mu::engraving::Pid propertyId, QVariant newValue);
    void applyToStyleRequested(mu::engraving::Sid styledId, QVariant newStyleValue);

private:
    mu::engraving::Pid m_propertyId = mu::engraving::Pid::END;
    mu::engraving::Sid m_styleId = mu::engraving::Sid::NOSTYLE;

    QVariant m_defaultValue;
    QVariant m_currentValue;
    bool m_isEnabled = false;
    bool m_isVisible = false;
};
}

#endif // MU_INSPECTOR_INSPECTORPROPERTY_H
