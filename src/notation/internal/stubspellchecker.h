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
#ifndef MU_NOTATION_STUBSPELLCHECKER_H
#define MU_NOTATION_STUBSPELLCHECKER_H

#include "../ispellchecker.h"

namespace mu::notation {
class StubSpellChecker : public ISpellChecker
{
public:
    bool isAvailable() const override { return false; }
    QString language() const override { return QString(); }
    QStringList availableLanguages() const override { return QStringList(); }
    bool setLanguage(const QString&) override { return false; }
    bool isCorrect(const QString&) const override { return true; }
    QStringList suggestions(const QString&) const override { return QStringList(); }
};
}

#endif // MU_NOTATION_STUBSPELLCHECKER_H
