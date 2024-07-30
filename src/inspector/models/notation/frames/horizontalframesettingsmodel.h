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
#ifndef MU_INSPECTOR_HORIZONTALFRAMESETTINGSMODEL_H
#define MU_INSPECTOR_HORIZONTALFRAMESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class HorizontalFrameSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * frameWidth READ frameWidth CONSTANT)
    Q_PROPERTY(PropertyItem * leftGap READ leftGap CONSTANT)
    Q_PROPERTY(PropertyItem * rightGap READ rightGap CONSTANT)
    Q_PROPERTY(PropertyItem * shouldDisplayKeysAndBrackets READ shouldDisplayKeysAndBrackets CONSTANT)
    Q_PROPERTY(PropertyItem * isSizeSpatiumDependent READ isSizeSpatiumDependent CONSTANT)

public:
    explicit HorizontalFrameSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* frameWidth() const;
    PropertyItem* leftGap() const;
    PropertyItem* rightGap() const;
    PropertyItem* shouldDisplayKeysAndBrackets() const;
    PropertyItem* isSizeSpatiumDependent() const;

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    PropertyItem* m_frameWidth = nullptr;
    PropertyItem* m_leftGap = nullptr;
    PropertyItem* m_rightGap = nullptr;
    PropertyItem* m_shouldDisplayKeysAndBrackets = nullptr;
    PropertyItem* m_isSizeSpatiumDependent = nullptr;
};
}

#endif // MU_INSPECTOR_HORIZONTALFRAMESETTINGSMODEL_H
