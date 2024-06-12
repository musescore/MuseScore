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
#ifndef MU_INSPECTOR_HAIRPINSETTINGSMODEL_H
#define MU_INSPECTOR_HAIRPINSETTINGSMODEL_H

#include "textlinesettingsmodel.h"

namespace mu::inspector {
class HairpinSettingsModel : public TextLineSettingsModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isNienteCircleVisible READ isNienteCircleVisible CONSTANT)

    Q_PROPERTY(PropertyItem * height READ height CONSTANT)
    Q_PROPERTY(PropertyItem * continuousHeight READ continuousHeight CONSTANT)
    Q_PROPERTY(PropertyItem * snapBefore READ snapBefore CONSTANT)
    Q_PROPERTY(PropertyItem * snapAfter READ snapAfter CONSTANT)

public:
    explicit HairpinSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* isNienteCircleVisible() const;

    PropertyItem* height() const;
    PropertyItem* continuousHeight() const;

    PropertyItem* snapBefore() const;
    PropertyItem* snapAfter() const;

private:
    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;
    void requestElements() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    PropertyItem* m_isNienteCircleVisible = nullptr;

    PropertyItem* m_height = nullptr;
    PropertyItem* m_continuousHeight = nullptr;

    PropertyItem* m_snapBefore = nullptr;
    PropertyItem* m_snapAfter = nullptr;
};
}

#endif // MU_INSPECTOR_HAIRPINSETTINGSMODEL_H
