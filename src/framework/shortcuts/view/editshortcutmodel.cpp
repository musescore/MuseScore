//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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

#include "editshortcutmodel.h"

#include "log.h"

using namespace mu::shortcuts;

EditShortcutModel::EditShortcutModel(QObject* parent)
    : QObject(parent)
{
}

void EditShortcutModel::load(const QString& sequence)
{
    m_originSequence = sequence;
    emit originSequenceChanged(sequence);
}

void EditShortcutModel::handleKey(int key, Qt::KeyboardModifiers modifiers)
{
    if (needIgnoreKey(key)) {
        return;
    }

    key += modifiers;

    switch (m_sequence.count()) {
    case 0:
        m_sequence = QKeySequence(key);
        break;
    case 1:
        m_sequence = QKeySequence(m_sequence[0], key);
        break;
    case 2:
        m_sequence = QKeySequence(m_sequence[0], m_sequence[1], key);
        break;
    case 3:
        m_sequence = QKeySequence(m_sequence[0], m_sequence[1], m_sequence[2], key);
        break;
    }

    emit inputedSequenceChanged(inputedSequence());
}

bool EditShortcutModel::needIgnoreKey(int key) const
{
    if (key == 0) {
        return true;
    }

    static const QSet<Qt::Key> ignoredKeys {
        Qt::Key_Shift,
        Qt::Key_Control,
        Qt::Key_Meta,
        Qt::Key_Alt,
        Qt::Key_AltGr,
        Qt::Key_CapsLock,
        Qt::Key_NumLock,
        Qt::Key_ScrollLock,
        Qt::Key_unknown
    };

    return ignoredKeys.contains(static_cast<Qt::Key>(key));
}

QString EditShortcutModel::originSequence() const
{
    return m_originSequence;
}

QString EditShortcutModel::inputedSequence() const
{
    return m_sequence.toString();
}

QString EditShortcutModel::unitedSequence() const
{
    NOT_IMPLEMENTED;
    return QString();
}
