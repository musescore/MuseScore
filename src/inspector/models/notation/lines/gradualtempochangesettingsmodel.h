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
#ifndef MU_INSPECTOR_GRADUALTEMPOCHANGESETTINGSMODEL_H
#define MU_INSPECTOR_GRADUALTEMPOCHANGESETTINGSMODEL_H

#include "textlinesettingsmodel.h"

namespace mu::inspector {
class GradualTempoChangeSettingsModel : public TextLineSettingsModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * snapBefore READ snapBefore CONSTANT)
    Q_PROPERTY(PropertyItem * snapAfter READ snapAfter CONSTANT)

public:
    explicit GradualTempoChangeSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* snapBefore() const;
    PropertyItem* snapAfter() const;

private:
    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* m_snapBefore = nullptr;
    PropertyItem* m_snapAfter = nullptr;
};
}

#endif // MU_INSPECTOR_GRADUALTEMPOCHANGESETTINGSMODEL_H
