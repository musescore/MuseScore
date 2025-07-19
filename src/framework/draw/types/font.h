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
#ifndef MUSE_DRAW_FONT_H
#define MUSE_DRAW_FONT_H

#include "global/types/string.h"
#include "global/types/flags.h"

#ifndef NO_QT_SUPPORT
#include <QFont>
#endif

namespace muse::draw {
class Font
{
public:
    struct FontFamily {
        FontFamily() = default;
        FontFamily(const char16_t* id)
            : m_id(id) {}

        FontFamily(const std::string& id)
            : m_id(String::fromStdString(id)) {}

        FontFamily(const String& id)
            : m_id(id) {}

        inline bool valid() const { return !m_id.empty(); }
        const String& id() const { return m_id; }

        //! NOTE: Case insensitive comparison
        inline bool operator==(const FontFamily& o) const
        {
            return m_id.toLower() == o.id().toLower();
        }

    private:
        String m_id;
    };

    enum class Type {
        Undefined = 0,
        Unknown,
        Icon,
        Text,
        MusicSymbolText,
        MusicSymbol,
        Tablature,
        Harmony
    };

    Font() = default;
    Font(const FontFamily& family, Type type);

    enum class Style {
        //Undefined   = -1,
        Normal      = 0,
        Bold        = 1 << 0,
        Italic      = 1 << 1,
        Underline   = 1 << 2,
        Strike      = 1 << 3
    };

    enum class Hinting {
        PreferDefaultHinting = 0,
        PreferNoHinting = 1,
        PreferVerticalHinting = 2,
        PreferFullHinting = 3
    };

    enum Weight {
        Thin     = 0,    // 100
        ExtraLight = 12, // 200
        Light    = 25,   // 300
        Normal   = 50,   // 400
        Medium   = 57,   // 500
        DemiBold = 63,   // 600
        Bold     = 75,   // 700
        ExtraBold = 81,  // 800
        Black    = 87    // 900
    };

    void setFamily(const FontFamily& family, Type type);
    FontFamily family() const;
    Type type() const;

    double pointSizeF() const;
    void setPointSizeF(double s);

    int pixelSize() const;
    void setPixelSize(int s);

    Weight weight() const;
    void setWeight(Weight w);

    bool bold() const;
    void setBold(bool arg);
    bool italic() const;
    void setItalic(bool arg);
    bool underline() const;
    void setUnderline(bool arg);
    bool strike() const;
    void setStrike(bool arg);

    void setNoFontMerging(bool arg);
    bool noFontMerging() const;

    Hinting hinting() const;
    void setHinting(Hinting hinting);

    bool operator ==(const Font& other) const;
    bool operator !=(const Font& other) const { return !this->operator ==(other); }

#ifndef NO_QT_SUPPORT
    QFont toQFont() const;
    static Font fromQFont(const QFont& qf, Type type);
#endif

    static bool g_disableFontMerging;

private:

    FontFamily m_family;
    Type m_type = Type::Undefined;
    double m_pointSizeF = -1.0;
    int m_pixelSize = -1;
    Weight m_weight = Weight::Normal;
    muse::Flags<Style> m_style{ Style::Normal };
    bool m_noFontMerging = false;
    Hinting m_hinting = Hinting::PreferDefaultHinting;
};
}

#endif // MUSE_DRAW_FONT_H
