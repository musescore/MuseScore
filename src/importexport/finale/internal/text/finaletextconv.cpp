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
#include "musx/musx.h"

#include "finaletextconv.h"

#include "engraving/infrastructure/smufl.h"

#include "io/dir.h"
#include "serialization/json.h"

#include "engraving/types/symnames.h"

#include "engraving/iengravingfont.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace mu::engraving;

namespace mu::iex::finale {

bool FinaleTextConv::init()
{
    bool ok = initConversionJson();

    return ok;
}

/// @todo
// MaestroTimes: import 194 as 2, 205 as 3, 202/203/193 as 2, 216 as 217 (no suitable smufl equivalent)"

String FinaleTextConv::symIdFromFinaleChar(char c, const std::string& font)
{
    std::vector<CharacterMap> characterList = muse::value(m_convertedFonts, font, {});
    char16_t newC = char16_t(c);
    if (characterList.empty()) {
        return String(newC);
    }
    for (size_t i = 0; i < characterList.size(); ++i) {
        auto [finaleCode, smuflCode, sym, index] = characterList.at(i);
        if (finaleCode > newC) {
            break;
        } else if (finaleCode != newC) {
            continue;
        }
        String newCharList(u"<sym>" + String::fromAscii(SymNames::nameForSymId(sym).ascii()) + u"</sym>");
        for (size_t j = i + 1; j < characterList.size(); ++j) {
            auto [finaleCodeB, smuflCodeB, symB, indexB] = characterList.at(j);
            if (indexB == 0) {
                break;
            }
            newCharList.append(u"<sym>" + String::fromAscii(SymNames::nameForSymId(symB).ascii()) + u"</sym>");
        }
        return newCharList;
    }
    return String(newC);
}

String FinaleTextConv::smuflCodeList(char c, const std::string& font)
{
    std::vector<std::tuple<char16_t, char16_t, SymId, int>> characterList = muse::value(m_convertedFonts, font, {});
    char16_t newC = char16_t(c);
    if (characterList.empty()) {
        return String(newC);
    }
    for (size_t i = 0; i < characterList.size(); ++i) {
        auto [finaleCode, smuflCode, sym, index] = characterList.at(i);
        if (finaleCode > newC) {
            break;
        } else if (finaleCode != newC) {
            continue;
        }
        String newCharList(smuflCode);
        for (size_t j = i + 1; j < characterList.size(); ++j) {
            auto [finaleCodeB, smuflCodeB, symB, indexB] = characterList.at(j);
            if (indexB == 0) {
                break;
            }
            newCharList.append(smuflCodeB);
        }
        return newCharList;
    }
    return String(newC);
}

bool FinaleTextConv::initConversionJson()
{
    m_convertedFonts.clear();

    Dir thirdPartyFontsDir(":/src/importexport/finale/third_party/FinaleToSMuFL/");
    RetVal<io::paths_t> thirdPartyFiles = thirdPartyFontsDir.scanFiles(thirdPartyFontsDir.path(), { "*.json" });
    if (!thirdPartyFiles.ret) {
        LOGE() << "Failed to scan files in directory: " << thirdPartyFontsDir.path();
        return false;
    }

    Dir homemadeFontsDir("./fontdata/");
    RetVal<io::paths_t> homemadeFiles = homemadeFontsDir.scanFiles(homemadeFontsDir.path(), { "*.json" });
    if (!homemadeFiles.ret) {
        LOGE() << "Failed to scan files in directory: " << homemadeFontsDir.path();
        return false;
    }

    io::paths_t allFiles = thirdPartyFiles.val;
    muse::join(allFiles, homemadeFiles.val);
    std::sort(allFiles.begin(), allFiles.end());
	// There are max. 2 json files per font: The default one and a manual patch.

    std::string prevFont;
    std::vector<CharacterMap> prevCharacterList;

    for (const io::path_t& file : allFiles) {
        /// @todo RGP what is fileSystem?
        RetVal<ByteArray> data;// = fileSystem->readFile(file);
        if (!data.ret) {
            LOGE() << "Failed to read file: " << file.toStdString();
            continue;
        }
        const std::string fileName = io::basename(file).toStdString();
        std::string error;
        JsonObject glyphNamesJson = JsonDocument::fromJson(data.val, &error).rootObject();

        if (!error.empty()) {
            LOGE() << "JSON parse error in file '" << fileName << "': " << error;
            return false;
        }

        IF_ASSERT_FAILED(!glyphNamesJson.empty()) {
            LOGE() << "Could not read file '" << fileName << "'.";
            return false;
        }

        std::vector<CharacterMap> characterList;

        for (std::string key : glyphNamesJson.keys()) {
            JsonObject symObj = glyphNamesJson.value(key).toObject();
            if (!symObj.isValid()) {
                continue;
            }

            // Symbol is not mapped to a SMuFL equivalent by default.
            if (symObj.contains("nameIsMakeMusic")) {
                continue;
            }

            SymId sym = SymNames::symIdByName(key);
            // We should never get here
            if (sym == SymId::noSym || sym == SymId::lastSym) {
                continue;
            }

            bool ok;
            char16_t smuflCode = symObj.value("codepoint").toString().mid(2).toUInt(&ok, 16);
            if (!ok) {
                // smuflCode = Smufl::smuflCode(sym); not matching types (16_t / 32_t)
                LOGD() << "could not read smufl codepoint for glyph " << key << "in font " << fileName;
                continue;
            }
            char16_t finaleCode = symObj.value("legacyCodepoint").toString().toUInt(&ok, 10);
            if (!ok) {
                LOGD() << "could not read finale codepoint for glyph " << key << "in font " << fileName;
                continue;
            }
			// compound symbols
			int index = symObj.contains("index") ? symObj.value("index").toInt() : 0;
            characterList.emplace_back(std::make_tuple(finaleCode, smuflCode, sym, index));
        }
		if (fileName == prevFont) {
			prevCharacterList = characterList;
			prevFont = fileName;
		} else {
            muse::join(characterList, prevCharacterList);
			prevCharacterList.clear();
			prevFont.clear();
			if (characterList.empty()) {
				continue;
			}
			std::sort(characterList.begin(), characterList.end(),[] (const auto& a, const auto& b) {
				auto [finaleCodeA, smuflCodeA, symA, indexA] = a;
                auto [finaleCodeB, smuflCodeB, symB, indexB] = b;
				if (finaleCodeA == finaleCodeB) {
					return indexA < indexB;
				}
				return finaleCodeA < finaleCodeB;
			});
            m_convertedFonts.emplace(std::make_pair(fileName, characterList));
		}
    }
    return true;
}

}
