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
#ifndef MU_INSPECTOR_MEASURESSETTINGSMODEL_H
#define MU_INSPECTOR_MEASURESSETTINGSMODEL_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "../ui/iuiactionsregister.h"
#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class MeasuresSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QString shortcutMoveMeasureUp READ shortcutMoveMeasureUp CONSTANT)
    Q_PROPERTY(QString shortcutMoveMeasureDown READ shortcutMoveMeasureDown CONSTANT)
    Q_PROPERTY(QString shortcutToggleSystemLock READ shortcutToggleSystemLock CONSTANT)
    Q_PROPERTY(QString shortcutMakeIntoSystem READ shortcutMakeIntoSystem CONSTANT)
    Q_PROPERTY(bool allSystemsAreLocked READ allSystemsAreLocked NOTIFY allSystemsAreLockedChanged)
    Q_PROPERTY(bool scoreIsInPageView READ scoreIsInPageView NOTIFY scoreIsInPageViewChanged)
    Q_PROPERTY(bool isMakeIntoSystemAvailable READ isMakeIntoSystemAvailable NOTIFY isMakeIntoSystemAvailableChanged)

public:
    explicit MeasuresSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override { }
    void loadProperties() override;
    void resetProperties() override { }
    void requestElements() override { }
    void onCurrentNotationChanged() override;

    bool isEmpty() const override;

    enum class InsertMeasuresTarget {
        AfterSelection,
        BeforeSelection,
        AtStartOfScore,
        AtEndOfScore
    };
    Q_ENUM(InsertMeasuresTarget)

    Q_INVOKABLE void insertMeasures(int numberOfMeasures, InsertMeasuresTarget target);
    Q_INVOKABLE void deleteSelectedMeasures();

    Q_INVOKABLE void moveMeasureUp();
    QString shortcutMoveMeasureUp() const;

    Q_INVOKABLE void moveMeasureDown();
    QString shortcutMoveMeasureDown() const;

    Q_INVOKABLE void toggleSystemLock();
    QString shortcutToggleSystemLock() const;
    bool allSystemsAreLocked() const;

    Q_INVOKABLE void makeIntoSystem();
    QString shortcutMakeIntoSystem() const;

    bool scoreIsInPageView() const;
    bool isMakeIntoSystemAvailable() const;

protected:
    void onNotationChanged(const mu::engraving::PropertyIdSet&, const mu::engraving::StyleIdSet&) override;

private:
    void updateAllSystemsAreLocked();
    void updateScoreIsInPageView();
    void updateIsMakeIntoSystemAvailable();

signals:
    void allSystemsAreLockedChanged(bool allLocked);
    void scoreIsInPageViewChanged(bool isInPageView);
    void isMakeIntoSystemAvailableChanged(bool isMakeIntoSystemAvailable);

private:
    bool m_allSystemsAreLocked = false;
    bool m_scoreIsInPageView = false;
    bool m_isMakeIntoSystemAvailable = false;
};
}

#endif // MU_INSPECTOR_MEASURESSETTINGSMODEL_H
