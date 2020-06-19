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
#include "shortcutsinstancemodel.h"

#include "log.h"

using namespace mu::shortcuts;

ShortcutsInstanceModel::ShortcutsInstanceModel(QObject* parent)
    : QObject(parent)
{
}

void ShortcutsInstanceModel::load()
{
    m_shortcuts.clear();

    const std::list<Shortcut>& shortcuts = shortcutsRegister()->shortcuts();
    for (const Shortcut& sc : shortcuts) {
        QString sequence = QString::fromStdString(sc.sequence);

        //! NOTE There may be several identical shortcuts for different contexts.
        //! We only need a list of unique ones.
        if (!m_shortcuts.contains(sequence)) {
            m_shortcuts << sequence;
        }
    }

    emit shortcutsChanged();
}

QStringList ShortcutsInstanceModel::shortcuts() const
{
    return m_shortcuts;
}

void ShortcutsInstanceModel::activate(const QString& key)
{
    controller()->activate(key.toStdString());
}
