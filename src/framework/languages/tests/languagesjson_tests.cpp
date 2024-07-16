/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include <gtest/gtest.h>

#include "io/ifilesystem.h"
#include "io/path.h"
#include "modularity/ioc.h"
#include "serialization/json.h"
#include "types/retval.h"

using namespace muse;

TEST(Languages_Json, Correctness)
{
    GlobalInject<io::IFileSystem> fileSystem;

    const io::path_t filePath(muse_languages_test_DATA_ROOT "/../../../../share/locale/languages.json");

    RetVal<ByteArray> data = fileSystem()->readFile(filePath);
    ASSERT_TRUE(data.ret);

    std::string err;
    JsonDocument json = JsonDocument::fromJson(data.val, &err);
    ASSERT_TRUE(err.empty());

    ASSERT_TRUE(json.isObject());

    JsonObject rootObject = json.rootObject();

    std::vector languageCodes = rootObject.keys();

    for (const std::string& languageCode : languageCodes) {
        SCOPED_TRACE("languageCode = " + languageCode);

        JsonValue languageVal = rootObject.value(languageCode);
        ASSERT_TRUE(languageVal.isObject());

        JsonObject languageObject = languageVal.toObject();

        EXPECT_TRUE(!languageObject.value("name").toStdString().empty());

        if (!languageObject.contains("fallbackLanguages")) {
            continue;
        }

        JsonValue fallbacksVal = languageObject.value("fallbackLanguages");
        ASSERT_TRUE(fallbacksVal.isArray());

        JsonArray fallbacksArray = languageObject.value("fallbackLanguages").toArray();

        for (size_t i = 0; i < fallbacksArray.size(); ++i) {
            SCOPED_TRACE("fallbacksArray[" + std::to_string(i) + "]");

            std::string fallback = fallbacksArray.at(i).toStdString();

            EXPECT_TRUE(!fallback.empty());

            // Fallback must not be the language itself
            EXPECT_NE(fallback, languageCode);

            // Fallback must be known language
            EXPECT_TRUE(muse::contains(languageCodes, fallback));
        }
    }
}
