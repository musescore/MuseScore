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
#pragma once

#include "engraving/infrastructure/imimedata.h"

class QMimeData;

namespace mu::engraving {
class QMimeDataAdapter : public IMimeData
{
public:
    QMimeDataAdapter(const QMimeData* d);

    std::vector<std::string> formats() const override;

    bool hasFormat(const std::string& mimeType) const override;
    muse::ByteArray data(const std::string& mimeType) const override;

    bool hasImage() const override;
    std::shared_ptr<muse::draw::Pixmap> imageData() const override;

private:
    const QMimeData* m_data = nullptr;
};
}
