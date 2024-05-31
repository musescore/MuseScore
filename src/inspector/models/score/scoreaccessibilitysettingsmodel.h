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
#ifndef MU_INSPECTOR_SCOREACCESSIBILITYSETTINGSMODEL_H
#define MU_INSPECTOR_SCOREACCESSIBILITYSETTINGSMODEL_H

#include <QObject>

#include "models/abstractinspectormodel.h"
#include "context/iglobalcontext.h"
#include "global/iglobalconfiguration.h"
#include "engraving/iengravingconfiguration.h"
#include "engraving/types/accessibilitystyle.h"

namespace mu::inspector {
class ScoreAccessibilitySettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    INJECT(mu::context::IGlobalContext, globalContext)
    INJECT(muse::IGlobalConfiguration, globalConfiguration);
    INJECT(engraving::IEngravingConfiguration, engravingConfiguration);

public:
    explicit ScoreAccessibilitySettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    muse::io::path_t getStyleFilePath(int preset) const;
    Q_INVOKABLE QVariantList possibleScoreStylePreset() const;
    Q_INVOKABLE void loadStyle(int preset);
    Q_INVOKABLE int scoreStylePreset() const;
    Q_INVOKABLE void readStyleFileAccessibilityStyleEdited();

private:
    bool isAccessibilityStyleEdited = false;
    bool m_ignoreStyleChange = false;

signals:
    void accessibilityStyleEditedChanged();
};
}

#endif // MU_INSPECTOR_SCOREACCESSIBILITYSETTINGSMODEL_H
