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
#ifndef MU_INSPECTOR_BARLINESETTINGSMODEL_H
#define MU_INSPECTOR_BARLINESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class BarlineSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * type READ type CONSTANT)
    Q_PROPERTY(PropertyItem * isSpanToNextStaff READ isSpanToNextStaff CONSTANT)
    Q_PROPERTY(PropertyItem * spanFrom READ spanFrom CONSTANT)
    Q_PROPERTY(PropertyItem * spanTo READ spanTo CONSTANT)
    Q_PROPERTY(PropertyItem * hasToShowTips READ hasToShowTips CONSTANT)
    Q_PROPERTY(bool isRepeatStyleChangingAllowed READ isRepeatStyleChangingAllowed NOTIFY isRepeatStyleChangingAllowedChanged)

public:
    explicit BarlineSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void applySpanPreset(const int presetType);
    Q_INVOKABLE void setSpanIntervalAsStaffDefault();

    PropertyItem* type() const;
    PropertyItem* isSpanToNextStaff() const;
    PropertyItem* spanFrom() const;
    PropertyItem* spanTo() const;
    PropertyItem* hasToShowTips() const;

    bool isRepeatStyleChangingAllowed() const;

signals:
    void isRepeatStyleChangingAllowedChanged();

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    PropertyItem* m_type = nullptr;
    PropertyItem* m_isSpanToNextStaff = nullptr;
    PropertyItem* m_spanFrom = nullptr;
    PropertyItem* m_spanTo = nullptr;
    PropertyItem* m_hasToShowTips = nullptr;
};
}

#endif // MU_INSPECTOR_BARLINESETTINGSMODEL_H
