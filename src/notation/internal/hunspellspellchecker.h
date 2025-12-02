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
#ifndef MU_NOTATION_HUNSPELLSPELLCHECKER_H
#define MU_NOTATION_HUNSPELLSPELLCHECKER_H

#include "../ispellchecker.h"

#ifdef MUE_BUILD_NOTATION_HUNSPELL
#include <memory>
class Hunspell;
#endif

namespace mu::notation {
class HunspellSpellChecker : public ISpellChecker
{
public:
    HunspellSpellChecker();
    ~HunspellSpellChecker() override;

    bool isAvailable() const override;
    QString language() const override;
    QStringList availableLanguages() const override;
    bool setLanguage(const QString& langCode) override;

    bool isCorrect(const QString& word) const override;
    QStringList suggestions(const QString& word) const override;

private:
    void discoverDictionaries();
    QString findDictionaryPath(const QString& langCode) const;

    struct DictionaryInfo {
        QString langCode;
        QString affPath;
        QString dicPath;
    };

#ifdef MUE_BUILD_NOTATION_HUNSPELL
    std::unique_ptr<Hunspell> m_hunspell;
#endif
    QString m_currentLanguage;
    QList<DictionaryInfo> m_availableDictionaries;
};
}

#endif // MU_NOTATION_HUNSPELLSPELLCHECKER_H
