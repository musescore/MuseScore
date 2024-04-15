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
#ifndef MU_INSPECTOR_IMAGESETTINGSMODEL_H
#define MU_INSPECTOR_IMAGESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class ImageSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * shouldScaleToFrameSize READ shouldScaleToFrameSize CONSTANT)
    Q_PROPERTY(PropertyItem * height READ height CONSTANT)
    Q_PROPERTY(PropertyItem * width READ width CONSTANT)
    Q_PROPERTY(PropertyItem * isAspectRatioLocked READ isAspectRatioLocked CONSTANT)
    Q_PROPERTY(PropertyItem * isSizeInSpatiums READ isSizeInSpatiums CONSTANT)
    Q_PROPERTY(PropertyItem * isImageFramed READ isImageFramed CONSTANT)

public:
    explicit ImageSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* shouldScaleToFrameSize() const;
    PropertyItem* height() const;
    PropertyItem* width() const;
    PropertyItem* isAspectRatioLocked() const;
    PropertyItem* isSizeInSpatiums() const;
    PropertyItem* isImageFramed() const;

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);
    void updateFrameScalingAvailability();

    PropertyItem* m_shouldScaleToFrameSize = nullptr;
    PropertyItem* m_height = nullptr;
    PropertyItem* m_width = nullptr;
    PropertyItem* m_isAspectRatioLocked = nullptr;
    PropertyItem* m_isSizeInSpatiums = nullptr;
    PropertyItem* m_isImageFramed = nullptr;
};
}

#endif // MU_INSPECTOR_IMAGESETTINGSMODEL_H
