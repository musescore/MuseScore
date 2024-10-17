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
#pragma once

#include <QObject>

#include "models/abstractinspectormodel.h"
#include "context/iglobalcontext.h"
#include "global/iglobalconfiguration.h"
#include "engraving/iengravingconfiguration.h"
#include "engraving/types/types.h"

namespace mu::inspector {
class ScoreAccessibilitySettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    INJECT(mu::context::IGlobalContext, globalContext)
    INJECT(muse::IGlobalConfiguration, globalConfiguration);
    INJECT(engraving::IEngravingConfiguration, engravingConfiguration);

    Q_PROPERTY(QVariantList possibleScoreStylePresets READ possibleScoreStylePresets NOTIFY possibleScoreStylePresetsChanged);
    Q_PROPERTY(int scoreStylePresetIndex READ scoreStylePresetIndex WRITE setScoreStylePresetIndex NOTIFY scoreStylePresetIndexChanged);
public:
    explicit ScoreAccessibilitySettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    QVariantList possibleScoreStylePresets() const;
    int scoreStylePresetIndex() const;
    void setScoreStylePresetIndex(int index);
    Q_INVOKABLE void updateScoreStylePreset();

private:
    ScoreStylePreset m_scoreStylePreset = ScoreStylePreset::DEFAULT;
    bool m_scoreStylePresetEdited = false;
    bool m_ignoreStyleChange = false;

    void loadStyle(ScoreStylePreset preset);
    muse::io::path_t getStyleFilePath(ScoreStylePreset preset) const;
signals:
    void possibleScoreStylePresetsChanged();
    void scoreStylePresetIndexChanged();
};
}
