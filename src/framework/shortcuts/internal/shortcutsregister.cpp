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
#include "global/xmlwriter.h"

using namespace mu::shortcuts;
using namespace mu::framework;
using namespace mu::async;

constexpr std::string_view SHORTCUTS_TAG("Shortcuts");
constexpr std::string_view SHORTCUT_TAG("SC");
constexpr std::string_view ACTION_CODE_TAG("key");
constexpr std::string_view STANDARD_KEY_TAG("std");
constexpr std::string_view SEQUENCE_TAG("seq");

static const Shortcut& findShortcut(const ShortcutList& shortcuts, const std::string& actionCode)
{
    for (const Shortcut& shortcut: shortcuts) {
        if (shortcut.action == actionCode) {
            return shortcut;
        }
    }

    static Shortcut null;
    return null;
}

void ShortcutsRegister::load()
{
    m_shortcuts.clear();

    ValCh<io::path> userPath = configuration()->shortcutsUserPath();
    userPath.ch.onReceive(this, [this](const io::path&) {
        load();
    });

    bool ok = readFromFile(m_defaultShortcuts, configuration()->shortcutsDefaultPath());

    if (ok) {
        ok = readFromFile(m_shortcuts, userPath.val);

        if (!ok) {
            m_shortcuts = m_defaultShortcuts;
            ok = true;
        }
    }

    if (ok) {
        expandStandardKeys(m_shortcuts);
        m_shortcutsChanged.notify();
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

bool ShortcutsRegister::readFromFile(ShortcutList& shortcuts, const io::path& path) const
{
    XmlReader reader(path);

    reader.readNextStartElement();
    if (reader.tagName() != SHORTCUTS_TAG) {
        return false;
    }

    while (reader.readNextStartElement()) {
        if (reader.tagName() != SHORTCUT_TAG) {
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

        if (tag == ACTION_CODE_TAG) {
            shortcut.action = reader.readString();
        } else if (tag == STANDARD_KEY_TAG) {
            shortcut.standardKey = QKeySequence::StandardKey(reader.readInt());
        } else if (tag == SEQUENCE_TAG) {
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

mu::Ret ShortcutsRegister::setShortcuts(const ShortcutList& shortcuts)
{
    if (shortcuts == m_shortcuts) {
        return true;
    }

    bool ok = writeToFile(shortcuts, configuration()->shortcutsUserPath().val);

    if (ok) {
        m_shortcuts = shortcuts;
        m_shortcutsChanged.notify();
    }

    return ok;
}

bool ShortcutsRegister::writeToFile(const ShortcutList& shortcuts, const io::path& path) const
{
    TRACEFUNC;

    XmlWriter writer(path);

    writer.writeStartDocument();
    writer.writeStartElement(SHORTCUTS_TAG);

    for (const Shortcut& shortcut : shortcuts) {
        writeShortcut(writer, shortcut);
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    return writer.success();
}

void ShortcutsRegister::writeShortcut(framework::XmlWriter& writer, const Shortcut& shortcut) const
{
    writer.writeStartElement(SHORTCUT_TAG);
    writer.writeTextElement(ACTION_CODE_TAG, shortcut.action);

    if (shortcut.standardKey != QKeySequence::UnknownKey) {
        writer.writeTextElement(STANDARD_KEY_TAG, QString("%1").arg(shortcut.standardKey).toStdString());
    }

    writer.writeTextElement(SEQUENCE_TAG, shortcut.sequence);
    writer.writeEndElement();
}

Notification ShortcutsRegister::shortcutsChanged() const
{
    return m_shortcutsChanged;
}

const Shortcut& ShortcutsRegister::shortcut(const std::string& actionCode) const
{
    return findShortcut(m_shortcuts, actionCode);
}

const Shortcut& ShortcutsRegister::defaultShortcut(const std::string& actionCode) const
{
    return findShortcut(m_defaultShortcuts, actionCode);
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

mu::Ret ShortcutsRegister::saveToFile(const io::path& filePath) const
{
    return writeToFile(m_shortcuts, filePath);
}
