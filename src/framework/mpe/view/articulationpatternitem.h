/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MUSE_MPE_ARTICULATIONPATTERNITEM_H
#define MUSE_MPE_ARTICULATIONPATTERNITEM_H

#include <QAbstractListModel>
#include <QList>

#include "mpetypes.h"
#include "articulationpatternsegmentitem.h"

namespace muse::mpe {
class ArticulationPatternItem : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool isSelected READ isSelected WRITE setIsSelected NOTIFY isSelectedChanged)
    Q_PROPERTY(bool isSingleNoteType READ isSingleNoteType CONSTANT)
    Q_PROPERTY(
        ArticulationPatternSegmentItem
        * currentPatternSegment READ currentPatternSegment WRITE setCurrentPatternSegment NOTIFY currentPatternSegmentChanged)

public:
    explicit ArticulationPatternItem(QObject* parent, const ArticulationType type, const bool isSingleNoteType);

    enum Roles {
        PatternSegmentItem = Qt::UserRole + 1,
    };

    const QString& title() const;
    const ArticulationType& type() const;
    ArticulationPattern patternData() const;

    ArticulationPatternSegmentItem* currentPatternSegment() const;
    void setCurrentPatternSegment(ArticulationPatternSegmentItem* newCurrentPattern);

    void load(const ArticulationPattern& pattern);
    Q_INVOKABLE void appendNewSegment();
    Q_INVOKABLE void removeCurrentSegment();
    Q_INVOKABLE bool isAbleToRemoveCurrentSegment();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool isActive() const;
    void setIsActive(bool newIsChecked);

    bool isSingleNoteType() const;

    bool isSelected() const;
    void setIsSelected(bool newIsSelected);

signals:
    void titleChanged();
    void currentPatternSegmentChanged();
    void requestedToBeRemoved();

    void isActiveChanged();
    void isSelectedChanged();

private:
    void updateType(const ArticulationType type);

    ArticulationPatternSegment buildBlankPatternSegment() const;

    ArticulationType m_type = ArticulationType::Undefined;
    QString m_title;
    ArticulationPatternSegmentItem* m_currentPattern = nullptr;

    QList<ArticulationPatternSegmentItem*> m_items;

    bool m_isActive = false;
    bool m_isSingleNoteType = true;
    bool m_isSelected = false;
};
}

#endif // MUSE_MPE_ARTICULATIONPATTERNITEM_H
