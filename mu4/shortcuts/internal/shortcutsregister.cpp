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

#include "log.h"

using namespace mu::shortcuts;

void ShortcutsRegister::load()
{
//    m_shortcuts.push_back(Shortcut { "1", "sample/1" });
//    m_shortcuts.push_back(Shortcut { "2", "sample/2" });

    m_shortcuts.clear();

    std::string userPath = globalConfiguration()->dataPath() + "/shortcuts.xml";
    bool ok = loadFromFile(m_shortcuts, userPath);
    if (!ok) {
        ok = loadFromFile(m_shortcuts, ":/data/shortcuts.xml");
    }

    if (!ok) {
        LOGE() << "Failed load shortcuts.xml";
    }
}

bool ShortcutsRegister::loadFromFile(std::list<Shortcut>& shortcuts, const std::string& path) const
{
    QFile f(QString::fromStdString(path));
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
                    shortcuts.push_back(shortcut);
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
