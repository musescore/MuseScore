//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGIN_API_SELECTION_H__
#define __PLUGIN_API_SELECTION_H__

#include "elements.h"
#include "score.h"

namespace Ms {
namespace PluginAPI {
//---------------------------------------------------------
//   Selection
//    Wrapper class for internal Ms::Selection
///  \since MuseScore 3.3
//---------------------------------------------------------

class Selection : public QObject
{
    Q_OBJECT
    /// Current GUI selections for the score.
    /// \since MuseScore 3.3
    Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Element> elements READ elements)

    /**
     * Whether this selection covers a range of a score, as opposed to
     * a list of distinct elements.
     * \since MuseScore 3.5
     */
    Q_PROPERTY(bool isRange READ isRange)
    /**
     * Start segment of selection, included. This property is valid
     * only for range selection.
     * \since MuseScore 3.5
     * \see \ref isRange
     */
    Q_PROPERTY(Ms::PluginAPI::Segment* startSegment READ startSegment)
    /**
     * End segment of selection, excluded. This property is valid
     * only for range selection.
     * \since MuseScore 3.5
     * \see \ref isRange
     */
    Q_PROPERTY(Ms::PluginAPI::Segment* endSegment READ endSegment)
    /**
     * First staff of selection, included. This property is valid
     * only for range selection.
     * \since MuseScore 3.5
     * \see \ref isRange
     */
    Q_PROPERTY(int startStaff READ startStaff)
    /**
     * End staff of selection, included. This property is valid
     * only for range selection.
     * \since MuseScore 3.5
     * \see \ref isRange
     */
    Q_PROPERTY(int endStaff READ endStaff)

protected:
    /// \cond MS_INTERNAL
    Ms::Selection* _select;

    bool checkSelectionIsNotLocked() const;
    /// \endcond
public:
    /// \cond MS_INTERNAL
    Selection(Ms::Selection* select)
        : QObject(), _select(select) {}
    virtual ~Selection() { }

    QQmlListProperty<Element> elements()
    { return wrapContainerProperty<Element>(this, _select->elements()); }

    bool isRange() const { return _select->isRange(); }

    Segment* startSegment() const { return wrap<Segment>(_select->startSegment()); }
    Segment* endSegment() const { return wrap<Segment>(_select->endSegment()); }
    int startStaff() const { return _select->staffStart(); }
    int endStaff() const { return _select->staffEnd(); }
    /// \endcond

    Q_INVOKABLE bool select(Ms::PluginAPI::Element* e, bool add = false);
    Q_INVOKABLE bool selectRange(int startTick, int endTick, int startStaff, int endStaff);
    Q_INVOKABLE bool deselect(Ms::PluginAPI::Element* e);
    Q_INVOKABLE bool clear();
};

extern Selection* selectionWrap(Ms::Selection* select);
} // namespace PluginAPI
} // namespace Ms
#endif
