/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "shortcutsregister.h"

#include <QKeySequence>

#include "log.h"

#include "global/xmlreader.h"
#include "global/xmlwriter.h"
#include "multiinstances/resourcelockguard.h"

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

    io::path defPath = configuration()->shortcutsAppDataPath();
    io::path userPath = configuration()->shortcutsUserAppDataPath();

    bool ok = readFromFile(m_defaultShortcuts, defPath);

    if (ok) {
        {
            //! NOTE The user shortcut file may change, so we need to lock it
            mi::ResourceLockGuard(multiInstancesProvider(), "shortcuts");
            ok = readFromFile(m_shortcuts, userPath);
        }
        if (!ok) {
            m_shortcuts = m_defaultShortcuts;
        } else {
            mergeShortcuts(m_shortcuts, m_defaultShortcuts);
        }
        ok = true;
    }

    if (ok) {
        expandStandardKeys(m_shortcuts);
        m_shortcutsChanged.notify();
    }
}

void ShortcutsRegister::mergeShortcuts(ShortcutList& shortcuts, const ShortcutList& defaultShortcuts) const
{
    ShortcutList needadd;
    for (const Shortcut& sh : defaultShortcuts) {
        auto it = std::find_if(shortcuts.begin(), shortcuts.end(), [sh](const Shortcut& i) {
            return i.action == sh.action;
        });

        if (it == shortcuts.end()) {
            needadd.push_back(sh);
        }
    }

    if (!needadd.empty()) {
        shortcuts.splice(shortcuts.end(), needadd);
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
    TRACEFUNC;

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
            if (shortcut.sequence.empty()) {
                shortcuts.push_back(shortcut);
            } else {
                auto seqList = QString::fromStdString(shortcut.sequence).split("\n", Qt::SkipEmptyParts);
                for (QString seq : seqList) {
                    shortcut.sequence = seq.toStdString();
                    shortcuts.push_back(shortcut);
                }
            }
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
            shortcut.sequence += reader.readString() + "\n";
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

    bool ok = writeToFile(shortcuts, configuration()->shortcutsUserAppDataPath());

    if (ok) {
        m_shortcuts = shortcuts;
        mergeShortcuts(m_shortcuts, m_defaultShortcuts);
        m_shortcutsChanged.notify();
    }

    return ok;
}

bool ShortcutsRegister::writeToFile(const ShortcutList& shortcuts, const io::path& path) const
{
    TRACEFUNC;

    mi::ResourceLockGuard(multiInstancesProvider(), "shortcuts");

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

mu::Ret ShortcutsRegister::importFromFile(const io::path& filePath)
{
    mi::ResourceLockGuard(multiInstancesProvider(), "shortcuts");

    Ret ret = fileSystem()->copy(filePath, configuration()->shortcutsUserAppDataPath(), true);
    if (!ret) {
        LOGE() << "failed import file: " << ret.toString();
        return ret;
    }

    load();

    return make_ret(Ret::Code::Ok);
}

mu::Ret ShortcutsRegister::exportToFile(const io::path& filePath) const
{
    return writeToFile(m_shortcuts, filePath);
}
