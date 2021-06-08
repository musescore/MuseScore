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

#include <QFile>
#include <QHash>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BBOX_H

#include "log.h"

static FT_Library ftlib = nullptr;

using namespace mu::draw;

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

struct mu::draw::FTGlyphMetrics
{
    FT_BBox bb;
    double linearHoriAdvance = 0.0;
};

struct mu::draw::FTData
{
    QByteArray fontData;
    FT_Face face = nullptr;
    QHash<uint, FTGlyphMetrics> metrics;
};

FontEngineFT::FontEngineFT()
{
    m_data = new FTData();
}

FontEngineFT::~FontEngineFT()
{
    delete m_data;
}

bool FontEngineFT::load(const QString& path)
{
    if (!_init_ft()) {
        return false;
    }

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        LOGE() << "failed open font: " << path;
        return false;
    }

    m_data->fontData = f.readAll();

    int rval = FT_New_Memory_Face(ftlib, (FT_Byte*)m_data->fontData.data(), m_data->fontData.size(), 0, &m_data->face);
    if (rval) {
        LOGE() << "freetype: cannot create face: " << path << ", rval: " << rval;
        return false;
    }

    //! NOTE Moved form sym.cpp ScoreFont::load as is
    qreal pixelSize = 200.0;
    FT_Set_Pixel_Sizes(m_data->face, 0, int(pixelSize + .5));

    return true;
}

QRectF FontEngineFT::bbox(uint ucs4, qreal DPI_F) const
{
    FTGlyphMetrics* gm = glyphMetrics(ucs4);
    if (!gm) {
        return QRectF();
    }

    const FT_BBox& bb = gm->bb;
    //! NOTE Moved form sym.cpp ScoreFont::computeMetrics as is
    double m = 640.0 / DPI_F;
    QRectF bbox;
    bbox.setCoords(bb.xMin / m, -bb.yMax / m, bb.xMax / m, -bb.yMin / m);
    return bbox;
}

qreal FontEngineFT::advance(uint ucs4, qreal DPI_F) const
{
    FTGlyphMetrics* gm = glyphMetrics(ucs4);
    if (!gm) {
        return 0.0;
    }

    //! NOTE Moved form sym.cpp ScoreFont::computeMetrics as is
    return gm->linearHoriAdvance * DPI_F / 655360.0;
}

FTGlyphMetrics* FontEngineFT::glyphMetrics(uint ucs4) const
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
