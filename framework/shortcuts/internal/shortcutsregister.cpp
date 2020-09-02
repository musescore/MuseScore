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

#include <QFile>
#include <QXmlStreamReader>
#include <QKeySequence>

#include "log.h"

using namespace mu::shortcuts;

void ShortcutsRegister::load()
{
    m_shortcuts.clear();

    io::path userPath = globalConfiguration()->dataPath() + "/shortcuts.xml";
    bool ok = loadFromFile(m_shortcuts, userPath);
    if (!ok) {
        ok = loadFromFile(m_shortcuts, ":/data/shortcuts.xml");
    }

    if (ok) {
        expandStandartKeys(m_shortcuts);
    }

    if (!ok) {
        LOGE() << "Failed load shortcuts.xml";
    }
}

void ShortcutsRegister::expandStandartKeys(std::list<Shortcut>& shortcuts) const
{
    std::list<Shortcut> expanded;
    std::list<Shortcut> notbonded;
    for (Shortcut& sc : shortcuts) {
        if (!sc.sequence.empty()) {
            continue;
        }

        QList<QKeySequence> kslist = QKeySequence::keyBindings(sc.standartKey);
        if (kslist.isEmpty()) {
            LOGW() << "not bind key sequence for standart key: " << sc.standartKey;
            notbonded.push_back(sc);
            continue;
        }

        const QKeySequence& first = kslist.first();
        sc.sequence = first.toString().toStdString();
        //LOGD() << "for standart key: " << sc.standartKey << ", sequence: " << sc.sequence;

        //! NOTE If the keyBindings contains more than one result,
        //! these can be considered alternative shortcuts on the same platform for the given key.
        for (int i = 1; i < kslist.count(); ++i) {
            const QKeySequence& seq = kslist.at(i);
            Shortcut esc = sc;
            esc.sequence = seq.toString().toStdString();
            //LOGD() << "for standart key: " << esc.standartKey << ", alternative sequence: " << esc.sequence;
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

bool ShortcutsRegister::loadFromFile(std::list<Shortcut>& shortcuts, const io::path& path) const
{
    QFile f(path.toQString());
    if (!f.exists()) {
        LOGE() << "Not exists shortcuts file: " << path;
        return false;
    }

    if (!f.open(QIODevice::ReadOnly)) {
        LOGE() << "Cannot open shortcuts file: " << path;
        return false;
    }

    QXmlStreamReader xml(&f);

    while (xml.readNextStartElement()) {
        if (xml.name() == "Shortcuts") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "SC") {
                    Shortcut shortcut;
                    while (xml.readNextStartElement()) {
                        const QStringRef& tag(xml.name());
                        if (tag == "key") {
                            shortcut.action = xml.readElementText().toStdString();
                        } else if (tag == "std") {
                            shortcut.standartKey = QKeySequence::StandardKey(xml.readElementText().toInt());
                        } else if (tag == "seq") {
                            shortcut.sequence = xml.readElementText().toStdString();
                        } else {
                            xml.skipCurrentElement();
                        }
                    }

                    if (shortcut.isValid()) {
                        shortcuts.push_back(shortcut);
                    }
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else {
            xml.skipCurrentElement();
        }
    }

    return true;
}

const std::list<Shortcut>& ShortcutsRegister::shortcuts() const
{
    return m_shortcuts;
}

std::list<Shortcut> ShortcutsRegister::shortcutsForSequence(const std::string& sequence) const
{
    std::list<Shortcut> list;
    for (const Shortcut& sh : m_shortcuts) {
        if (sh.sequence == sequence) {
            list.push_back(sh);
        }
    }
    return list;
}
