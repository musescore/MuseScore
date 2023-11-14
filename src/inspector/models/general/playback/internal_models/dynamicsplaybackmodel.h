/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_INSPECTOR_DYNAMICSPLAYBACKMODEL_H
#define MU_INSPECTOR_DYNAMICSPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class DynamicsPlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * applyToVoice READ applyToVoice CONSTANT)

public:
    explicit DynamicsPlaybackModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* applyToVoice() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void setDynamicsVoice(int value);
    void loadApplyToVoiceProperty();

    PropertyItem* m_applyToVoice = nullptr;
};
}

#endif // MU_INSPECTOR_DYNAMICSPLAYBACKMODEL_H
