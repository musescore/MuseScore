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
#include "translation.h"

using namespace mu::shortcuts;

EditShortcutModel::EditShortcutModel(QObject* parent)
    : QObject(parent)
{
}

void EditShortcutModel::load(const QString& sequence, const QVariantList& allShortcuts)
{
    clear();

    m_allShortcuts = allShortcuts;
    m_originSequence = sequence;
    emit originSequenceChanged(sequence);
}

void EditShortcutModel::clear()
{
    m_inputedSequence = QKeySequence();
    m_errorMessage.clear();

    emit inputedSequenceChanged(QString());
}

void EditShortcutModel::inputKey(int key, Qt::KeyboardModifiers modifiers)
{
    if (needIgnoreKey(key)) {
        return;
    }

    key += modifiers;

    switch (m_inputedSequence.count()) {
    case 0:
        m_inputedSequence = QKeySequence(key);
        break;
    case 1:
        m_inputedSequence = QKeySequence(m_inputedSequence[0], key);
        break;
    case 2:
        m_inputedSequence = QKeySequence(m_inputedSequence[0], m_inputedSequence[1], key);
        break;
    case 3:
        m_inputedSequence = QKeySequence(m_inputedSequence[0], m_inputedSequence[1], m_inputedSequence[2], key);
        break;
    }

    validateInputedSequence();

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

void EditShortcutModel::validateInputedSequence()
{
    m_errorMessage.clear();

    for (const QVariant& shortcut: m_allShortcuts) {
        QVariantMap map = shortcut.toMap();

        if (map["sequence"].toString() == inputedSequence()) {
            QString title = map["title"].toString();
            m_errorMessage = qtrc("shortcuts", "Shortcut conflicts with %1").arg(title);
            return;
        }
    }
}

QString EditShortcutModel::originSequence() const
{
    return m_originSequence;
}

QString EditShortcutModel::inputedSequence() const
{
    return m_inputedSequence.toString();
}

QString EditShortcutModel::errorMessage() const
{
    return m_errorMessage;
}

bool EditShortcutModel::canApplySequence() const
{
    return m_errorMessage.isEmpty() && !m_inputedSequence.isEmpty();
}

QString EditShortcutModel::unitedSequence() const
{
    QStringList sequences {
        m_originSequence,
        inputedSequence()
    };

    return sequences.join(", ");
}
