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
#ifndef MUSE_DRAW_IIMAGEPROVIDER_H
#define MUSE_DRAW_IIMAGEPROVIDER_H

#include "global/modularity/ioc.h"
#include "global/io/iodevice.h"

#include "types/geometry.h"
#include "types/pixmap.h"
#include "types/color.h"
#include "ipaintprovider.h"

namespace muse::draw {
class IImageProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IImageProvider)

public:
    virtual ~IImageProvider() = default;

    virtual std::shared_ptr<Pixmap> createPixmap(const ByteArray& data) const = 0;
    virtual std::shared_ptr<Pixmap> createPixmap(int w, int h, int dpm, const Color& color) const = 0;

    virtual Pixmap scaled(const Pixmap& origin, const Size& s) const = 0;

    virtual IPaintProviderPtr painterForImage(std::shared_ptr<Pixmap> pixmap) = 0;

    virtual void saveAsPng(std::shared_ptr<Pixmap> px, io::IODevice* device) = 0;
};
}

#endif // MUSE_DRAW_IIMAGEPROVIDER_H
