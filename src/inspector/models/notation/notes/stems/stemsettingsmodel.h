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
#ifndef MU_INSPECTOR_STEMSETTINGSMODEL_H
#define MU_INSPECTOR_STEMSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class StemSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * thickness READ thickness CONSTANT)
    Q_PROPERTY(PropertyItem * length READ length CONSTANT)
    Q_PROPERTY(PropertyItem * offset READ offset CONSTANT)
    Q_PROPERTY(PropertyItem * stemDirection READ stemDirection CONSTANT)

    Q_PROPERTY(bool useStraightNoteFlags READ useStraightNoteFlags WRITE setUseStraightNoteFlags NOTIFY useStraightNoteFlagsChanged)

public:
    explicit StemSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* thickness() const;
    PropertyItem* length() const;

    PropertyItem* offset() const;
    PropertyItem* stemDirection() const;

    bool useStraightNoteFlags() const;
    void setUseStraightNoteFlags(bool use);

signals:
    void useStraightNoteFlagsChanged();

private:
    void onStemDirectionChanged(mu::engraving::DirectionV newDirection);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    PropertyItem* m_thickness = nullptr;
    PropertyItem* m_length = nullptr;
    PointFPropertyItem* m_offset = nullptr;
    PropertyItem* m_stemDirection = nullptr;
};
}

#endif // MU_INSPECTOR_STEMSETTINGSMODEL_H
