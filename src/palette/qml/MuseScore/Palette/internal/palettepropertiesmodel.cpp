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

#include "palettepropertiesmodel.h"

#include <QJsonDocument>

using namespace mu::palette;

void PalettePropertiesModel::load(const QVariant& properties)
{
    QJsonDocument document = QJsonDocument::fromJson(properties.toByteArray());
    QVariantMap map = document.toVariant().toMap();

    m_paletteId = map["paletteId"].toString();
    m_originConfig.name = QString(QByteArray::fromPercentEncoding(map["name"].toByteArray()));
    m_originConfig.size.setWidth(map["cellWidth"].toInt());
    m_originConfig.size.setHeight(map["cellHeight"].toInt());
    m_originConfig.scale = map["scale"].toDouble();
    m_originConfig.elementOffset = map["elementOffset"].toDouble();
    m_originConfig.showGrid = map["showGrid"].toBool();
    m_currentConfig = m_originConfig;

    emit propertiesChanged();
}

void PalettePropertiesModel::reject()
{
    setConfig(m_originConfig);
}

void PalettePropertiesModel::setConfig(const IPaletteConfiguration::PaletteConfig& config)
{
    configuration()->setPaletteConfig(m_paletteId, config);

    emit propertiesChanged();
}

QString PalettePropertiesModel::name() const
{
    return m_currentConfig.name;
}

int PalettePropertiesModel::cellWidth() const
{
    return m_currentConfig.size.width();
}

int PalettePropertiesModel::cellHeight() const
{
    return m_currentConfig.size.height();
}

double PalettePropertiesModel::elementOffset() const
{
    return m_currentConfig.elementOffset;
}

double PalettePropertiesModel::scaleFactor() const
{
    return m_currentConfig.scale;
}

bool PalettePropertiesModel::showGrid() const
{
    return m_currentConfig.showGrid;
}

void PalettePropertiesModel::setName(const QString& name)
{
    if (this->name() == name) {
        return;
    }

    m_currentConfig.name = name;
    setConfig(m_currentConfig);
}

void PalettePropertiesModel::setCellWidth(int width)
{
    if (cellWidth() == width) {
        return;
    }

    m_currentConfig.size.setWidth(width);
    setConfig(m_currentConfig);
}

void PalettePropertiesModel::setCellHeight(int height)
{
    if (cellHeight() == height) {
        return;
    }

    m_currentConfig.size.setHeight(height);
    setConfig(m_currentConfig);
}

void PalettePropertiesModel::setElementOffset(double offset)
{
    if (qFuzzyCompare(elementOffset(), offset)) {
        return;
    }

    m_currentConfig.elementOffset = offset;
    setConfig(m_currentConfig);
}

void PalettePropertiesModel::setScaleFactor(double scale)
{
    if (qFuzzyCompare(scaleFactor(), scale)) {
        return;
    }

    m_currentConfig.scale = scale;
    setConfig(m_currentConfig);
}

void PalettePropertiesModel::setShowGrid(bool showGrid)
{
    if (this->showGrid() == showGrid) {
        return;
    }

    m_currentConfig.showGrid = showGrid;
    setConfig(m_currentConfig);
}
