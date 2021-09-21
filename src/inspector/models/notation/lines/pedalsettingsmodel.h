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
#ifndef MU_INSPECTOR_PEDALSSETTINGSMODEL_H
#define MU_INSPECTOR_PEDALSSETTINGSMODEL_H

#include "linesettingsmodel.h"

namespace mu::inspector {
class PedalSettingsModel : public LineSettingsModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * showPedalSymbol READ showPedalSymbol CONSTANT)
    Q_PROPERTY(PropertyItem * showLineWithRosette READ showLineWithRosette CONSTANT)

    Q_PROPERTY(bool showLineWithRosetteVisible READ showLineWithRosetteVisible NOTIFY showLineWithRosetteVisibleChanged)

public:
    explicit PedalSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* showPedalSymbol() const;
    PropertyItem* showLineWithRosette() const;

    bool showLineWithRosetteVisible() const;

signals:
    void showLineWithRosetteVisibleChanged();

private:
    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* m_showPedalSymbol = nullptr;
    PropertyItem* m_showLineWithRosette = nullptr;
    bool m_showLineWithRosetteVisible = false;
};
}

#endif // MU_INSPECTOR_PEDALSSETTINGSMODEL_H
