/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include <map>
#include <memory>

#include <QString>

#include "engraving/iscoretexttranslator.h"

namespace mu::project {
class InstrumentNamesTranslator;

//! Translates the texts of a score into the language of the score, see IScoreTextTranslator.
//! The translation files of each language are loaded the first time they are needed, and kept.
class ScoreTextTranslator : public engraving::IScoreTextTranslator
{
public:
    ScoreTextTranslator();
    ~ScoreTextTranslator() override;

    muse::String translate(const muse::String& languageCode, const char* context, const muse::String& source,
                           const muse::String& disambiguation = muse::String()) const override;

    void translateInstrumentNames(const muse::String& languageCode, engraving::InstrumentTemplate& templ) const override;

private:
    InstrumentNamesTranslator& translator(const muse::String& languageCode) const;

    mutable std::map<QString, std::unique_ptr<InstrumentNamesTranslator> > m_translators;
};
}
