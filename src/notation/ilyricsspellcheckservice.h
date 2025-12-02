/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#ifndef MU_NOTATION_ILYRICSSPELLCHECKSERVICE_H
#define MU_NOTATION_ILYRICSSPELLCHECKSERVICE_H

#include <QString>
#include <QList>

#include "modularity/imoduleinterface.h"
#include "notation/inotation.h"

namespace mu::engraving {
class Lyrics;
}

namespace mu::notation {
struct LyricsSpellCheckIssue {
    QString word;
    int occurrenceCount = 0;
    QString firstLocation;
    mu::engraving::Lyrics* firstLyrics = nullptr;
    QList<mu::engraving::Lyrics*> allOccurrences;
};

struct LyricsSpellCheckResult {
    bool spellCheckerAvailable = false;
    QString language;
    QString errorMessage;
    QList<LyricsSpellCheckIssue> issues;
    int totalLyricsChecked = 0;
    int totalWordsChecked = 0;
};

class ILyricsSpellCheckService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ILyricsSpellCheckService)

public:
    virtual ~ILyricsSpellCheckService() = default;

    virtual LyricsSpellCheckResult checkLyrics(INotationPtr notation) = 0;
    virtual bool isAvailable() const = 0;
    virtual QStringList availableLanguages() const = 0;
    virtual QString currentLanguage() const = 0;
    virtual bool setLanguage(const QString& langCode) = 0;
};
}

#endif // MU_NOTATION_ILYRICSSPELLCHECKSERVICE_H
