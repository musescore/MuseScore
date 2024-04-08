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
#include "drawdatapaint.h"

#include "log.h"

using namespace muse;
using namespace muse::draw;

static void drawItem(IPaintProviderPtr& provider, const DrawData::Item& item, const std::map<int, DrawData::State>& states,
                     const Color& overlay)
{
    // first draw obj itself
    for (const DrawData::Data& d : item.datas) {
        DrawData::State st = states.at(d.state);
        if (overlay.isValid()) {
            st.pen.setColor(overlay);
            st.brush.setColor(overlay);
        }

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

    // second draw chilren
    for (const DrawData::Item& ch : item.chilren) {
        drawItem(provider, ch, states, overlay);
    }
}

void DrawDataPaint::paint(Painter* painter, const DrawDataPtr& data, const Color& overlay)
{
    IPaintProviderPtr provider = painter->provider();
    drawItem(provider, data->item, data->states, overlay);
}
