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
#include "notationstyleeditor.h"

#include "notation.h"

#include "libmscore/excerpt.h"

using namespace mu;
using namespace mu::domain::notation;

NotationStyleEditor::NotationStyleEditor(Notation* notation)
    : m_notation(notation)
{
}

Style NotationStyleEditor::style() const
{
    return m_notation->score()->style();
}

void NotationStyleEditor::changeStyle(ChangeStyleVal* newStyleValue)
{
    newStyleValue->setScore(m_notation->score());
    m_notation->score()->undo(newStyleValue);
}

void NotationStyleEditor::update()
{
    m_styleChanged.notify();
}

bool NotationStyleEditor::isMaster() const
{
    return m_notation->score()->isMaster();
}

QList<QMap<QString, QString> > NotationStyleEditor::metaTags() const
{
    QList<QMap<QString, QString> > tags;

    if (isMaster()) {
        tags << m_notation->score()->masterScore()->metaTags();
    }

    tags << m_notation->score()->metaTags();
    return tags;
}

QString NotationStyleEditor::textStyleUserName(Tid tid)
{
    return m_notation->score()->getTextStyleUserName(tid);
}

void NotationStyleEditor::setConcertPitch(bool status)
{
    m_notation->score()->cmdConcertPitchChanged(status, true);
}

void NotationStyleEditor::startEdit()
{
    m_notation->score()->startCmd();
}

void NotationStyleEditor::apply()
{
    m_notation->score()->endCmd();
}

void NotationStyleEditor::applyAllParts()
{
    for (Ms::Excerpt* e : m_notation->score()->masterScore()->excerpts()) {
        e->partScore()->undo(new ChangeStyle(e->partScore(), style()));
        e->partScore()->update();
    }
}

void NotationStyleEditor::cancel()
{
    m_notation->score()->endCmd(true);
}

async::Notification NotationStyleEditor::styleChanged() const
{
    return m_styleChanged;
}
