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
#ifndef MU_INSPECTOR_MARKERSETTINGSMODEL_H
#define MU_INSPECTOR_MARKERSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class MarkerSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * type READ type CONSTANT)
    Q_PROPERTY(PropertyItem * label READ label CONSTANT)
    Q_PROPERTY(PropertyItem * symbolSize READ symbolSize CONSTANT)
    Q_PROPERTY(PropertyItem * position READ position CONSTANT)
    Q_PROPERTY(PropertyItem * centerOnSymbol READ centerOnSymbol CONSTANT)

public:
    explicit MarkerSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* type() const;
    PropertyItem* label() const;
    PropertyItem* symbolSize() const;
    PropertyItem* position() const;
    PropertyItem* centerOnSymbol() const;

private:
    PropertyItem* m_type = nullptr;
    PropertyItem* m_label = nullptr;
    PropertyItem* m_symbolSize = nullptr;
    PropertyItem* m_position = nullptr;
    PropertyItem* m_centerOnSymbol = nullptr;
};
}

#endif // MU_INSPECTOR_MARKERSETTINGSMODEL_H
