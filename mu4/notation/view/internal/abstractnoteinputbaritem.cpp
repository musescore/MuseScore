//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "abstractnoteinputbaritem.h"

#include "log.h"

using namespace mu::notation;

AbstractNoteInputBarItem::AbstractNoteInputBarItem(QObject* parent)
    : QObject(parent)
{
}

AbstractNoteInputBarItem::AbstractNoteInputBarItem(const ItemType& type, QObject* parent)
    : QObject(parent)
{
    setType(type);
}

QString AbstractNoteInputBarItem::id() const
{
    return m_id;
}

QString AbstractNoteInputBarItem::title() const
{
    return m_title;
}

int AbstractNoteInputBarItem::type() const
{
    return static_cast<int>(m_type);
}

void AbstractNoteInputBarItem::setType(const ItemType type)
{
    if (m_type == type) {
        return;
    }

    m_type = type;
    emit typeChanged(m_type);
}

void AbstractNoteInputBarItem::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged(m_title);
}

void AbstractNoteInputBarItem::setId(const QString& id)
{
    m_id = id;
}
