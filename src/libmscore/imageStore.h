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

#include <QList>
#include <QString>
#include <QByteArray>

namespace Ms {
class Image;
class Score;

//---------------------------------------------------------
//   ImageStoreItem
//---------------------------------------------------------

class ImageStoreItem
{
    QList<Image*> _references;
    QString _path;                  // original location of image
    QString _type;                  // image type (file extension)
    QByteArray _buffer;
    QByteArray _hash;               // 16 byte md4 hash of _buffer

public:
    ImageStoreItem(const QString& p);
    void dereference(Image*);
    void reference(Image*);

    const QString& path() const { return _path; }
    QByteArray& buffer() { return _buffer; }
    const QByteArray& buffer() const { return _buffer; }
    bool loaded() const { return !_buffer.isEmpty(); }
    void setPath(const QString& val);
    bool isUsed(Score*) const;
    bool isUsed() const { return !_references.empty(); }
    void load();
    QString hashName() const;
    const QByteArray& hash() const { return _hash; }
    void set(const QByteArray& b, const QByteArray& h) { _buffer = b; _hash = h; }
};

//---------------------------------------------------------
//   ImageStore
//---------------------------------------------------------

class ImageStore
{
    typedef std::vector<ImageStoreItem*> ItemList;
    ItemList _items;

public:
    ImageStore() = default;
    ImageStore(const ImageStore&) = delete;
    ImageStore& operator=(const ImageStore&) = delete;
    ~ImageStore();

    ImageStoreItem* getImage(const QString& path) const;
    ImageStoreItem* add(const QString& path, const QByteArray&);
    void clearUnused();

    typedef ItemList::iterator iterator;
    typedef ItemList::const_iterator const_iterator;

    iterator begin() { return _items.begin(); }
    const_iterator begin() const { return _items.begin(); }
    iterator end() { return _items.end(); }
    const_iterator end() const { return _items.end(); }
};

extern ImageStore imageStore;       // this is the global imageStore
}     // namespace Ms
#endif
