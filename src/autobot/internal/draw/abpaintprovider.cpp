//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "abpaintprovider.h"

#include "log.h"
#include "libmscore/draw/painter.h"

static const QColor REMOVED_COLOR("#cc0000");
static const QColor ADDED_COLOR("#009900");

static const std::string NOTATION_DEFAULT_OBJ("notationview_default");

using namespace mu::autobot;

const std::shared_ptr<AbPaintProvider>& AbPaintProvider::instance()
{
    static std::shared_ptr<AbPaintProvider> p = std::shared_ptr<AbPaintProvider>(new AbPaintProvider());
    return p;
}

void AbPaintProvider::beginTarget(const std::string& name)
{
    BufferedPaintProvider::beginTarget(name);
}

void AbPaintProvider::beforeEndTargetHook(draw::Painter* painter)
{
    IF_ASSERT_FAILED(painter) {
        return;
    }

    if (!m_isDiffDrawEnabled) {
        return;
    }

    draw::IPaintProviderPtr provider = painter->provider();

    if (m_diff.dataRemoved && !m_diff.dataRemoved->objects.empty()) {
        paintData(provider, m_diff.dataRemoved, REMOVED_COLOR);
    }

    if (m_diff.dataAdded && !m_diff.dataAdded->objects.empty()) {
        paintData(provider, m_diff.dataAdded, ADDED_COLOR);
    }
}

void AbPaintProvider::paintData(draw::IPaintProviderPtr provider, const draw::DrawDataPtr& data, const QColor& overcolor)
{
    using namespace mu::draw;

    for (const DrawData::Object& obj : data->objects) {
        if (obj.name == NOTATION_DEFAULT_OBJ) {
            continue;
        }

        for (const DrawData::Data& d : obj.datas) {
            DrawData::State st = d.state;
            st.pen.setColor(overcolor);
            st.pen.setWidth(10);
            st.brush.setColor(overcolor);

            provider->setPen(st.pen);
            provider->setBrush(st.brush);
            provider->setFont(st.font);
            provider->setTransform(st.transform);
            provider->setAntialiasing(st.isAntialiasing);
            provider->setCompositionMode(st.compositionMode);

            for (const DrawPath& path : d.paths) {
                provider->setPen(path.pen);
                provider->setBrush(path.brush);
                provider->drawPath(path.path);
            }

            for (const DrawPolygon& pl : d.polygons) {
                if (pl.polygon.empty()) {
                    continue;
                }
                provider->drawPolygon(&pl.polygon[0], pl.polygon.size(), pl.mode);
            }

            for (const DrawText& t : d.texts) {
                provider->drawText(t.pos, t.text);
            }

            for (const DrawRectText& t : d.rectTexts) {
                provider->drawText(t.rect, t.flags, t.text);
            }

            for (const DrawGlyphRun& g : d.glyphs) {
                provider->drawGlyphRun(g.pos, g.glyphRun);
            }

            for (const DrawPixmap& px : d.pixmaps) {
                provider->drawPixmap(px.pos, px.pm);
            }

            for (const DrawTiledPixmap& px : d.tiledPixmap) {
                provider->drawTiledPixmap(px.rect, px.pm, px.offset);
            }
        }
    }
}

bool AbPaintProvider::endTarget(bool endDraw)
{
    bool ok = BufferedPaintProvider::endTarget(endDraw);
    if (ok && drawData().name == "notationview") {
        m_notationViewDrawData = drawData();
    }
    return ok;
}

const mu::draw::DrawData& AbPaintProvider::notationViewDrawData() const
{
    return m_notationViewDrawData;
}

void AbPaintProvider::setDiff(const draw::Diff& diff)
{
    m_diff = diff;
}

void AbPaintProvider::setIsDiffDrawEnabled(bool arg)
{
    m_isDiffDrawEnabled = arg;
}
