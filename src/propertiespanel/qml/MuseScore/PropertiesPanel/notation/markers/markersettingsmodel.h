/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
class MarkerSettingsModel : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::propertiespanel::PropertyItem * type READ type CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * label READ label CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * position READ position CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * centerOnSymbol READ centerOnSymbol CONSTANT)

public:
    explicit MarkerSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* type() const;
    PropertyItem* label() const;
    PropertyItem* position() const;
    PropertyItem* centerOnSymbol() const;

    Q_INVOKABLE QString markerTypeName() const;

private:
    PropertyItem* m_type = nullptr;
    PropertyItem* m_label = nullptr;
    PropertyItem* m_position = nullptr;
    PropertyItem* m_centerOnSymbol = nullptr;
};
}
