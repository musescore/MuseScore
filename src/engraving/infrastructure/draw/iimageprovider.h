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
#ifndef MU_DRAW_IIMAGEPROVIDER_H
#define MU_DRAW_IIMAGEPROVIDER_H

#include <QByteArray>
#include <QBuffer>

#include "modularity/ioc.h"

#include "geometry.h"
#include "pixmap.h"
#include "color.h"
#include "painter.h"
#include "infrastructure/draw/ipaintprovider.h"

namespace mu::draw {
class IImageProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IImageProvider)

public:
    virtual ~IImageProvider() = default;

    virtual std::shared_ptr<Pixmap> createPixmap(const QByteArray& data) const = 0;
    virtual std::shared_ptr<Pixmap> createPixmap(int w, int h, int dpm, const Color& color) const = 0;

    virtual Pixmap scaled(const Pixmap& origin, const Size& s) const = 0;

    virtual IPaintProviderPtr painterForImage(std::shared_ptr<Pixmap> pixmap) = 0;

    virtual void saveAsPng(std::shared_ptr<Pixmap> px, QIODevice* device) = 0;
    virtual std::shared_ptr<Pixmap> pixmapFromQVariant(const QVariant& val) = 0;
};
}

#endif // MU_DRAW_IIMAGEPROVIDER_H
