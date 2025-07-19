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
#ifndef MU_INSPECTOR_TEXTFRAMESETTINGSMODEL_H
#define MU_INSPECTOR_TEXTFRAMESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class TextFrameSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * gapAbove READ gapAbove CONSTANT)
    Q_PROPERTY(PropertyItem * gapBelow READ gapBelow CONSTANT)
    Q_PROPERTY(PropertyItem * frameLeftMargin READ frameLeftMargin CONSTANT)
    Q_PROPERTY(PropertyItem * frameRightMargin READ frameRightMargin CONSTANT)
    Q_PROPERTY(PropertyItem * frameTopMargin READ frameTopMargin CONSTANT)
    Q_PROPERTY(PropertyItem * frameBottomMargin READ frameBottomMargin CONSTANT)
    Q_PROPERTY(PropertyItem * isSizeSpatiumDependent READ isSizeSpatiumDependent CONSTANT)

public:
    explicit TextFrameSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* gapAbove() const;
    PropertyItem* gapBelow() const;
    PropertyItem* frameLeftMargin() const;
    PropertyItem* frameRightMargin() const;
    PropertyItem* frameTopMargin() const;
    PropertyItem* frameBottomMargin() const;
    PropertyItem* isSizeSpatiumDependent() const;

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    PropertyItem* m_gapAbove = nullptr;
    PropertyItem* m_gapBelow = nullptr;
    PropertyItem* m_frameLeftMargin = nullptr;
    PropertyItem* m_frameRightMargin = nullptr;
    PropertyItem* m_frameTopMargin = nullptr;
    PropertyItem* m_frameBottomMargin = nullptr;
    PropertyItem* m_isSizeSpatiumDependent = nullptr;
};
}

#endif // MU_INSPECTOR_TEXTFRAMESETTINGSMODEL_H
