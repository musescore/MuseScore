/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include <QFontDatabase>

#include "draw/internal/fontprovider.h"
#include "draw/internal/fontsdatabase.h"
#include "draw/internal/fontsengine.h"
#include "draw/internal/qfontprovider.h"
#include "global/modularity/ioc.h"

using namespace muse;
using namespace muse::draw;

class Draw_FontsProviderQtTests : public ::testing::Test
{
public:
    struct Env {
        Env()
            : xProvider(muse::modularity::globalCtx()),
            fontsDatabase(std::make_shared<FontsDatabase>()),
            fontsEngine(std::make_shared<FontsEngine>(muse::modularity::globalCtx()))
        {
            muse::modularity::globalIoc()->registerExport<IFontsDatabase>("draw", fontsDatabase);
            muse::modularity::globalIoc()->registerExport<IFontsEngine>("draw", fontsEngine);

            loadFont("Edwin", edwinPath());
            loadFont("FreeSerif", freeSerifPath());
            loadFont("Leland", lelandPath());
            loadFont("Bravura", bravuraPath());

            fontsDatabase->addFont(FontDataKey(u"Edwin"), edwinPath());
            fontsDatabase->addFont(FontDataKey(u"FreeSerif"), freeSerifPath());
            fontsDatabase->addFont(FontDataKey(u"Leland"), lelandPath());
            fontsDatabase->addFont(FontDataKey(u"Bravura"), bravuraPath());

            fontsDatabase->setDefaultFont(Font::Type::Unknown, FontDataKey(u"Edwin"));
            fontsDatabase->setDefaultFont(Font::Type::Text, FontDataKey(u"Edwin"));
            fontsDatabase->setDefaultFont(Font::Type::MusicSymbol, FontDataKey(u"Leland"));

            qProvider.addSymbolFont(u"Leland", lelandPath());
            qProvider.addSymbolFont(u"Bravura", bravuraPath());
            fontsEngine->init();
        }

        ~Env()
        {
            muse::modularity::globalIoc()->unregister<IFontsEngine>("draw");
            muse::modularity::globalIoc()->unregister<IFontsDatabase>("draw");
        }

        static muse::io::path_t fontRoot()
        {
            return MUSE_DRAW_TEST_FONTS_DIR;
        }

        static muse::io::path_t edwinPath()
        {
            return fontRoot() + "/edwin/Edwin-Roman.otf";
        }

        static muse::io::path_t freeSerifPath()
        {
            return fontRoot() + "/FreeSerif.ttf";
        }

        static muse::io::path_t lelandPath()
        {
            return fontRoot() + "/leland/Leland.otf";
        }

        static muse::io::path_t bravuraPath()
        {
            return fontRoot() + "/bravura/Bravura.otf";
        }

        static void loadFont(const QString& expectedFamily, const muse::io::path_t& path)
        {
            int qtFontId = QFontDatabase::addApplicationFont(path.toQString());
            EXPECT_NE(qtFontId, -1) << "Unable to load font: " << path.toStdString();

            if (qtFontId != -1) {
                QStringList qtFamilies = QFontDatabase::applicationFontFamilies(qtFontId);
                EXPECT_TRUE(qtFamilies.contains(expectedFamily))
                    << "Qt loaded families: " << qtFamilies.join(", ").toStdString();
            }
        }

        QFontProvider qProvider;
        FontProvider xProvider;
        std::shared_ptr<FontsDatabase> fontsDatabase;
        std::shared_ptr<FontsEngine> fontsEngine;
    };
};

static const std::vector<double> TEST_POINTSIZES = { 5.62, 12.0, 18.0, 24.0, 32.0, 48.0 };
static const std::vector<muse::Char> TEST_CHARS
    = { muse::Char(u'a'), muse::Char(u'x'), muse::Char(u' '), muse::Char(u'"'), muse::Char(u'\''),
        muse::Char(u'*'), muse::Char(u'-'), muse::Char(u':'), muse::Char(u';'),
        muse::Char(u'~') };

static const std::vector<muse::String> TEST_STRINGS = { muse::String(u"qwer"), muse::String(u"qwer\nqwer"), muse::String(u"qwerqwer"),
                                                        muse::String(u"AW"), muse::String(u"musescore"), muse::String(u"musegroup"),
                                                        muse::String(u"getmedrink"), muse::String(u"pop") };

static const std::vector<char16_t> TEST_SYMBOLS = {
    0xE050, 0xE05C, 0xE062, 0xE0A2, 0xE0A3, 0xE0A4,
    0xE1D7, 0xE1D9, 0xE260, 0xE261, 0xE262,
    0xE4A0, 0xE4A2, 0xE4E3, 0xE520, 0xE521, 0xE522,
    0xE560, 0xE561, 0xE562, 0xE565, 0xE566, 0xE56A,
    0xE4E5, 0xE4E6, 0xE4E7, 0xE4E8, 0xE4E9, 0xE4EA
};

static std::string to_string(const muse::RectF& r)
{
    return "x: " + std::to_string(r.x())
           + " y: " + std::to_string(r.y())
           + " w: " + std::to_string(r.width())
           + " h: " + std::to_string(r.height());
}

static std::string to_string(double v)
{
    return std::to_string(v);
}

inline bool ValIsEqual(double p1, double p2, double epsilon)
{
    return std::abs(std::max(p1, p2) - std::min(p1, p2)) <= epsilon;
}

inline bool ValIsEqual(muse::RectF p1, muse::RectF p2, double epsilon)
{
    return ValIsEqual(p1.top(), p2.top(), epsilon) && ValIsEqual(p1.left(), p2.left(), epsilon)
           && ValIsEqual(p1.width(), p2.width(), epsilon) && ValIsEqual(p1.height(), p2.height(), epsilon);
}

template<typename T>
static ::testing::AssertionResult ValuesMatch(double pointSize, const T& qVal, const T& xVal, double epsilon)
{
    if (ValIsEqual(qVal, xVal, epsilon)) {
        return ::testing::AssertionSuccess();
    }

    return ::testing::AssertionFailure()
           << "pointSize: " << pointSize
           << "\n    q_val: " << to_string(qVal)
           << "\n    x_val: " << to_string(xVal);
}

TEST_F(Draw_FontsProviderQtTests, lineSpacing)
{
    Env env;
    Font f(u"Edwin", Font::Type::Text);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        double qVal = env.qProvider.lineSpacing(f);
        double xVal = env.xProvider.lineSpacing(f);

        EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.05));
    }
}

TEST_F(Draw_FontsProviderQtTests, insertSubstitution)
{
    Env env;
    env.fontsDatabase->insertSubstitution(u"Edwin", u"FreeSerif");

    std::vector<FontDataKey> substitutions = env.fontsDatabase->substitutionFonts(FontDataKey(u"Edwin", true, false));

    ASSERT_EQ(1u, substitutions.size());
    EXPECT_EQ(substitutions.front(), FontDataKey(u"FreeSerif"));
    EXPECT_TRUE(env.fontsDatabase->substitutionFonts(FontDataKey(u"Leland")).empty());
}

TEST_F(Draw_FontsProviderQtTests, xHeight)
{
    Env env;
    Font f(u"Edwin", Font::Type::Text);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        double qVal = env.qProvider.xHeight(f);
        double xVal = env.xProvider.xHeight(f);

        EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.2));
    }
}

TEST_F(Draw_FontsProviderQtTests, height)
{
    Env env;
    Font f(u"Edwin", Font::Type::Text);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        double qVal = env.qProvider.height(f);
        double xVal = env.xProvider.height(f);

        EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.8));
    }
}

TEST_F(Draw_FontsProviderQtTests, ascent)
{
    Env env;
    Font f(u"Edwin", Font::Type::Text);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        double qVal = env.qProvider.ascent(f);
        double xVal = env.xProvider.ascent(f);

        EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.5));
    }
}

TEST_F(Draw_FontsProviderQtTests, descent)
{
    Env env;
    Font f(u"Edwin", Font::Type::Text);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        double qVal = env.qProvider.descent(f);
        double xVal = env.xProvider.descent(f);

        EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.8));
    }
}

TEST_F(Draw_FontsProviderQtTests, horizontalAdvance_Char)
{
    Env env;
    Font f(u"Edwin", Font::Type::Text);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        for (const muse::Char& ch : TEST_CHARS) {
            double qVal = env.qProvider.horizontalAdvance(f, ch);
            double xVal = env.xProvider.horizontalAdvance(f, ch);

            EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.0));
        }
    }
}

TEST_F(Draw_FontsProviderQtTests, horizontalAdvance_String)
{
    Env env;
    Font f(u"Edwin", Font::Type::Text);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        for (const muse::String& str : TEST_STRINGS) {
            double qVal = env.qProvider.horizontalAdvance(f, str);
            double xVal = env.xProvider.horizontalAdvance(f, str);

            EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.0));
        }
    }
}

TEST_F(Draw_FontsProviderQtTests, boundingRect_Char)
{
    Env env;
    Font f(u"Edwin", Font::Type::Text);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        for (const muse::Char& ch : TEST_CHARS) {
            char32_t ucs4 = static_cast<char32_t>(ch.unicode());
            muse::RectF qVal = env.qProvider.boundingRect(f, ucs4);
            muse::RectF xVal = env.xProvider.boundingRect(f, ucs4);

            EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.0));
        }
    }
}

TEST_F(Draw_FontsProviderQtTests, boundingRect_String)
{
    Env env;
    Font f(u"Edwin", Font::Type::Text);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        for (const muse::String& str : TEST_STRINGS) {
            muse::RectF qVal = env.qProvider.boundingRect(f, str);
            muse::RectF xVal = env.xProvider.boundingRect(f, str);

            EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.0));
        }
    }
}

TEST_F(Draw_FontsProviderQtTests, tightBoundingRect_String)
{
    Env env;
    Font f(u"Edwin", Font::Type::Text);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        for (const muse::String& str : TEST_STRINGS) {
            muse::RectF qVal = env.qProvider.tightBoundingRect(f, str);
            muse::RectF xVal = env.xProvider.tightBoundingRect(f, str);

            EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.0));
        }
    }
}

TEST_F(Draw_FontsProviderQtTests, tightBoundingRect_String_FreeSerif)
{
    Env env;
    Font f(u"FreeSerif", Font::Type::Text);
    muse::String str(u"Corno in G.");

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        muse::RectF qVal = env.qProvider.tightBoundingRect(f, str);
        muse::RectF xVal = env.xProvider.tightBoundingRect(f, str);

        EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 3));
    }
}

TEST_F(Draw_FontsProviderQtTests, boundingRect_Symbol)
{
    Env env;
    Font f(u"Leland", Font::Type::MusicSymbol);
    std::vector<double> points = { 12.0 };

    for (double pointSize : points) {
        f.setPointSizeF(pointSize);

        for (const char16_t ch : TEST_SYMBOLS) {
            muse::RectF qVal = env.qProvider.boundingRect(f, ch);
            muse::RectF xVal = env.xProvider.boundingRect(f, ch);

            EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 1.1));
        }
    }
}

TEST_F(Draw_FontsProviderQtTests, boundingRect_Symbol_Bravura)
{
    Env env;
    Font f(u"Bravura", Font::Type::MusicSymbol);
    std::vector<double> points = { 12.0 };

    for (double pointSize : points) {
        f.setPointSizeF(pointSize);

        for (const char16_t ch : TEST_SYMBOLS) {
            muse::RectF qVal = env.qProvider.boundingRect(f, ch);
            muse::RectF xVal = env.xProvider.boundingRect(f, ch);

            EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.7));
        }
    }
}

TEST_F(Draw_FontsProviderQtTests, tightBoundingRect_Symbol)
{
    Env env;
    Font f(u"Leland", Font::Type::MusicSymbol);
    std::vector<double> points = { 12.0 };

    for (double pointSize : points) {
        f.setPointSizeF(pointSize);

        for (const char16_t ch : TEST_SYMBOLS) {
            const char16_t text[] = { ch, 0 };
            muse::RectF qVal = env.qProvider.tightBoundingRect(f, muse::String(text));
            muse::RectF xVal = env.xProvider.tightBoundingRect(f, muse::String(text));

            EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.0));
        }
    }
}

TEST_F(Draw_FontsProviderQtTests, tightBoundingRect_Symbol_Bravura)
{
    Env env;
    Font f(u"Bravura", Font::Type::MusicSymbol);
    std::vector<double> points = { 12.0 };

    for (double pointSize : points) {
        f.setPointSizeF(pointSize);

        for (const char16_t ch : TEST_SYMBOLS) {
            const char16_t text[] = { ch, 0 };
            muse::RectF qVal = env.qProvider.tightBoundingRect(f, muse::String(text));
            muse::RectF xVal = env.xProvider.tightBoundingRect(f, muse::String(text));

            EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.0));
        }
    }
}

TEST_F(Draw_FontsProviderQtTests, horizontalAdvance_Symbol)
{
    Env env;
    Font f(u"Leland", Font::Type::MusicSymbol);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        for (const char16_t ch : TEST_SYMBOLS) {
            double qVal = env.qProvider.horizontalAdvance(f, ch);
            double xVal = env.xProvider.horizontalAdvance(f, ch);

            EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.3));
        }
    }
}

TEST_F(Draw_FontsProviderQtTests, horizontalAdvance_Symbol_Bravura)
{
    Env env;
    Font f(u"Bravura", Font::Type::MusicSymbol);

    for (double pointSize : TEST_POINTSIZES) {
        f.setPointSizeF(pointSize);

        for (const char16_t ch : TEST_SYMBOLS) {
            double qVal = env.qProvider.horizontalAdvance(f, ch);
            double xVal = env.xProvider.horizontalAdvance(f, ch);

            EXPECT_TRUE(ValuesMatch(f.pointSizeF(), qVal, xVal, 0.2));
        }
    }
}
