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
#ifndef MU_NOTATION_CHORDSYMBOLSTYLEMANAGER_H
#define MU_NOTATION_CHORDSYMBOLSTYLEMANAGER_H

#include "modularity/ioc.h"
#include "system/ifilesystem.h"
#include "notation/inotationconfiguration.h"
#include "engraving/infrastructure/io/xml.h"

namespace mu::notation {
// Probably have to rename. name already used in harmony.cpp
struct ChordSymbolStyle {
    QString styleName;
    QString fileName;
    bool usePresets = false;
    QString description = "";
};

struct QualitySymbol {
    QString qualitySymbol = "";
    qreal qualMag = -1;
    qreal qualAdjust = 999;
    qreal extMag = -1;
    qreal extAdjust = 999;
    qreal modMag = -1;
    qreal modAdjust = 999;
};

class ChordSymbolStyleManager
{
    INJECT(notation, mu::system::IFileSystem, fileSystem)
    INJECT(notation, mu::notation::INotationConfiguration, configuration)

public:
    ChordSymbolStyleManager();

    mu::io::paths scanFileSystemForChordStyles();
    void retrieveChordStyles();
    bool isChordSymbolStylesFile(mu::io::path& f);
    void extractChordStyleInfo(mu::io::path& f);

    QList<ChordSymbolStyle> getChordStyles();
    QHash<QString, QList<QualitySymbol> > getQualitySymbols(QString path);

private:
    QList<ChordSymbolStyle> _chordStyles;
};
}

#endif // CHORDSYMBOLSTYLEMANAGER_H
