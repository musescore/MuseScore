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
#include "fontengineft.h"

#include <QHash>

#include "global/io/file.h"
#include "global/types/bytearray.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BBOX_H

#include "log.h"

static FT_Library ftlib = nullptr;

using namespace muse::io;
using namespace muse::draw;

static bool _init_ft()
{
    int error = 0;
    if (!ftlib) {
        error = FT_Init_FreeType(&ftlib);
        if (!ftlib || error) {
            LOGE() << "init freetype library failed";
        }
    }
    return error == 0;
}

struct muse::draw::FTGlyphMetrics
{
    FT_BBox bb;
    double linearHoriAdvance = 0.0;
};

struct muse::draw::FTData
{
    ByteArray fontData;
    FT_Face face = nullptr;
    QHash<char32_t, FTGlyphMetrics> metrics;
};

FontEngineFT::FontEngineFT()
{
    m_data = new FTData();
}

FontEngineFT::~FontEngineFT()
{
    delete m_data;
}

bool FontEngineFT::load(const io::path_t& path)
{
    if (!_init_ft()) {
        return false;
    }

    File f(path);
    if (!f.open(IODevice::ReadOnly)) {
        LOGE() << "failed open font: " << path;
        return false;
    }

    m_data->fontData = f.readAll();

    int rval = FT_New_Memory_Face(ftlib, (FT_Byte*)m_data->fontData.constData(), (FT_Long)m_data->fontData.size(), 0, &m_data->face);
    if (rval) {
        LOGE() << "freetype: cannot create face: " << path << ", rval: " << rval;
        return false;
    }

    //! NOTE Moved form sym.cpp ScoreFont::load as is
    double pixelSize = 200.0;
    FT_Set_Pixel_Sizes(m_data->face, 0, int(pixelSize + .5));

    return true;
}

QRectF FontEngineFT::bbox(char32_t ucs4, double dpi_f) const
{
    FTGlyphMetrics* gm = glyphMetrics(ucs4);
    if (!gm) {
        return QRectF();
    }

    const FT_BBox& bb = gm->bb;
    //! NOTE Moved form sym.cpp ScoreFont::computeMetrics as is
    double m = 640.0 / dpi_f;
    QRectF bbox;
    bbox.setCoords(bb.xMin / m, -bb.yMax / m, bb.xMax / m, -bb.yMin / m);
    return bbox;
}

double FontEngineFT::advance(char32_t ucs4, double dpi_f) const
{
    FTGlyphMetrics* gm = glyphMetrics(ucs4);
    if (!gm) {
        return 0.0;
    }

    //! NOTE Moved form sym.cpp ScoreFont::computeMetrics as is
    return gm->linearHoriAdvance * dpi_f / 655360.0;
}

FTGlyphMetrics* FontEngineFT::glyphMetrics(char32_t ucs4) const
{
    if (m_data->metrics.contains(ucs4)) {
        return &m_data->metrics[ucs4];
    }

    FT_UInt index = FT_Get_Char_Index(m_data->face, ucs4);
    if (index == 0) {
        return nullptr;
    }

    if (FT_Load_Glyph(m_data->face, index, FT_LOAD_DEFAULT) != 0) {
        return nullptr;
    }

    FT_BBox bb;
    if (FT_Outline_Get_BBox(&m_data->face->glyph->outline, &bb) != 0) {
        return nullptr;
    }

    FTGlyphMetrics& gm = m_data->metrics[ucs4];
    gm.bb = bb;
    gm.linearHoriAdvance = m_data->face->glyph->linearHoriAdvance;

    return &gm;
}
