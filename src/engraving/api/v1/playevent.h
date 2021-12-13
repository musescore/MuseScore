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

#ifndef MU_ENGRAVING_APIV1_PLAYEVENT_H
#define MU_ENGRAVING_APIV1_PLAYEVENT_H

#include <QQmlEngine>
#include <QQmlListProperty>

#include "engraving/dom/noteevent.h"

namespace mu::engraving::apiv1 {
class Note;

//---------------------------------------------------------
//   PlayEvent
//    Wrapper class for internal mu::engraving::NoteEvent
//
//   This is based on the wrapper in excerpt.h.
///  \since MuseScore 3.3
//---------------------------------------------------------

class PlayEvent : public QObject
{
    Q_OBJECT
    /// The relative pitch to the note pitch.
    /// This is added to the parent note's actual pitch.
    /// \since MuseScore 3.3
    Q_PROPERTY(int pitch READ pitch WRITE setPitch)
    /// Time to turn on the note event.
    /// This value is expressed in 1/1000 of the nominal note length.
    /// \since MuseScore 3.3
    Q_PROPERTY(int ontime READ ontime WRITE setOntime)
    /// The length of time for the event.
    /// This value is expressed in 1/1000 of the nominal note length.
    /// \since MuseScore 3.3
    Q_PROPERTY(int len READ len WRITE setLen)
    /// Time note will turn off.
    /// This value derived from onTime and len. This value is expressed
    /// in 1/1000 of the nominal note length.
    /// \since MuseScore 3.3
    Q_PROPERTY(int offtime READ offtime)
    /// \cond MS_INTERNAL

protected:
    mu::engraving::NoteEvent* ne;
    Note* parentNote;

public:

    PlayEvent(mu::engraving::NoteEvent* _ne = new mu::engraving::NoteEvent(), Note* _parent = nullptr)
        : QObject(), ne(_ne), parentNote(_parent) {}
    // Delete the NoteEvent if parentless.
    virtual ~PlayEvent()
    {
        if (parentNote == nullptr) {
            delete ne;
        }
    }

    const mu::engraving::NoteEvent& getNoteEvent() { return *ne; }
    void setParentNote(Note* parent) { this->parentNote = parent; }
    Note* note() { return parentNote; }

    int pitch() const { return ne->pitch(); }
    int ontime() const { return ne->ontime(); }
    int offtime() const { return ne->offtime(); }
    int len() const { return ne->len(); }
    void setPitch(int v);
    void setOntime(int v);
    void setLen(int v);
    /// \endcond
};

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   \relates mu::engraving::NoteEvent
//---------------------------------------------------------

inline PlayEvent* playEventWrap(mu::engraving::NoteEvent* t, Note* parent)
{
    PlayEvent* w = t ? new PlayEvent(t, parent) : nullptr;
    // All wrapper objects should belong to JavaScript code.
    QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
    return w;
}

//---------------------------------------------------------
//   QML access to containers of NoteEvent
//
//   QmlNoteEventsListAccess provides a convenience interface
//   for QQmlListProperty providing access to plugins for NoteEvent
//   Containers.
//
//   Based on QmlListAccess in excerpt.h
//---------------------------------------------------------

class QmlPlayEventsListAccess : public QQmlListProperty<PlayEvent>
{
public:
    QmlPlayEventsListAccess(QObject* obj, engraving::NoteEventList& container)
        : QQmlListProperty<PlayEvent>(obj, &container, &append, &count, &at, &clear) {}

    static qsizetype count(QQmlListProperty<PlayEvent>* l) { return static_cast<engraving::NoteEventList*>(l->data)->size(); }
    static PlayEvent* at(QQmlListProperty<PlayEvent>* l, qsizetype i)
    {
        return playEventWrap(&(*(static_cast<engraving::NoteEventList*>(l->data)))[i], reinterpret_cast<Note*>(l->object));
    }

    static void clear(QQmlListProperty<PlayEvent>* l);
    static void append(QQmlListProperty<PlayEvent>* l, PlayEvent* v);
};

/** \cond PLUGIN_API \private \endcond */
inline QmlPlayEventsListAccess wrapPlayEventsContainerProperty(QObject* obj, engraving::NoteEventList& c)
{
    return QmlPlayEventsListAccess(obj, c);
}
}

#endif
