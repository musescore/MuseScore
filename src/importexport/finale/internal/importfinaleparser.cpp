/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "internal/importfinaleparser.h"
#include "internal/importfinalelogger.h"
#include "internal/finaletypesconv.h"

#include <vector>
#include <exception>

#include "musx/musx.h"

#include "global/stringutils.h"
#include "types/string.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;
using namespace mu::iex::finale;

namespace mu::iex::finale {

FinaleParser::FinaleParser(engraving::Score* score, const std::shared_ptr<musx::dom::Document>& doc, FinaleLoggerPtr& logger)
    : m_score(score), m_doc(doc), m_logger(logger)
{
    m_finaleOptions.init(*this);
    const std::vector<IEngravingFontPtr> fonts = engravingFonts()->fonts();
    for (const IEngravingFontPtr& font : fonts) {
        m_engravingFonts.emplace(muse::strings::toLower(font->name()), font);
    }
}

void FinaleParser::parse()
{
    // scoremap
    importParts();
    importBrackets();
    importMeasures();
}

}
