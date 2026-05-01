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

    Q_PROPERTY(QString shortcutMoveMeasureUpPage READ shortcutMoveMeasureUpPage CONSTANT)
    Q_PROPERTY(QString shortcutMoveMeasureDownPage READ shortcutMoveMeasureDownPage CONSTANT)
    Q_PROPERTY(QString shortcutMoveMeasureUpSystem READ shortcutMoveMeasureUpSystem CONSTANT)
    Q_PROPERTY(QString shortcutMoveMeasureDownSystem READ shortcutMoveMeasureDownSystem CONSTANT)
    Q_PROPERTY(QString shortcutToggleSystemLock READ shortcutToggleSystemLock CONSTANT)
    Q_PROPERTY(QString shortcutTogglePageLock READ shortcutTogglePageLock CONSTANT)
    Q_PROPERTY(QString shortcutMakeIntoSystem READ shortcutMakeIntoSystem CONSTANT)
    Q_PROPERTY(QString shortcutMakeIntoPage READ shortcutMakeIntoPage CONSTANT)
    Q_PROPERTY(bool allSystemsAreLocked READ allSystemsAreLocked NOTIFY allSystemsAreLockedChanged)
    Q_PROPERTY(bool allPagesAreLocked READ allPagesAreLocked NOTIFY allPagesAreLockedChanged)
    Q_PROPERTY(bool scoreIsInPageView READ scoreIsInPageView NOTIFY scoreIsInPageViewChanged)
    Q_PROPERTY(bool isMakeIntoSystemAvailable READ isMakeIntoSystemAvailable NOTIFY isMakeIntoSystemAvailableChanged)
    Q_PROPERTY(bool isMakeIntoPageAvailable READ isMakeIntoPageAvailable NOTIFY isMakeIntoPageAvailableChanged)
    Q_PROPERTY(int systemCount READ systemCount NOTIFY systemCountChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)

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

    Q_INVOKABLE void moveMeasureUpPage();
    QString shortcutMoveMeasureUpPage() const;
    Q_INVOKABLE void moveMeasureUpSystem();
    QString shortcutMoveMeasureUpSystem() const;

    Q_INVOKABLE void moveMeasureDownPage();
    QString shortcutMoveMeasureDownPage() const;
    Q_INVOKABLE void moveMeasureDownSystem();
    QString shortcutMoveMeasureDownSystem() const;

    Q_INVOKABLE void toggleSystemLock();
    QString shortcutToggleSystemLock() const;
    Q_INVOKABLE void togglePageLock();
    QString shortcutTogglePageLock() const;
    bool allSystemsAreLocked() const;
    bool allPagesAreLocked() const;

    Q_INVOKABLE void makeIntoSystem();
    QString shortcutMakeIntoSystem() const;
    Q_INVOKABLE void makeIntoPage();
    QString shortcutMakeIntoPage() const;

    bool scoreIsInPageView() const;
    bool isMakeIntoSystemAvailable() const;
    bool isMakeIntoPageAvailable() const;

    int systemCount() const;
    int pageCount() const;

protected:
    void onNotationChanged(const mu::engraving::PropertyIdSet&, const mu::engraving::StyleIdSet&) override;

private:
    void updateAllSystemsAreLocked();
    void updateAllPagesAreLocked();
    void updateScoreIsInPageView();
    void updateIsMakeIntoSystemAvailable();
    void updateIsMakeIntoPageAvailable();
    void updateSystemCount();
    void updatePageCount();

signals:
    void allSystemsAreLockedChanged(bool allLocked);
    void allPagesAreLockedChanged(bool allLocked);
    void scoreIsInPageViewChanged(bool isInPageView);
    void isMakeIntoSystemAvailableChanged(bool isMakeIntoSystemAvailable);
    void isMakeIntoPageAvailableChanged(bool isMakeIntoPageAvailable);
    void systemCountChanged(int count);
    void pageCountChanged(int count);

private:
    bool m_allSystemsAreLocked = false;
    bool m_allPagesAreLocked = false;
    bool m_scoreIsInPageView = false;
    bool m_isMakeIntoSystemAvailable = false;
    bool m_isMakeIntoPageAvailable = false;
    size_t m_systemCount = 0;
    size_t m_pageCount = 0;
};
}
