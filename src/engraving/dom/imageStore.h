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
    muse::GlobalInject<muse::ICryptographicHash> cryptographicHash;

public:
    ImageStoreItem(const muse::io::path_t& p);
    void dereference(Image*);
    void reference(Image*);

    const muse::io::path_t& path() const { return m_path; }
    muse::ByteArray& buffer() { return m_buffer; }
    const muse::ByteArray& buffer() const { return m_buffer; }
    bool loaded() const { return !m_buffer.empty(); }
    void setPath(const muse::io::path_t& val);
    bool isUsed(Score*) const;
    bool isUsed() const { return !m_references.empty(); }
    void load();
    muse::String hashName() const;
    const muse::ByteArray& hash() const { return m_hash; }
    void set(const muse::ByteArray& b, const muse::ByteArray& h) { m_buffer = b; m_hash = h; }

private:

    std::list<Image*> m_references;
    muse::io::path_t m_path;                  // original location of image
    muse::String m_type;                      // image type (file extension)
    muse::ByteArray m_buffer;
    muse::ByteArray m_hash;               // 16 byte md4 hash of _buffer
};

//---------------------------------------------------------
//   ImageStore
//---------------------------------------------------------

class ImageStore
{
    muse::GlobalInject<muse::ICryptographicHash> cryptographicHash;

public:
    ImageStore() = default;
    ImageStore(const ImageStore&) = delete;
    ImageStore& operator=(const ImageStore&) = delete;
    ~ImageStore();

    ImageStoreItem* getImage(const muse::io::path_t& path) const;
    ImageStoreItem* add(const muse::io::path_t& path, const muse::ByteArray&);
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
