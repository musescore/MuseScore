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

#include "imageStore.h"

#include "io/fileinfo.h"
#include "io/file.h"

#include "image.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::io;

namespace mu::engraving {
ImageStore imageStore;  // the global image store

//---------------------------------------------------------
//   ImageStoreItem
//---------------------------------------------------------

ImageStoreItem::ImageStoreItem(const path_t& p)
{
    setPath(p);
}

//---------------------------------------------------------
//   dereference
//    decrement usage count of image in score
//---------------------------------------------------------

void ImageStoreItem::dereference(Image* image)
{
    m_references.remove(image);
}

//---------------------------------------------------------
//   reference
//    increment usage count of image in score
//---------------------------------------------------------

void ImageStoreItem::reference(Image* image)
{
    m_references.push_back(image);
}

//---------------------------------------------------------
//   isUsed
//    check if item is used in score
//---------------------------------------------------------

bool ImageStoreItem::isUsed(Score* score) const
{
    for (Image* image : m_references) {
        if (image->score() == score && image->explicitParent()) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void ImageStoreItem::load()
{
    if (!m_buffer.empty()) {
        return;
    }
    File inFile(m_path);
    if (!inFile.open(IODevice::ReadOnly)) {
        LOGD("Cannot open picture file");
        return;
    }
    m_buffer = inFile.readAll();
    inFile.close();

    m_hash = cryptographicHash()->hash(m_buffer, ICryptographicHash::Algorithm::Md4);
}

//---------------------------------------------------------
//   hashName
//---------------------------------------------------------

String ImageStoreItem::hashName() const
{
    const char hex[17] = "0123456789abcdef";
    char p[33];
    for (int i = 0; i < 16; ++i) {
        p[i * 2]     = hex[(m_hash[i] >> 4) & 0xf];
        p[i * 2 + 1] = hex[m_hash[i] & 0xf];
    }
    p[32] = 0;
    return String::fromAscii(p) + u"." + m_type;
}

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void ImageStoreItem::setPath(const path_t& val)
{
    m_path = val;
    m_type = FileInfo::suffix(m_path);
}

//---------------------------------------------------------
//   toInt
//---------------------------------------------------------

inline static int toInt(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    return c - 'a' + 10;
}

//---------------------------------------------------------
//   ~ImageStore
//---------------------------------------------------------

ImageStore::~ImageStore()
{
    muse::DeleteAll(m_items);
}

//---------------------------------------------------------
//   getImage
//---------------------------------------------------------

ImageStoreItem* ImageStore::getImage(const path_t& path) const
{
    String s = FileInfo(path).completeBaseName();
    if (s.size() != 32) {
        //
        // some limited support for backward compatibility
        //
        for (ImageStoreItem* item: m_items) {
            if (item->path() == path) {
                return item;
            }
        }
        return nullptr;
    }
    ByteArray hash(16);
    for (int i = 0; i < 16; ++i) {
        hash[i] = toInt(s.at(i * 2).toAscii()) * 16 + toInt(s.at(i * 2 + 1).toAscii());
    }
    for (ImageStoreItem* item : m_items) {
        if (item->hash() == hash) {
            return item;
        }
    }
    LOGW() << "image not found: " << path;
    return nullptr;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

ImageStoreItem* ImageStore::add(const path_t& path, const ByteArray& ba)
{
    ByteArray hash = cryptographicHash()->hash(ba, ICryptographicHash::Algorithm::Md4);
    for (ImageStoreItem* item : m_items) {
        if (item->hash() == hash) {
            return item;
        }
    }
    ImageStoreItem* item = new ImageStoreItem(path);
    item->set(ba, hash);
    m_items.push_back(item);
    return item;
}

//---------------------------------------------------------
//   clearUnused
//---------------------------------------------------------

void ImageStore::clearUnused()
{
    m_items.erase(
        std::remove_if(m_items.begin(), m_items.end(), [](ImageStoreItem* i) {
        const bool remove = !i->isUsed();
        if (remove) {
            delete i;
        }
        return remove;
    }),
        m_items.end()
        );
}
}
