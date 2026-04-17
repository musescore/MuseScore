/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <QColor>
#include <QVariantMap>

#include "ui/internal/themeconverter.h"
#include "ui/uitypes.h"

using namespace muse;
using namespace muse::ui;

class Ui_ThemeConverterTests : public ::testing::Test
{
};

TEST_F(Ui_ThemeConverterTests, FromMap_SkipsUnknownKeys)
{
    //! [GIVEN] A map containing metadata keys, a known style key, and an unknown key
    QVariantMap map;
    map["codeKey"] = "dark";
    map["title"] = "Dark";
    map["backgroundPrimaryColor"] = QColor("#2D2D30");
    map["someRandomKey"] = QColor("#FF0000");

    //! [WHEN] Deserializing via fromMap
    ThemeInfo theme = ThemeConverter::fromMap(map);

    //! [THEN] Only the known style key is in values; UNKNOWN is not present
    EXPECT_EQ(theme.codeKey, "dark");
    EXPECT_EQ(theme.title, "Dark");
    EXPECT_TRUE(theme.values.contains(BACKGROUND_PRIMARY_COLOR));
    EXPECT_FALSE(theme.values.contains(UNKNOWN));
    EXPECT_EQ(theme.values.size(), 1);
}

TEST_F(Ui_ThemeConverterTests, FromMap_DoesNotPopulateExtra)
{
    //! [GIVEN] A map with known style keys
    QVariantMap map;
    map["codeKey"] = "dark";
    map["title"] = "Dark";
    map["accentColor"] = QColor("#2093FE");
    map["fontPrimaryColor"] = QColor("#EBEBEB");

    //! [WHEN] Deserializing via fromMap
    ThemeInfo theme = ThemeConverter::fromMap(map);

    //! [THEN] The extra map remains empty
    EXPECT_TRUE(theme.extra.isEmpty());
}

TEST_F(Ui_ThemeConverterTests, ToMap_DoesNotSerializeExtra)
{
    //! [GIVEN] A theme with both values and extra entries
    ThemeInfo theme;
    theme.codeKey = "dark";
    theme.title = "Dark";
    theme.values[ACCENT_COLOR] = QColor("#2093FE");
    theme.extra["custom_highlight"] = QColor("#FF0000");
    theme.extra["custom_shadow"] = QColor("#000000");

    //! [WHEN] Serializing via toMap
    QVariantMap map = ThemeConverter::toMap(theme);

    //! [THEN] Known keys are present but extra keys are not serialized
    EXPECT_TRUE(map.contains("codeKey"));
    EXPECT_TRUE(map.contains("accentColor"));
    EXPECT_FALSE(map.contains("custom_highlight"));
    EXPECT_FALSE(map.contains("custom_shadow"));
}

TEST_F(Ui_ThemeConverterTests, ToMap_FromMap_RoundTrip_PreservesKnownValues)
{
    //! [GIVEN] A theme with color, width, and opacity values
    ThemeInfo original;
    original.codeKey = "dark";
    original.title = "Dark";
    original.values[BACKGROUND_PRIMARY_COLOR] = QColor("#2D2D30");
    original.values[ACCENT_COLOR] = QColor("#2093FE");
    original.values[FONT_PRIMARY_COLOR] = QColor("#EBEBEB");
    original.values[BORDER_WIDTH] = 1.0;
    original.values[ACCENT_OPACITY_NORMAL] = 0.5;

    //! [WHEN] Round-tripping through toMap then fromMap
    QVariantMap map = ThemeConverter::toMap(original);
    ThemeInfo roundTripped = ThemeConverter::fromMap(map);

    //! [THEN] codeKey and all values survive the round-trip
    EXPECT_EQ(roundTripped.codeKey, original.codeKey);

    for (auto key : original.values.keys()) {
        ASSERT_TRUE(roundTripped.values.contains(key))
            << "Missing key after round-trip: " << static_cast<int>(key);
        EXPECT_EQ(roundTripped.values[key], original.values[key])
            << "Value mismatch for key: " << static_cast<int>(key);
    }
}

TEST_F(Ui_ThemeConverterTests, MergeScenario_ExtrasPreservedAfterRoundTrip)
{
    //! CASE Simulates the full updateThemes() data flow:
    //!      writeThemes (toMap) -> readThemes (fromMap) -> merge into cfg-backed theme

    //! [GIVEN] A cfg-backed theme with values AND extras (as makeStandardTheme produces)
    ThemeInfo baseTheme;
    baseTheme.codeKey = "dark";
    baseTheme.title = "Dark";
    baseTheme.values[BACKGROUND_PRIMARY_COLOR] = QColor("#2D2D30");
    baseTheme.values[ACCENT_COLOR] = QColor("#2093FE");
    baseTheme.values[FONT_PRIMARY_COLOR] = QColor("#EBEBEB");
    baseTheme.values[BORDER_WIDTH] = 0.0;
    baseTheme.extra["accent_color"] = QColor("#2093FE");
    baseTheme.extra["background_primary_color"] = QColor("#2D2D30");
    baseTheme.extra["custom_plugin_color"] = QColor("#AABBCC");

    //! [GIVEN] The theme is serialized to settings (extras are dropped by toMap)
    QVariantMap serialized = ThemeConverter::toMap(baseTheme);

    //! [GIVEN] The user customizes accent color
    serialized["accentColor"] = QColor("#FF5500");

    //! [WHEN] The theme is deserialized from settings and merged back
    //!        (replicating the updateThemes loop from uiconfiguration.cpp)
    ThemeInfo fromSettings = ThemeConverter::fromMap(serialized);
    for (auto key : fromSettings.values.keys()) {
        if (key != UNKNOWN) {
            baseTheme.values[key] = fromSettings.values[key];
        }
    }

    //! [THEN] The customized value is applied
    EXPECT_EQ(baseTheme.values[ACCENT_COLOR].value<QColor>(), QColor("#FF5500"));

    //! [THEN] Other values are preserved
    EXPECT_EQ(baseTheme.values[BACKGROUND_PRIMARY_COLOR].value<QColor>(), QColor("#2D2D30"));
    EXPECT_EQ(baseTheme.values[FONT_PRIMARY_COLOR].value<QColor>(), QColor("#EBEBEB"));

    //! [THEN] Extras are fully intact
    EXPECT_EQ(baseTheme.extra.size(), 3);
    EXPECT_EQ(baseTheme.extra["accent_color"].value<QColor>(), QColor("#2093FE"));
    EXPECT_EQ(baseTheme.extra["background_primary_color"].value<QColor>(), QColor("#2D2D30"));
    EXPECT_EQ(baseTheme.extra["custom_plugin_color"].value<QColor>(), QColor("#AABBCC"));
}
