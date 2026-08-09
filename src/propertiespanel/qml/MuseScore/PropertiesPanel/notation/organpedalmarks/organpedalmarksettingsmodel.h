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
class OrganPedalMarkSettingsModel : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::propertiespanel::PropertyItem * placement READ placement CONSTANT)
    Q_PROPERTY(PropertyItem * size READ size CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * style READ style CONSTANT)

    Q_PROPERTY(QVariantList textStyles READ textStyles NOTIFY textStylesChanged)

public:
    explicit OrganPedalMarkSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository,
                                       PropertiesPanelModelType type = PropertiesPanelModelType::TYPE_ORGAN_PEDAL_MARK);

    PropertyItem* placement() const;
    PropertyItem* size() const;
    PropertyItem* style() const;

    QVariantList textStyles();

signals:
    void textStylesChanged();

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;

    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    PropertyItem* m_placement = nullptr;
    PropertyItem* m_size = nullptr;
    PropertyItem* m_style = nullptr;

    QVariantList m_textStyles;
};
}
