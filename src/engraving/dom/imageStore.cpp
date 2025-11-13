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

#include "io/path.h"

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

ImageStoreItem::ImageStoreItem(const std::string& type)
    : m_type(type)
{
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
//   hashName
//---------------------------------------------------------

std::string ImageStoreItem::hashName() const
{
    const char hex[17] = "0123456789abcdef";
    char p[33];
    for (int i = 0; i < 16; ++i) {
        p[i * 2]     = hex[(m_hash[i] >> 4) & 0xf];
        p[i * 2 + 1] = hex[m_hash[i] & 0xf];
    }
    p[32] = 0;
    return std::string(p) + "." + m_type;
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

ImageStoreItem* ImageStore::getImage(std::string name) const
{
    name = muse::io::completeBasename(name).toStdString();
    if (name.size() != 32) {
        return nullptr;
    }
    ByteArray hash(16);
    for (int i = 0; i < 16; ++i) {
        hash[i] = toInt(name.at(i * 2)) * 16 + toInt(name.at(i * 2 + 1));
    }
    for (ImageStoreItem* item : m_items) {
        if (item->hash() == hash) {
            return item;
        }
    }
    LOGW() << "image not found: " << name;
    return nullptr;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

ImageStoreItem* ImageStore::add(const std::string& name, const ByteArray& ba)
{
    ByteArray hash = cryptographicHash()->hash(ba, ICryptographicHash::Algorithm::Md4);
    for (ImageStoreItem* item : m_items) {
        if (item->hash() == hash) {
            return item;
        }
    }
    ImageStoreItem* item = new ImageStoreItem(muse::io::suffix(name));
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
