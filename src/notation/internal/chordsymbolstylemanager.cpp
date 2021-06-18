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
#include "chordsymbolstylemanager.h"
#include "framework/global/io/path.h"
#include "engraving/libmscore/mscore.h"

using namespace mu::notation;
ChordSymbolStyleManager::ChordSymbolStyleManager()
{
    retrieveChordStyles();
}

void ChordSymbolStyleManager::retrieveChordStyles()
{
    mu::io::paths filesFound = scanFileSystemForChordStyles();

    for (mu::io::path& file: filesFound) {
        if (isChordSymbolStylesFile(file)) {
            extractChordStyleInfo(file);
        }
    }
}

mu::io::paths ChordSymbolStyleManager::scanFileSystemForChordStyles()
{
    mu::io::paths result;

    mu::io::path dirPath = Ms::MScore::globalShare();
    mu::RetVal<mu::io::paths> files = fileSystem()->scanFiles(dirPath, { "*.xml" });

    result.insert(result.end(), files.val.begin(), files.val.end());
    return result;
}

QList<ChordSymbolStyle> ChordSymbolStyleManager::getChordStyles()
{
    return _chordStyles;
}

bool ChordSymbolStyleManager::isChordSymbolStylesFile(mu::io::path& f)
{
    bool isStyleFile = false;
    QString path = f.toQString();
    QFile file(path);
    Ms::XmlReader e(&file);
    QFileInfo fi(path);

    if (!file.open(QIODevice::ReadOnly)) {
        return isStyleFile;
    }

    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            if (e.attribute("type") == "chordStyle") {
                isStyleFile = true;
                break;
            }
        }
    }

    return isStyleFile;
}

void ChordSymbolStyleManager::extractChordStyleInfo(mu::io::path& f)
{
    QString path = f.toQString();
    QFile file(path);
    Ms::XmlReader e(&file);
    QFileInfo fi(path);
    QString styleName = "";

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            if (e.attribute("type") == "chordStyle") {
                styleName = e.attribute("styleName");
                break;
            }
        }
    }

    QHash<QString, QHash<QString, bool> > dummyDefaults = {
        { "major", { { "maj", 0 }, { "Ma", 1 } } },
        { "minor", { { "min", 0 }, { "m", 1 } } },
    };

    _chordStyles.push_back({ styleName, fi.fileName(), dummyDefaults });
}
