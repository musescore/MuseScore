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

#include "palettecellpropertiesmodel.h"

#include <QJsonDocument>

using namespace mu::palette;

void PaletteCellPropertiesModel::load(const QVariant& properties)
{
    QJsonDocument document = QJsonDocument::fromJson(properties.toByteArray());
    QVariantMap map = document.toVariant().toMap();

    m_cellId = map["cellId"].toString();
    m_originConfig.name = map["name"].toString();
    m_originConfig.xOffset = map["xOffset"].toDouble();
    m_originConfig.yOffset = map["yOffset"].toDouble();
    m_originConfig.scale = map["scale"].toDouble();
    m_originConfig.drawStaff = map["drawStaff"].toBool();
    m_currentConfig = m_originConfig;

    emit propertiesChanged();
}

void PaletteCellPropertiesModel::reject()
{
    setConfig(m_originConfig);
}

void PaletteCellPropertiesModel::setConfig(const IPaletteConfiguration::PaletteCellConfig& config)
{
    configuration()->setPaletteCellConfig(m_cellId, config);

    emit propertiesChanged();
}

QString PaletteCellPropertiesModel::name() const
{
    return m_currentConfig.name;
}

double PaletteCellPropertiesModel::xOffset() const
{
    return m_currentConfig.xOffset;
}

double PaletteCellPropertiesModel::yOffset() const
{
    return m_currentConfig.yOffset;
}

double PaletteCellPropertiesModel::scaleFactor() const
{
    return m_currentConfig.scale;
}

bool PaletteCellPropertiesModel::drawStaff() const
{
    return m_currentConfig.drawStaff;
}

void PaletteCellPropertiesModel::setName(const QString& name)
{
    if (this->name() == name) {
        return;
    }

    m_currentConfig.name = name;
    setConfig(m_currentConfig);
}

void PaletteCellPropertiesModel::setXOffset(double xOffset)
{
    if (qFuzzyCompare(this->xOffset(), xOffset)) {
        return;
    }

    m_currentConfig.xOffset = xOffset;
    setConfig(m_currentConfig);
}

void PaletteCellPropertiesModel::setYOffset(double yOffset)
{
    if (qFuzzyCompare(this->yOffset(), yOffset)) {
        return;
    }

    m_currentConfig.yOffset = yOffset;
    setConfig(m_currentConfig);
}

void PaletteCellPropertiesModel::setScaleFactor(double scale)
{
    if (qFuzzyCompare(scaleFactor(), scale)) {
        return;
    }

    m_currentConfig.scale = scale;
    setConfig(m_currentConfig);
}

void PaletteCellPropertiesModel::setDrawStaff(bool drawStaff)
{
    if (this->drawStaff() == drawStaff) {
        return;
    }

    m_currentConfig.drawStaff = drawStaff;
    setConfig(m_currentConfig);
}
