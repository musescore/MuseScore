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
#include "shortcutsregister.h"

#include <QKeySequence>

#include "log.h"

#include "global/xmlreader.h"

using namespace mu::shortcuts;
using namespace mu::framework;

void ShortcutsRegister::load()
{
    m_shortcuts.clear();

    io::path shortcutsPath = configuration()->shortcutsUserPath();
    bool ok = loadFromFile(m_shortcuts, shortcutsPath);

    if (!ok) {
        shortcutsPath = configuration()->shortcutsDefaultPath();
        ok = loadFromFile(m_shortcuts, shortcutsPath);
    }

    if (ok) {
        expandStandardKeys(m_shortcuts);
    }
}

void ShortcutsRegister::expandStandardKeys(ShortcutList& shortcuts) const
{
    ShortcutList expanded;
    ShortcutList notbonded;

    for (Shortcut& shortcut : shortcuts) {
        if (!shortcut.sequence.empty()) {
            continue;
        }

        QList<QKeySequence> kslist = QKeySequence::keyBindings(shortcut.standardKey);
        if (kslist.isEmpty()) {
            LOGW() << "not bind key sequence for standard key: " << shortcut.standardKey;
            notbonded.push_back(shortcut);
            continue;
        }

        const QKeySequence& first = kslist.first();
        shortcut.sequence = first.toString().toStdString();
        //LOGD() << "for standard key: " << sc.standardKey << ", sequence: " << sc.sequence;

        //! NOTE If the keyBindings contains more than one result,
        //! these can be considered alternative shortcuts on the same platform for the given key.
        for (int i = 1; i < kslist.count(); ++i) {
            const QKeySequence& seq = kslist.at(i);
            Shortcut esc = shortcut;
            esc.sequence = seq.toString().toStdString();
            //LOGD() << "for standard key: " << esc.standardKey << ", alternative sequence: " << esc.sequence;
            expanded.push_back(esc);
        }
    }

    if (!notbonded.empty()) {
        LOGD() << "removed " << notbonded.size() << " shortcut, because they are not bound to standard key";
        for (const Shortcut& sc : notbonded) {
            shortcuts.remove(sc);
        }
    }

    if (!expanded.empty()) {
        LOGD() << "added " << expanded.size() << " shortcut, because they are alternative shortcuts for the given standard keys";

        shortcuts.splice(shortcuts.end(), expanded);
    }
}

bool ShortcutsRegister::loadFromFile(ShortcutList& shortcuts, const io::path& path) const
{
    XmlReader reader(path);

    reader.readNextStartElement();
    if (reader.tagName() != "Shortcuts") {
        return false;
    }

    while (reader.readNextStartElement()) {
        if (reader.tagName() != "SC") {
            reader.skipCurrentElement();
            continue;
        }

        Shortcut shortcut = readShortcut(reader);
        if (shortcut.isValid()) {
            shortcuts.push_back(shortcut);
        }
    }

    if (!reader.success()) {
        LOGE() << "failed parse xml, error: " << reader.error() << ", path: " << path;
    }

    return reader.success();
}

Shortcut ShortcutsRegister::readShortcut(framework::XmlReader& reader) const
{
    Shortcut shortcut;

    while (reader.readNextStartElement()) {
        std::string tag(reader.tagName());

        if (tag == "key") {
            shortcut.action = reader.readString();
        } else if (tag == "std") {
            shortcut.standardKey = QKeySequence::StandardKey(reader.readInt());
        } else if (tag == "seq") {
            shortcut.sequence = reader.readString();
        } else {
            reader.skipCurrentElement();
        }
    }

    return shortcut;
}

const ShortcutList& ShortcutsRegister::shortcuts() const
{
    return m_shortcuts;
}

Shortcut ShortcutsRegister::shortcut(const std::string& actionCode) const
{
    for (const Shortcut& shortcut: m_shortcuts) {
        if (shortcut.action == actionCode) {
            return shortcut;
        }
    }

    return Shortcut();
}

ShortcutList ShortcutsRegister::shortcutsForSequence(const std::string& sequence) const
{
    ShortcutList list;
    for (const Shortcut& sh : m_shortcuts) {
        if (sh.sequence == sequence) {
            list.push_back(sh);
        }
    }
    return list;
}
