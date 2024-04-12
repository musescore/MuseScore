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
#ifndef MU_INSPECTOR_KEYSIGNATURESETTINGSMODEL_H
#define MU_INSPECTOR_KEYSIGNATURESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class KeySignatureSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * hasToShowCourtesy READ hasToShowCourtesy CONSTANT)
    Q_PROPERTY(PropertyItem * mode READ mode CONSTANT)
public:
    explicit KeySignatureSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* hasToShowCourtesy() const;
    PropertyItem* mode() const;

private:
    PropertyItem* m_hasToShowCourtesy = nullptr;
    PropertyItem* m_mode = nullptr;
};
}

#endif // MU_INSPECTOR_KEYSIGNATURESETTINGSMODEL_H
