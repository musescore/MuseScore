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
#ifndef MU_INSPECTOR_JUMPSETTINGSMODEL_H
#define MU_INSPECTOR_JUMPSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class JumpSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * jumpTo READ jumpTo CONSTANT)
    Q_PROPERTY(PropertyItem * playUntil READ playUntil CONSTANT)
    Q_PROPERTY(PropertyItem * continueAt READ continueAt CONSTANT)
    Q_PROPERTY(PropertyItem * hasToPlayRepeats READ hasToPlayRepeats CONSTANT)
public:
    explicit JumpSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* jumpTo() const;
    PropertyItem* playUntil() const;
    PropertyItem* continueAt() const;
    PropertyItem* hasToPlayRepeats() const;

private:
    PropertyItem* m_jumpTo = nullptr;
    PropertyItem* m_playUntil = nullptr;
    PropertyItem* m_continueAt = nullptr;
    PropertyItem* m_hasToPlayRepeats = nullptr;
};
}

#endif // MU_INSPECTOR_JUMPSETTINGSMODEL_H
