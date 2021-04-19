/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_INSPECTOR_PEDALSSETTINGSMODEL_H
#define MU_INSPECTOR_PEDALSSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class PedalSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * hookType READ hookType CONSTANT)
    Q_PROPERTY(PropertyItem * thickness READ thickness CONSTANT)
    Q_PROPERTY(PropertyItem * hookHeight READ hookHeight CONSTANT)
    Q_PROPERTY(PropertyItem * lineStyle READ lineStyle CONSTANT)
    Q_PROPERTY(PropertyItem * dashLineLength READ dashLineLength CONSTANT)
    Q_PROPERTY(PropertyItem * dashGapLength READ dashGapLength CONSTANT)
    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)

    Q_PROPERTY(bool hasToShowBothHooks READ hasToShowBothHooks WRITE setHasToShowBothHooks NOTIFY hasToShowBothHooksChanged)
public:
    explicit PedalSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* hookType() const;
    PropertyItem* thickness() const;
    PropertyItem* hookHeight() const;
    PropertyItem* lineStyle() const;
    PropertyItem* dashLineLength() const;
    PropertyItem* dashGapLength() const;
    PropertyItem* placement() const;

    bool hasToShowBothHooks() const;

public slots:
    void setHasToShowBothHooks(bool hasToShowBothHooks);

signals:
    void hasToShowBothHooksChanged(bool hasToShowBothHooks);

private:
    PropertyItem* m_hookType = nullptr;
    PropertyItem* m_thickness = nullptr;
    PropertyItem* m_hookHeight = nullptr;
    PropertyItem* m_lineStyle = nullptr;
    PropertyItem* m_dashLineLength = nullptr;
    PropertyItem* m_dashGapLength = nullptr;
    PropertyItem* m_placement = nullptr;

    bool m_hasToShowBothHooks = false;
};
}

#endif // MU_INSPECTOR_PEDALSSETTINGSMODEL_H
