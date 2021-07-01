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

    QFileInfo ftest(path);
    if (!ftest.isAbsolute()) {
#if defined(Q_OS_IOS)
        path = QString("%1/%2").arg(Ms::MScore::globalShare()).arg(path);
#elif defined(Q_OS_ANDROID)
        path = QString(":/styles/%1").arg(path);
#else
        path = QString("%1styles/%2").arg(Ms::MScore::globalShare(), path);
#endif
    }

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

    QFileInfo ftest(path);
    if (!ftest.isAbsolute()) {
#if defined(Q_OS_IOS)
        path = QString("%1/%2").arg(Ms::MScore::globalShare()).arg(path);
#elif defined(Q_OS_ANDROID)
        path = QString(":/styles/%1").arg(path);
#else
        path = QString("%1styles/%2").arg(Ms::MScore::globalShare(), path);
#endif
    }

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

    _chordStyles.push_back({ styleName, fi.fileName() });
}

// TODO: Rewrite the XML format
QHash<QString, QStringList> ChordSymbolStyleManager::getQualitySymbols(QString path)
{
    QHash<QString, QStringList> qualitySymbols;

    QFileInfo ftest(path);
    if (!ftest.isAbsolute()) {
#if defined(Q_OS_IOS)
        path = QString("%1/%2").arg(Ms::MScore::globalShare()).arg(path);
#elif defined(Q_OS_ANDROID)
        path = QString(":/styles/%1").arg(path);
#else
        path = QString("%1styles/%2").arg(Ms::MScore::globalShare(), path);
#endif
    }

    QFile file(path);
    Ms::XmlReader e(&file);

    if (!file.open(QIODevice::ReadOnly)) {
        return qualitySymbols;
    }

    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            if (e.attribute("type") == "chordStyle") {
                // Inside the MuseScore element now, start reading the quality symbols
                while (e.readNextStartElement()) {
                    if (e.name() == "quality") {
                        QString qual = e.attribute("q");
                        QStringList rep;
                        while (e.readNextStartElement()) {
                            if (e.name() == "sym") {
                                rep << e.readElementText();
                            }
                        }
                        qualitySymbols.insert(qual, rep);
                    } else if (e.name() == "modifier") {
                        // Created a separate tag for omit just in case
                        // in the future, if modifiers are handled differently
                        QString mod = e.attribute("m");
                        QStringList rep;
                        while (e.readNextStartElement()) {
                            if (e.name() == "sym") {
                                rep << e.readElementText();
                            }
                        }
                        qualitySymbols.insert(mod, rep);
                    }
                }
            }
        }
    }

    return qualitySymbols;
}
