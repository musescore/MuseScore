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
#ifndef MU_INSPECTOR_VIBRATOSETTINGSMODEL_H
#define MU_INSPECTOR_VIBRATOSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class VibratoSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * lineType READ lineType CONSTANT)
    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)

public:
    explicit VibratoSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* lineType() const;
    PropertyItem* placement() const;

    Q_INVOKABLE QVariantList possibleLineTypes() const;

private:
    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* m_lineType = nullptr;
    PropertyItem* m_placement = nullptr;
};
}

#endif // MU_INSPECTOR_VIBRATOSETTINGSMODEL_H
