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

#ifndef MU_ENGRAVING_IMAGE_CACHE_H
#define MU_ENGRAVING_IMAGE_CACHE_H

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

public:
    ImageStoreItem(const io::path_t& p);
    void dereference(Image*);
    void reference(Image*);

    const io::path_t& path() const { return m_path; }
    mu::ByteArray& buffer() { return m_buffer; }
    const mu::ByteArray& buffer() const { return m_buffer; }
    bool loaded() const { return !m_buffer.empty(); }
    void setPath(const io::path_t& val);
    bool isUsed(Score*) const;
    bool isUsed() const { return !m_references.empty(); }
    void load();
    String hashName() const;
    const mu::ByteArray& hash() const { return m_hash; }
    void set(const mu::ByteArray& b, const mu::ByteArray& h) { m_buffer = b; m_hash = h; }

private:

    std::list<Image*> m_references;
    io::path_t m_path;                  // original location of image
    String m_type;                      // image type (file extension)
    mu::ByteArray m_buffer;
    mu::ByteArray m_hash;               // 16 byte md4 hash of _buffer
};

//---------------------------------------------------------
//   ImageStore
//---------------------------------------------------------

class ImageStore
{
    INJECT(ICryptographicHash, cryptographicHash)

public:
    ImageStore() = default;
    ImageStore(const ImageStore&) = delete;
    ImageStore& operator=(const ImageStore&) = delete;
    ~ImageStore();

    ImageStoreItem* getImage(const io::path_t& path) const;
    ImageStoreItem* add(const io::path_t& path, const mu::ByteArray&);
    void clearUnused();

    typedef std::vector<ImageStoreItem*> ItemList;
    typedef ItemList::iterator iterator;
    typedef ItemList::const_iterator const_iterator;

    iterator begin() { return m_items.begin(); }
    const_iterator begin() const { return m_items.begin(); }
    iterator end() { return m_items.end(); }
    const_iterator end() const { return m_items.end(); }

private:

    ItemList m_items;
};

extern ImageStore imageStore;       // this is the global imageStore
} // namespace mu::engraving
#endif
