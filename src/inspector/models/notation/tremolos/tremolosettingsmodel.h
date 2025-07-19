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
#ifndef MU_INSPECTOR_TREMOLOSETTINGSMODEL_H
#define MU_INSPECTOR_TREMOLOSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class TremoloSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * style READ style CONSTANT)
    Q_PROPERTY(PropertyItem * direction READ direction CONSTANT)

public:
    explicit TremoloSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    PropertyItem* style() const;
    PropertyItem* direction() const;

private:
    void loadProperties(const mu::engraving::PropertyIdSet& allowedPropertyIdSet);

    PropertyItem* m_style = nullptr;
    PropertyItem* m_direction = nullptr;
};
}

#endif // MU_INSPECTOR_TREMOLOSETTINGSMODEL_H
