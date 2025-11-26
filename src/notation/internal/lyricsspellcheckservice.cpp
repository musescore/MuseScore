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
#include "lyricsspellcheckservice.h"

#include <QTextDocument>
#include <QMap>

#include "engraving/dom/lyrics.h"
#include "engraving/dom/score.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/part.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/chordrest.h"
#include "engraving/types/types.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

LyricsSpellCheckResult LyricsSpellCheckService::checkLyrics(INotationPtr notation)
{
    LyricsSpellCheckResult result;

    if (!spellChecker()) {
        result.spellCheckerAvailable = false;
        result.errorMessage = "Spell checker not initialized";
        return result;
    }

    if (!spellChecker()->isAvailable()) {
        result.spellCheckerAvailable = false;
        result.errorMessage = "No spelling dictionaries found. Please install hunspell dictionaries for your language.";
        return result;
    }

    result.spellCheckerAvailable = true;
    result.language = spellChecker()->language();

    if (!notation) {
        result.errorMessage = "No score open";
        return result;
    }

    Score* score = notation->elements()->msScore();
    if (!score) {
        result.errorMessage = "No score available";
        return result;
    }

    // Collect all lyrics
    std::vector<Lyrics*> allLyrics = score->lyrics();
    result.totalLyricsChecked = static_cast<int>(allLyrics.size());

    if (allLyrics.empty()) {
        return result; // No lyrics to check
    }

    // Build words from syllables and track occurrences
    // Key: lowercase word, Value: issue with occurrences
    QMap<QString, LyricsSpellCheckIssue> issueMap;

    // Track word building state per track/verse combination
    struct WordBuilder {
        QString currentWord;
        QList<Lyrics*> currentSyllables;
    };
    QMap<QPair<int, int>, WordBuilder> wordBuilders; // key: (track, verse)

    auto checkAndRecordWord = [&](const QString& word, const QList<Lyrics*>& syllables) {
        if (word.isEmpty() || syllables.isEmpty()) {
            return;
        }

        QString cleanWord = cleanWordForSpellCheck(word);
        if (cleanWord.isEmpty()) {
            return;
        }

        result.totalWordsChecked++;

        if (!spellChecker()->isCorrect(cleanWord)) {
            QString key = cleanWord.toLower();

            if (issueMap.contains(key)) {
                issueMap[key].occurrenceCount++;
                issueMap[key].allOccurrences.append(syllables.first());
            } else {
                LyricsSpellCheckIssue issue;
                issue.word = cleanWord;
                issue.occurrenceCount = 1;
                issue.firstLyrics = syllables.first();
                issue.firstLocation = formatLocation(syllables.first());
                issue.allOccurrences.append(syllables.first());
                issueMap[key] = issue;
            }
        }
    };

    // Process lyrics in order, reconstructing words from syllables
    for (Lyrics* lyric : allLyrics) {
        if (!lyric) {
            continue;
        }

        int track = static_cast<int>(lyric->track());
        int verse = lyric->verse();
        QPair<int, int> key(track, verse);

        QString syllableText = lyric->plainText().trimmed();
        LyricsSyllabic syllabic = lyric->syllabic();

        WordBuilder& builder = wordBuilders[key];

        switch (syllabic) {
        case LyricsSyllabic::SINGLE:
            // Complete any pending word first
            checkAndRecordWord(builder.currentWord, builder.currentSyllables);
            builder.currentWord.clear();
            builder.currentSyllables.clear();

            // Check this single-syllable word
            checkAndRecordWord(syllableText, { lyric });
            break;

        case LyricsSyllabic::BEGIN:
            // Complete any pending word first
            checkAndRecordWord(builder.currentWord, builder.currentSyllables);

            // Start new word
            builder.currentWord = syllableText;
            builder.currentSyllables = { lyric };
            break;

        case LyricsSyllabic::MIDDLE:
            // Continue building word
            builder.currentWord += syllableText;
            builder.currentSyllables.append(lyric);
            break;

        case LyricsSyllabic::END:
            // Complete the word
            builder.currentWord += syllableText;
            builder.currentSyllables.append(lyric);
            checkAndRecordWord(builder.currentWord, builder.currentSyllables);
            builder.currentWord.clear();
            builder.currentSyllables.clear();
            break;
        }
    }

    // Check any remaining incomplete words
    for (auto it = wordBuilders.begin(); it != wordBuilders.end(); ++it) {
        checkAndRecordWord(it.value().currentWord, it.value().currentSyllables);
    }

    // Convert map to list
    result.issues = issueMap.values();

    // Sort by word
    std::sort(result.issues.begin(), result.issues.end(), [](const LyricsSpellCheckIssue& a, const LyricsSpellCheckIssue& b) {
        return a.word.toLower() < b.word.toLower();
    });

    LOGI() << "Spell check complete:" << result.issues.size() << "issues found,"
           << result.totalWordsChecked << "words checked";

    return result;
}

QString LyricsSpellCheckService::formatLocation(Lyrics* lyrics) const
{
    if (!lyrics) {
        return QString();
    }

    // Get measure number (1-based)
    Measure* measure = lyrics->measure();
    int measureNo = measure ? measure->no() + 1 : 0;

    // Get staff name
    QString staffName;
    Staff* staff = lyrics->staff();
    if (staff && staff->part()) {
        QTextDocument doc;
        doc.setHtml(staff->part()->longName(lyrics->tick()));
        staffName = doc.toPlainText();
        if (staffName.isEmpty()) {
            staffName = staff->partName();
        }
    }
    if (staffName.isEmpty()) {
        staffName = QString("Staff %1").arg(lyrics->staffIdx() + 1);
    }

    // Get verse number (1-based)
    int verseNo = lyrics->verse() + 1;

    return QString("m. %1, %2, Verse %3")
           .arg(measureNo)
           .arg(staffName)
           .arg(verseNo);
}

QString LyricsSpellCheckService::cleanWordForSpellCheck(const QString& word) const
{
    QString cleaned = word.trimmed();

    // Remove leading/trailing punctuation but keep internal apostrophes
    while (!cleaned.isEmpty() && !cleaned.at(0).isLetterOrNumber()) {
        cleaned = cleaned.mid(1);
    }
    while (!cleaned.isEmpty() && !cleaned.at(cleaned.length() - 1).isLetterOrNumber()) {
        cleaned.chop(1);
    }

    return cleaned;
}

bool LyricsSpellCheckService::isAvailable() const
{
    return spellChecker() && spellChecker()->isAvailable();
}

QStringList LyricsSpellCheckService::availableLanguages() const
{
    return spellChecker() ? spellChecker()->availableLanguages() : QStringList();
}

QString LyricsSpellCheckService::currentLanguage() const
{
    return spellChecker() ? spellChecker()->language() : QString();
}

bool LyricsSpellCheckService::setLanguage(const QString& langCode)
{
    return spellChecker() ? spellChecker()->setLanguage(langCode) : false;
}
