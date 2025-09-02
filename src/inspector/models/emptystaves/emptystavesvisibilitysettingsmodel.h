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

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class EmptyStavesVisibilitySettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(bool canHideEmptyStavesInSelection READ canHideEmptyStavesInSelection NOTIFY canHideEmptyStavesInSelectionChanged)
    Q_PROPERTY(bool canShowAllEmptyStaves READ canShowAllEmptyStaves NOTIFY canShowAllEmptyStavesChanged)
    Q_PROPERTY(bool canResetEmptyStavesVisibility READ canResetEmptyStavesVisibility NOTIFY canResetEmptyStavesVisibilityChanged)

public:
    explicit EmptyStavesVisibilitySettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override {}
    void loadProperties() override;
    void resetProperties() override {}
    void requestElements() override {}

    bool isEmpty() const override;
    bool shouldUpdateOnEmptyPropertyAndStyleIdSets() const override;

    bool canHideEmptyStavesInSelection() const { return m_canHideEmptyStavesInSelection; }
    bool canShowAllEmptyStaves() const { return m_canShowAllEmptyStaves; }
    bool canResetEmptyStavesVisibility() const { return m_canResetEmptyStavesVisibility; }

    Q_INVOKABLE void hideEmptyStavesInSelection();
    Q_INVOKABLE void showAllEmptyStaves();
    Q_INVOKABLE void resetEmptyStavesVisibility();

signals:
    void canHideEmptyStavesInSelectionChanged();
    void canShowAllEmptyStavesChanged();
    void canResetEmptyStavesVisibilityChanged();

private:
    void onNotationChanged(const mu::engraving::PropertyIdSet&, const mu::engraving::StyleIdSet&) override;

    void updateCanHideEmptyStavesInSelection();
    void updateCanShowAllEmptyStaves();
    void updateCanResetEmptyStavesVisibility();

    bool m_canHideEmptyStavesInSelection = false;
    bool m_canShowAllEmptyStaves = false;
    bool m_canResetEmptyStavesVisibility = false;
};
}
