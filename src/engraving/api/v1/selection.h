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

#ifndef MU_ENGRAVING_APIV1_SELECTION_H
#define MU_ENGRAVING_APIV1_SELECTION_H

// api
#include "elements.h"

namespace mu::engraving::apiv1 {
//---------------------------------------------------------
//   Selection
//    Wrapper class for internal mu::engraving::Selection
///  \since MuseScore 3.3
//---------------------------------------------------------

class Selection : public QObject
{
    Q_OBJECT
    /// Current GUI selections for the score.
    /// \since MuseScore 3.3
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> elements READ elements)

    /// Whether this selection covers a range of a score, as opposed to
    /// a list of distinct elements.
    /// \since MuseScore 3.5
    Q_PROPERTY(bool isRange READ isRange)
    /// Start segment of selection, included. This property is valid
    /// only for range selection.
    /// \since MuseScore 3.5
    /// \see \ref isRange
    Q_PROPERTY(apiv1::Segment * startSegment READ startSegment)
    /// End segment of selection, excluded. This property is valid
    /// only for range selection.
    /// \since MuseScore 3.5
    /// \see \ref isRange
    Q_PROPERTY(apiv1::Segment * endSegment READ endSegment)
    /// First staff of selection, included. This property is valid
    /// only for range selection.
    /// \since MuseScore 3.5
    /// \see \ref isRange
    Q_PROPERTY(int startStaff READ startStaff)
    /// End staff of selection, included. This property is valid
    /// only for range selection.
    /// \since MuseScore 3.5
    /// \see \ref isRange
    Q_PROPERTY(int endStaff READ endStaff)

protected:
    /// \cond MS_INTERNAL
    mu::engraving::Selection* m_selection;

    bool checkSelectionIsNotLocked() const;
    /// \endcond
public:
    /// \cond MS_INTERNAL
    Selection(mu::engraving::Selection* selection)
        : QObject(), m_selection(selection) {}
    virtual ~Selection() { }

    QQmlListProperty<EngravingItem> elements()
    { return wrapContainerProperty<EngravingItem>(this, m_selection->elements()); }

    bool isRange() const { return m_selection->isRange(); }

    Segment* startSegment() const { return wrap<Segment>(m_selection->startSegment()); }
    Segment* endSegment() const { return wrap<Segment>(m_selection->endSegment()); }
    int startStaff() const { return static_cast<int>(m_selection->staffStart()); }
    int endStaff() const { return static_cast<int>(m_selection->staffEnd()); }
    /// \endcond

    Q_INVOKABLE bool select(apiv1::EngravingItem* e, bool add = false);
    Q_INVOKABLE bool selectRange(int startTick, int endTick, int startStaff, int endStaff);
    Q_INVOKABLE bool deselect(apiv1::EngravingItem* e);
    Q_INVOKABLE bool clear();
};

extern Selection* selectionWrap(mu::engraving::Selection* selection);
}

#endif
