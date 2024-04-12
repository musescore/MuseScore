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
#include "qmimedataadapter.h"

#include <QMimeData>

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

QMimeDataAdapter::QMimeDataAdapter(const QMimeData* d)
    : m_data(d)
{
}

std::vector<std::string> QMimeDataAdapter::formats() const
{
    std::vector<std::string> ret;
    QStringList fs = m_data->formats();
    for (const QString& f : fs) {
        ret.push_back(f.toStdString());
    }
    return ret;
}

bool QMimeDataAdapter::hasFormat(const std::string& mimeType) const
{
    return m_data->hasFormat(QString::fromStdString(mimeType));
}

muse::ByteArray QMimeDataAdapter::data(const std::string& mimeType) const
{
    return muse::ByteArray::fromQByteArray(m_data->data(QString::fromStdString(mimeType)));
}

bool QMimeDataAdapter::hasImage() const
{
    return m_data->hasImage();
}

static std::shared_ptr<Pixmap> pixmapFromQVariant(const QVariant& val)
{
    using namespace muse::draw;
    IF_ASSERT_FAILED(val.canConvert<Pixmap>() || val.canConvert<QImage>()) {
    }

    if (val.canConvert<Pixmap>()) {
        return std::make_shared<Pixmap>(val.value<Pixmap>());
    } else if (val.canConvert<QImage>()) {
        return std::make_shared<Pixmap>(Pixmap::fromQPixmap(QPixmap::fromImage(val.value<QImage>())));
    }
    return nullptr;
}

std::shared_ptr<Pixmap> QMimeDataAdapter::imageData() const
{
    return pixmapFromQVariant(m_data->imageData());
}
