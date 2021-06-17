/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_INSPECTOR_SCOREAPPEARANCESETTINGSMODEL_H
#define MU_INSPECTOR_SCOREAPPEARANCESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class ScoreAppearanceSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

public:
    explicit ScoreAppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void showPageSettings();
    Q_INVOKABLE void showStyleSettings();

    virtual void createProperties() override { }
    virtual void requestElements() override { }
    virtual void loadProperties() override { }
    virtual void resetProperties() override { }

    bool hasAcceptableElements() const override;
};
}

#endif // MU_INSPECTOR_SCOREAPPEARANCESETTINGSMODEL_H
