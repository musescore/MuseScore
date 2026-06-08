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
class MeasuresSettingsModel : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

public:
    explicit MeasuresSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository);

    void createProperties() override { }
    void loadProperties() override;
    void requestElements() override { }

    bool isEmpty() const override;
    bool shouldUpdateOnEmptyPropertyAndStyleIdSets() const override;

    enum class InsertMeasuresTarget {
        AfterSelection,
        BeforeSelection,
        AtStartOfScore,
        AtEndOfScore
    };
    Q_ENUM(InsertMeasuresTarget)

    Q_INVOKABLE void insertMeasures(int numberOfMeasures, InsertMeasuresTarget target);
    Q_INVOKABLE void deleteSelectedMeasures();

protected:
    void onNotationChanged(const mu::engraving::PropertyIdSet&, const mu::engraving::StyleIdSet&) override;
};
}
