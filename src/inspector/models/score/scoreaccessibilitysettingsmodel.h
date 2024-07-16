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

#include "models/abstractinspectormodel.h"
#include "models/propertyitem.h"
#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "engraving/style/scorestylepreset.h"

namespace mu::inspector {
class ScoreAccessibilitySettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    INJECT(mu::context::IGlobalContext, globalContext)
    Q_PROPERTY(PropertyItem * scoreStylePreset READ scoreStylePreset WRITE setScoreStylePreset NOTIFY scoreStylePresetChanged)
    Q_PROPERTY(PropertyItem * accessibleNoteHead READ accessibleNoteHead WRITE setAccessibleNoteHead NOTIFY accessibleNoteHeadChanged)
    Q_PROPERTY(
        PropertyItem
        * accessibleNoteHeadColor READ accessibleNoteHeadColor WRITE setAccessibleNoteHeadColor NOTIFY accessibleNoteHeadColorChanged)

public:
    explicit ScoreAccessibilitySettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* scoreStylePreset() const;
    PropertyItem* accessibleNoteHead() const;
    PropertyItem* accessibleNoteHeadColor() const;
    void setScoreStylePreset(PropertyItem* preset);
    void setAccessibleNoteHead(PropertyItem* headSystem);
    void setAccessibleNoteHeadColor(PropertyItem* headColor);
    Q_INVOKABLE QVariantList possibleScoreStylePreset() const;
    Q_INVOKABLE QVariantList possibleAccessibleNoteHeadTypes() const;
    Q_INVOKABLE QVariantList possibleAccessibleNoteHeadColorTypes() const;
    void loadStyle(PropertyItem* preset);
    void loadAccessibleNoteHead(PropertyItem* noteHeadScheme);
    void loadAccessibleNoteHeadColor(PropertyItem* noteHeadColor);

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

signals:
    void scoreStylePresetChanged();
    void accessibleNoteHeadChanged();
    void accessibleNoteHeadColorChanged();

private:
    PropertyItem* m_scoreStylePreset = nullptr;
    PropertyItem* m_accessibleNoteHead = nullptr;
    PropertyItem* m_accessibleNoteHeadColor = nullptr;
    bool m_ignoreStyleChange = false;
};
}

#endif // MU_INSPECTOR_SCOREACCESSIBILITYSETTINGSMODEL_H
