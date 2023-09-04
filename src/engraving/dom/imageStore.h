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

#ifndef __IMAGE_CACHE_H__
#define __IMAGE_CACHE_H__

#include <list>

#include "types/string.h"
#include "types/bytearray.h"
#include "io/path.h"

#include "modularity/ioc.h"
#include "global/icryptographichash.h"

namespace mu::engraving {
class Image;
class Score;

//---------------------------------------------------------
//   ImageStoreItem
//---------------------------------------------------------

class ImageStoreItem
{
    INJECT(ICryptographicHash, cryptographicHash)

    std::list<Image*> _references;
    io::path_t _path;                  // original location of image
    String _type;                  // image type (file extension)
    mu::ByteArray _buffer;
    mu::ByteArray _hash;               // 16 byte md4 hash of _buffer

public:
    ImageStoreItem(const io::path_t& p);
    void dereference(Image*);
    void reference(Image*);

    const io::path_t& path() const { return _path; }
    mu::ByteArray& buffer() { return _buffer; }
    const mu::ByteArray& buffer() const { return _buffer; }
    bool loaded() const { return !_buffer.empty(); }
    void setPath(const io::path_t& val);
    bool isUsed(Score*) const;
    bool isUsed() const { return !_references.empty(); }
    void load();
    String hashName() const;
    const mu::ByteArray& hash() const { return _hash; }
    void set(const mu::ByteArray& b, const mu::ByteArray& h) { _buffer = b; _hash = h; }
};

//---------------------------------------------------------
//   ImageStore
//---------------------------------------------------------

class ImageStore
{
    INJECT(ICryptographicHash, cryptographicHash)

    typedef std::vector<ImageStoreItem*> ItemList;
    ItemList _items;

public:
    ImageStore() = default;
    ImageStore(const ImageStore&) = delete;
    ImageStore& operator=(const ImageStore&) = delete;
    ~ImageStore();

    ImageStoreItem* getImage(const io::path_t& path) const;
    ImageStoreItem* add(const io::path_t& path, const mu::ByteArray&);
    void clearUnused();

    typedef ItemList::iterator iterator;
    typedef ItemList::const_iterator const_iterator;

    iterator begin() { return _items.begin(); }
    const_iterator begin() const { return _items.begin(); }
    iterator end() { return _items.end(); }
    const_iterator end() const { return _items.end(); }
};

extern ImageStore imageStore;       // this is the global imageStore
} // namespace mu::engraving
#endif
