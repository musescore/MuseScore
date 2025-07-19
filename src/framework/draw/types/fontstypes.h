/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MUSE_DRAW_FONTSTYPES_H
#define MUSE_DRAW_FONTSTYPES_H

#include <string>

#include "global/types/bytearray.h"
#include "global/stringutils.h"
#include "font.h"
#include "geometry.h"

namespace muse::draw {
using glyph_idx_t = uint32_t;

static constexpr double DPI_F = 5.0;
static constexpr double DPI = 72.0 * DPI_F;

struct FontDataKey {
    FontDataKey() = default;
    FontDataKey(const Font::FontFamily& fa)
        : m_family(fa), m_bold(false), m_italic(false) {}

    FontDataKey(const Font::FontFamily& fa, bool bo, bool it)
        : m_family(fa), m_bold(bo), m_italic(it) {}

    inline bool valid() const { return m_family.valid(); }

    const Font::FontFamily& family() const { return m_family; }
    bool bold() const { return m_bold; }
    bool italic() const { return m_italic; }

    inline bool operator==(const FontDataKey& o) const
    {
        return m_bold == o.m_bold && m_italic == o.m_italic && m_family == o.m_family;
    }

    inline bool operator!=(const FontDataKey& o) const { return !this->operator==(o); }

    inline bool operator<(const FontDataKey& o) const
    {
        if (m_bold != o.m_bold) {
            return m_bold < o.m_bold;
        }

        if (m_italic != o.m_italic) {
            return m_italic < o.m_italic;
        }

        return m_family.id() < o.m_family.id();
    }

private:
    Font::FontFamily m_family;
    bool m_bold = false;
    bool m_italic = false;
};

inline FontDataKey dataKeyForFont(const Font& f)
{
    return FontDataKey(f.family(), f.bold(), f.italic());
}

struct FontData {
    FontDataKey key;
    muse::ByteArray data;

    inline bool valid() const { return key.valid() && !data.empty(); }
};

struct FaceKey {
    FontDataKey dataKey;
    Font::Type type = Font::Type::Undefined;
    int pixelSize = 0;

    FaceKey() = default;
    FaceKey(const FontDataKey& dk, Font::Type t, int ps)
        : dataKey(dk), type(t), pixelSize(ps) {}

    inline bool operator==(const FaceKey& o) const
    {
        return type == o.type && dataKey == o.dataKey && pixelSize == o.pixelSize;
    }

    inline bool operator!=(const FaceKey& o) const { return !this->operator==(o); }
    inline bool operator<(const FaceKey& o) const
    {
        if (type != o.type) {
            return type < o.type;
        } else if (dataKey != o.dataKey) {
            return dataKey < o.dataKey;
        } else {
            return pixelSize < o.pixelSize;
        }
    }
};

inline int pixelSizeForFont(const Font& f)
{
    if (f.pixelSize() > 0) {
        return f.pixelSize();
    } else {
        return f.pointSizeF() * DPI / 72.0;
    }
}

inline FaceKey faceKeyForFont(const Font& f)
{
    return FaceKey(dataKeyForFont(f), f.type(), pixelSizeForFont(f));
}

struct Sdf {
    muse::ByteArray bitmap;
    uint32_t width = 0;
    uint32_t height = 0;
    float threshold = 0.;
    size_t hash = 0;
};

struct GlyphImage {
    muse::RectF rect;
    Sdf sdf;

    bool isNull() const { return rect.isNull(); }
};

struct FontParams {
    std::string name;
    Font::Type type = Font::Type::Undefined;
    bool bold = false;
    bool italic = false;
    float pointSize = 0.0f;
};
}

#endif // MUSE_DRAW_FONTSTYPES_H
