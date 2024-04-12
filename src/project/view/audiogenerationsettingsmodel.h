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
#ifndef MU_PROJECT_AUDIOGENERATIONSETTINGSMODEL_H
#define MU_PROJECT_AUDIOGENERATIONSETTINGSMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "iprojectconfiguration.h"

namespace mu::project {
class AudioGenerationSettingsModel : public QObject
{
    Q_OBJECT

    INJECT(IProjectConfiguration, configuration)

    Q_PROPERTY(int timePeriodType READ timePeriodType WRITE setTimePeriodType NOTIFY timePeriodTypeChanged)
    Q_PROPERTY(int numberOfSaves READ numberOfSaves WRITE setNumberOfSaves NOTIFY numberOfSavesChanged)

public:
    explicit AudioGenerationSettingsModel(QObject* parent = nullptr);

    int timePeriodType() const;
    int numberOfSaves() const;

public slots:
    void setTimePeriodType(int type);
    void setNumberOfSaves(int number);

signals:
    void timePeriodTypeChanged();
    void numberOfSavesChanged();
};
}

#endif // MU_PROJECT_AUDIOGENERATIONSETTINGSMODEL_H
