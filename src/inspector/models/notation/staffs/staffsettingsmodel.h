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
#ifndef MU_INSPECTOR_STAFFSETTINGSMODEL_H
#define MU_INSPECTOR_STAFFSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class StaffSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * barlinesSpanFrom READ barlinesSpanFrom CONSTANT)
    Q_PROPERTY(PropertyItem * barlinesSpanTo READ barlinesSpanTo CONSTANT)
public:
    explicit StaffSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* barlinesSpanFrom() const;
    PropertyItem* barlinesSpanTo() const;

private:
    PropertyItem* m_barlinesSpanFrom = nullptr;
    PropertyItem* m_barlinesSpanTo = nullptr;
};
}

#endif // MU_INSPECTOR_STAFFSETTINGSMODEL_H
