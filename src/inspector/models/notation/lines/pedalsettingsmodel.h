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
#ifndef MU_INSPECTOR_PEDALSSETTINGSMODEL_H
#define MU_INSPECTOR_PEDALSSETTINGSMODEL_H

#include "textlinesettingsmodel.h"

namespace mu::inspector {
class PedalSettingsModel : public TextLineSettingsModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * lineType READ lineType CONSTANT)
    Q_PROPERTY(bool isChangingLineVisibilityAllowed READ isChangingLineVisibilityAllowed NOTIFY isChangingLineVisibilityAllowedChanged)

public:
    explicit PedalSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* lineType() const;
    bool isChangingLineVisibilityAllowed() const;

signals:
    void isChangingLineVisibilityAllowedChanged();

private:
    bool isStarSymbolVisible() const;

    void createProperties() override;
    void loadProperties() override;

    void setLineType(int newType);

    PropertyItem* m_lineType = nullptr;
    bool m_rosetteHookSelected = false;
};
}

#endif // MU_INSPECTOR_PEDALSSETTINGSMODEL_H
