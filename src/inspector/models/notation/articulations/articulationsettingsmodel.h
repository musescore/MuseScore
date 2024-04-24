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
#ifndef MU_INSPECTOR_ARTICULATIONSETTINGSMODEL_H
#define MU_INSPECTOR_ARTICULATIONSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class ArticulationSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)
    Q_PROPERTY(PropertyItem * articStemHAlign READ articStemHAlign CONSTANT)

    Q_PROPERTY(bool isArticStemHAlignAvailable READ isArticStemHAlignAvailable NOTIFY isArticStemHAlignAvailableChanged)

public:
    explicit ArticulationSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* placement() const;
    PropertyItem* articStemHAlign() const;

    bool isArticStemHAlignAvailable() const;

public slots:
    void setIsArticStemHAlignAvailable(bool isArticStemHAlignAvailable);

signals:
    void isArticStemHAlignAvailableChanged(bool isArticStemHAlignAvailable);

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    void updateIsArticStemHAlignAvailable();

    PropertyItem* m_direction = nullptr;
    PropertyItem* m_placement = nullptr;
    PropertyItem* m_articStemHAlign = nullptr;

    bool m_isArticStemHAlignAvailable = false;
};
}

#endif // MU_INSPECTOR_ARTICULATIONSETTINGSMODEL_H
