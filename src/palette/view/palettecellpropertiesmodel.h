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

#ifndef MU_PALETTE_PALETTECELLPROPERTIESMODEL_H
#define MU_PALETTE_PALETTECELLPROPERTIESMODEL_H

#include <QObject>

#include "modularity/ioc.h"

#include "ipaletteconfiguration.h"

namespace mu::palette {
class PaletteCellPropertiesModel : public QObject
{
    Q_OBJECT

    INJECT(IPaletteConfiguration, configuration)

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY propertiesChanged)
    Q_PROPERTY(double xOffset READ xOffset WRITE setXOffset NOTIFY propertiesChanged)
    Q_PROPERTY(double yOffset READ yOffset WRITE setYOffset NOTIFY propertiesChanged)
    Q_PROPERTY(double scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY propertiesChanged)
    Q_PROPERTY(bool drawStaff READ drawStaff WRITE setDrawStaff NOTIFY propertiesChanged)

public:
    QString name() const;
    double xOffset() const;
    double yOffset() const;
    double scaleFactor() const;
    bool drawStaff() const;

    Q_INVOKABLE void load(const QVariant& properties);
    Q_INVOKABLE void reject();

public slots:
    void setName(const QString& name);
    void setXOffset(double xOffset);
    void setYOffset(double yOffset);
    void setScaleFactor(double scale);
    void setDrawStaff(bool drawStaff);

signals:
    void propertiesChanged();

private:
    void setConfig(const IPaletteConfiguration::PaletteCellConfig& config);

    QString m_cellId;
    IPaletteConfiguration::PaletteCellConfig m_currentConfig;
    IPaletteConfiguration::PaletteCellConfig m_originConfig;
};
}

#endif // MU_PALETTE_PALETTECELLPROPERTIESMODEL_H
