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
class TimeSignatureSettingsModel : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::propertiespanel::PropertyItem * horizontalScale READ horizontalScale CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * verticalScale READ verticalScale CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * shouldShowCourtesy READ shouldShowCourtesy CONSTANT)
    Q_PROPERTY(bool isGenerated READ isGenerated CONSTANT)

public:
    explicit TimeSignatureSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository);

    Q_INVOKABLE void showTimeSignatureProperties();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;

    PropertyItem* horizontalScale() const;
    PropertyItem* verticalScale() const;
    PropertyItem* shouldShowCourtesy() const;

    bool isGenerated() const { return m_isGenerated; }

private:

    PropertyItem* m_horizontalScale = nullptr;
    PropertyItem* m_verticalScale = nullptr;
    PropertyItem* m_shouldShowCourtesy = nullptr;

    bool m_isGenerated = false;
};
}
