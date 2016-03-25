//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __IMAGE_CACHE_H__
#define __IMAGE_CACHE_H__

namespace Ms {

class Image;
class Score;

//---------------------------------------------------------
//   ImageStoreItem
//---------------------------------------------------------

class ImageStoreItem {
      QList<Image*> _references;
      QString _path;                // original location of image
      QString _type;                // image type (file extension)
      QByteArray _buffer;
      QByteArray _hash;             // 16 byte md4 hash of _buffer

   public:
      ImageStoreItem(const QString& p);
      void dereference(Image*);
      void reference(Image*);

      const QString& path() const      { return _path;     }
      QByteArray& buffer()             { return _buffer;   }
      bool loaded() const              { return !_buffer.isEmpty();   }
      void setPath(const QString& val);
      bool isUsed(Score*) const;
      void load();
      QString hashName() const;
      const QByteArray& hash() const   { return _hash; }
      void set(const QByteArray& b, const QByteArray& h) { _buffer = b; _hash = h; }
      };

//---------------------------------------------------------
//   ImageStore
//---------------------------------------------------------

class ImageStore : public QList<ImageStoreItem*>  {

   public:
      ImageStoreItem* getImage(const QString& path) const;
      ImageStoreItem* add(const QString& path, const QByteArray&);
      };

extern ImageStore imageStore;       // this is the global imageStore

}     // namespace Ms
#endif

