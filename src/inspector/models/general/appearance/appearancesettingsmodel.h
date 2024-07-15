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
#ifndef MU_INSPECTOR_APPEARANCESETTINGSMODEL_H
#define MU_INSPECTOR_APPEARANCESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

#include "notation/inotationconfiguration.h"

namespace mu::inspector {
class AppearanceSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    INJECT(notation::INotationConfiguration, notationConfiguration)

    Q_PROPERTY(PropertyItem * leadingSpace READ leadingSpace CONSTANT)
    Q_PROPERTY(PropertyItem * measureWidth READ measureWidth CONSTANT)
    Q_PROPERTY(PropertyItem * minimumDistance READ minimumDistance CONSTANT)
    Q_PROPERTY(PropertyItem * color READ color CONSTANT)
    Q_PROPERTY(PropertyItem * arrangeOrder READ arrangeOrder CONSTANT)
    Q_PROPERTY(PropertyItem * offset READ offset CONSTANT)
    Q_PROPERTY(bool isSnappedToGrid READ isSnappedToGrid WRITE setIsSnappedToGrid NOTIFY isSnappedToGridChanged)
    Q_PROPERTY(bool isVerticalOffsetAvailable READ isVerticalOffsetAvailable NOTIFY isVerticalOffsetAvailableChanged)

public:
    explicit AppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void pushBackwardsInOrder();
    Q_INVOKABLE void pushForwardsInOrder();
    Q_INVOKABLE void pushToBackInOrder();
    Q_INVOKABLE void pushToFrontInOrder();

    Q_INVOKABLE void configureGrid();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* leadingSpace() const;
    PropertyItem* measureWidth() const;
    PropertyItem* minimumDistance() const;
    PropertyItem* color() const;
    PropertyItem* arrangeOrder() const;
    PropertyItem* offset() const;

    bool isSnappedToGrid() const;

    bool isVerticalOffsetAvailable() const;

public slots:
    void setIsSnappedToGrid(bool isSnapped);
    void setIsVerticalOffsetAvailable(bool isAvailable);

signals:
    void isSnappedToGridChanged(bool isSnappedToGrid);
    void isVerticalOffsetAvailableChanged(bool isVerticalOffsetAvailable);

private:
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;
    void loadProperties(const mu::engraving::PropertyIdSet& allowedPropertyIdSet);

    void updateIsVerticalOffsetAvailable();

    mu::engraving::Page* page() const;
    std::vector<mu::engraving::EngravingItem*> allElementsInPage() const;
    std::vector<mu::engraving::EngravingItem*> allOverlappingElements() const;

    PropertyItem* m_leadingSpace = nullptr;
    PropertyItem* m_measureWidth = nullptr;
    PropertyItem* m_minimumDistance = nullptr;
    PropertyItem* m_color = nullptr;
    PropertyItem* m_arrangeOrder = nullptr;
    PointFPropertyItem* m_offset = nullptr;

    bool m_isVerticalOffsetAvailable = true;

    QList<engraving::EngravingItem*> m_elementsForOffsetProperty;
    QList<engraving::EngravingItem*> m_elementsForArrangeProperty;
};
}

#endif // MU_INSPECTOR_APPEARANCESETTINGSMODEL_H
