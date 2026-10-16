/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "global/modularity/imoduleinterface.h"
#include "global/types/string.h"

namespace mu::engraving {
class InstrumentTemplate;

//! Name of the meta tag that holds the language of the texts written into a score (e.g. instrument names),
//! if it was chosen independently of the language of the interface. Missing or empty means the interface language.
inline const muse::String SCORE_TEXT_LANGUAGE_META_TAG = u"textLanguage";

//! Translates strings of the program into a language other than the interface language,
//! so that the texts written into a score can be in the language of the score.
class IScoreTextTranslator : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IScoreTextTranslator)

public:
    virtual ~IScoreTextTranslator() = default;

    //! Returns the translation of \a source into the language, or \a source itself if there is none
    virtual muse::String translate(const muse::String& languageCode, const char* context, const muse::String& source,
                                   const muse::String& disambiguation = muse::String()) const = 0;

    //! Gives the names of the template (long name, short name, transposition and track name) in the language,
    //! as if it were the language of the interface
    virtual void translateInstrumentNames(const muse::String& languageCode, InstrumentTemplate& templ) const = 0;
};
}
