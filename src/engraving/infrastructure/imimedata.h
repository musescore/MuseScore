/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_ENGRAVING_IMIMEDATA_H
#define MU_ENGRAVING_IMIMEDATA_H

#include <string>
#include <vector>
#include <memory>

#include "types/bytearray.h"
#include "draw/types/pixmap.h"

namespace mu::engraving {
class IMimeData
{
public:
    virtual ~IMimeData() = default;

    virtual std::vector<std::string> formats() const = 0;

    virtual bool hasFormat(const std::string& mimeType) const = 0;
    virtual muse::ByteArray data(const std::string& mimeType) const = 0;

    virtual bool hasImage() const = 0;
    virtual std::shared_ptr<muse::draw::Pixmap> imageData() const = 0;
};
}

#endif // MU_ENGRAVING_IMIMEDATA_H
