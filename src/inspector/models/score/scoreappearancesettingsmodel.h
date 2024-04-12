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
#ifndef MU_INSPECTOR_SCOREAPPEARANCESETTINGSMODEL_H
#define MU_INSPECTOR_SCOREAPPEARANCESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

#include "async/asyncable.h"

namespace mu::inspector {
class ScoreAppearanceSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(bool hideEmptyStaves READ hideEmptyStaves WRITE setHideEmptyStaves NOTIFY hideEmptyStavesChanged)
    Q_PROPERTY(
        bool dontHideEmptyStavesInFirstSystem READ dontHideEmptyStavesInFirstSystem WRITE setDontHideEmptyStavesInFirstSystem NOTIFY dontHideEmptyStavesInFirstSystemChanged)
    Q_PROPERTY(
        bool showBracketsWhenSpanningSingleStaff READ showBracketsWhenSpanningSingleStaff WRITE setShowBracketsWhenSpanningSingleStaff NOTIFY showBracketsWhenSpanningSingleStaffChanged)

public:
    explicit ScoreAppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository);

    bool hideEmptyStaves() const;
    void setHideEmptyStaves(bool hide);

    bool dontHideEmptyStavesInFirstSystem() const;
    void setDontHideEmptyStavesInFirstSystem(bool dont);

    bool showBracketsWhenSpanningSingleStaff() const;
    void setShowBracketsWhenSpanningSingleStaff(bool show);

    Q_INVOKABLE void showPageSettings();
    Q_INVOKABLE void showStyleSettings();

    bool isEmpty() const override;

signals:
    void hideEmptyStavesChanged();
    void dontHideEmptyStavesInFirstSystemChanged();
    void showBracketsWhenSpanningSingleStaffChanged();

private:
    void createProperties() override { }
    void requestElements() override { }
    void resetProperties() override { }
    void loadProperties() override {}

    void onCurrentNotationChanged() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;
};
}

#endif // MU_INSPECTOR_SCOREAPPEARANCESETTINGSMODEL_H
