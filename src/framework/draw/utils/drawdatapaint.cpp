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

using namespace mu;
using namespace mu::draw;

void DrawDataPaint::paint(Painter* painter, const DrawDataPtr& data, const Color& overlay)
{
    IPaintProviderPtr provider = painter->provider();
    for (const DrawData::Object& obj : data->objects) {
        for (const DrawData::Data& d : obj.datas) {
            DrawData::State st = d.state;
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
    }
}
