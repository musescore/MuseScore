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
#include "global/iglobalconfiguration.h"
#include "project/iprojectconfiguration.h"
#include "engraving/iengravingconfiguration.h"
#include "engraving/types/types.h"

namespace mu::inspector {
class ScoreAccessibilitySettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantList possibleStylePresets
               READ possibleStylePresets
               NOTIFY possibleStylePresetsChanged);

    Q_PROPERTY(int currentStylePresetIndex
               READ currentStylePresetIndex
               WRITE setCurrentStylePresetIndex
               NOTIFY currentStylePresetIndexChanged);

    muse::Inject<engraving::IEngravingConfiguration> engravingConfiguration = { this };
    muse::Inject<project::IProjectConfiguration> projectConfiguration = { this };
    muse::Inject<muse::IGlobalConfiguration> globalConfiguration = { this };

public:
    explicit ScoreAccessibilitySettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;
    void requestElements() override;

    QVariantList possibleStylePresets() const;
    int currentStylePresetIndex() const;
    void setCurrentStylePresetIndex(int index);
    Q_INVOKABLE void updateCurrentStylePreset();

signals:
    void possibleStylePresetsChanged();
    void currentStylePresetIndexChanged();

private:
    bool applyStylePreset(ScoreStylePreset preset) const;
    muse::io::path_t styleFilePath(ScoreStylePreset preset) const;
    mu::engraving::MStyle& engravingStyle() const;
    mu::notation::INotationStylePtr notationStyle() const;

    ScoreStylePreset m_currentPreset = ScoreStylePreset::DEFAULT;
    bool m_currentPresetEdited = false;
    bool m_ignoreStyleChanges = false;
};
}
