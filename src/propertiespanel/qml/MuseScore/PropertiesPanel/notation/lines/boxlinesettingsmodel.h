/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <qqmlintegration.h>

#include "propertiespanelabstractmodel.h"

namespace mu::propertiespanel {
class BoxLineSettingsModel : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::propertiespanel::PropertyItem * position READ position CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * height READ height CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * thickness READ thickness CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * lineStyle READ lineStyle CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * dashLineLength READ dashLineLength CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * dashGapLength READ dashGapLength CONSTANT)

    Q_PROPERTY(double minHeight READ minHeight CONSTANT)
    Q_PROPERTY(double maxHeight READ maxHeight CONSTANT)

public:
    // Combines Pid::PLACEMENT and Pid::BOXLINE_CENTER_ON_STAFF into a single choice
    enum class Position {
        Above = int(mu::engraving::PlacementV::ABOVE),
        Below = int(mu::engraving::PlacementV::BELOW),
        Center
    };
    Q_ENUM(Position)

    explicit BoxLineSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository);

    PropertyItem* position() const;
    PropertyItem* height() const;
    PropertyItem* thickness() const;
    PropertyItem* lineStyle() const;
    PropertyItem* dashLineLength() const;
    PropertyItem* dashGapLength() const;

    double minHeight() const;
    double maxHeight() const;

private:
    void createProperties() override;
    void loadProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);
    void updateDashPropertiesAvailability();

    void loadPosition();
    void setPosition(Position position);
    void resetPosition();

    PropertyItem* m_position = nullptr;
    PropertyItem* m_height = nullptr;
    PropertyItem* m_thickness = nullptr;
    PropertyItem* m_lineStyle = nullptr;
    PropertyItem* m_dashLineLength = nullptr;
    PropertyItem* m_dashGapLength = nullptr;
};
}
