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
#ifndef MU_INSPECTOR_TIMESIGNATURESETTINGSMODEL_H
#define MU_INSPECTOR_TIMESIGNATURESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class TimeSignatureSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * horizontalScale READ horizontalScale CONSTANT)
    Q_PROPERTY(PropertyItem * verticalScale READ verticalScale CONSTANT)
    Q_PROPERTY(PropertyItem * shouldShowCourtesy READ shouldShowCourtesy CONSTANT)

public:
    explicit TimeSignatureSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void showTimeSignatureProperties();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* horizontalScale() const;
    PropertyItem* verticalScale() const;
    PropertyItem* shouldShowCourtesy() const;

private:

    PropertyItem* m_horizontalScale = nullptr;
    PropertyItem* m_verticalScale = nullptr;
    PropertyItem* m_shouldShowCourtesy = nullptr;
};
}

#endif // TIMESIGNATURESETTINGSMODEL_H
