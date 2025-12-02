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
#include "hunspellspellchecker.h"

#include <QDir>
#include <QStandardPaths>
#include <QLocale>

#ifdef MUE_BUILD_NOTATION_HUNSPELL
#include <hunspell/hunspell.hxx>
#endif

#include "log.h"

using namespace mu::notation;

HunspellSpellChecker::HunspellSpellChecker()
{
    discoverDictionaries();

    // Try to set language from system locale
    QString systemLang = QLocale::system().name(); // e.g., "en_US"
    if (!setLanguage(systemLang)) {
        // Try just the language part
        QString langOnly = systemLang.split('_').first(); // e.g., "en"
        for (const DictionaryInfo& dict : m_availableDictionaries) {
            if (dict.langCode.startsWith(langOnly)) {
                setLanguage(dict.langCode);
                break;
            }
        }
    }
}

HunspellSpellChecker::~HunspellSpellChecker()
{
}

void HunspellSpellChecker::discoverDictionaries()
{
    m_availableDictionaries.clear();

    QStringList searchPaths;

#ifdef Q_OS_LINUX
    searchPaths << "/usr/share/hunspell"
                << "/usr/share/myspell"
                << "/usr/share/myspell/dicts"
                << QDir::homePath() + "/.local/share/hunspell";
#endif

#ifdef Q_OS_MACOS
    searchPaths << "/Library/Spelling"
                << QDir::homePath() + "/Library/Spelling"
                << "/opt/homebrew/share/hunspell"
                << "/usr/local/share/hunspell";
#endif

#ifdef Q_OS_WIN
    searchPaths << QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/hunspell"
                << "C:/Program Files/hunspell";
#endif

    for (const QString& searchPath : searchPaths) {
        QDir dir(searchPath);
        if (!dir.exists()) {
            continue;
        }

        QStringList affFiles = dir.entryList(QStringList() << "*.aff", QDir::Files);
        for (const QString& affFile : affFiles) {
            QString baseName = affFile.left(affFile.length() - 4); // Remove .aff
            QString dicFile = baseName + ".dic";

            if (dir.exists(dicFile)) {
                DictionaryInfo info;
                info.langCode = baseName;
                info.affPath = dir.absoluteFilePath(affFile);
                info.dicPath = dir.absoluteFilePath(dicFile);
                m_availableDictionaries.append(info);

                LOGD() << "Found dictionary:" << info.langCode << "at" << info.affPath;
            }
        }
    }

    LOGI() << "Discovered" << m_availableDictionaries.size() << "dictionaries";
}

bool HunspellSpellChecker::isAvailable() const
{
#ifdef MUE_BUILD_NOTATION_HUNSPELL
    return m_hunspell != nullptr;
#else
    return false;
#endif
}

QString HunspellSpellChecker::language() const
{
    return m_currentLanguage;
}

QStringList HunspellSpellChecker::availableLanguages() const
{
    QStringList languages;
    for (const DictionaryInfo& dict : m_availableDictionaries) {
        languages << dict.langCode;
    }
    return languages;
}

bool HunspellSpellChecker::setLanguage(const QString& langCode)
{
#ifdef MUE_BUILD_NOTATION_HUNSPELL
    QString affPath;
    QString dicPath;

    for (const DictionaryInfo& dict : m_availableDictionaries) {
        if (dict.langCode == langCode) {
            affPath = dict.affPath;
            dicPath = dict.dicPath;
            break;
        }
    }

    if (affPath.isEmpty() || dicPath.isEmpty()) {
        LOGW() << "Dictionary not found for language:" << langCode;
        return false;
    }

    try {
        m_hunspell = std::make_unique<Hunspell>(affPath.toStdString().c_str(),
                                                dicPath.toStdString().c_str());
        m_currentLanguage = langCode;
        LOGI() << "Loaded dictionary:" << langCode;
        return true;
    } catch (const std::exception& e) {
        LOGE() << "Failed to load dictionary:" << e.what();
        m_hunspell.reset();
        return false;
    }
#else
    Q_UNUSED(langCode);
    return false;
#endif
}

bool HunspellSpellChecker::isCorrect(const QString& word) const
{
#ifdef MUE_BUILD_NOTATION_HUNSPELL
    if (!m_hunspell) {
        return true; // Assume correct if no spellchecker
    }

    // Skip empty strings and numbers
    if (word.isEmpty()) {
        return true;
    }

    bool hasLetter = false;
    for (const QChar& ch : word) {
        if (ch.isLetter()) {
            hasLetter = true;
            break;
        }
    }
    if (!hasLetter) {
        return true;
    }

    return m_hunspell->spell(word.toStdString());
#else
    Q_UNUSED(word);
    return true;
#endif
}

QStringList HunspellSpellChecker::suggestions(const QString& word) const
{
#ifdef MUE_BUILD_NOTATION_HUNSPELL
    QStringList result;
    if (!m_hunspell) {
        return result;
    }

    std::vector<std::string> suggestList = m_hunspell->suggest(word.toStdString());
    for (const std::string& s : suggestList) {
        result << QString::fromStdString(s);
    }
    return result;
#else
    Q_UNUSED(word);
    return QStringList();
#endif
}
