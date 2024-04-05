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
#include "abpaintprovider.h"

#include "log.h"
#include "draw/painter.h"

static const QColor REMOVED_COLOR("#cc0000");
static const QColor ADDED_COLOR("#009900");

static const std::string NOTATION_DEFAULT_OBJ("notationview_default");

using namespace muse::autobot;

const std::shared_ptr<AbPaintProvider>& AbPaintProvider::instance()
{
    static std::shared_ptr<AbPaintProvider> p = std::shared_ptr<AbPaintProvider>(new AbPaintProvider());
    return p;
}

void AbPaintProvider::beginTarget(const std::string& name)
{
    BufferedPaintProvider::beginTarget(name);
}

void AbPaintProvider::beforeEndTargetHook(muse::draw::Painter* painter)
{
    IF_ASSERT_FAILED(painter) {
        return;
    }

    if (!m_isDiffDrawEnabled) {
        return;
    }

    muse::draw::IPaintProviderPtr provider = painter->provider();

    if (m_diff.dataRemoved && !m_diff.dataRemoved->empty()) {
        paintData(provider, m_diff.dataRemoved, REMOVED_COLOR);
    }

    if (m_diff.dataAdded && !m_diff.dataAdded->empty()) {
        paintData(provider, m_diff.dataAdded, ADDED_COLOR);
    }
}

void AbPaintProvider::paintData(muse::draw::IPaintProviderPtr provider, const muse::draw::DrawDataPtr& data, const QColor& overcolor)
{
    using namespace muse::draw;

    const DrawData::Item& obj = data->item;

    for (const DrawData::Data& d : obj.datas) {
        DrawData::State st = data->states.at(d.state);
        st.pen.setColor(overcolor);
        st.pen.setWidthF(10.);
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
            if (t.mode == DrawText::Point) {
                provider->drawText(t.rect.topLeft(), t.text);
            } else {
                provider->drawText(t.rect, t.flags, t.text);
            }
        }

        for (const DrawPixmap& px : d.pixmaps) {
            if (px.mode == DrawPixmap::Single) {
                provider->drawPixmap(px.rect.topLeft(), px.pm);
            } else {
                provider->drawTiledPixmap(px.rect, px.pm, px.offset);
            }
        }
    }
}

bool AbPaintProvider::endTarget(bool endDraw)
{
    bool ok = BufferedPaintProvider::endTarget(endDraw);
    if (ok && drawData()->name == "notationview") {
        m_notationViewDrawData = drawData();
    }
    return ok;
}

const muse::draw::DrawDataPtr& AbPaintProvider::notationViewDrawData() const
{
    return m_notationViewDrawData;
}

void AbPaintProvider::setDiff(const muse::draw::Diff& diff)
{
    m_diff = diff;
}

void AbPaintProvider::setIsDiffDrawEnabled(bool arg)
{
    m_isDiffDrawEnabled = arg;
}
