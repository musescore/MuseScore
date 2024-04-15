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

#ifndef MU_PALETTE_PALETTEPROPERTIESMODEL_H
#define MU_PALETTE_PALETTEPROPERTIESMODEL_H

#include <QObject>

#include "modularity/ioc.h"

#include "ipaletteconfiguration.h"

namespace mu::palette {
class PalettePropertiesModel : public QObject
{
    Q_OBJECT

    INJECT(IPaletteConfiguration, configuration)

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY propertiesChanged)
    Q_PROPERTY(int cellWidth READ cellWidth WRITE setCellWidth NOTIFY propertiesChanged)
    Q_PROPERTY(int cellHeight READ cellHeight WRITE setCellHeight NOTIFY propertiesChanged)
    Q_PROPERTY(double elementOffset READ elementOffset WRITE setElementOffset NOTIFY propertiesChanged)
    Q_PROPERTY(double scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY propertiesChanged)
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY propertiesChanged)

public:
    QString name() const;
    int cellWidth() const;
    int cellHeight() const;
    double elementOffset() const;
    double scaleFactor() const;
    bool showGrid() const;

    Q_INVOKABLE void load(const QVariant& properties);
    Q_INVOKABLE void reject();

public slots:
    void setName(const QString& name);
    void setCellWidth(int width);
    void setCellHeight(int height);
    void setElementOffset(double offset);
    void setScaleFactor(double scale);
    void setShowGrid(bool showGrid);

signals:
    void propertiesChanged();

private:
    void setConfig(const IPaletteConfiguration::PaletteConfig& config);

    QString m_paletteId;
    IPaletteConfiguration::PaletteConfig m_currentConfig;
    IPaletteConfiguration::PaletteConfig m_originConfig;
};
}

#endif // MU_PALETTE_PALETTEPROPERTIESMODEL_H
