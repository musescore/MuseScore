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

#include "imageStore.h"

#include "io/fileinfo.h"
#include "io/file.h"

#include "image.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace mu::io;

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
    _references.remove(image);
}

//---------------------------------------------------------
//   reference
//    increment usage count of image in score
//---------------------------------------------------------

void ImageStoreItem::reference(Image* image)
{
    _references.push_back(image);
}

//---------------------------------------------------------
//   isUsed
//    check if item is used in score
//---------------------------------------------------------

bool ImageStoreItem::isUsed(Score* score) const
{
    for (Image* image : _references) {
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
    if (!_buffer.empty()) {
        return;
    }
    File inFile(_path);
    if (!inFile.open(IODevice::ReadOnly)) {
        LOGD("Cannot open picture file");
        return;
    }
    _buffer = inFile.readAll();
    inFile.close();

    _hash = cryptographicHash()->hash(_buffer, ICryptographicHash::Algorithm::Md4);
}

//---------------------------------------------------------
//   hashName
//---------------------------------------------------------

String ImageStoreItem::hashName() const
{
    const char hex[17] = "0123456789abcdef";
    char p[33];
    for (int i = 0; i < 16; ++i) {
        p[i * 2]     = hex[(_hash[i] >> 4) & 0xf];
        p[i * 2 + 1] = hex[_hash[i] & 0xf];
    }
    p[32] = 0;
    return String::fromAscii(p) + u"." + _type;
}

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void ImageStoreItem::setPath(const path_t& val)
{
    _path = val;
    _type = FileInfo::suffix(_path);
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
    DeleteAll(_items);
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
        for (ImageStoreItem* item: _items) {
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
    for (ImageStoreItem* item : _items) {
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
    for (ImageStoreItem* item : _items) {
        if (item->hash() == hash) {
            return item;
        }
    }
    ImageStoreItem* item = new ImageStoreItem(path);
    item->set(ba, hash);
    _items.push_back(item);
    return item;
}

//---------------------------------------------------------
//   clearUnused
//---------------------------------------------------------

void ImageStore::clearUnused()
{
    _items.erase(
        std::remove_if(_items.begin(), _items.end(), [](ImageStoreItem* i) {
        const bool remove = !i->isUsed();
        if (remove) {
            delete i;
        }
        return remove;
    }),
        _items.end()
        );
}
}
