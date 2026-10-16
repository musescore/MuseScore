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

#include "scoretexttranslator.h"

#include "instrumentnamestranslator.h"

#include "log.h"

using namespace mu::project;
using namespace mu::engraving;
using namespace muse;

ScoreTextTranslator::ScoreTextTranslator() = default;
ScoreTextTranslator::~ScoreTextTranslator() = default;

InstrumentNamesTranslator& ScoreTextTranslator::translator(const String& languageCode) const
{
    const QString code = languageCode.toQString();

    auto it = m_translators.find(code);
    if (it == m_translators.end()) {
        auto translator = std::make_unique<InstrumentNamesTranslator>();
        if (!translator->load(code)) {
            LOGW() << "No translations for language " << code;
        }
        it = m_translators.emplace(code, std::move(translator)).first;
    }

    return *it->second;
}

String ScoreTextTranslator::translate(const String& languageCode, const char* context, const String& source,
                                      const String& disambiguation) const
{
    if (languageCode.empty() || source.empty()) {
        return source;
    }

    return translator(languageCode).translate(context, source, disambiguation);
}

void ScoreTextTranslator::translateInstrumentNames(const String& languageCode, InstrumentTemplate& templ) const
{
    if (languageCode.empty()) {
        return;
    }

    translator(languageCode).translate(templ);
}
